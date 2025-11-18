# PTT Button Usage Guide

This guide explains how to use a Push-To-Talk (PTT) button with the authenticated repeater control system.

## Overview

PTT (Push-To-Talk) is a method of controlling radio transmission where the operator must press and hold a button to transmit. This is common in amateur radio and repeater systems.

## Hardware Setup

### GPIO-based PTT (Raspberry Pi)

**Hardware Requirements:**
- Raspberry Pi with GPIO header
- PTT button (momentary push button)
- Radio with PTT input (typically active low)

**Wiring:**
```
PTT Button → GPIO Pin (e.g., GPIO 18)
Radio PTT Input → GPIO Pin (via level shifter if needed)
Ground → Common ground
```

**GPIO Pin Selection:**
- Use any available GPIO pin
- Common choices: GPIO 18, GPIO 23, GPIO 24
- Avoid pins used by other functions (I2C, SPI, UART)

### Serial PTT (USB/Serial Radio Controllers)

**Hardware Requirements:**
- Radio with serial control interface
- USB-to-serial adapter (if needed)
- Appropriate cables

**Connection:**
```
Computer → USB/Serial Adapter → Radio Control Port
```

**Serial Parameters:**
- Baud rate: Typically 9600 or 19200
- Data bits: 8
- Stop bits: 1
- Parity: None

### Hardware PTT via SDR

Some SDR devices support hardware PTT:
- HackRF One: Uses GPIO or external PTT
- PlutoSDR: Uses GPIO pins
- RTL-SDR: Typically requires external PTT

## Software Implementation

### Method 1: GPIO PTT in GNU Radio Flowgraph

**Using Embedded Python Block:**

1. Add an Embedded Python block to your flowgraph
2. Configure the block with PTT control code:

```python
import RPi.GPIO as GPIO
import time

class ptt_control(gr.sync_block):
    def __init__(self, ptt_pin=18, tx_delay=0.1, hang_time=0.2):
        gr.sync_block.__init__(
            self,
            name="PTT Control",
            in_sig=[np.float32],
            out_sig=[np.float32]
        )
        self.ptt_pin = ptt_pin
        self.tx_delay = tx_delay  # Delay before TX (seconds)
        self.hang_time = hang_time  # Hang time after TX (seconds)
        self.ptt_active = False
        self.tx_start_time = 0
        
        # Setup GPIO
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.ptt_pin, GPIO.OUT)
        GPIO.output(self.ptt_pin, GPIO.LOW)  # PTT inactive (active low)
    
    def work(self, input_items, output_items):
        noutput_items = len(output_items[0])
        
        # Check if we have input data (transmission)
        has_data = np.any(np.abs(input_items[0]) > 0.01)
        
        if has_data and not self.ptt_active:
            # Start transmission
            time.sleep(self.tx_delay)
            GPIO.output(self.ptt_pin, GPIO.HIGH)  # PTT active
            self.ptt_active = True
            self.tx_start_time = time.time()
        
        if not has_data and self.ptt_active:
            # End transmission
            elapsed = time.time() - self.tx_start_time
            if elapsed > self.hang_time:
                GPIO.output(self.ptt_pin, GPIO.LOW)  # PTT inactive
                self.ptt_active = False
        
        # Pass through audio
        output_items[0][:] = input_items[0]
        return noutput_items
```

**Block Configuration:**
```
Block: Embedded Python
ID: ptt_control_0
PTT Pin: 18
TX Delay: 0.1
Hang Time: 0.2
```

### Method 2: GUI Push Button PTT

**Using QT GUI Push Button:**

1. Add a QT GUI Push Button block
2. Connect button output to a message handler
3. Use message handler to control PTT

**Flowgraph Structure:**
```
[QT GUI Push Button] 
    ↓ (message)
[Message Handler Block]
    ↓ (controls)
[PTT Control Block] → [GPIO/Serial]
```

**Message Handler Code:**
```python
class ptt_message_handler(gr.basic_block):
    def __init__(self, ptt_pin=18):
        gr.basic_block.__init__(self, name="PTT Message Handler")
        self.message_port_register_in(pmt.intern("button"))
        self.set_msg_handler(pmt.intern("button"), self.handle_button)
        
        # Setup GPIO
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(ptt_pin, GPIO.OUT)
        self.ptt_pin = ptt_pin
    
    def handle_button(self, msg):
        # Button pressed: PTT ON
        if pmt.to_python(msg):
            GPIO.output(self.ptt_pin, GPIO.HIGH)
        # Button released: PTT OFF
        else:
            GPIO.output(self.ptt_pin, GPIO.LOW)
```

### Method 3: Serial PTT Control

**Using Serial Port Block:**

1. Add a Serial Port Sink block
2. Configure for your radio's control protocol
3. Send PTT commands when needed

**Example Code:**
```python
import serial

class serial_ptt(gr.sync_block):
    def __init__(self, port="/dev/ttyUSB0", baud=9600):
        gr.sync_block.__init__(self, name="Serial PTT")
        self.ser = serial.Serial(port, baud)
        self.ptt_active = False
    
    def work(self, input_items, output_items):
        has_data = np.any(np.abs(input_items[0]) > 0.01)
        
        if has_data and not self.ptt_active:
            self.ser.write(b"PTT_ON\n")
            self.ptt_active = True
        elif not has_data and self.ptt_active:
            self.ser.write(b"PTT_OFF\n")
            self.ptt_active = False
        
        output_items[0][:] = input_items[0]
        return len(output_items[0])
```

