#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Audio File with Signed Frames
# Author: gr-packet-protocols
# Description: Transmit audio file with signed signature and ASCII message frames in AX.25 at 2400 baud
# GNU Radio version: 3.10.12.0

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import linux_crypto
from gnuradio import packet_protocols
import threading
import tx_audio_signed_epy_block_2 as epy_block_2  # embedded python block



class tx_audio_signed(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Audio File with Signed Frames", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Audio File with Signed Frames")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except BaseException as exc:
            print(f"Qt GUI: Could not set Icon: {str(exc)}", file=sys.stderr)
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

        self.settings = Qt.QSettings("gnuradio/flowgraphs", "tx_audio_signed")

        try:
            geometry = self.settings.value("geometry")
            if geometry:
                self.restoreGeometry(geometry)
        except BaseException as exc:
            print(f"Qt GUI: Could not restore geometry: {str(exc)}", file=sys.stderr)
        self.flowgraph_started = threading.Event()

        ##################################################
        # Variables
        ##################################################
        self.use_pkcs11 = use_pkcs11 = True
        self.src_callsign = src_callsign = 'N0CALL'
        self.samp_rate = samp_rate = 48000
        self.output_audio_file = output_audio_file = '/home/haaken/Musikk/cq+ax25.wav'
        self.message_text = message_text = ''
        self.dest_callsign = dest_callsign = 'N0CALL'
        self.audio_file_path = audio_file_path = '/home/haaken/Musikk/cq.wav'
        self.is_running = False

        ##################################################
        # Blocks
        ##################################################

        # Message Text input
        self._message_text_tool_bar = Qt.QToolBar(self)
        self._message_text_tool_bar.addWidget(Qt.QLabel("Message Text" + ": "))
        self._message_text_line_edit = Qt.QLineEdit(str(self.message_text))
        self._message_text_tool_bar.addWidget(self._message_text_line_edit)
        self._message_text_line_edit.editingFinished.connect(
            lambda: self.set_message_text(str(str(self._message_text_line_edit.text()))))
        self.top_layout.addWidget(self._message_text_tool_bar)

        # Source Callsign
        self._src_callsign_tool_bar = Qt.QToolBar(self)
        self._src_callsign_tool_bar.addWidget(Qt.QLabel("Src Callsign" + ": "))
        self._src_callsign_line_edit = Qt.QLineEdit(str(self.src_callsign))
        self._src_callsign_tool_bar.addWidget(self._src_callsign_line_edit)
        self._src_callsign_line_edit.editingFinished.connect(
            lambda: self.set_src_callsign(str(str(self._src_callsign_line_edit.text()))))
        self.top_layout.addWidget(self._src_callsign_tool_bar)

        # Destination Callsign
        self._dest_callsign_tool_bar = Qt.QToolBar(self)
        self._dest_callsign_tool_bar.addWidget(Qt.QLabel("Dest Callsign" + ": "))
        self._dest_callsign_line_edit = Qt.QLineEdit(str(self.dest_callsign))
        self._dest_callsign_tool_bar.addWidget(self._dest_callsign_line_edit)
        self._dest_callsign_line_edit.editingFinished.connect(
            lambda: self.set_dest_callsign(str(str(self._dest_callsign_line_edit.text()))))
        self.top_layout.addWidget(self._dest_callsign_tool_bar)

        # PKCS#11 checkbox
        _use_pkcs11_check_box = Qt.QCheckBox("Use PKCS#11 (unchecked = Kernel Keyring)")
        self._use_pkcs11_choices = {True: True, False: False}
        self._use_pkcs11_choices_inv = dict((v,k) for k,v in self._use_pkcs11_choices.items())
        self._use_pkcs11_callback = lambda i: Qt.QMetaObject.invokeMethod(_use_pkcs11_check_box, "setChecked", Qt.Q_ARG("bool", self._use_pkcs11_choices_inv[i]))
        self._use_pkcs11_callback(self.use_pkcs11)
        _use_pkcs11_check_box.stateChanged.connect(lambda i: self.set_use_pkcs11(self._use_pkcs11_choices[bool(i)]))
        self.top_layout.addWidget(_use_pkcs11_check_box)

        # Output Audio File Path
        self._output_audio_file_tool_bar = Qt.QToolBar(self)
        self._output_audio_file_tool_bar.addWidget(Qt.QLabel("Output Audio File Path" + ": "))
        self._output_audio_file_line_edit = Qt.QLineEdit(str(self.output_audio_file))
        self._output_audio_file_tool_bar.addWidget(self._output_audio_file_line_edit)
        self._output_audio_file_line_edit.editingFinished.connect(
            lambda: self.set_output_audio_file(str(str(self._output_audio_file_line_edit.text()))))
        self.top_layout.addWidget(self._output_audio_file_tool_bar)

        # Audio File Path (Input)
        self._audio_file_path_tool_bar = Qt.QToolBar(self)
        self._audio_file_path_tool_bar.addWidget(Qt.QLabel("Audio File Path" + ": "))
        self._audio_file_path_line_edit = Qt.QLineEdit(str(self.audio_file_path))
        self._audio_file_path_tool_bar.addWidget(self._audio_file_path_line_edit)
        self._audio_file_path_line_edit.editingFinished.connect(
            lambda: self.set_audio_file_path(str(str(self._audio_file_path_line_edit.text()))))
        self.top_layout.addWidget(self._audio_file_path_tool_bar)

        # START BUTTON
        self._start_button = Qt.QPushButton("Start Transmission")
        self._start_button.clicked.connect(self._on_start_clicked)
        self._start_button.setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; }")
        self.top_layout.addWidget(self._start_button)

        # STOP BUTTON
        self._stop_button = Qt.QPushButton("Stop Transmission")
        self._stop_button.clicked.connect(self._on_stop_clicked)
        self._stop_button.setEnabled(False)
        self._stop_button.setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 10px; }")
        self.top_layout.addWidget(self._stop_button)

        # Status label
        self._status_label = Qt.QLabel("Status: Ready")
        self._status_label.setStyleSheet("QLabel { font-weight: bold; padding: 10px; }")
        self.top_layout.addWidget(self._status_label)

        # Initialize blocks (but don't connect yet)
        self.packet_protocols_ax25_encoder_0 = packet_protocols.ax25_encoder(dest_callsign, '0', src_callsign, '0', '', False, False)
        self.linux_crypto_kernel_keyring_source_0 = linux_crypto.kernel_keyring_source(0, False)
        self.epy_block_2 = epy_block_2.blk()
        self.blocks_wavfile_source_0 = blocks.wavfile_source(self.audio_file_path, False)
        self.blocks_wavfile_sink_0 = blocks.wavfile_sink(
            self.output_audio_file,
            1,
            samp_rate,
            blocks.FORMAT_WAV,
            blocks.FORMAT_PCM_16,
            False
            )
        self.blocks_throttle2_0 = blocks.throttle( gr.sizeof_char*1, 2400, True, 0 if "auto" == "auto" else max( int(float(0.1) * 2400) if "auto" == "time" else int(0.1), 1) )
        self.blocks_repeat_0 = blocks.repeat(gr.sizeof_float*1, 20)
        self.blocks_char_to_float_0 = blocks.char_to_float(1, (1.0/127.0))
        self.blocks_add_xx_0 = blocks.add_vff(1)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_add_xx_0, 0), (self.blocks_wavfile_sink_0, 0))
        self.connect((self.blocks_char_to_float_0, 0), (self.blocks_repeat_0, 0))
        self.connect((self.blocks_repeat_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_throttle2_0, 0), (self.packet_protocols_ax25_encoder_0, 0))
        self.connect((self.blocks_wavfile_source_0, 0), (self.epy_block_2, 0))
        self.connect((self.epy_block_2, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.epy_block_2, 1), (self.blocks_throttle2_0, 0))
        self.connect((self.linux_crypto_kernel_keyring_source_0, 0), (self.epy_block_2, 1))
        self.connect((self.packet_protocols_ax25_encoder_0, 0), (self.blocks_char_to_float_0, 0))

    def _on_start_clicked(self):
        """Handle Start button click"""
        try:
            # Update file paths if they were changed
            if self.audio_file_path:
                # Recreate wavfile source with new path
                self.disconnect((self.blocks_wavfile_source_0, 0), (self.epy_block_2, 0))
                self.blocks_wavfile_source_0 = blocks.wavfile_source(self.audio_file_path, False)
                self.connect((self.blocks_wavfile_source_0, 0), (self.epy_block_2, 0))

            if self.output_audio_file:
                # Recreate wavfile sink with new path
                self.disconnect((self.blocks_add_xx_0, 0), (self.blocks_wavfile_sink_0, 0))
                self.blocks_wavfile_sink_0 = blocks.wavfile_sink(
                    self.output_audio_file,
                    1,
                    self.samp_rate,
                    blocks.FORMAT_WAV,
                    blocks.FORMAT_PCM_16,
                    False
                )
                self.connect((self.blocks_add_xx_0, 0), (self.blocks_wavfile_sink_0, 0))

            # Update AX.25 encoder with current callsigns
            self.disconnect((self.blocks_throttle2_0, 0), (self.packet_protocols_ax25_encoder_0, 0))
            self.disconnect((self.packet_protocols_ax25_encoder_0, 0), (self.blocks_char_to_float_0, 0))
            self.packet_protocols_ax25_encoder_0 = packet_protocols.ax25_encoder(
                self.dest_callsign, '0', self.src_callsign, '0', '', False, False
            )
            self.connect((self.blocks_throttle2_0, 0), (self.packet_protocols_ax25_encoder_0, 0))
            self.connect((self.packet_protocols_ax25_encoder_0, 0), (self.blocks_char_to_float_0, 0))

            # Start the flowgraph
            self.start()
            self.is_running = True

            # Update UI
            self._start_button.setEnabled(False)
            self._stop_button.setEnabled(True)
            self._status_label.setText("Status: Transmitting...")
            self._status_label.setStyleSheet("QLabel { font-weight: bold; padding: 10px; color: green; }")

            print(f"Started transmission:")
            print(f"  Audio input: {self.audio_file_path}")
            print(f"  Audio output: {self.output_audio_file}")
            print(f"  Source: {self.src_callsign}")
            print(f"  Dest: {self.dest_callsign}")
            print(f"  Message: {self.message_text}")
            print(f"  Using PKCS#11: {self.use_pkcs11}")

        except Exception as e:
            print(f"Error starting transmission: {e}")
            self._status_label.setText(f"Status: Error - {str(e)}")
            self._status_label.setStyleSheet("QLabel { font-weight: bold; padding: 10px; color: red; }")

    def _on_stop_clicked(self):
        """Handle Stop button click"""
        try:
            self.stop()
            self.wait()
            self.is_running = False

            # Update UI
            self._start_button.setEnabled(True)
            self._stop_button.setEnabled(False)
            self._status_label.setText("Status: Stopped")
            self._status_label.setStyleSheet("QLabel { font-weight: bold; padding: 10px; color: orange; }")

            print("Transmission stopped")

        except Exception as e:
            print(f"Error stopping transmission: {e}")
            self._status_label.setText(f"Status: Error - {str(e)}")
            self._status_label.setStyleSheet("QLabel { font-weight: bold; padding: 10px; color: red; }")

    def closeEvent(self, event):
        self.settings = Qt.QSettings("gnuradio/flowgraphs", "tx_audio_signed")
        self.settings.setValue("geometry", self.saveGeometry())
        if self.is_running:
            self.stop()
            self.wait()
        event.accept()

    def get_use_pkcs11(self):
        return self.use_pkcs11

    def set_use_pkcs11(self, use_pkcs11):
        self.use_pkcs11 = use_pkcs11
        self._use_pkcs11_callback(self.use_pkcs11)

    def get_src_callsign(self):
        return self.src_callsign

    def set_src_callsign(self, src_callsign):
        self.src_callsign = src_callsign
        Qt.QMetaObject.invokeMethod(self._src_callsign_line_edit, "setText", Qt.Q_ARG("QString", str(self.src_callsign)))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

    def get_output_audio_file(self):
        return self.output_audio_file

    def set_output_audio_file(self, output_audio_file):
        self.output_audio_file = output_audio_file
        Qt.QMetaObject.invokeMethod(self._output_audio_file_line_edit, "setText", Qt.Q_ARG("QString", str(self.output_audio_file)))

    def get_message_text(self):
        return self.message_text

    def set_message_text(self, message_text):
        self.message_text = message_text
        Qt.QMetaObject.invokeMethod(self._message_text_line_edit, "setText", Qt.Q_ARG("QString", str(self.message_text)))

    def get_dest_callsign(self):
        return self.dest_callsign

    def set_dest_callsign(self, dest_callsign):
        self.dest_callsign = dest_callsign
        Qt.QMetaObject.invokeMethod(self._dest_callsign_line_edit, "setText", Qt.Q_ARG("QString", str(self.dest_callsign)))

    def get_audio_file_path(self):
        return self.audio_file_path

    def set_audio_file_path(self, audio_file_path):
        self.audio_file_path = audio_file_path
        Qt.QMetaObject.invokeMethod(self._audio_file_path_line_edit, "setText", Qt.Q_ARG("QString", str(self.audio_file_path)))




def main(top_block_cls=tx_audio_signed, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.show()

    def sig_handler(sig=None, frame=None):
        if tb.is_running:
            tb.stop()
            tb.wait()
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()

