
import sys
from PyQt5 import Qt
from tx_audio_signed import tx_audio_signed

app = Qt.QApplication(sys.argv)
tb = tx_audio_signed()

# Try adding buttons as a menu bar instead
menubar = tb.menuBar() if hasattr(tb, 'menuBar') else None
if menubar:
    print("Has menuBar")
else:
    print("No menuBar, using toolbar approach")

# Add buttons to a toolbar
toolbar = Qt.QToolBar("Controls")
toolbar.setMovable(False)

start_btn = Qt.QPushButton("Start Processing")
start_btn.setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 10px; min-width: 150px; }")
stop_btn = Qt.QPushButton("Stop")
stop_btn.setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 10px; min-width: 150px; }")
stop_btn.setEnabled(False)

toolbar.addWidget(start_btn)
toolbar.addWidget(stop_btn)

# Add toolbar to window
tb.addToolBar(Qt.Qt.TopToolBarArea, toolbar)

print("Buttons added via toolbar")
print(f"Top layout children: {tb.top_scroll_layout.count()}")
print(f"Window visible: {tb.isVisible()}")

tb.show()
sys.exit(app.exec_())

