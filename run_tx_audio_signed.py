#!/usr/bin/env python3
import sys
from PyQt5 import Qt
from tx_audio_signed import tx_audio_signed
import signal

def main():
    qapp = Qt.QApplication(sys.argv)
    tb = tx_audio_signed()

    # Add Start/Stop buttons
    start_button = Qt.QPushButton("Start Processing")
    start_button.setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; min-width: 150px; }")
    stop_button = Qt.QPushButton("Stop")
    stop_button.setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 10px; min-width: 150px; }")
    stop_button.setEnabled(False)

    button_layout = Qt.QHBoxLayout()
    button_layout.addWidget(start_button)
    button_layout.addWidget(stop_button)
    button_widget = Qt.QWidget()
    button_widget.setLayout(button_layout)

    # Add button widget to the top layout
    tb.top_layout.insertWidget(0, button_widget)

    def start_processing():
        """Start the flowgraph processing"""
        if not tb.flowgraph_started.is_set():
            # Validate inputs
            if not tb.message_text or not tb.message_text.strip():
                print("Error: Please enter a message text")
                Qt.QMessageBox.warning(None, "Error", "Please enter a message text")
                return
            if not tb.audio_file_path or not tb.audio_file_path.strip():
                print("Error: Please enter an input audio file path")
                Qt.QMessageBox.warning(None, "Error", "Please enter an input audio file path")
                return
            if not tb.output_audio_file or not tb.output_audio_file.strip():
                print("Error: Please enter an output audio file path")
                Qt.QMessageBox.warning(None, "Error", "Please enter an output audio file path")
                return

            # Make sure message_text and file paths are set in __main__ for epy_block to read
            # Note: tb.message_text, tb.use_pkcs11 are Qt variable objects with .value() method
            import __main__
            __main__.message_text = tb.message_text  # Qt variable object
            __main__.audio_file_path = tb.audio_file_path  # String value
            __main__.output_audio_file = tb.output_audio_file  # String value
            __main__.use_pkcs11 = tb.use_pkcs11  # Qt variable object

            # Update wavfile_source and wavfile_sink file paths
            # Need to recreate blocks if flowgraph is not running
            from gnuradio import blocks
            if not tb.flowgraph_started.is_set():
                # Disconnect old blocks
                tb.disconnect((tb.blocks_wavfile_source_0, 0), (tb.epy_block_2, 0))
                tb.disconnect((tb.blocks_add_xx_0, 0), (tb.blocks_wavfile_sink_0, 0))
                
                # Recreate wavfile_source with new path
                tb.blocks_wavfile_source_0 = blocks.wavfile_source(tb.audio_file_path, False)
                tb.connect((tb.blocks_wavfile_source_0, 0), (tb.epy_block_2, 0))
                
                # Recreate wavfile_sink with new path
                tb.blocks_wavfile_sink_0 = blocks.wavfile_sink(
                    tb.output_audio_file,
                    1,
                    tb.samp_rate,
                    blocks.FORMAT_WAV,
                    blocks.FORMAT_PCM_16,
                    False
                )
                tb.connect((tb.blocks_add_xx_0, 0), (tb.blocks_wavfile_sink_0, 0))
            else:
                # If already running, try to update sink (source can't be changed while running)
                try:
                    tb.blocks_wavfile_sink_0.open(tb.output_audio_file)
                except:
                    pass

            print(f"Starting processing...")
            print(f"  Message: '{tb.message_text}'")
            print(f"  Input file: {tb.audio_file_path}")
            print(f"  Output file: {tb.output_audio_file}")

            tb.start()
            tb.flowgraph_started.set()
            start_button.setEnabled(False)
            stop_button.setEnabled(True)

    def stop_processing():
        """Stop the flowgraph processing"""
        if tb.flowgraph_started.is_set():
            tb.stop()
            tb.wait()
            # Ensure WAV file is properly closed
            try:
                tb.blocks_wavfile_sink_0.close()
            except:
                pass
            tb.flowgraph_started.clear()
            start_button.setEnabled(True)
            stop_button.setEnabled(False)
            print("Processing stopped.")

    start_button.clicked.connect(start_processing)
    stop_button.clicked.connect(stop_processing)

    # Don't auto-start
    # tb.start()
    # tb.flowgraph_started.set()

    tb.show()

    def sig_handler(sig=None, frame=None):
        if tb.flowgraph_started.is_set():
            tb.stop()
            tb.wait()
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    qapp.exec_()

if __name__ == '__main__':
    main()

