# PTT (Push-To-Talk) Control with gr-osmosdr

This document explains how to control PTT (Push-To-Talk) functionality when using gr-qradiolink blocks with gr-osmosdr and similar SDR hardware.

## Overview

PTT control is essential for half-duplex radio operation, allowing you to switch between transmit and receive modes. The method for controlling PTT depends on your SDR hardware and how it's integrated with your flowgraph.

## Supported Hardware

Different SDR devices support PTT control in different ways:

- **HackRF One**: GPIO pins or software control
- **LimeSDR**: GPIO pins or software control
- **USRP (Ettus)**: GPIO pins or software control
- **ADALM-Pluto**: GPIO pins or software control
- **bladeRF**: GPIO pins or software control
- **RTL-SDR**: Receive-only (no PTT support)

## Methods for PTT Control

### Method 1: Software TX Enable (Recommended)

The `osmosdr.sink` block provides a `set_tx_enable()` method that can be called programmatically:

```python
import osmosdr

# Create osmosdr sink block
osmosdr_sink = osmosdr.sink(
    args="hackrf=0",
    channels=[(0, 0)],
)

# Enable transmit (PTT on)
osmosdr_sink.set_tx_enable(True)

# Disable transmit (PTT off)
osmosdr_sink.set_tx_enable(False)
```

### Method 2: Direct Serial/GPIO Control (Low Latency)

Most GNU Radio implementations handle PTT directly through serial port RTS/DTR lines or GPIO pins, bypassing Hamlib entirely. This is simpler and has lower latency than software-based control methods.

#### Serial Port RTS/DTR Control

Many radios and transverters can be controlled via serial port RTS (Request To Send) or DTR (Data Terminal Ready) lines. This provides hardware-level PTT control with minimal latency:

```python
import serial
import time

class SerialPTT:
    """Direct serial port PTT control using RTS/DTR lines"""
    def __init__(self, port='/dev/ttyUSB0', use_rts=True):
        self.serial = serial.Serial(port, timeout=1)
        self.use_rts = use_rts
        
    def enable_tx(self):
        """Enable transmit (PTT on)"""
        if self.use_rts:
            self.serial.setRTS(True)  # RTS high = PTT on
        else:
            self.serial.setDTR(True)  # DTR high = PTT on
        print("PTT enabled via serial")
    
    def disable_tx(self):
        """Disable transmit (PTT off)"""
        if self.use_rts:
            self.serial.setRTS(False)  # RTS low = PTT off
        else:
            self.serial.setDTR(False)  # DTR low = PTT off
        print("PTT disabled via serial")
    
    def close(self):
        self.serial.close()

# Usage example
ptt = SerialPTT(port='/dev/ttyUSB0', use_rts=True)
ptt.enable_tx()
time.sleep(5)  # Transmit for 5 seconds
ptt.disable_tx()
ptt.close()
```

**Advantages of Serial Port Control:**
- **Low latency**: Hardware-level control, typically < 1ms response time
- **Simple**: No additional libraries or drivers needed (just pyserial)
- **Reliable**: Direct hardware control, no software abstraction layer
- **Widely supported**: Most radios with serial interfaces support RTS/DTR control

**Common Serial Port Configurations:**
- **RTS (Request To Send)**: Most common, active high (True = PTT on)
- **DTR (Data Terminal Ready)**: Alternative, active high (True = PTT on)
- Some radios may use inverted logic (active low)

#### Direct GPIO Pin Control

For SBCs (Single Board Computers) like Raspberry Pi, BeagleBone, or devices with accessible GPIO pins:

