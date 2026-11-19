#include "uart1.h"

void uart1_init(uint32_t baud_div)
{
    REG_UART1_BAUD_DIV = baud_div;

    /* Establecer RTS = 0 y limpiar los errores con una escritura. */
    REG_UART1_CTRL = UART1_CTRL_CLR_ERR;
}

uint32_t uart1_rx_level(void)
{
    uint32_t status = REG_UART1_STATUS;

    return UART1_ST_RX_LEVEL(status);
}

int uart1_getc(uint8_t *b)
{
    if (b == 0) {
        return 0;
    }

    /* Cada lectura valida extrae un byte de la FIFO. */
    uint32_t data = REG_UART1_RX_DATA;

    if ((data & UART1_RX_VALID) == 0u) {
        return 0;
    }

    *b = (uint8_t)data;
    return 1;
}

void uart1_putc(uint8_t b)
{
    while ((REG_UART1_STATUS & UART1_ST_TX_FULL) != 0u) {
    }

    REG_UART1_TX_DATA = b;
}

void uart1_write(const uint8_t *buf, uint32_t n)
{
    if (buf == 0) {
        return;
    }

    for (uint32_t i = 0u; i < n; i++) {
        uart1_putc(buf[i]);
    }
}

int uart1_tx_idle(void)
{
    return (REG_UART1_STATUS & UART1_ST_TX_IDLE) != 0u;
}

uint32_t uart1_errors(void)
{
    uint32_t status = REG_UART1_STATUS;
    uint32_t errors = 0u;

    if ((status & UART1_ST_FRAME_ERR) != 0u) {
        errors |= UART1_ERR_FRAME;
    }

    if ((status & UART1_ST_OVERRUN) != 0u) {
        errors |= UART1_ERR_OVERRUN;
    }

    return errors;
}

void uart1_clear_errors(void)
{
    uint32_t rts = REG_UART1_CTRL & UART1_CTRL_RTS;

    /* La escritura genera el pulso CLR_ERR y conserva RTS. */
    REG_UART1_CTRL = rts | UART1_CTRL_CLR_ERR;
}

void uart1_set_rts(int ready)
{
    /* TODO TP6 */
    (void)ready;
}