#include "lcd_ui.h"
#include "lcd_driver.h"
#include <stdio.h>

static char buffer[32];

void lcd_ui_init(void) {
    lcd_init();
    lcd_clear(COLOR_BLACK);
    
    // Title
    lcd_draw_string(70, 10, "I2Console", COLOR_CYAN, COLOR_BLACK);
    
    // Static labels
    lcd_draw_string(10, 30, "I2C Addr:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(10, 45, "TX Buf:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(10, 60, "RX Buf:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(10, 75, "TX:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(10, 90, "RX:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(10, 105, "USB:", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(10, 120, "Errors:", COLOR_WHITE, COLOR_BLACK);
}

void lcd_ui_update(uint8_t i2c_addr, uint16_t tx_avail, uint16_t rx_avail,
                   uint32_t tx_bytes, uint32_t rx_bytes, bool usb_connected,
                   uint32_t errors) {
    
    // I2C Address
    snprintf(buffer, sizeof(buffer), "0x%02X  ", i2c_addr);
    lcd_draw_string(90, 30, buffer, COLOR_GREEN, COLOR_BLACK);
    
    // TX Buffer
    snprintf(buffer, sizeof(buffer), "%3d/256 ", tx_avail);
    lcd_draw_string(90, 45, buffer, tx_avail > 200 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);
    
    // RX Buffer
    snprintf(buffer, sizeof(buffer), "%4d/1024", rx_avail);
    lcd_draw_string(90, 60, buffer, rx_avail > 900 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);
    
    // TX Bytes
    snprintf(buffer, sizeof(buffer), "%lu    ", tx_bytes);
    lcd_draw_string(90, 75, buffer, COLOR_YELLOW, COLOR_BLACK);
    
    // RX Bytes
    snprintf(buffer, sizeof(buffer), "%lu    ", rx_bytes);
    lcd_draw_string(90, 90, buffer, COLOR_YELLOW, COLOR_BLACK);
    
    // USB Status
    lcd_draw_string(90, 105, usb_connected ? "CONN " : "DISC ", 
                    usb_connected ? COLOR_GREEN : COLOR_RED, COLOR_BLACK);
    
    // Errors
    snprintf(buffer, sizeof(buffer), "%lu    ", errors);
    lcd_draw_string(90, 120, buffer, errors > 0 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);
}
