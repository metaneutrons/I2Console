# I2Console ESP-IDF Component

I2C to USB-CDC console bridge driver for ESP-IDF. Automatically mirrors ESP_LOG output to I2Console device.

## Features

- **Auto-detection**: Probes I2C bus for I2Console device (ID: 0x12C0)
- **Automatic logging**: Hooks into ESP-IDF logging system
- **Non-blocking**: Queue-based writes
- **Graceful fallback**: Continues with UART if device not found
- **Version query**: Read I2Console firmware version

## Requirements

ESP-IDF **5.2 or newer**. The component uses the `i2c_master` driver
(`driver/i2c_master.h`), which arrived in 5.2; it will not build against the
legacy `driver/i2c.h` API.

## Installation

Copy this directory into your project's `components/` folder. It is not
published to the ESP Component Registry, so `idf.py add-dependency` will not
resolve it; see the note at the end.

## Quick Start

The component does **not** create the I2C bus. A board usually has more than
one device on it, so the bus belongs to the application and is handed in.

```c
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "i2console.h"

void app_main(void)
{
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

    if (i2console_init(bus, I2CONSOLE_DEFAULT_ADDR) == ESP_OK) {
        ESP_LOGI("APP", "Hello I2Console!");   // mirrored to the device
    }
    // Not finding the device is a normal outcome; logging carries on over UART.
}
```

## Hardware Setup

Connect I2Console device (RP2350-GEEK with I2Console firmware) to ESP32:

```
ESP32 GPIO21 (SDA) → I2Console GPIO28 (SDA)
ESP32 GPIO22 (SCL) → I2Console GPIO29 (SCL)
GND                → GND
```

## API Reference

### `i2console_init()`
```c
esp_err_t i2console_init(i2c_master_bus_handle_t bus, uint8_t addr);
```
Adds the device to a bus the caller already created, probes it, and on success
mirrors `ESP_LOG` output to it in addition to the usual UART.

- `bus` — an initialised I2C master bus handle
- `addr` — I2C slave address, `I2CONSOLE_DEFAULT_ADDR` is `0x37`

Returns `ESP_OK`, `ESP_ERR_INVALID_ARG` for a NULL bus,
`ESP_ERR_INVALID_STATE` if already initialised, `ESP_ERR_NOT_FOUND` if nothing
answers at that address, or `ESP_ERR_NO_MEM`. Every failure after the device
was added removes it again, so a failed attempt leaves the bus as it found it.

### `i2console_deinit()`
```c
esp_err_t i2console_deinit(void);
```
Restores the default log output, stops the transmit task and removes the device
from the bus. The bus itself belongs to the caller and is untouched.

### `i2console_write()`
```c
esp_err_t i2console_write(const char *data, size_t len);
```
Write data directly, bypassing `ESP_LOG`.

### `i2console_is_connected()`
```c
bool i2console_is_connected(void);
```

### `i2console_get_version()`
```c
esp_err_t i2console_get_version(char *version);
```
Reads the I2Console firmware version. The buffer must be at least 16 bytes.

## Configuration

Via `idf.py menuconfig` → Component config → I2Console Configuration:
- Enable/disable component
- Change I2C address

## Example

See [example](../../) directory for complete working example.

## License

LGPL-3.0-or-later. The firmware in this repository is GPL-3.0-or-later;
this component is a library meant to be linked into third-party firmware,
so it carries the lesser licence and linking it does not place your
application under the GPL. See LICENSE in this directory.

## Component registry

The release pipeline publishes this component to the ESP Component Registry as
`metaneutrons/i2console`, sharing its version with the firmware. The channel is
optional and activates when the `IDF_COMPONENT_API_TOKEN` secret is present in
the repository's `release` environment; without it the release skips the
channel and says so.

Before anything is published, the release preflight checks that the token is
accepted, that the version is not already in the registry, and that the
component packs and validates through `compote component upload --dry-run`.
After publishing, it reads the version back from the public registry API before
the release is promoted to `latest`.

## Links

- [I2Console Firmware](https://github.com/metaneutrons/I2Console)
- [Documentation](https://github.com/metaneutrons/I2Console/blob/main/README.md)
- [Issues](https://github.com/metaneutrons/I2Console/issues)