```python
import RPi.GPIO as GPIO  # For Raspberry Pi
import time

class GPIOPTT:
    """Direct GPIO pin control for PTT"""
    def __init__(self, pin=18):
        self.pin = pin
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.pin, GPIO.OUT)
        GPIO.output(self.pin, GPIO.LOW)  # Start with PTT off
        
    def enable_tx(self):
        """Enable transmit (PTT on)"""
        GPIO.output(self.pin, GPIO.HIGH)
        print("PTT enabled via GPIO")
    
    def disable_tx(self):
        """Disable transmit (PTT off)"""
        GPIO.output(self.pin, GPIO.LOW)
        print("PTT disabled via GPIO")
    
    def cleanup(self):
        GPIO.cleanup(self.pin)

# Usage example
ptt = GPIOPTT(pin=18)
ptt.enable_tx()
time.sleep(5)
ptt.disable_tx()
ptt.cleanup()
```

**GPIO Control with SDR Devices:**

Some SDR devices also support direct GPIO control through their API:

```python
import osmosdr

osmosdr_sink = osmosdr.sink(
    args="hackrf=0",
    channels=[(0, 0)],
)

# Direct GPIO pin control (device-specific)
# Pin number and behavior vary by device
osmosdr_sink.set_gpio_pin(0, True)   # PTT on
osmosdr_sink.set_gpio_pin(0, False)  # PTT off
```

**Advantages of Direct GPIO Control:**
- **Lowest latency**: Direct hardware access, typically < 100μs
- **No dependencies**: Works with standard GPIO libraries
- **Flexible**: Can control multiple pins for complex radio interfaces
- **Reliable**: Hardware-level control with predictable timing

### Method 3: GPIO Pin Control (SDR Device)

For SDR devices that support hardware GPIO control through their API:

```python
import osmosdr

osmosdr_sink = osmosdr.sink(
    args="hackrf=0",
    channels=[(0, 0)],
)

# Set GPIO pin for PTT control
# Pin number and behavior vary by device
osmosdr_sink.set_gpio_pin(0, True)   # PTT on
osmosdr_sink.set_gpio_pin(0, False)  # PTT off
```

### Method 4: Tag-Based Control

Use GNU Radio tags to control PTT based on stream data:

```python
import pmt
from gnuradio import gr

# Create a tag for TX enable
tx_enable_tag = pmt.intern("tx_enable")

# In your work function or block:
# Add tag to stream when you want to transmit
self.add_item_tag(0, offset, tx_enable_tag, pmt.PMT_T)
```

### Method 5: Message Port Control

Use message ports for asynchronous PTT control:

```python
# Create a message port
self.message_port_register_in(pmt.intern("ptt_control"))
self.set_msg_handler(pmt.intern("ptt_control"), self.handle_ptt_msg)

def handle_ptt_msg(self, msg):
    enable = pmt.to_bool(msg)
    self.osmosdr_sink.set_tx_enable(enable)
```

## Integration with gr-qradiolink

### Direct Serial/GPIO PTT Integration Example

Here's a complete example showing how to integrate direct serial port PTT control with gr-qradiolink blocks:

