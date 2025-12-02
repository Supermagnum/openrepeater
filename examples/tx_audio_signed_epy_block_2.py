import numpy as np
from gnuradio import gr
import os

try:
    from pkcs11 import lib as pkcs11_lib
    from pkcs11 import Token
    from pkcs11.constants import UserType, ObjectClass, KeyType, Mechanism
    PKCS11_AVAILABLE = True
except ImportError:
    PKCS11_AVAILABLE = False

try:
    from gnuradio import nacl
    NACL_AVAILABLE = True
except ImportError:
    NACL_AVAILABLE = False

class blk(gr.sync_block):
    def __init__(self):
        gr.sync_block.__init__(
            self,
            name='Burst Trigger Audio + Signed Frames',
            in_sig=[np.float32, np.uint8],
            out_sig=[np.float32, np.uint8]
        )
        self._key_buffer = bytearray()
        self._pkcs11_lib = None
        self._use_pkcs11 = True
        self._audio_eof_detected = False
        self._data_frames_bytes = None
        self._data_output_idx = 0
        self._silence_count = 0
        self._silence_threshold = 48000  # 1 second of silence
        self._total_audio_samples = 0
        self._min_audio_samples = 96000  # Require at least 2 seconds of audio
        self._frames_complete = False  # Track when all frames have been output
        self._eof_flush_samples = 0  # Count samples output after EOF to flush buffers
        self._eof_flush_needed = 1000  # Flush a small amount to ensure audio is written
        
        if PKCS11_AVAILABLE:
            self._init_pkcs11()
    
    def _init_pkcs11(self):
        lib_paths = [
            '/usr/lib/x86_64-linux-gnu/opensc-pkcs11.so',
            '/usr/lib/opensc-pkcs11.so',
            '/usr/lib/x86_64-linux-gnu/p11-kit-proxy.so',
            '/usr/lib/p11-kit-proxy.so',
        ]
        for path in lib_paths:
            if os.path.exists(path):
                try:
                    self._pkcs11_lib = pkcs11_lib(path)
                    print(f"PKCS#11: Loaded {path}")
                    return True
                except Exception as e:
                    continue
        return False
    
    def _get_token(self, pin=None):
        if not self._pkcs11_lib:
            return None
        try:
            tokens = self._pkcs11_lib.get_tokens()
            if not tokens:
                return None
            token = tokens[0]
            if pin:
                token.open(UserType.USER, pin.encode())
            else:
                try:
                    token.open(UserType.USER)
                except:
                    return None
            return token
        except:
            return None
    
    def _sign_with_pkcs11(self, data, pin=None):
        if not PKCS11_AVAILABLE or not self._pkcs11_lib:
            return None
        try:
            token = self._get_token(pin)
            if not token:
                return None
            session = token.open(UserType.USER, pin.encode() if pin else None)
            private_keys = session.get_objects({ObjectClass.PRIVATE_KEY, KeyType.EC})
            if not private_keys:
                return None
            signature = private_keys[0].sign(data, mechanism=Mechanism.ECDSA)
            return signature
        except Exception as e:
            print(f"PKCS#11 signing error: {e}")
            return None
    
    def _generate_frames_bytes(self):
        try:
            import __main__
            if hasattr(__main__, 'message_text'):
                msg = getattr(__main__, 'message_text').value()
                if msg:
                    msg_bytes = msg.encode('utf-8')
                    signature = None
                    
                    if self._use_pkcs11:
                        signature = self._sign_with_pkcs11(msg_bytes)
                    
                    if not signature and len(self._key_buffer) >= 32 and NACL_AVAILABLE:
                        key = bytes(self._key_buffer[:32])
                        signature = nacl.sign_ed25519(msg_bytes, key)
                    
                    frames = bytearray()
                    
                    if signature:
                        frames.extend(b'SIG')
                        frames.extend(signature)
                    
                    frames.extend(msg_bytes)
                    
                    # Generate TWO frames by duplicating the data
                    # The AX.25 encoder will create a separate frame for each transmission
                    frames_duplicated = bytearray(frames)
                    frames_duplicated.extend(frames)  # Append second frame
                    
                    return bytes(frames_duplicated)
        except Exception as e:
            print(f"Frame generation error: {e}")
            pass
        return None
    
    def work(self, input_items, output_items):
        try:
            import __main__
            if hasattr(__main__, 'use_pkcs11'):
                self._use_pkcs11 = getattr(__main__, 'use_pkcs11').value()
        except:
            pass
        
        audio_in = input_items[0]
        key_in = input_items[1]
        audio_out = output_items[0]
        data_out = output_items[1]
        n = len(output_items[0])  # Use output buffer size, not input size
        
        if len(key_in) > 0 and not self._use_pkcs11:
            self._key_buffer.extend(key_in.tolist())
            if len(self._key_buffer) > 32:
                self._key_buffer = self._key_buffer[-32:]
        
        # Track audio samples processed
        if len(audio_in) > 0 and not self._audio_eof_detected:
            self._total_audio_samples += len(audio_in)
        
        # Detect EOF when source stops (len(audio_in) == 0) AND we've processed audio
        # n comes from output buffer, so can be > 0 even when input is empty
        if len(audio_in) == 0 and not self._audio_eof_detected and self._total_audio_samples > 0:
            self._audio_eof_detected = True
            frames_bytes = self._generate_frames_bytes()
            if frames_bytes:
                self._data_frames_bytes = bytearray(frames_bytes)
                self._data_output_idx = 0
                self._frames_complete = False
                print(f"File source ended after {self._total_audio_samples} samples, generating AX.25 frames ({len(frames_bytes)} bytes)")
            else:
                print(f"File source ended after {self._total_audio_samples} samples, but no frames generated")
                self._frames_complete = True
        
        # Process audio or data frames
        if not self._audio_eof_detected:
            # Pass through audio while receiving it
            # Handle case where audio_in might be shorter than n
            audio_len = min(n, len(audio_in)) if len(audio_in) > 0 else 0
            if audio_len > 0:
                audio_out[:audio_len] = audio_in[:audio_len]
            # Zero-pad remaining audio output
            audio_out[audio_len:n] = 0.0
            data_out[:n] = 0
            # Always return n to keep flowgraph running
            return n
        else:
            # After EOF detected: flush buffers first, then output data frames
            # Continue outputting zeros for a while to ensure all audio is flushed from buffers
            if self._eof_flush_samples < self._eof_flush_needed:
                audio_out[:n] = 0.0
                data_out[:n] = 0
                self._eof_flush_samples += n
                return n
            
            # After flush period: output data frames
            audio_out[:n] = 0.0
            
            # Output data frames if available
            if not self._frames_complete and self._data_frames_bytes is not None:
                if self._data_output_idx < len(self._data_frames_bytes):
                    remaining = len(self._data_frames_bytes) - self._data_output_idx
                    n_output = min(n, remaining) if n > 0 else 0
                    
                    if n_output > 0:
                        data_out[:n_output] = np.frombuffer(
                            self._data_frames_bytes[self._data_output_idx:self._data_output_idx+n_output],
                            dtype=np.uint8
                        )
                        self._data_output_idx += n_output
                        
                        # Check if all frames are output
                        if self._data_output_idx >= len(self._data_frames_bytes):
                            self._frames_complete = True
                            print(f"All AX.25 frames output ({self._data_output_idx} bytes)")
                        
                        # Zero-pad remaining output
                        if n_output < n:
                            data_out[n_output:n] = 0
                        
                        # Return n to keep flowgraph running
                        return n
                    else:
                        # n == 0 but we still have frames to output
                        # Don't return 0 yet - wait for next call with n > 0
                        # But if n is always 0, we need to handle this
                        data_out[:n] = 0
                        # Return 0 but frames aren't complete, so flowgraph should call us again
                        # when other sources (like kernel_keyring) produce data
                        return 0
                else:
                    # All frames output
                    self._frames_complete = True
                    data_out[:n] = 0
                    return 0
            else:
                # No frames or frames complete
                data_out[:n] = 0
                return 0