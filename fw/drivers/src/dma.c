#include "dma.h"
#include "config.h"

static int dma_range_valid(uintptr_t address, uint32_t n_words)
{
    const uintptr_t map_end =
        ADDR_START_REG_FILE + ADDR_SIZE_REG_FILE;

    if ((address & 3u) != 0u) {
        return 0;
    }

    if (address >= map_end) {
        return 0;
    }

    return n_words <= (map_end - address) / sizeof(uint32_t);
}

void dma_init(void)
{
    REG_DMA_START = 0u;
    REG_DMA_SOURCE_ADDR = 0u;
    REG_DMA_DEST_ADDR = 0u;
    REG_DMA_SIZE = 0u;
}

void dma_copy(const void *src, void *dst, uint32_t n_words)
{
    uintptr_t source = (uintptr_t)src;
    uintptr_t destination = (uintptr_t)dst;

    if (n_words == 0u) {
        return;
    }

    if (!dma_range_valid(source, n_words) ||
        !dma_range_valid(destination, n_words)) {
        return;
    }

    uintptr_t n_bytes = (uintptr_t)n_words * sizeof(uint32_t);

    if (source < destination + n_bytes &&
        destination < source + n_bytes) {
        return;
    }

    __asm__ volatile ("" ::: "memory");

    REG_DMA_START = 0u;
    REG_DMA_SOURCE_ADDR = (uint32_t)source;
    REG_DMA_DEST_ADDR = (uint32_t)destination;
    REG_DMA_SIZE = n_words;
    REG_DMA_START = 1u;

    while (!dma_is_done()) {
    }

    __asm__ volatile ("" ::: "memory");
}

int dma_is_done(void)
{
    return REG_DMA_DONE != 0u;
}