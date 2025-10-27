#include "lcd_driver.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

static const uint8_t font_5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
};

static inline void lcd_write_cmd(uint8_t cmd) {
    gpio_put(LCD_DC_PIN, 0);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(spi1, &cmd, 1);
    gpio_put(LCD_CS_PIN, 1);
}

static inline void lcd_write_data(uint8_t data) {
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(spi1, &data, 1);
    gpio_put(LCD_CS_PIN, 1);
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcd_write_cmd(0x2A);
    lcd_write_data(x0 >> 8);
    lcd_write_data(x0 & 0xFF);
    lcd_write_data(x1 >> 8);
    lcd_write_data(x1 & 0xFF);
    
    lcd_write_cmd(0x2B);
    lcd_write_data(y0 >> 8);
    lcd_write_data(y0 & 0xFF);
    lcd_write_data(y1 >> 8);
    lcd_write_data(y1 & 0xFF);
    
    lcd_write_cmd(0x2C);
}

void lcd_init(void) {
    spi_init(spi1, 62500000);
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
    
    gpio_init(LCD_DC_PIN);
    gpio_set_dir(LCD_DC_PIN, GPIO_OUT);
    gpio_init(LCD_CS_PIN);
    gpio_set_dir(LCD_CS_PIN, GPIO_OUT);
    gpio_init(LCD_RST_PIN);
    gpio_set_dir(LCD_RST_PIN, GPIO_OUT);
    
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(5);
    gpio_put(LCD_RST_PIN, 0);
    sleep_ms(20);
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(150);
    
    lcd_write_cmd(0x11);
    sleep_ms(120);
    
    lcd_write_cmd(0x36);
    lcd_write_data(0x70);
    
    lcd_write_cmd(0x3A);
    lcd_write_data(0x05);
    
    lcd_write_cmd(0xB2);
    lcd_write_data(0x0C);
    lcd_write_data(0x0C);
    lcd_write_data(0x00);
    lcd_write_data(0x33);
    lcd_write_data(0x33);
    
    lcd_write_cmd(0xB7);
    lcd_write_data(0x35);
    
    lcd_write_cmd(0xBB);
    lcd_write_data(0x19);
    
    lcd_write_cmd(0xC0);
    lcd_write_data(0x2C);
    
    lcd_write_cmd(0xC2);
    lcd_write_data(0x01);
    
    lcd_write_cmd(0xC3);
    lcd_write_data(0x12);
    
    lcd_write_cmd(0xC4);
    lcd_write_data(0x20);
    
    lcd_write_cmd(0xC6);
    lcd_write_data(0x0F);
    
    lcd_write_cmd(0xD0);
    lcd_write_data(0xA4);
    lcd_write_data(0xA1);
    
    lcd_write_cmd(0xE0);
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0D);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2B);
    lcd_write_data(0x3F);
    lcd_write_data(0x54);
    lcd_write_data(0x4C);
    lcd_write_data(0x18);
    lcd_write_data(0x0D);
    lcd_write_data(0x0B);
    lcd_write_data(0x1F);
    lcd_write_data(0x23);
    
    lcd_write_cmd(0xE1);
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0C);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2C);
    lcd_write_data(0x3F);
    lcd_write_data(0x44);
    lcd_write_data(0x51);
    lcd_write_data(0x2F);
    lcd_write_data(0x1F);
    lcd_write_data(0x1F);
    lcd_write_data(0x20);
    lcd_write_data(0x23);
    
    lcd_write_cmd(0x21);
    lcd_write_cmd(0x29);
    
    gpio_init(LCD_BL_PIN);
    gpio_set_dir(LCD_BL_PIN, GPIO_OUT);
    gpio_put(LCD_BL_PIN, 1);
}

void lcd_clear(uint16_t color) {
    lcd_fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    lcd_set_window(x, y, x + w - 1, y + h - 1);
    
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    
    for (uint32_t i = 0; i < w * h; i++) {
        spi_write_blocking(spi1, &hi, 1);
        spi_write_blocking(spi1, &lo, 1);
    }
    
    gpio_put(LCD_CS_PIN, 1);
}

void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg) {
    if (c < 32 || c > 90) c = 32;
    const uint8_t *glyph = font_5x7[c - 32];
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 7; j++) {
            if (glyph[i] & (1 << j)) {
                lcd_fill_rect(x + i, y + j, 1, 1, color);
            } else {
                lcd_fill_rect(x + i, y + j, 1, 1, bg);
            }
        }
    }
}

void lcd_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg) {
    while (*str) {
        lcd_draw_char(x, y, *str++, color, bg);
        x += 6;
    }
}

void lcd_set_backlight(uint8_t brightness) {
    gpio_put(LCD_BL_PIN, brightness > 0 ? 1 : 0);
}
