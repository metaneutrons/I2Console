#include "button.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/structs/io_qspi.h"
#include "hardware/structs/sio.h"

// BOOTSEL button is on QSPI CS pin, index 1 in io[] array on RP2350
// (io[0]=SCLK, io[1]=SS, io[2]=SD0, io[3]=SD1, io[4]=SD2, io[5]=SD3)
#define QSPI_SS_INDEX 1

static bool pressed = false;
static uint32_t press_start = 0;

static bool __no_inline_not_in_flash_func(read_bootsel)(void) {
    uint32_t flags = save_and_disable_interrupts();

    // Disable QSPI SS output driver so we can read the button
    hw_write_masked(&io_qspi_hw->io[QSPI_SS_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    for (volatile int i = 0; i < 1000; ++i);

    // INFROMPAD is bit 17 in status register
    bool button = !(io_qspi_hw->io[QSPI_SS_INDEX].status & (1u << 17));

    hw_write_masked(&io_qspi_hw->io[QSPI_SS_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);
    return button;
}

void button_init(void) {
    pressed = false;
    press_start = 0;
}

button_event_t button_task(void) {
    bool raw = read_bootsel();
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (raw && !pressed) {
        // Button just pressed
        pressed = true;
        press_start = now;
    } else if (!raw && pressed) {
        // Button released
        pressed = false;
        uint32_t duration = now - press_start;
        if (duration >= BUTTON_LONG_PRESS_MS) {
            return BUTTON_EVENT_LONG_PRESS;
        } else if (duration >= BUTTON_DEBOUNCE_MS) {
            return BUTTON_EVENT_SHORT_PRESS;
        }
    }

    return BUTTON_EVENT_NONE;
}
