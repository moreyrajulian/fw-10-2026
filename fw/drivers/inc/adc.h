#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define ADC_MODE_SINGLE   1u
#define ADC_MODE_CIRCULAR 2u

/* Configurar e iniciar con el ADC inactivo. */
void adc_init(uint32_t clk_div);
void set_adc_clk_divider(uint32_t clk_div);
void adc_start(uint8_t adc_mode);

/* Consulta inmediata del indicador de mitad de captura. */
int adc_half_done(void);

/* Espera bloqueante. Requiere una adquisicion iniciada. */
int adc_done(void);

/*
 * Inicia una captura unica completa y copia hasta 256 muestras.
 * Requiere el ADC inactivo al entrar.
 * Devuelve la cantidad copiada, o cero para puntero nulo
 * o longitud cero.
 */
uint32_t adc_read(uint32_t *data, uint32_t size);

#endif