#include "led.h"
#include "config.h"

#define LED_VALID_MASK 0xFFu

void led_set_mask(uint32_t mask)
{
    REG_LED = mask & LED_VALID_MASK;
}

uint32_t led_get_mask(void)
{
    return REG_LED & LED_VALID_MASK;
}

void led_init(void)
{
    led_set_mask(0u);
}

void led_write(uint8_t value)
{
    led_set_mask(value);
}

void led_on(uint8_t idx)
{
    if (idx >= 8u) {
        return;
    }

    led_set_mask(led_get_mask() | (1u << idx));
}

void led_off(uint8_t idx)
{
    if (idx >= 8u) {
        return;
    }

    led_set_mask(led_get_mask() & ~(1u << idx));
}

void led_toggle(uint8_t idx)
{
    if (idx >= 8u) {
        return;
    }

    led_set_mask(led_get_mask() ^ (1u << idx));
}

uint8_t led_get_status(uint8_t idx)
{
    if (idx >= 8u) {
        return 0u;
    }

    return (uint8_t)((led_get_mask() >> idx) & 1u);
}