```python
#!/usr/bin/env python3
from gnuradio import gr, audio, blocks
from gnuradio import qradiolink
import osmosdr
import serial
import time
import threading

class SerialPTTControl:
    """Thread-safe serial port PTT control"""
    def __init__(self, port='/dev/ttyUSB0', use_rts=True):
        self.serial = serial.Serial(port, timeout=1)
        self.use_rts = use_rts
        self.lock = threading.Lock()
        
    def enable_tx(self):
        with self.lock:
            if self.use_rts:
                self.serial.setRTS(True)
            else:
                self.serial.setDTR(True)
    
    def disable_tx(self):
        with self.lock:
            if self.use_rts:
                self.serial.setRTS(False)
            else:
                self.serial.setDTR(False)
    
    def close(self):
        self.serial.close()

class PTTTransmitterWithSerial(gr.top_block):
    def __init__(self):
        gr.top_block.__init__(self, "PTT Transmitter with Serial Control")
        
        # Serial PTT control
        self.ptt = SerialPTTControl(port='/dev/ttyUSB0', use_rts=True)
        
        # Audio source
        self.audio_source = audio.source(32000, "hw:0,0")
        
        # NBFM modulator
        self.mod = qradiolink.mod_nbfm(
            sps=125,
            samp_rate=250000,
            carrier_freq=1700,
            filter_width=8000
        )
        
        # osmosdr sink
        self.osmosdr_sink = osmosdr.sink(
            args="hackrf=0",
            channels=[(0, 0)],
        )
        self.osmosdr_sink.set_sample_rate(250000)
        self.osmosdr_sink.set_center_freq(145000000)
        self.osmosdr_sink.set_tx_gain(40)
        
        # Connect blocks
        self.connect(self.audio_source, self.mod, self.osmosdr_sink)
        
    def enable_tx(self):
        """Enable transmit with hardware PTT"""
        self.ptt.enable_tx()  # Hardware PTT via serial
        self.osmosdr_sink.set_tx_enable(True)  # Software TX enable
        print("TX enabled (PTT on)")
    
    def disable_tx(self):
        """Disable transmit"""
        self.osmosdr_sink.set_tx_enable(False)  # Software TX disable first
        time.sleep(0.01)  # Small delay for clean shutdown
        self.ptt.disable_tx()  # Hardware PTT off
        print("TX disabled (PTT off)")
    
    def cleanup(self):
        self.ptt.close()

if __name__ == '__main__':
    tb = PTTTransmitterWithSerial()
    tb.start()
    
    try:
        # Transmit for 5 seconds
        tb.enable_tx()
        time.sleep(5)
        tb.disable_tx()
        
        input("Press Enter to exit...")
    except KeyboardInterrupt:
        pass
    finally:
        tb.disable_tx()  # Ensure PTT is off
        tb.cleanup()
        tb.stop()
        tb.wait()
```

## Integration with gr-qradiolink

### Basic Transmit Flowgraph

A typical transmit flowgraph using gr-qradiolink blocks:

```
[Audio Source] -> [qradiolink.mod_nbfm] -> [osmosdr.sink]
                                              |
                                    [PTT Control]
```

### Python Example: Manual PTT Control

```python
#!/usr/bin/env python3
from gnuradio import gr, audio, blocks
from gnuradio import qradiolink
import osmosdr
import time

class PTTTransmitter(gr.top_block):
    def __init__(self):
        gr.top_block.__init__(self, "PTT Transmitter Example")
        
        # Audio source
        self.audio_source = audio.source(32000, "hw:0,0")
        
        # NBFM modulator
        self.mod = qradiolink.mod_nbfm(
            sps=125,
            samp_rate=250000,
            carrier_freq=1700,
            filter_width=8000
        )
        
        # osmosdr sink
        self.osmosdr_sink = osmosdr.sink(
            args="hackrf=0",
            channels=[(0, 0)],
        )
        self.osmosdr_sink.set_sample_rate(250000)
        self.osmosdr_sink.set_center_freq(145000000)  # 145 MHz
        self.osmosdr_sink.set_tx_gain(40)
        
        # Connect blocks
        self.connect(self.audio_source, self.mod, self.osmosdr_sink)
        
    def enable_tx(self):
        """Enable transmit (PTT on)"""
        self.osmosdr_sink.set_tx_enable(True)
        print("TX enabled (PTT on)")
    
    def disable_tx(self):
        """Disable transmit (PTT off)"""
        self.osmosdr_sink.set_tx_enable(False)
        print("TX disabled (PTT off)")

if __name__ == '__main__':
    tb = PTTTransmitter()
    tb.start()
    
    try:
        # Transmit for 5 seconds
        tb.enable_tx()
        time.sleep(5)
        tb.disable_tx()
        
        # Keep running
        input("Press Enter to exit...")
    except KeyboardInterrupt:
        pass
    finally:
        tb.stop()
        tb.wait()
```

### Python Example: VOX (Voice Operated Transmit)

