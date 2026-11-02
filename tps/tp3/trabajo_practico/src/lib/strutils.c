#include "strutils.h"

void strbuf_init(strbuf_t *buf)
{
    buf->len = 0;
    buf->data[0] = '\0';
}

void strbuf_set(strbuf_t *buf, const char *src)
{
    strbuf_init(buf);
    strbuf_append(buf, src);
}

size_t strbuf_append(strbuf_t *buf, const char *src)
{
    size_t copiados = 0;

    while (src[copiados] != '\0' &&
           buf->len < STR_MAX_LEN - 1) {
        buf->data[buf->len] = src[copiados];
        buf->len++;
        copiados++;
    }

    buf->data[buf->len] = '\0';
    return copiados;
}

size_t strbuf_append_int(strbuf_t *buf, int32_t value)
{
    char texto[12];
    size_t posicion = sizeof(texto) - 1;
    int64_t numero = value;

    texto[posicion] = '\0';

    if (numero < 0) {
        numero = -numero;
    }

    do {
        texto[--posicion] = (char)('0' + numero % 10);
        numero /= 10;
    } while (numero != 0);

    if (value < 0) {
        texto[--posicion] = '-';
    }

    return strbuf_append(buf, &texto[posicion]);
}

const char *strbuf_cstr(const strbuf_t *buf)
{
    return buf->data;
}

static int es_espacio(char c)
{
    return c == ' '  || c == '\t' || c == '\n' ||
           c == '\r' || c == '\v' || c == '\f';
}

size_t str_trim(char *dst, const char *src, size_t max_len)
{
    if (max_len == 0) {
        return 0;
    }

    while (es_espacio(*src)) {
        src++;
    }

    const char *fin = src;

    while (*fin != '\0') {
        fin++;
    }

    while (fin > src && es_espacio(fin[-1])) {
        fin--;
    }

    size_t copiados = 0;

    while (src < fin && copiados < max_len - 1) {
        dst[copiados] = *src;
        copiados++;
        src++;
    }

    dst[copiados] = '\0';
    return copiados;
}

uint8_t str_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = STR_CRC8_INIT;

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];

        for (unsigned int bit = 0; bit < 8; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1) ^ 0x07U);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }

    return crc;
}