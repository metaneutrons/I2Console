// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 Metaneutrons
/**
 * @file main.c
 * @brief I2Console ESP-IDF Example
 *
 * Creates an I2C master bus, hands it to the I2Console component and lets the
 * component mirror ESP_LOG output to the device.
 *
 * The bus is created here rather than inside the component on purpose: a real
 * board usually has several devices on the same bus, and whoever owns the bus
 * should be the application.
 */

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2console.h"

#include <stdio.h>

static const char *TAG = "example";

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_PORT   I2C_NUM_0

static i2c_master_bus_handle_t bus_init(void)
{
    const i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus = NULL;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));
    ESP_LOGI(TAG, "I2C master bus up on SDA=%d SCL=%d", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    return bus;
}

void app_main(void)
{
    ESP_LOGI(TAG, "I2Console example starting");

    i2c_master_bus_handle_t bus = bus_init();

    esp_err_t ret = i2console_init(bus, I2CONSOLE_DEFAULT_ADDR);
    if (ret == ESP_OK) {
        char version[16];
        if (i2console_get_version(version) == ESP_OK) {
            ESP_LOGI(TAG, "I2Console firmware: %s", version);
        }
        ESP_LOGI(TAG, "ESP_LOG output is now mirrored to I2Console");
    } else {
        // Not finding the device is a normal outcome, not a failure of the
        // application: logging carries on over UART alone.
        ESP_LOGW(TAG, "I2Console not available (%s), continuing with UART only",
                 esp_err_to_name(ret));
    }

    int counter = 0;
    while (1) {
        ESP_LOGI(TAG, "Counter: %d", counter);
        ESP_LOGD(TAG, "Debug message %d", counter);

        if (counter % 10 == 0) {
            ESP_LOGW(TAG, "Warning at count %d", counter);
        }
        if (counter % 20 == 0) {
            ESP_LOGE(TAG, "Error simulation at count %d", counter);
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
