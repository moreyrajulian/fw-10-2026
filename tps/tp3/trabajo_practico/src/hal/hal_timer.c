#include "hal_timer.h"

static uint32_t milisegundos = 0;

void timer_init(void)
{
    milisegundos = 0;
}

void timer_tick(void)
{
    milisegundos++;
}

uint32_t timer_get_ms(void)
{
    return milisegundos;
}