```python
#!/usr/bin/env python3
from gnuradio import gr, audio, blocks
from gnuradio import qradiolink
import osmosdr
import numpy as np

class VOXTransmitter(gr.top_block):
    def __init__(self):
        gr.top_block.__init__(self, "VOX Transmitter Example")
        
        # Audio source
        self.audio_source = audio.source(32000, "hw:0,0")
        
        # Level detector for VOX
        self.level_detector = blocks.complex_to_mag_squared(1)
        self.moving_average = blocks.moving_average_ff(3200, 1.0/3200, 4000)
        self.threshold = blocks.threshold_ff(0.01, 0.01, 0)
        
        # NBFM modulator
        self.mod = qradiolink.mod_nbfm(
            sps=125,
            samp_rate=250000,
            carrier_freq=1700,
            filter_width=8000
        )
        
        # osmosdr sink
        self.osmosdr_sink = osmosdr.sink(
            args="hackrf=0",
            channels=[(0, 0)],
        )
        self.osmosdr_sink.set_sample_rate(250000)
        self.osmosdr_sink.set_center_freq(145000000)
        self.osmosdr_sink.set_tx_gain(40)
        
        # Connect blocks
        self.connect(self.audio_source, self.mod, self.osmosdr_sink)
        
        # VOX detection path
        self.connect(self.audio_source, self.level_detector, 
                    self.moving_average, self.threshold)
        
        # Monitor threshold for PTT control
        self.ptt_control = PTTControlBlock(self.osmosdr_sink)
        self.connect(self.threshold, self.ptt_control)

class PTTControlBlock(gr.sync_block):
    """Custom block to control PTT based on audio level"""
    def __init__(self, osmosdr_sink):
        gr.sync_block.__init__(
            self,
            name="PTT Control",
            in_sig=[np.float32],
            out_sig=None
        )
        self.osmosdr_sink = osmosdr_sink
        self.tx_enabled = False
        
    def work(self, input_items, output_items):
        level = input_items[0]
        
        # Enable TX if level exceeds threshold
        if np.any(level > 0.5) and not self.tx_enabled:
            self.osmosdr_sink.set_tx_enable(True)
            self.tx_enabled = True
            print("VOX: TX enabled")
        
        # Disable TX if level drops below threshold
        elif np.all(level < 0.3) and self.tx_enabled:
            self.osmosdr_sink.set_tx_enable(False)
            self.tx_enabled = False
            print("VOX: TX disabled")
        
        return len(input_items[0])

if __name__ == '__main__':
    tb = VOXTransmitter()
    tb.start()
    
    try:
        input("Press Enter to exit...")
    except KeyboardInterrupt:
        pass
    finally:
        tb.stop()
        tb.wait()
```

## Device-Specific Notes

### HackRF One

```python
osmosdr_sink = osmosdr.sink(
    args="hackrf=0",
    channels=[(0, 0)],
)

# Software control
osmosdr_sink.set_tx_enable(True)

# GPIO control (if using external PTT)
# HackRF has GPIO pins that can be controlled
```

### LimeSDR

```python
osmosdr_sink = osmosdr.sink(
    args="soapy=0",
    channels=[(0, 0)],
)

# Software control
osmosdr_sink.set_tx_enable(True)

# GPIO control via SoapySDR
# LimeSDR supports GPIO pins for PTT
```

### ADALM-Pluto

```python
osmosdr_sink = osmosdr.sink(
    args="pluto=192.168.2.1",
    channels=[(0, 0)],
)

# Software control
osmosdr_sink.set_tx_enable(True)

# GPIO control
# ADALM-Pluto has GPIO pins that can be used for PTT
```

### USRP Devices

```python
osmosdr_sink = osmosdr.sink(
    args="uhd=0",
    channels=[(0, 0)],
)

# Software control
osmosdr_sink.set_tx_enable(True)

# GPIO control
# USRP devices have GPIO banks for PTT control
osmosdr_sink.set_gpio_pin(0, True)  # Set GPIO pin 0
```

