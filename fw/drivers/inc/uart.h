#ifndef UART_H
#define UART_H

typedef enum {
    UART_0 = 0
} uart_id_t;

void uart_init(uart_id_t uart);
void uart_write_byte(uart_id_t uart, char c);
void uart_write_str(uart_id_t uart, const char *s);

#endif