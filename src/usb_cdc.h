#ifndef USB_CDC_H
#define USB_CDC_H

#include <stdint.h>
#include <stdbool.h>

#define CDC_ITF_DATA  0
#define CDC_ITF_UART  1
#define CDC_ITF_DEBUG 2

#define UART_BRIDGE_TX_PIN 4
#define UART_BRIDGE_RX_PIN 5

typedef struct {
    uint32_t tx_bytes;
    uint32_t rx_bytes;
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    bool connected;
} uart_bridge_stats_t;

void usb_cdc_init(void);
void usb_cdc_task(void);
bool usb_cdc_connected(void);
int usb_cdc_read(uint8_t *buffer, int len);
int usb_cdc_write(const uint8_t *buffer, int len);
void usb_cdc_check_bootloader_cmd(void);

void uart_bridge_init(void);
void uart_bridge_task(void);
uart_bridge_stats_t uart_bridge_get_stats(void);

#endif
