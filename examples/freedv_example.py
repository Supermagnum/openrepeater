#!/usr/bin/env python3
"""
FreeDV Modulation and Demodulation Example

This example demonstrates FreeDV digital voice modulation and demodulation
using gr-qradiolink blocks. FreeDV is a digital voice mode for HF radio.

Note: This example uses Python blocks since FreeDV blocks don't have GRC
definitions yet. You can use this as a reference for creating GRC flowgraphs
or use it directly as a Python script.

Requirements:
- GNU Radio >= 3.10
- gr-qradiolink module installed
- gnuradio-vocoder with Codec2 support
"""

from gnuradio import gr
from gnuradio import audio
from gnuradio import blocks
from gnuradio import analog
from gnuradio import filter
from gnuradio.qtgui import qtgui
from PyQt5 import Qt
import sys
import signal

try:
    from gnuradio import qradiolink
    from gnuradio import vocoder
except ImportError:
    print("Error: gr-qradiolink module not found. Please install it first.")
    sys.exit(1)

class freedv_example(gr.top_block, Qt.QWidget):
    def __init__(self):
        gr.top_block.__init__(self, "FreeDV Example")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("FreeDV Example")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        # Variables
        self.samp_rate = samp_rate = 8000
        self.audio_rate = audio_rate = 8000
        self.carrier_freq = carrier_freq = 1700
        self.filter_width = filter_width = 2000
        self.low_cutoff = low_cutoff = 200
        self.freedv_mode = vocoder.freedv_api.MODE_1600
        self.sideband = 0  # 0 = USB, 1 = LSB

        # Blocks
        # Audio source (microphone input)
        try:
            self.audio_source = audio.source(audio_rate, '', True)
        except:
            # Fallback to signal source if audio not available
            self.audio_source = analog.sig_source_f(
                audio_rate, analog.GR_SIN_WAVE, 1000, 0.5, 0, 0)

        # FreeDV Modulator
        self.freedv_mod = qradiolink.mod_freedv(
            sps=125,
            samp_rate=samp_rate,
            carrier_freq=carrier_freq,
            filter_width=filter_width,
            low_cutoff=low_cutoff,
            mode=self.freedv_mode,
            sb=self.sideband
        )

        # Throttle to prevent excessive CPU usage
        self.throttle = blocks.throttle(gr.sizeof_gr_complex * 1, samp_rate, True)

        # FreeDV Demodulator
        self.freedv_demod = qradiolink.demod_freedv(
            sps=125,
            samp_rate=samp_rate,
            carrier_freq=carrier_freq,
            filter_width=filter_width,
            low_cutoff=low_cutoff,
            mode=self.freedv_mode,
            sb=self.sideband
        )

        # Audio sink (speaker output)
        try:
            self.audio_sink = audio.sink(audio_rate, '', True)
        except:
            # Fallback to null sink if audio not available
            self.audio_sink = blocks.null_sink(gr.sizeof_float * 1)

        # Frequency sink for modulated signal
        self.qtgui_freq_sink_c_0 = qtgui.freq_sink_c(
            1024,
            filter.firdes.WIN_BLACKMAN_HARRIS,
            0,
            samp_rate,
            "Modulated Signal",
            1,
            None
        )
        self.qtgui_freq_sink_c_0.set_update_time(0.10)
        self.qtgui_freq_sink_c_0.set_y_axis(-140, 10)
        self.qtgui_freq_sink_c_0.set_y_label('Relative', '')
        self.qtgui_freq_sink_c_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_c_0.enable_autoscale(False)
        self.qtgui_freq_sink_c_0.enable_grid(False)
        self.qtgui_freq_sink_c_0.set_fft_average(1.0)
        self.qtgui_freq_sink_c_0.enable_axis_labels(True)
        self.qtgui_freq_sink_c_0.enable_control_panel(False)
        self.qtgui_freq_sink_c_0.set_fft_window_normalized(False)

        # Time sink for demodulated audio
        self.qtgui_time_sink_f_0 = qtgui.time_sink_f(
            1024,
            audio_rate,
            "Demodulated Audio",
            1,
            None
        )
        self.qtgui_time_sink_f_0.set_update_time(0.10)
        self.qtgui_time_sink_f_0.set_y_axis(-1, 1)
        self.qtgui_time_sink_f_0.set_y_label('Amplitude', '')
        self.qtgui_time_sink_f_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_time_sink_f_0.enable_autoscale(False)
        self.qtgui_time_sink_f_0.enable_grid(False)
        self.qtgui_time_sink_f_0.enable_axis_labels(True)
        self.qtgui_time_sink_f_0.enable_control_panel(False)
        self.qtgui_time_sink_f_0.enable_stem_plot(False)

        # Connections
        self.connect((self.audio_source, 0), (self.freedv_mod, 0))
        self.connect((self.freedv_mod, 0), (self.throttle, 0))
        self.connect((self.throttle, 0), (self.freedv_demod, 0))
        self.connect((self.throttle, 0), (self.qtgui_freq_sink_c_0, 0))
        self.connect((self.freedv_demod, 1), (self.qtgui_time_sink_f_0, 0))
        self.connect((self.freedv_demod, 1), (self.audio_sink, 0))

        # Qt GUI
        self._qtgui_freq_sink_c_0_win = sip.wrapinstance(
            self.qtgui_freq_sink_c_0.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_c_0_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)

        self._qtgui_time_sink_f_0_win = sip.wrapinstance(
            self.qtgui_time_sink_f_0.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_time_sink_f_0_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)

    def closeEvent(self, event):
        self.stop()
        self.wait()
        event.accept()


def main(top_block_cls=freedv_example, options=None):
    if sys.platform.startswith('linux'):
        try:
            import x11
            x11.XInitThreads()
        except:
            pass

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    try:
        from distutils.version import StrictVersion
    except ImportError:
        from packaging.version import StrictVersion
    try:
        import sip
    except ImportError:
        import PyQt5.sip as sip
    main()

