# I2Console ESP-IDF Example

Example demonstrating I2Console integration with ESP-IDF 5.2 or newer.

## Features

- **Auto-detection**: Automatically detects I2Console on I2C bus
- **Log mirroring**: All `ESP_LOG` output automatically sent to I2Console
- **Non-blocking**: Queue-based writes won't block your application
- **Graceful fallback**: If I2Console not found, continues with UART only

## Hardware Setup

1. **I2Console Device** (RP2350-GEEK running I2Console firmware)
   - Connect to ESP32 I2C bus
   - Default address: 0x37

2. **ESP32 Connections**:
   ```
   ESP32 GPIO21 (SDA) → I2Console GPIO28 (SDA)
   ESP32 GPIO22 (SCL) → I2Console GPIO29 (SCL)
   GND                → GND
   ```

3. **USB Connection**:
   - Connect I2Console to PC via USB
   - Open serial terminal to see mirrored logs

## Building

```bash
# Set ESP-IDF environment
. $HOME/esp/esp-idf/export.sh

# Configure (optional - defaults work)
idf.py menuconfig

# Build
idf.py build

# Flash
idf.py -p /dev/ttyUSB0 flash monitor
```

## Configuration

Via `idf.py menuconfig`:
- **Component config → I2Console Configuration**
  - Enable/disable I2Console
  - Change I2C address (default: 0x37)

## Usage in Your Project

1. Copy `components/i2console/` to your project's `components/` directory

2. Create the I2C bus and hand it to the component:
   ```c
   #include "driver/i2c_master.h"
   #include "esp_log.h"
   #include "i2console.h"

   const i2c_master_bus_config_t bus_cfg = {
       .clk_source = I2C_CLK_SRC_DEFAULT,
       .i2c_port = I2C_NUM_0,
       .scl_io_num = 22,
       .sda_io_num = 21,
       .glitch_ignore_cnt = 7,
       .flags.enable_internal_pullup = true,
   };

   i2c_master_bus_handle_t bus = NULL;
   ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

   i2console_init(bus, I2CONSOLE_DEFAULT_ADDR);

   ESP_LOGI("APP", "Hello I2Console!");   // mirrored to the device
   ```

   The component does not create the bus, because a board usually has other
   devices on it and the bus belongs to the application.

3. That's it! All logging automatically goes to both UART and I2Console.

## API Reference

### `i2console_init()`
```c
esp_err_t i2console_init(i2c_master_bus_handle_t bus, uint8_t addr);
```
Adds the device to a bus the caller created. Returns `ESP_ERR_NOT_FOUND` if
nothing answers at that address.

### `i2console_write()`
```c
esp_err_t i2console_write(const char *data, size_t len);
```
Manually write data to I2Console (bypasses ESP_LOG).

### `i2console_is_connected()`
```c
bool i2console_is_connected(void);
```
Check if I2Console device is connected and operational.

### `i2console_get_version()`
```c
esp_err_t i2console_get_version(char *version);
```
Get I2Console firmware version string.

## Troubleshooting

**I2Console not detected:**
- Check I2C connections (SDA, SCL, GND)
- Verify I2Console is powered and running
- Check I2C address matches (default: 0x37)
- Use `i2cdetect` to scan bus

**Logs not appearing:**
- Verify USB connection to I2Console
- Open serial terminal to I2Console USB port
- Check ESP_LOG level in menuconfig

## License

The example application in `main/` is GPL-3.0-or-later, like the rest of the
repository. The component under `components/i2console/` is LGPL-3.0-or-later and
carries its own LICENSE file.
