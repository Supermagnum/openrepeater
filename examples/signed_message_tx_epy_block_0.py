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
            name='Message Signer (PKCS#11/Kernel Keyring)',
            in_sig=[np.uint8],  # Key data from kernel keyring (fallback)
            out_sig=[np.uint8]
        )
        self._key_buffer = bytearray()
        self._signed_msg = None
        self._output_idx = 0
        self._last_send = False
        self._pkcs11_lib = None
        self._use_pkcs11 = True  # Default to PKCS#11
        
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
    
    def work(self, input_items, output_items):
        # Check configuration
        try:
            import __main__
            if hasattr(__main__, 'use_pkcs11'):
                self._use_pkcs11 = getattr(__main__, 'use_pkcs11').value()
        except:
            pass
        
        # Collect key data from kernel keyring (if not using PKCS#11)
        key_in = input_items[0]
        if len(key_in) > 0 and not self._use_pkcs11:
            self._key_buffer.extend(key_in.tolist())
            if len(self._key_buffer) > 32:
                self._key_buffer = self._key_buffer[-32:]
        
        try:
            import __main__
            if hasattr(__main__, 'send_button'):
                btn = getattr(__main__, 'send_button')
                if btn.value() and not self._last_send:
                    if hasattr(__main__, 'message_text'):
                        msg = getattr(__main__, 'message_text').value()
                        if msg:
                            msg_bytes = msg.encode('utf-8')
                            signature = None
                            
                            # Use PKCS#11 if configured
                            if self._use_pkcs11:
                                signature = self._sign_with_pkcs11(msg_bytes)
                            
                            # Fallback to kernel keyring + nacl
                            if not signature and len(self._key_buffer) >= 32 and NACL_AVAILABLE:
                                key = bytes(self._key_buffer[:32])
                                signature = nacl.sign_ed25519(msg_bytes, key)
                            
                            if signature:
                                self._signed_msg = msg_bytes + b'\x00' + signature
                                self._output_idx = 0
                
                self._last_send = btn.value()
        except Exception as e:
            pass
        
        if self._signed_msg and self._output_idx < len(self._signed_msg):
            n = min(len(output_items[0]), len(self._signed_msg) - self._output_idx)
            if n > 0:
                output_items[0][:n] = np.frombuffer(
                    self._signed_msg[self._output_idx:self._output_idx+n],
                    dtype=np.uint8
                )
                self._output_idx += n
                if self._output_idx >= len(self._signed_msg):
                    self._signed_msg = None
                    self._output_idx = 0
                return n
        
        if len(output_items[0]) > 0:
            output_items[0][:len(output_items[0])] = 0
            return len(output_items[0])
        return 0