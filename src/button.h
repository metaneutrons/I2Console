#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS,
} button_event_t;

#define BUTTON_LONG_PRESS_MS 5000
#define BUTTON_DEBOUNCE_MS   50

void button_init(void);
button_event_t button_task(void);

#endif
