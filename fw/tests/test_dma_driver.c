#include <stdint.h>
#include "includes.h"
#include "dma.h"

#define CAPACITY 1002u
#define SENTINEL 0xA5A5A5A5u

static volatile uint32_t source[CAPACITY];
static volatile uint32_t destination[CAPACITY];
static unsigned int failures;

static uint32_t pattern(uint32_t index)
{
    return 0x12340000u ^ (index * 17u);
}

static void check(const char *name, int condition)
{
    if (!condition) {
        failures++;
        print_str("FAIL: ");
        print_str(name);
        print_chr('\n');
    }
}

static void prepare_buffers(void)
{
    for (uint32_t i = 0u; i < CAPACITY; i++) {
        source[i] = pattern(i);
        destination[i] = SENTINEL;
    }
}

static int buffers_unchanged(void)
{
    for (uint32_t i = 0u; i < CAPACITY; i++) {
        if (source[i] != pattern(i) ||
            destination[i] != SENTINEL) {
            return 0;
        }
    }

    return 1;
}

static void test_copy(uint32_t count)
{
    prepare_buffers();

    dma_copy((const void *)&source[1],
             (void *)&destination[1], count);

    check("DONE despues de copiar", dma_is_done());

    int source_ok = 1;
    int destination_ok = 1;

    for (uint32_t i = 0u; i < CAPACITY; i++) {
        source_ok &= source[i] == pattern(i);

        uint32_t expected = SENTINEL;
        if (i >= 1u && i <= count) {
            expected = pattern(i);
        }

        destination_ok &= destination[i] == expected;
    }

    check("origen conservado", source_ok);
    check("contenido y limites del destino", destination_ok);
}

static void test_rejected(const char *name,
                          const void *src,
                          void *dst,
                          uint32_t count)
{
    prepare_buffers();

    dma_copy(src, dst, count);

    check(name, buffers_unchanged());
}

int main(void)
{
    static const uint32_t lengths[] = {
        1u, 2u, 16u, 255u, 256u, 1000u
    };

    const uintptr_t map_end =
        ADDR_START_REG_FILE + ADDR_SIZE_REG_FILE;

    REG_FW_STATUS = 0u;
    REG_FW_REPORT = 0u;
    failures = 0u;

    dma_init();
    print_str("DMA TEST BEGIN\n");

    for (uint32_t i = 0u;
         i < sizeof(lengths) / sizeof(lengths[0]); i++) {
        test_copy(lengths[i]);
    }

    test_rejected("longitud cero",
                  (const void *)source,
                  (void *)destination, 0u);

    test_rejected("origen desalineado",
                  (const void *)((uintptr_t)source + 1u),
                  (void *)destination, 1u);

    test_rejected("destino desalineado",
                  (const void *)source,
                  (void *)((uintptr_t)destination + 1u), 1u);

    test_rejected("origen fuera del mapa",
                  (const void *)map_end,
                  (void *)destination, 1u);

    test_rejected("destino fuera del mapa",
                  (const void *)source,
                  (void *)map_end, 1u);

    test_rejected("origen cruza el final del mapa",
                  (const void *)(map_end - 4u),
                  (void *)destination, 2u);

    test_rejected("destino cruza el final del mapa",
                  (const void *)source,
                  (void *)(map_end - 4u), 2u);

    test_rejected("longitud extrema",
                  (const void *)source,
                  (void *)destination, UINT32_MAX);

    test_rejected("solapamiento hacia adelante",
                  (const void *)&source[0],
                  (void *)&source[1], 8u);

    test_rejected("solapamiento hacia atras",
                  (const void *)&source[1],
                  (void *)&source[0], 8u);

    test_rejected("origen igual al destino",
                  (const void *)source,
                  (void *)source, 8u);

    /* El DMA debe seguir funcionando despues de los rechazos. */
    test_copy(16u);

    if (failures == 0u) {
        print_str("DMA TEST PASS\n");
    } else {
        print_str("DMA TEST FAIL - errores: ");
        print_dec(failures);
        print_chr('\n');
    }

    REG_FW_REPORT = failures;
    REG_FW_STATUS = failures == 0u ? 0xFFu : 0xBADu;

    while (1) {
    }
}