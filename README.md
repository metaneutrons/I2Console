# I2Console

Enterprise-grade I2C to USB-CDC console bridge for RP2350-GEEK

## Overview

I2Console provides a robust console interface over I2C when UART is not available. It bridges I2C slave communication to USB-CDC, enabling console access through standard USB serial terminals.

## Features

- **I2C Slave Interface**: Configurable address (default 0x37) on GPIO28 (SDA) and GPIO29 (SCL)
- **USB-CDC Device**: Standard USB serial port for console I/O
- **Dual Buffers**: 256-byte TX buffer (I2C→USB) and 1024-byte RX buffer (USB→I2C)
- **Visual Display**: 1.14" LCD with real-time statistics and status
- **Flash Persistence**: Configuration stored in flash memory
- **Watchdog Timer**: Automatic recovery from hangs
- **USB Firmware Update**: Compatible with picotool
- **Drop-Oldest Policy**: Prevents buffer deadlocks

## Hardware Requirements

- Waveshare RP2350-GEEK board
- USB connection for power and CDC interface
- I2C master device

## I2C Register Map

| Register | Access | Description |
|----------|--------|-------------|
| 0x00-0x01 | R | Device ID (0x12C0) |
| 0x01 | R | Firmware version |
| 0x02 | R/W | I2C address configuration |
| 0x03 | R/W | Clock stretching enable (bit 0) |
| 0x10 | R | TX buffer available bytes (low) |
| 0x11 | R | TX buffer available bytes (high) |
| 0x12 | R | RX buffer available bytes |
| 0x20+ | R/W | Data read/write operations |

## Usage

### Identifying the Device

```python
# Read device ID to verify I2Console presence
device_id = i2c.read_word(0x37, 0x00)  # Should return 0x12C0
```

### Writing to Console (I2C → USB-CDC)

```python
# Write data to be sent to USB-CDC
data = b"Hello Console\n"
i2c.write_byte(0x37, 0x20)  # Set register to data area
i2c.write_block(0x37, data)
```

### Reading from Console (USB-CDC → I2C)

```python
# Check available bytes
avail = i2c.read_byte(0x37, 0x12)
if avail > 0:
    # Read data
    i2c.write_byte(0x37, 0x20)  # Set register to data area
    data = i2c.read_block(0x37, min(avail, 256))
```

### Changing I2C Address

```python
# Change address to 0x40 (persisted in flash)
i2c.write_byte(0x37, 0x02, 0x40)
# Device will now respond at 0x40 after reboot
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Flashing

### Using picotool

```bash
# Hold BOOTSEL button while connecting USB
picotool load I2Console.uf2
picotool reboot
```

### Using drag-and-drop

1. Hold BOOTSEL button while connecting USB
2. Drag `I2Console.uf2` to RPI-RP2 drive
3. Device will reboot automatically

## LCD Display

The LCD shows real-time information:
- **I2Console**: Branding
- **I2C Addr**: Current I2C address
- **TX Buf**: TX buffer usage (turns red when >200 bytes)
- **RX Buf**: RX buffer usage (turns red when >900 bytes)
- **TX/RX**: Total bytes transferred
- **USB**: Connection status (CONN/DISC)
- **Errors**: Total error count

## Error Handling

- **Buffer Overflow**: Oldest data is dropped automatically
- **USB Disconnect**: Data is discarded, no errors generated
- **Watchdog**: System resets after 8 seconds of inactivity
- **I2C Errors**: Tracked and displayed on LCD

## Configuration

All configuration is stored in flash and persists across reboots:
- I2C slave address
- Clock stretching enable/disable

## License

GPL-3.0 - See LICENSE file for details

## Author

Metaneutrons

## Support

For issues and feature requests, visit: https://github.com/metaneutrons/I2Console
