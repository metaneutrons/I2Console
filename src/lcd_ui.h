#ifndef LCD_UI_H
#define LCD_UI_H

#include <stdint.h>
#include <stdbool.h>
#include "usb_cdc.h"
#include "i2c_slave.h"

typedef enum {
    LCD_SCREEN_I2C = 0,
    LCD_SCREEN_UART,
    LCD_SCREEN_COUNT
} lcd_screen_t;

void lcd_ui_init(void);
void lcd_ui_switch_screen(void);
void lcd_ui_update_i2c(uint8_t i2c_addr, uint16_t tx_avail, uint16_t rx_avail,
                       uint32_t tx_bytes, uint32_t rx_bytes, bool usb_connected,
                       uint32_t errors);
void lcd_ui_update_uart(uart_bridge_stats_t *stats);

#endif