## PTT Timing Considerations

### TX Delay (Pre-Key)

**Purpose**: Allow radio to stabilize before transmission

**Typical Values:**
- VHF/UHF: 50-100ms
- HF: 100-200ms
- SDR: 50-150ms

**Implementation:**
```python
time.sleep(tx_delay)  # Before enabling PTT
GPIO.output(ptt_pin, GPIO.HIGH)
```

### Hang Time (Post-Key)

**Purpose**: Keep PTT active briefly after transmission ends

**Typical Values:**
- Voice: 100-300ms
- Data: 50-200ms
- Repeater: 200-500ms

**Implementation:**
```python
# After detecting end of transmission
time.sleep(hang_time)  # Before disabling PTT
GPIO.output(ptt_pin, GPIO.LOW)
```

### VOX Alternative

**VOX (Voice Operated Exchange)** automatically keys the transmitter when audio is detected:

**Advantages:**
- Hands-free operation
- Automatic timing
- No button needed

**Disadvantages:**
- May key on background noise
- Requires threshold adjustment
- Less control than manual PTT

## Flowgraph Integration

### Transmitter with PTT

```
[Audio Source/Message]
    ↓
[Sign Message Block]
    ↓
[AX.25 Encoder]
    ↓
[NFM Modulator]
    ↓
[PTT Control Block] ← [GPIO/Serial]
    ↓
[Radio Sink]
```

### Receiver (No PTT Needed)

```
[Radio Source]
    ↓
[NFM Demodulator]
    ↓
[AX.25 Decoder]
    ↓
[Verify Message Block]
    ↓
[Command Handler]
```

### Bidirectional with PTT Toggle

```
[PTT Button] → [Mode Selector]
                    ↓
            [TX Mode] or [RX Mode]
                    ↓
        [TX Flowgraph] or [RX Flowgraph]
```

## Testing PTT

### Test GPIO PTT

```bash
# Install GPIO library
sudo apt-get install python3-rpi.gpio

# Test script
python3 << EOF
import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT)

print("PTT ON")
GPIO.output(18, GPIO.HIGH)
time.sleep(1)

print("PTT OFF")
GPIO.output(18, GPIO.LOW)

GPIO.cleanup()
EOF
```

### Test Serial PTT

```bash
# Install serial library
sudo apt-get install python3-serial

# Test script
python3 << EOF
import serial
import time

ser = serial.Serial("/dev/ttyUSB0", 9600)

print("PTT ON")
ser.write(b"PTT_ON\n")
time.sleep(1)

print("PTT OFF")
ser.write(b"PTT_OFF\n")

ser.close()
EOF
```

## Troubleshooting

### PTT Not Activating

**Check:**
1. GPIO pin number is correct
2. Wiring is correct (check with multimeter)
3. Radio PTT input polarity (active high vs active low)
4. Permissions for GPIO access (may need `sudo` or add user to `gpio` group)

### PTT Stuck On

**Causes:**
- Software crash leaving GPIO high
- Hardware short circuit
- Incorrect polarity

**Fix:**
```bash
# Manually reset GPIO
echo 18 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio18/direction
echo 0 > /sys/class/gpio/gpio18/value
```

### Timing Issues

**Symptoms:**
- Audio cut off at start
- Audio cut off at end
- Radio not keying properly

**Solutions:**
- Increase TX delay
- Increase hang time
- Check radio specifications for timing requirements

### Serial PTT Not Working

**Check:**
1. Serial port path is correct (`/dev/ttyUSB0`, `/dev/ttyACM0`, etc.)
2. Baud rate matches radio
3. Radio is powered on
4. Cables are connected properly
5. User has permissions for serial port (add to `dialout` group)

## Best Practices

1. **Use Appropriate Delays**: Set TX delay and hang time based on your radio's specifications
2. **Test Before Deployment**: Always test PTT functionality before using in production
3. **Monitor PTT State**: Add logging to track PTT activation/deactivation
4. **Handle Errors**: Implement error handling for GPIO and serial operations
5. **Clean Up Resources**: Always clean up GPIO on exit
6. **Document Configuration**: Record GPIO pin numbers and timing values

## Safety Considerations

1. **RF Exposure**: Ensure proper RF safety practices when transmitting
2. **Power Levels**: Start with low power when testing
3. **Antenna**: Never transmit without proper antenna connected
4. **Licensing**: Ensure you have proper licenses for the frequencies used
5. **Regulations**: Comply with local amateur radio regulations

## Additional Resources

- [Raspberry Pi GPIO Documentation](https://www.raspberrypi.org/documentation/usage/gpio/)
- [PySerial Documentation](https://pyserial.readthedocs.io/)
- [GNU Radio Embedded Python Blocks](https://wiki.gnuradio.org/index.php/Embedded_Python_Block)
- [Flowgraph Documentation](../flowgraphs/FLOWGRAPHS.md)

