#ifndef USB_CDC_H
#define USB_CDC_H

#include <stdint.h>
#include <stdbool.h>

void usb_cdc_init(void);
void usb_cdc_task(void);
bool usb_cdc_connected(void);
int usb_cdc_read(uint8_t *buffer, int len);
int usb_cdc_write(const uint8_t *buffer, int len);

#endif
