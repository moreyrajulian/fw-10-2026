#include "uart.h"
#include "config.h"

static void uart0_send(const char *data, unsigned int n)
{
    while (!REG_UART0_TRANSMISION_DONE) {
    }

    for (unsigned int i = 0u; i < n; i++) {
        REG_ARRAY_UART0_BUFF_OUT[i] =
            (unsigned int)(unsigned char)data[i];
    }

    REG_UART0_SIZE_TO_TRANSMIT = n;

    REG_UART0_START_TRANSMIT = 1u;
    REG_UART0_START_TRANSMIT = 0u;

    while (!REG_UART0_TRANSMISION_DONE) {
    }
}

void uart_init(uart_id_t uart)
{
    if (uart != UART_0) {
        return;
    }

    while (!REG_UART0_TRANSMISION_DONE) {
    }

    REG_UART0_START_TRANSMIT = 0u;
    REG_UART0_SIZE_TO_TRANSMIT = 0u;
}

void uart_write_byte(uart_id_t uart, char c)
{
    if (uart != UART_0) {
        return;
    }

    uart0_send(&c, 1u);
}

void uart_write_str(uart_id_t uart, const char *s)
{
    if (uart != UART_0 || s == 0) {
        return;
    }

    while (*s != '\0') {
        unsigned int n = 0u;

        while (n < REG_ARRAY_UART0_BUFF_OUT_SIZE &&
               s[n] != '\0') {
            n++;
        }

        uart0_send(s, n);
        s += n;
    }
}