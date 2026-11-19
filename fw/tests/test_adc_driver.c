#include <stdint.h>
#include "includes.h"
#include "adc.h"

#define SAMPLE_COUNT 256u
#define BUFFER_COUNT 260u
#define SENTINEL 0xA5A5A5A5u
#define TEST_DIVIDER 16u

static uint32_t buffer[BUFFER_COUNT];
static unsigned int failures;

static void check(const char *name, int condition)
{
    if (!condition) {
        failures++;
        print_str("FAIL: ");
        print_str(name);
        print_chr('\n');
    }
}

static void prepare_buffer(void)
{
    for (uint32_t i = 0u; i < BUFFER_COUNT; i++) {
        buffer[i] = SENTINEL;
    }
}

static void check_guards(uint32_t count)
{
    check("limite anterior",
          buffer[0] == SENTINEL && buffer[1] == SENTINEL);

    int preserved = 1;

    for (uint32_t i = 2u + count; i < BUFFER_COUNT; i++) {
        preserved &= buffer[i] == SENTINEL;
    }

    check("limite posterior", preserved);
}

/* Hasta diez digitos, un salto de linea y el terminador. */
static void print_sample(uint32_t value)
{
    char text[12];
    unsigned int position = sizeof(text) - 1u;

    text[position] = '\0';
    text[--position] = '\n';

    do {
        text[--position] = (char)('0' + value % 10u);
        value /= 10u;
    } while (value != 0u);

    print_str(&text[position]);
}

int main(void)
{
    static const uint32_t lengths[] = {
        1u, 16u, 64u, 100u, 256u, 260u
    };

    REG_FW_STATUS = 0u;
    REG_FW_REPORT = 0u;
    failures = 0u;

    adc_init(TEST_DIVIDER);
    print_str("ADC TEST BEGIN\n");

    check("puntero nulo", adc_read(0, 8u) == 0u);

    prepare_buffer();
    check("longitud cero", adc_read(&buffer[2], 0u) == 0u);
    check_guards(0u);

    for (uint32_t i = 0u;
         i < sizeof(lengths) / sizeof(lengths[0]); i++) {
        uint32_t requested = lengths[i];
        uint32_t expected = requested;

        if (expected > SAMPLE_COUNT) {
            expected = SAMPLE_COUNT;
        }

        prepare_buffer();

        uint32_t copied = adc_read(&buffer[2], requested);

        check("cantidad copiada", copied == expected);
        check_guards(expected);
    }

    /*
     * Captura completa adicional para comprobar variacion
     * y obtener los datos que se imprimiran.
     */
    prepare_buffer();

    uint32_t copied = adc_read(&buffer[2], SAMPLE_COUNT);

    check("captura final", copied == SAMPLE_COUNT);
    check_guards(SAMPLE_COUNT);

    unsigned int changes = 0u;

    for (uint32_t i = 1u; i < SAMPLE_COUNT; i++) {
        if (buffer[2u + i] != buffer[1u + i]) {
            changes++;
        }
    }

    check("variacion de muestras", changes >= 64u);

    print_str("-- ADC data read --\n");

    for (uint32_t i = 0u; i < SAMPLE_COUNT; i++) {
        print_sample(buffer[2u + i]);
    }

    if (failures == 0u) {
        print_str("ADC TEST PASS\n");
    } else {
        print_str("ADC TEST FAIL\n");
    }

    REG_FW_REPORT = failures;
    REG_FW_STATUS = failures == 0u ? 0xFFu : 0xBADu;

    while (1) {
    }
}