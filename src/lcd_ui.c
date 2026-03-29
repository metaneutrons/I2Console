#include "lcd_ui.h"
#include "lcd_driver.h"
#include "pico/stdlib.h"
#include <stdio.h>

static lcd_screen_t current_screen = LCD_SCREEN_I2C;
static char buf[32];

static void draw_labels_i2c(void) {
    lcd_clear(COLOR_BLACK);
    lcd_draw_string(60, 5, "I2Console", COLOR_CYAN, COLOR_BLACK);
    lcd_draw_string(5, 30, "Addr:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 50, "TX:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 70, "RX:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 90, "USB:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 110, "Err:", COLOR_WHITE, COLOR_BLACK);
}

static void draw_labels_uart(void) {
    lcd_clear(COLOR_BLACK);
    lcd_draw_string(66, 5, "UART Bridge", COLOR_YELLOW, COLOR_BLACK);
    lcd_draw_string(5, 30, "Baud:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 50, "Fmt:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 70, "TX:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 90, "RX:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(5, 110, "USB:", COLOR_WHITE, COLOR_BLACK);
}

void lcd_ui_init(void) {
    lcd_init();
    lcd_clear(COLOR_BLACK);
    draw_labels_i2c();
}

void lcd_ui_switch_screen(void) {
    current_screen = (current_screen + 1) % LCD_SCREEN_COUNT;
    if (current_screen == LCD_SCREEN_I2C) {
        draw_labels_i2c();
    } else {
        draw_labels_uart();
    }
}

void lcd_ui_update_i2c(uint8_t i2c_addr, uint16_t tx_avail, uint16_t rx_avail,
                       uint32_t tx_bytes, uint32_t rx_bytes, bool usb_connected,
                       uint32_t errors) {
    if (current_screen != LCD_SCREEN_I2C) return;

    snprintf(buf, sizeof(buf), "0x%02X ", i2c_addr);
    lcd_draw_string(80, 30, buf, COLOR_GREEN, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%3d/%3lu ", tx_avail, (unsigned long)tx_bytes);
    lcd_draw_string(50, 50, buf, tx_avail > 200 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%4d/%4lu ", rx_avail, (unsigned long)rx_bytes);
    lcd_draw_string(50, 70, buf, rx_avail > 900 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);

    lcd_draw_string(70, 90, usb_connected ? "CONN " : "DISC ",
                    usb_connected ? COLOR_GREEN : COLOR_RED, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%lu   ", (unsigned long)errors);
    lcd_draw_string(70, 110, buf, errors > 0 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);
}

void lcd_ui_update_uart(uart_bridge_stats_t *stats) {
    if (current_screen != LCD_SCREEN_UART) return;

    snprintf(buf, sizeof(buf), "%lu   ", (unsigned long)stats->baud_rate);
    lcd_draw_string(70, 30, buf, COLOR_GREEN, COLOR_BLACK);

    const char *par = "N";
    if (stats->parity == 1) par = "O";
    else if (stats->parity == 2) par = "E";
    snprintf(buf, sizeof(buf), "%u%s%u ", stats->data_bits, par, stats->stop_bits);
    lcd_draw_string(60, 50, buf, COLOR_GREEN, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%lu   ", (unsigned long)stats->tx_bytes);
    lcd_draw_string(50, 70, buf, COLOR_GREEN, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%lu   ", (unsigned long)stats->rx_bytes);
    lcd_draw_string(50, 90, buf, COLOR_GREEN, COLOR_BLACK);

    lcd_draw_string(70, 110, stats->connected ? "CONN " : "DISC ",
                    stats->connected ? COLOR_GREEN : COLOR_RED, COLOR_BLACK);
}