## Best Practices

1. **Always disable TX when not transmitting**: This prevents accidental transmission and reduces interference.

2. **Use appropriate delays**: Some devices need time to switch between RX and TX modes. Add small delays if needed:
   ```python
   osmosdr_sink.set_tx_enable(True)
   time.sleep(0.01)  # 10ms delay for hardware to settle
   ```

3. **Prefer direct serial/GPIO control for low latency**: For applications requiring fast PTT response (e.g., digital modes, repeater control), use direct serial port RTS/DTR or GPIO pin control instead of software-based methods. This bypasses abstraction layers and provides sub-millisecond response times.

4. **Monitor TX power**: Use appropriate TX gain settings to avoid overdriving amplifiers or violating regulations.

5. **Implement safety timeouts**: Add automatic TX timeout to prevent stuck transmissions:
   ```python
   def transmit_with_timeout(self, duration):
       self.enable_tx()
       time.sleep(duration)
       self.disable_tx()
   ```

6. **Check device capabilities**: Not all devices support all PTT methods. Check your device documentation.

7. **Use thread-safe PTT control**: When using direct serial/GPIO control from multiple threads, use locks to prevent race conditions:
   ```python
   import threading
   
   class ThreadSafePTT:
       def __init__(self):
           self.lock = threading.Lock()
           # Initialize serial/GPIO
       
       def enable_tx(self):
           with self.lock:
               # PTT control code
   ```

8. **Verify serial port permissions**: Ensure your user has access to serial ports (may need to add user to `dialout` group on Linux):
   ```bash
   sudo usermod -a -G dialout $USER
   ```

## Troubleshooting

### TX Enable Not Working

- Verify your device supports transmit mode
- Check that you're using the correct device argument string
- Ensure proper permissions (may need to run as root or add udev rules)

### GPIO Control Not Working

- Verify GPIO pin numbers for your specific device
- Check device documentation for GPIO pin mapping
- Some devices require additional configuration for GPIO access
- For Raspberry Pi, ensure you're using the correct GPIO library (RPi.GPIO) and have proper permissions
- For serial port control, verify the port exists and you have read/write permissions:
  ```bash
  ls -l /dev/ttyUSB0  # Check permissions
  sudo chmod 666 /dev/ttyUSB0  # If needed (or add user to dialout group)
  ```

### Serial Port Control Not Working

- Verify the serial port device exists: `ls -l /dev/ttyUSB*` or `ls -l /dev/ttyACM*`
- Check permissions: Ensure your user has access (add to `dialout` group)
- Verify RTS/DTR polarity: Some radios use inverted logic (active low)
- Test with a serial terminal first: `minicom` or `screen /dev/ttyUSB0 9600`
- Check if the port is already in use: `lsof /dev/ttyUSB0`

### Audio Level Detection Issues

- Adjust threshold values for your audio input
- Use appropriate sample rates and buffer sizes
- Consider using AGC (Automatic Gain Control) before level detection

## Additional Resources

- **gr-osmosdr Documentation**: https://github.com/osmocom/gr-osmosdr
- **GNU Radio Wiki**: https://wiki.gnuradio.org/
- **Device-Specific Documentation**: Check your SDR device manufacturer's documentation
- **SoapySDR Documentation**: https://github.com/pothosware/SoapySDR (for SoapySDR-based devices)

## Legal and Safety Considerations

⚠️ **IMPORTANT**: Before transmitting:

1. **Verify your license**: Ensure you have appropriate amateur radio or experimental licenses
2. **Check frequency allocations**: Only transmit on frequencies you're authorized to use
3. **Set appropriate power levels**: Start with low power and verify operation
4. **Follow local regulations**: Comply with all local RF regulations
5. **Use appropriate antennas**: Ensure your antenna is suitable for the frequency and power level

Always test in a safe, controlled environment before operating on-air.

