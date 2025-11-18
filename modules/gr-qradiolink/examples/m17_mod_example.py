#!/usr/bin/env python3
"""
M17 Modulation Example

This example demonstrates M17 digital voice modulation using gr-qradiolink.
M17 is a digital voice protocol for amateur radio.

Note: This example uses Python blocks since mod_m17 doesn't have a GRC
definition yet. You can use this as a reference for creating GRC flowgraphs
or use it directly as a Python script.

Requirements:
- GNU Radio >= 3.10
- gr-qradiolink module installed
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
except ImportError:
    print("Error: gr-qradiolink module not found. Please install it first.")
    sys.exit(1)

class m17_mod_example(gr.top_block, Qt.QWidget):
    def __init__(self):
        gr.top_block.__init__(self, "M17 Mod Example")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("M17 Modulation Example")
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
        self.samp_rate = samp_rate = 1000000
        self.audio_rate = audio_rate = 8000
        self.carrier_freq = carrier_freq = 1700
        self.filter_width = filter_width = 9000

        # Blocks
        # Audio source (microphone input)
        try:
            self.audio_source = audio.source(audio_rate, '', True)
        except:
            # Fallback to signal source if audio not available
            self.audio_source = analog.sig_source_f(
                audio_rate, analog.GR_SIN_WAVE, 1000, 0.5, 0, 0)

        # M17 Modulator
        self.m17_mod = qradiolink.mod_m17(
            sps=125,
            samp_rate=samp_rate,
            carrier_freq=carrier_freq,
            filter_width=filter_width
        )

        # Throttle to prevent excessive CPU usage
        self.throttle = blocks.throttle(gr.sizeof_gr_complex * 1, samp_rate, True)

        # Frequency sink for modulated signal
        self.qtgui_freq_sink_c_0 = qtgui.freq_sink_c(
            1024,
            filter.firdes.WIN_BLACKMAN_HARRIS,
            0,
            samp_rate,
            "M17 Modulated Signal",
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

        # File sink to save modulated signal
        self.file_sink = blocks.file_sink(
            gr.sizeof_gr_complex * 1,
            '/tmp/m17_modulated.bin',
            False
        )
        self.file_sink.set_unbuffered(False)

        # Connections
        self.connect((self.audio_source, 0), (self.m17_mod, 0))
        self.connect((self.m17_mod, 0), (self.throttle, 0))
        self.connect((self.throttle, 0), (self.qtgui_freq_sink_c_0, 0))
        self.connect((self.throttle, 0), (self.file_sink, 0))

        # Qt GUI
        self._qtgui_freq_sink_c_0_win = sip.wrapinstance(
            self.qtgui_freq_sink_c_0.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_c_0_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)

    def closeEvent(self, event):
        self.stop()
        self.wait()
        event.accept()


def main(top_block_cls=m17_mod_example, options=None):
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

