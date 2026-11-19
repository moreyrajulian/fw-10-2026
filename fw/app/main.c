#include <stdint.h>

#include "led.h"
#include "uart.h"
#include "dma.h"
#include "adc.h"
#include "fw_status.h"

#define DMA_WORDS 32u
#define ADC_SAMPLES 256u

static uint32_t source[DMA_WORDS];
static uint32_t destination[DMA_WORDS];
static uint32_t samples[ADC_SAMPLES];

static void fail(const char *message)
{
    led_write(0x80u);
    uart_write_str(message);
    uart_write_str("TP5 DEMO FAIL\n");
    fw_finish(0);
}

int main(void)
{
    uart_init();
    led_init();
    dma_init();
    adc_init(4u);

    uart_write_str("TP5 DEMO BEGIN\n");
    led_on(0u);

    /* Copia DMA con contenido conocido. */
    for (uint32_t i = 0u; i < DMA_WORDS; i++) {
        source[i] = 0x12340000u + i;
        destination[i] = 0u;
    }

    uart_write_str("DMA: iniciando copia de 32 palabras\n");
    dma_copy(source, destination, DMA_WORDS);

    if (!dma_is_done()) {
        fail("DMA: no indica finalizacion\n");
    }

    for (uint32_t i = 0u; i < DMA_WORDS; i++) {
        uint32_t expected = 0x12340000u + i;

        if (source[i] != expected || destination[i] != expected) {
            fail("DMA: contenido incorrecto\n");
        }
    }

    led_on(1u);
    uart_write_str("DMA OK\n");

    /* Adquisicion unica y lectura del bloque completo. */
    uart_write_str("ADC: iniciando captura de 256 muestras\n");

    uint32_t count = adc_read(samples, ADC_SAMPLES);

    if (count != ADC_SAMPLES) {
        fail("ADC: cantidad incorrecta\n");
    }

    unsigned int changes = 0u;

    for (uint32_t i = 1u; i < ADC_SAMPLES; i++) {
        if (samples[i] != samples[i - 1u]) {
            changes++;
        }
    }

    if (changes < 64u) {
        fail("ADC: variacion insuficiente\n");
    }

    led_on(2u);
    uart_write_str("ADC OK\n");

    led_on(3u);

    if (led_get_mask() != 0x0Fu) {
        fail("LED: estado final incorrecto\n");
    }

    uart_write_str("TP5 DEMO PASS\n");
    fw_finish(1);

    return 0;
}