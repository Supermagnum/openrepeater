#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Signed Message Transmitter
# Author: gr-packet-protocols
# Description: Transmit signed text messages using NFM and AX.25
# GNU Radio version: 3.10.12.0

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio import audio
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
from gnuradio import qradiolink
import signed_message_tx_epy_block_0 as epy_block_0  # embedded python block
import signed_message_tx_epy_block_1 as epy_block_1  # embedded python block
import threading



class signed_message_tx(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Signed Message Transmitter", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Signed Message Transmitter")
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

        self.settings = Qt.QSettings("gnuradio/flowgraphs", "signed_message_tx")

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
        self.send_button = send_button = False
        self.samp_rate = samp_rate = 48000
        self.nitrokey_slot = nitrokey_slot = 1
        self.message_text = message_text = ''
        self.dest_callsign = dest_callsign = 'N0CALL'

        ##################################################
        # Blocks
        ##################################################

        self._src_callsign_tool_bar = Qt.QToolBar(self)
        self._src_callsign_tool_bar.addWidget(Qt.QLabel("Src Callsign" + ": "))
        self._src_callsign_line_edit = Qt.QLineEdit(str(self.src_callsign))
        self._src_callsign_tool_bar.addWidget(self._src_callsign_line_edit)
        self._src_callsign_line_edit.editingFinished.connect(
            lambda: self.set_src_callsign(str(str(self._src_callsign_line_edit.text()))))
        self.top_layout.addWidget(self._src_callsign_tool_bar)
        self._dest_callsign_tool_bar = Qt.QToolBar(self)
        self._dest_callsign_tool_bar.addWidget(Qt.QLabel("Dest Callsign" + ": "))
        self._dest_callsign_line_edit = Qt.QLineEdit(str(self.dest_callsign))
        self._dest_callsign_tool_bar.addWidget(self._dest_callsign_line_edit)
        self._dest_callsign_line_edit.editingFinished.connect(
            lambda: self.set_dest_callsign(str(str(self._dest_callsign_line_edit.text()))))
        self.top_layout.addWidget(self._dest_callsign_tool_bar)
        _use_pkcs11_check_box = Qt.QCheckBox("Use PKCS#11 (unchecked = Kernel Keyring)")
        self._use_pkcs11_choices = {True: True, False: False}
        self._use_pkcs11_choices_inv = dict((v,k) for k,v in self._use_pkcs11_choices.items())
        self._use_pkcs11_callback = lambda i: Qt.QMetaObject.invokeMethod(_use_pkcs11_check_box, "setChecked", Qt.Q_ARG("bool", self._use_pkcs11_choices_inv[i]))
        self._use_pkcs11_callback(self.use_pkcs11)
        _use_pkcs11_check_box.stateChanged.connect(lambda i: self.set_use_pkcs11(self._use_pkcs11_choices[bool(i)]))
        self.top_layout.addWidget(_use_pkcs11_check_box)
        _send_button_push_button = Qt.QPushButton('Send')
        _send_button_push_button = Qt.QPushButton('Send')
        self._send_button_choices = {'Pressed': 1, 'Released': 0}
        _send_button_push_button.pressed.connect(lambda: self.set_send_button(self._send_button_choices['Pressed']))
        _send_button_push_button.released.connect(lambda: self.set_send_button(self._send_button_choices['Released']))
        self.top_layout.addWidget(_send_button_push_button)
        self.qradiolink_mod_nbfm_0 = qradiolink.mod_nbfm(sps=125, samp_rate=samp_rate, carrier_freq=1700, filter_width=8000)
        self.packet_protocols_ax25_encoder_0 = packet_protocols.ax25_encoder(dest_callsign, '0', src_callsign, '0', '', False, False)
        self._nitrokey_slot_tool_bar = Qt.QToolBar(self)
        self._nitrokey_slot_tool_bar.addWidget(Qt.QLabel("Nitrokey Slot" + ": "))
        self._nitrokey_slot_line_edit = Qt.QLineEdit(str(self.nitrokey_slot))
        self._nitrokey_slot_tool_bar.addWidget(self._nitrokey_slot_line_edit)
        self._nitrokey_slot_line_edit.editingFinished.connect(
            lambda: self.set_nitrokey_slot(int(str(self._nitrokey_slot_line_edit.text()))))
        self.top_layout.addWidget(self._nitrokey_slot_tool_bar)
        self._message_text_tool_bar = Qt.QToolBar(self)
        self._message_text_tool_bar.addWidget(Qt.QLabel("Message Text" + ": "))
        self._message_text_line_edit = Qt.QLineEdit(str(self.message_text))
        self._message_text_tool_bar.addWidget(self._message_text_line_edit)
        self._message_text_line_edit.editingFinished.connect(
            lambda: self.set_message_text(str(str(self._message_text_line_edit.text()))))
        self.top_grid_layout.addWidget(self._message_text_tool_bar, 0, 0, 2, 1)
        for r in range(0, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.linux_crypto_kernel_keyring_source_0 = linux_crypto.kernel_keyring_source(0, False)
        self.epy_block_1 = epy_block_1.blk(silence_threshold=0.001, hold_samples=2400)
        self.epy_block_0 = epy_block_0.blk()
        self.blocks_throttle2_0 = blocks.throttle( gr.sizeof_char*1, samp_rate, True, 0 if "auto" == "auto" else max( int(float(0.1) * samp_rate) if "auto" == "time" else int(0.1), 1) )
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_gr_complex*1)
        self.blocks_char_to_float_0 = blocks.char_to_float(1, (1.0/127.0))
        self.audio_source_0 = audio.source(samp_rate, 'default', True)


        ##################################################
        # Connections
        ##################################################
        self.connect((self.audio_source_0, 0), (self.epy_block_1, 0))
        self.connect((self.blocks_char_to_float_0, 0), (self.epy_block_1, 1))
        self.connect((self.blocks_throttle2_0, 0), (self.packet_protocols_ax25_encoder_0, 0))
        self.connect((self.epy_block_0, 0), (self.blocks_throttle2_0, 0))
        self.connect((self.epy_block_1, 0), (self.qradiolink_mod_nbfm_0, 0))
        self.connect((self.linux_crypto_kernel_keyring_source_0, 0), (self.epy_block_0, 0))
        self.connect((self.packet_protocols_ax25_encoder_0, 0), (self.blocks_char_to_float_0, 0))
        self.connect((self.qradiolink_mod_nbfm_0, 0), (self.blocks_null_sink_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("gnuradio/flowgraphs", "signed_message_tx")
        self.settings.setValue("geometry", self.saveGeometry())
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

    def get_send_button(self):
        return self.send_button

    def set_send_button(self, send_button):
        self.send_button = send_button

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle2_0.set_sample_rate(self.samp_rate)

    def get_nitrokey_slot(self):
        return self.nitrokey_slot

    def set_nitrokey_slot(self, nitrokey_slot):
        self.nitrokey_slot = nitrokey_slot
        Qt.QMetaObject.invokeMethod(self._nitrokey_slot_line_edit, "setText", Qt.Q_ARG("QString", str(self.nitrokey_slot)))

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




def main(top_block_cls=signed_message_tx, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()
    tb.flowgraph_started.set()

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

if __name__ == '__main__':
    main()
