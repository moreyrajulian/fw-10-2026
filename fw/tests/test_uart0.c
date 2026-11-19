#include "includes.h"
#include "uart.h"

int main(void)
{
    char message[261];

    REG_FW_STATUS = 0u;

    uart_init();

    uart_write_str("UART0 TEST BEGIN\n");

    uart_write_str("BYTE: ");
    uart_write_byte('A');
    uart_write_byte('\n');

    uart_write_str("STRING: Hola UART0\n");

    /* Estas llamadas deben retornar sin transmitir caracteres. */
    uart_write_str("EMPTY: [");
    uart_write_str("");
    uart_write_str("]\n");

    uart_write_str("NULL: [");
    uart_write_str(0);
    uart_write_str("]\n");

    /*
     * 260 caracteres de contenido y un terminador.
     * El driver debe transmitir dos bloques, de 256 y 4.
     */
    for (unsigned int i = 0u; i < 260u; i++) {
        message[i] = (char)('0' + (i % 10u));
    }
    message[260] = '\0';

    uart_write_str("LONG: [");
    uart_write_str(message);
    uart_write_str("]\n");

    /* Comprobar también la integración de print.c. */
    print_str("PRINT: ");
    print_chr('Z');
    print_chr(' ');
    print_dec(12345u);
    print_chr(' ');
    print_hex(0xA5u, 2);
    print_chr('\n');

    uart_write_str("UART0 TEST END\n");

    /* Canal de finalización del testbench. */
    REG_FW_STATUS = 0xFFu;

    while (1) {
    }
}