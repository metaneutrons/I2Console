// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Metaneutrons
/**
 * @file i2console.h
 * @brief I2Console ESP-IDF Component
 *
 * I2C to USB-CDC console bridge driver for ESP-IDF.
 * Automatically detects I2Console device and routes ESP_LOG output to it.
 */

#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2CONSOLE_DEVICE_ID    0x12C0
#define I2CONSOLE_DEFAULT_ADDR 0x37

/**
 * @brief Initialize I2Console component
 *
 * Adds the I2Console as a device on an I2C bus the caller has already created,
 * probes it, and on success routes ESP_LOG output to it in addition to the
 * usual UART.
 *
 * The bus is a parameter rather than something this component creates, because
 * a board almost always has other devices on the same bus and the owner of the
 * bus is the application. Nothing is freed on failure that this function did
 * not allocate.
 *
 * @param bus  An initialised I2C master bus handle
 * @param addr I2C slave address (default: I2CONSOLE_DEFAULT_ADDR)
 * @return ESP_OK on success,
 *         ESP_ERR_INVALID_ARG if bus is NULL,
 *         ESP_ERR_INVALID_STATE if already initialised,
 *         ESP_ERR_NOT_FOUND if no I2Console answers at that address
 */
esp_err_t i2console_init(i2c_master_bus_handle_t bus, uint8_t addr);

/**
 * @brief Release the I2Console
 *
 * Restores the default log output, stops the transmit task and removes the
 * device from the bus. The bus itself belongs to the caller and is untouched.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if not initialised
 */
esp_err_t i2console_deinit(void);

/**
 * @brief Write data to I2Console
 *
 * @param data Data buffer
 * @param len Data length
 * @return ESP_OK on success
 */
esp_err_t i2console_write(const char *data, size_t len);

/**
 * @brief Check if I2Console is connected
 *
 * @return true if device detected and operational
 */
bool i2console_is_connected(void);

/**
 * @brief Get I2Console firmware version
 *
 * @param version Buffer to store version string (min 16 bytes)
 * @return ESP_OK on success
 */
esp_err_t i2console_get_version(char *version);

#ifdef __cplusplus
}
#endif
