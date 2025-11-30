import numpy as np
from gnuradio import gr
try:
    from gnuradio import nacl
    NACL_AVAILABLE = True
except ImportError:
    NACL_AVAILABLE = False

class blk(gr.sync_block):
    def __init__(self):
        gr.sync_block.__init__(self, name='Message Verifier', in_sig=[np.uint8, np.uint8], out_sig=[np.uint8])
        self._msg_buffer = bytearray()
        self._key_buffer = bytearray()
    def work(self, input_items, output_items):
        msg_in = input_items[0]
        key_in = input_items[1]
        if len(msg_in) > 0:
            self._msg_buffer.extend(msg_in.tolist())
        if len(key_in) > 0:
            self._key_buffer.extend(key_in.tolist())
            if len(self._key_buffer) > 32:
                self._key_buffer = self._key_buffer[-32:]
        # Verify when we have message + signature (separated by 0x00)
        if len(self._msg_buffer) > 100 and len(self._key_buffer) >= 32:
            try:
                sep_idx = self._msg_buffer.find(0)
                if sep_idx > 0 and len(self._msg_buffer) >= sep_idx + 65:
                    msg_bytes = bytes(self._msg_buffer[:sep_idx])
                    sig_bytes = bytes(self._msg_buffer[sep_idx+1:sep_idx+65])
                    key = bytes(self._key_buffer[:32])
                    if NACL_AVAILABLE and len(sig_bytes) == 64:
                        is_valid = nacl.verify_ed25519(msg_bytes, sig_bytes, key)
                        if is_valid:
                            n = min(len(output_items[0]), len(msg_bytes))
                            if n > 0:
                                output_items[0][:n] = np.frombuffer(msg_bytes[:n], dtype=np.uint8)
                                self._msg_buffer = self._msg_buffer[sep_idx+65:]
                                return n
                    self._msg_buffer = self._msg_buffer[sep_idx+65:]
            except:
                pass
        return 0
