/*
 * bl_main.c — TP4 (Bloque 4): lógica del BOOTLOADER
 *
 * Implementación del arranque y la validación de la imagen.
 *   - Hito 5: con los pasos (0) y (6) alcanza para arrancar el FW sin validar nada.
 *   - Hito 7: se agregan las validaciones (1) a (5) y bl_fail(). El CRC es tu
 *     str_crc8() del TP3: el Makefile compila la implementación local de lib/src/strutils.c.
 *
 * Reglas:
 *   - El BL NO usa variables globales (todo en el stack o const): bl.ld lo
 *     tiene que hacer cumplir.
 *   - El BL nunca deja REG_FW_STATUS en 0xFF (¿por qué?).
 *          
 *      El BL no escribe 0xFF porque el testbench lo interpreta como
 *      finalización exitosa de la prueba. Validar la imagen todavía
 *      no demuestra que la aplicación se haya ejecutado correctamente.
 *
 *   - Para imprimir usen print_str() de fw/lib/src/print.c o bl_log_hex().
 */
#include <stdint.h>
#include "includes.h"   /* config.h (registros) + print_str() de print.c */
#include "fw_image.h"   /* header + vector table del FW (el contrato)    */
#include "strutils.h"   /* tu str_crc8() del TP3 */

/* ---- Símbolos que tiene que definir bootloader/bl.ld (salen de memory_map.ld) ----
 * Para el linker son direcciones; en C se usan con '&' (o como array),
 * igual que _sdata/_ebss en un startup. */
extern const fw_image_header_t __fw_slot_start;  /* ORIGIN(APP_ROM): acá espera al FW      */
extern const uint8_t           __fw_slot_size[]; /* LENGTH(APP_ROM): el valor ES la dirección */

/* ---- bl_start.S ---- */
void bl_jump(uint32_t sp, uint32_t entry) __attribute__((noreturn));
void bl_halt(void) __attribute__((noreturn));
void bl_main(void) __attribute__((noreturn));

/* ---- Marcas de avance en REG_FW_REPORT (como test_fw_status.c del TP2):
 *      el testbench no las imprime, pero se ven en las ondas (make wave) ---- */
#define BL_REPORT_START   0xB0070000u   /* el BL arrancó              */
#define BL_REPORT_IMG_OK  0xB0070100u   /* imagen del FW validada     */
#define BL_REPORT_JUMP    0xB0070200u   /* saltando al FW             */
#define BL_REPORT_ERROR   0xB007E000u   /* error; bits bajos = código */

enum bl_error {
    BL_ERR_MAGIC = 1,   /* no hay FW en el slot                     */
    BL_ERR_LOAD_ADDR,   /* el FW fue linkeado para otra dirección   */
    BL_ERR_SIZE,        /* image_size fuera de rango                */
    BL_ERR_CRC,         /* imagen corrupta                          */
    BL_ERR_ENTRY,       /* reset_handler fuera de la imagen         */
    BL_ERR_SP,          /* initial_sp fuera de la RAM / desalineado */
};

/* (YA ESTÁ HECHO) Arma "<msg>0xXXXXXXXX" en el stack y lo manda en UNA
 * transmisión de UART0 (el testbench imprime cada una como "UART: ..."). */
static void bl_log_hex(const char *msg, uint32_t val)
{
    char line[64];
    unsigned int n = 0;

    while (*msg != '\0' && n < sizeof(line) - 11u)
        line[n++] = *msg++;
    line[n++] = '0';
    line[n++] = 'x';
    for (int shift = 28; shift >= 0; shift -= 4)
        line[n++] = "0123456789ABCDEF"[(val >> shift) & 0xFu];
    line[n] = '\0';
    print_str(line);
}

/*
 * Informa el error por UART y registros, y detiene la ejecución.
 * 0xBAD no finaliza por sí solo la simulación; bl_halt()
 * ejecuta ebreak para que el testbench detecte trap.
 */
 static void __attribute__((noreturn)) bl_fail(enum bl_error code, const char *why)
{
    print_str(why);
    REG_FW_REPORT = BL_REPORT_ERROR | (uint32_t)code;
    REG_FW_STATUS = 0xBAD;
    bl_halt();
}

void bl_main(void)
{
    const fw_image_header_t *hdr = &__fw_slot_start;    /* header + vector table del FW */

    const uintptr_t slot_start = (uintptr_t)&__fw_slot_start;
    const uintptr_t slot_size  = (uintptr_t)__fw_slot_size;

    REG_FW_STATUS = 0;
    REG_FW_REPORT = BL_REPORT_START;

    print_str("[BL] Inicio");

    if (hdr->magic != FW_IMAGE_MAGIC) {
        bl_fail(BL_ERR_MAGIC, 
                "[BL] ERROR: magic invalido");
    }

    if (hdr->load_addr != slot_start) {
        bl_fail(BL_ERR_LOAD_ADDR,
                "[BL] ERROR: direccion de carga invalida");
    }

    if (hdr->image_size < FW_HDR_SIZE ||
        hdr->image_size > slot_size) {
        bl_fail(BL_ERR_SIZE,
                "[BL] ERROR: tamanio de imagen invalido");
    }

    const uint8_t *crc_start =
        (const uint8_t *)(slot_start + FW_IMAGE_CRC_START);

    const size_t crc_length =
        hdr->image_size - FW_IMAGE_CRC_START;

    const uint8_t calculated_crc =
        str_crc8(crc_start, crc_length);

    if (calculated_crc != hdr->image_crc) {
        bl_fail(BL_ERR_CRC, 
                "[BL] ERROR: CRC invalido");
    }

    const uint32_t app_sp = hdr->initial_sp;
    const uint32_t app_entry = hdr->reset_handler;

    const uintptr_t ram_start = ADDR_START_DATA;
    const uintptr_t ram_end = ram_start + ADDR_SIZE_DATA;

    if (app_sp <= ram_start ||
        app_sp > ram_end ||
        (app_sp & 0xFu) != 0u) {
        bl_fail(BL_ERR_SP, "[BL] ERROR: stack inicial invalido");
    }

    const uintptr_t entry_min = slot_start + FW_HDR_SIZE;
    const uintptr_t image_end = slot_start + hdr->image_size;

    if (app_entry < entry_min ||
        app_entry > image_end - 4u ||
        (app_entry & 0x3u) != 0u) {
        bl_fail(BL_ERR_ENTRY, "[BL] ERROR: entrada invalida");
    }

    REG_FW_REPORT = BL_REPORT_IMG_OK;

    bl_log_hex("[BL] SP: ", app_sp);
    bl_log_hex("[BL] Entrada: ", app_entry);

    REG_FW_REPORT = BL_REPORT_JUMP;
    print_str("[BL] Salto a la aplicacion");

    bl_jump(app_sp, app_entry); // transferimos el control a la aplicación
}
