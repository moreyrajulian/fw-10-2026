#include "demo.h"
#include "print.h"
#include "strutils.h"

void demo_str(void)
{
    strbuf_t mensaje;

    strbuf_set(&mensaje, "value=");
    strbuf_append_int(&mensaje, 42);

    print_str(strbuf_cstr(&mensaje));
    print_str("\n");

    char recortado[STR_MAX_LEN];

    str_trim(recortado, "  hello  ", sizeof(recortado));

    print_str("trimmed: '");
    print_str(recortado);
    print_str("'\n");

    const uint8_t datos[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t crc = str_crc8(datos, sizeof(datos));

    print_str("crc8: 0x");
    print_hex(crc, 2);
    print_str("\n");
}