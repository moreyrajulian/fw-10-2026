#include "demo.h"
#include "print.h"
#include "strutils.h"
#include "hal_ring.h"
#include "hal_timer.h"
#include "hal_uart.h"

void demo_embedded(void)
{
    uart_regs_t uart = {0};
    const uint8_t texto_uart[] = {'O', 'K'};

    uart_init(&uart, 115200);
    uart_send(&uart, texto_uart, 2);

    print_str("uart BRR=");
    print_dec(uart.BRR);

    print_str(" SR=0x");
    print_hex(uart.SR, 8);

    print_str(" DR=0x");
    print_hex(uart.DR, 8);

    print_str("\n");

    ring_buf_t cola;
    ring_init(&cola);

    for (uint8_t letra = 'A'; letra <= 'J'; letra++) {
        if (ring_push(&cola, letra) != 0) {
            print_str("ring: error al agregar\n");
            return;
        }
    }

    print_str("ring: ");

    uint8_t recibido;

    while (ring_pop(&cola, &recibido) == 0) {
        print_chr((char)recibido);
    }

    print_str("\n");
    
    uint8_t trama[6] = {0xAA, 0x01, 0x02, 0x03, 0x04, 0};
    uint8_t recibida[6] = {0};

    trama[5] = str_crc8(&trama[1], 4);

    ring_buf_t canal;
    ring_init(&canal);

    int transferencia_ok = 1;

    for (size_t i = 0; i < sizeof(trama); i++) {
        if (ring_push(&canal, trama[i]) != 0) {
            transferencia_ok = 0;
            break;
        }
    }

    for (size_t i = 0;
         i < sizeof(recibida) && transferencia_ok;
         i++) {
        if (ring_pop(&canal, &recibida[i]) != 0) {
            transferencia_ok = 0;
            break;
        }
    }

    if (transferencia_ok &&
        recibida[0] == 0xAA &&
        str_crc8(&recibida[1], 4) == recibida[5]) {
        print_str("frame: ok crc=0x");
        print_hex(recibida[5], 2);
        print_str("\n");
    } else {
        print_str("frame: error\n");
    }

    timer_init();
    timer_tick();
    timer_tick();
    timer_tick();

    print_str("timer_ms: ");
    print_dec((unsigned int)timer_get_ms());
    print_str("\n");
}