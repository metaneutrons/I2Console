#include "usb_cdc.h"
#include "tusb.h"

void usb_cdc_init(void) {
    tusb_init();
}

void usb_cdc_task(void) {
    tud_task();
}

bool usb_cdc_connected(void) {
    return tud_cdc_connected();
}

int usb_cdc_read(uint8_t *buffer, int len) {
    if (!tud_cdc_available()) {
        return 0;
    }
    return tud_cdc_read(buffer, len);
}

int usb_cdc_write(const uint8_t *buffer, int len) {
    if (!tud_cdc_connected()) {
        return 0;
    }
    return tud_cdc_write(buffer, len);
}
