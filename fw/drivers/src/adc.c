#include "adc.h"
#include "config.h"

#define ADC_STATUS_HALF_DONE (1u << 0)
#define ADC_STATUS_DONE      (1u << 1)

static uint32_t configured_divider = 1u;

void set_adc_clk_divider(uint32_t clk_div)
{
    if (clk_div == 0u) {
        return;
    }

    configured_divider = clk_div;
    REG_ADC_CLK_DIVIDER = clk_div;
}

void adc_init(uint32_t clk_div)
{
    REG_ADC_START = 0u;

    if (clk_div == 0u) {
        clk_div = 1u;
    }

    set_adc_clk_divider(clk_div);
}

void adc_start(uint8_t adc_mode)
{
    if (adc_mode != ADC_MODE_SINGLE &&
        adc_mode != ADC_MODE_CIRCULAR) {
        return;
    }

    REG_ADC_START = 0u;
    REG_ADC_CLK_DIVIDER = configured_divider;
    REG_ADC_START = adc_mode;

    /* Esperar a que se descarte el DONE de la captura anterior. */
    while ((REG_ADC_STATUS & ADC_STATUS_DONE) != 0u) {
    }
}

int adc_half_done(void)
{
    return (REG_ADC_STATUS & ADC_STATUS_HALF_DONE) != 0u;
}

int adc_done(void)
{
    while ((REG_ADC_STATUS & ADC_STATUS_DONE) == 0u) {
    }

    return 1;
}

uint32_t adc_read(uint32_t *data, uint32_t size)
{
    if (data == 0 || size == 0u) {
        return 0u;
    }

    if (size > REG_ARRAY_ADC_BUFF_SIZE) {
        size = REG_ARRAY_ADC_BUFF_SIZE;
    }

    adc_start(ADC_MODE_SINGLE);
    adc_done();

    for (uint32_t i = 0u; i < size; i++) {
        data[i] = REG_ARRAY_ADC_BUFF[i];
    }

    return size;
}