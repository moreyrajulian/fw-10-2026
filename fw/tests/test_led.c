#include "includes.h"
#include "led.h"

static unsigned int failures = 0u;

static void check(const char *name, int condition)
{
    if (!condition) {
        failures++;
        print_str("FAIL: ");
        print_str(name);
        print_chr('\n');
    }
}

static void finish_test(void)
{
    if (failures == 0u) {
        print_str("LED TEST PASS\n");
    } else {
        print_str("LED TEST FAIL - errores: ");
        print_dec(failures);
        print_chr('\n');
    }

    /* Canal de resultado del testbench, no registro del LED. */
    REG_FW_STATUS = (failures == 0u) ? 0xFFu : 0xBADu;

    while (1) {
        /* El testbench termina la simulacion. */
    }
}

int main(void)
{
    static const uint8_t invalid_indices[] = {
        8u, 9u, 31u, 200u, 255u
    };

    print_str("Inicio de prueba LED\n");

    /* Inicializar debe borrar un estado previo no nulo. */
    led_write(0xFFu);
    led_init();
    check("init", led_get_mask() == 0x00u);

    led_on(0u);
    check("on 0", led_get_mask() == 0x01u);

    led_on(7u);
    check("on 7 conserva LED 0", led_get_mask() == 0x81u);

    led_on(7u);
    check("on repetido", led_get_mask() == 0x81u);

    led_off(0u);
    check("off 0 conserva LED 7", led_get_mask() == 0x80u);

    led_off(0u);
    check("off repetido", led_get_mask() == 0x80u);

    led_toggle(7u);
    check("toggle apaga", led_get_mask() == 0x00u);

    led_toggle(3u);
    check("toggle enciende", led_get_mask() == 0x08u);

    led_write(0xA5u);
    check("write reemplaza el estado", led_get_mask() == 0xA5u);

    led_toggle(0u);
    check("toggle conserva otros bits", led_get_mask() == 0xA4u);

    led_toggle(0u);
    check("doble toggle restaura", led_get_mask() == 0xA5u);

    for (uint8_t idx = 0u; idx < 8u; idx++) {
        check("get_status",
              led_get_status(idx) == ((0xA5u >> idx) & 1u));
    }

    led_set_mask(0x1FFu);
    check("mask limita a ocho bits", led_get_mask() == 0xFFu);

    led_set_mask(0x3Cu);
    check("set_mask reemplaza el estado", led_get_mask() == 0x3Cu);

    for (unsigned int i = 0u;
         i < sizeof(invalid_indices) / sizeof(invalid_indices[0]);
         i++) {
        uint8_t idx = invalid_indices[i];

        led_on(idx);
        check("on invalido", led_get_mask() == 0x3Cu);

        led_off(idx);
        check("off invalido", led_get_mask() == 0x3Cu);

        led_toggle(idx);
        check("toggle invalido", led_get_mask() == 0x3Cu);

        check("get_status invalido", led_get_status(idx) == 0u);
        check("consulta no modifica", led_get_mask() == 0x3Cu);
    }

    check("estado final", led_get_mask() == 0x3Cu);

    finish_test();
    return 0;
}