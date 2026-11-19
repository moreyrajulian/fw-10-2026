// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

#include "includes.h"
#include "uart.h"

// REG_UART_COMMAND : 
//					   0x00 -> reset
//					   0x01 -> data ready
//					   0x02 -> flush peripheral buffer

static void uart0_send(const char *buf, unsigned int n)
{
    for (unsigned int i = 0u; i < n; i++) {
        uart_write_byte(buf[i]);
    }
}

void print_chr(char ch)
{
    uart_write_byte(ch);
}

void print_str(const char *p)
{
    uart_write_str(p);
}

void print_dec(unsigned int val)
{
    char buffer[10];
    char out[10];
    char *p = buffer;
    unsigned int n = 0;

    while (val || p == buffer) {
        *(p++) = val % 10;
        val = val / 10;
    }
    while (p != buffer)
        out[n++] = '0' + *(--p);

    uart0_send(out, n);
//    char buffer[10];
//    char *p = buffer;
//    while (val || p == buffer) {
//        *(p++) = val % 10;
//        val = val / 10;
//    }
//    while (p != buffer) {
//		REG_UART_COMMAND = 0;
//        REG_UART_DATA = '0' + *(--p);
//		REG_UART_COMMAND = 1;
//    }
//	print_flush_buffer();
}

void print_hex(unsigned int val, int digits)
{
    char out[8];
    unsigned int n = 0;

    if (digits > 8) digits = 8;
    for (int i = (4 * digits) - 4; i >= 0; i -= 4)
        out[n++] = "0123456789ABCDEF"[(val >> i) % 16];

    uart0_send(out, n);
//    for (int i = (4*digits)-4; i >= 0; i -= 4){
//		REG_UART_COMMAND = 0;
//        REG_UART_DATA = "0123456789ABCDEF"[(val >> i) % 16];
//		REG_UART_COMMAND = 1;
//	}
//	print_flush_buffer();
}

void print_flush_buffer()
{
//	REG_UART_COMMAND = 2;
//	REG_UART_COMMAND = 0;
}