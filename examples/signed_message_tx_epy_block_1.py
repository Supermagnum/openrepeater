import numpy as np
from gnuradio import gr


class blk(gr.sync_block):
    def __init__(self, silence_threshold=1e-3, hold_samples=2400):
        gr.sync_block.__init__(
            self,
            name='Audio/Data Scheduler',
            in_sig=[np.float32, np.float32],
            out_sig=[np.float32]
        )
        self._thresh = float(silence_threshold)
        self._hold = max(1, int(hold_samples))
        self._silence = self._hold

    def work(self, input_items, output_items):
        audio = input_items[0]
        data = input_items[1]
        out = output_items[0]
        n = min(len(out), len(audio), len(data))
        for i in range(n):
            sample = audio[i]
            if abs(sample) > self._thresh:
                out[i] = sample
                self._silence = 0
                continue
            if self._silence < self._hold:
                out[i] = sample
                self._silence += 1
                continue
            data_sample = data[i]
            if abs(data_sample) > 1e-12:
                out[i] = data_sample
                continue
            out[i] = 0.0
        return n