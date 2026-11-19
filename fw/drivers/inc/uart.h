#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_write_byte(char c);
void uart_write_str(const char *s);

#endif