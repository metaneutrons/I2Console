#include "usb_cdc.h"
#include "tusb.h"
#include "pico/bootrom.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <string.h>

#define UART_BRIDGE_INST uart1
#define UART_BRIDGE_DEFAULT_BAUD 115200

static uint8_t cmd_buffer[64];
static int cmd_len = 0;
static bool last_dtr_state = false;

static uart_bridge_stats_t bridge_stats = {
    .baud_rate = UART_BRIDGE_DEFAULT_BAUD,
    .data_bits = 8,
    .stop_bits = 1,
    .parity = 0,
};

void usb_cdc_init(void) {
    tusb_init();
}

void usb_cdc_task(void) {
    tud_task();
}

bool usb_cdc_connected(void) {
    return tud_cdc_n_connected(CDC_ITF_DATA);
}

int usb_cdc_read(uint8_t *buffer, int len) {
    if (!tud_cdc_n_available(CDC_ITF_DATA)) return 0;
    return tud_cdc_n_read(CDC_ITF_DATA, buffer, len);
}

int usb_cdc_write(const uint8_t *buffer, int len) {
    if (!tud_cdc_n_connected(CDC_ITF_DATA)) return 0;
    return tud_cdc_n_write(CDC_ITF_DATA, buffer, len);
}

void usb_cdc_check_bootloader_cmd(void) {
    if (tud_cdc_n_connected(CDC_ITF_DEBUG)) {
        bool dtr = tud_cdc_n_get_line_state(CDC_ITF_DEBUG) & 0x01;
        if (last_dtr_state && !dtr) {
            reset_usb_boot(0, 0);
        }
        last_dtr_state = dtr;
    }

    if (!tud_cdc_n_available(CDC_ITF_DEBUG)) return;

    while (tud_cdc_n_available(CDC_ITF_DEBUG) && cmd_len < (int)sizeof(cmd_buffer) - 1) {
        uint8_t c;
        tud_cdc_n_read(CDC_ITF_DEBUG, &c, 1);

        if (c == '\n' || c == '\r') {
            if (cmd_len > 0) {
                cmd_buffer[cmd_len] = '\0';
                if (strcmp((char *)cmd_buffer, "BOOTLOADER") == 0 ||
                    strcmp((char *)cmd_buffer, "bootloader") == 0 ||
                    strcmp((char *)cmd_buffer, "reboot") == 0) {
                    tud_cdc_n_write_str(CDC_ITF_DEBUG, "Entering bootloader mode...\n");
                    tud_cdc_n_write_flush(CDC_ITF_DEBUG);
                    sleep_ms(100);
                    reset_usb_boot(0, 0);
                }
                cmd_len = 0;
            }
        } else {
            cmd_buffer[cmd_len++] = c;
        }
    }

    if (cmd_len >= (int)sizeof(cmd_buffer) - 1) cmd_len = 0;
}

// --- UART Bridge ---

void uart_bridge_init(void) {
    uart_init(UART_BRIDGE_INST, UART_BRIDGE_DEFAULT_BAUD);
    gpio_set_function(UART_BRIDGE_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_BRIDGE_RX_PIN, GPIO_FUNC_UART);
}

void uart_bridge_task(void) {
    bridge_stats.connected = tud_cdc_n_connected(CDC_ITF_UART);

    // CDC → UART
    if (tud_cdc_n_available(CDC_ITF_UART)) {
        uint8_t buf[64];
        uint32_t count = tud_cdc_n_read(CDC_ITF_UART, buf, sizeof(buf));
        for (uint32_t i = 0; i < count; i++) {
            uart_putc_raw(UART_BRIDGE_INST, buf[i]);
        }
        bridge_stats.rx_bytes += count;
    }

    // UART → CDC
    if (bridge_stats.connected) {
        int count = 0;
        while (uart_is_readable(UART_BRIDGE_INST) && count < 64) {
            uint8_t c = uart_getc(UART_BRIDGE_INST);
            tud_cdc_n_write(CDC_ITF_UART, &c, 1);
            count++;
        }
        if (count > 0) {
            tud_cdc_n_write_flush(CDC_ITF_UART);
            bridge_stats.tx_bytes += count;
        }
    } else {
        // Drain UART when USB not connected
        while (uart_is_readable(UART_BRIDGE_INST)) {
            uart_getc(UART_BRIDGE_INST);
        }
    }
}

uart_bridge_stats_t uart_bridge_get_stats(void) {
    return bridge_stats;
}

// TinyUSB line coding callback – update UART when host changes baud/format
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *p_line_coding) {
    if (itf != CDC_ITF_UART) return;

    bridge_stats.baud_rate = p_line_coding->bit_rate;
    bridge_stats.data_bits = p_line_coding->data_bits;
    bridge_stats.stop_bits = p_line_coding->stop_bits;
    bridge_stats.parity = p_line_coding->parity;

    uart_deinit(UART_BRIDGE_INST);
    uart_init(UART_BRIDGE_INST, p_line_coding->bit_rate);

    uart_parity_t parity = UART_PARITY_NONE;
    if (p_line_coding->parity == 1) parity = UART_PARITY_ODD;
    else if (p_line_coding->parity == 2) parity = UART_PARITY_EVEN;

    uint stop = (p_line_coding->stop_bits == 2) ? 2 : 1;
    uint data = (p_line_coding->data_bits >= 5 && p_line_coding->data_bits <= 8)
                    ? p_line_coding->data_bits : 8;

    uart_set_format(UART_BRIDGE_INST, data, stop, parity);
}
