#ifndef DMA_H
#define DMA_H

#include <stdint.h>

void dma_init(void);

void dma_copy(const void *src, void *dst, uint32_t n_words);

int dma_is_done(void);

#endif