#include "fw_image.h"

#define REG_FW_STATUS_ADDR  0x00040000      /* project_config.def: REG_FW_STATUS */
#define FW_STATUS_PASS      0xFF

/* ------------------------------------------------------------------------ */
/*  1) Header + vector table del FW (app.ld lo pone al inicio de APP_ROM)    */
/* ------------------------------------------------------------------------ */
    .section .fw_header, "a"
    .balign 4
    .globl  __fw_header
    .type   __fw_header, @object
__fw_header:
    /*
     *   ¿Cuáles salen de símbolos del linker? ¿Qué valor ponen en image_crc,
     *   si lo escribe tools/mkimage.py DESPUÉS de linkear? 
     *   Los que se definen en el linker script son:  __fw_start, __fw_size 
     *   y _stack_top aunque Reset_Handler requiere que el linker resuelva
     *   su valor. En image_crc usamos FW_IMAGE_CRC_UNSET = 0xFFFFFFFF, 
     *   un marcador distinto de cualquier resultado CRC-8 válido
     */
    .word   FW_IMAGE_MAGIC      
    .word   __fw_start           
    .word   __fw_size           
    .word   FW_IMAGE_CRC_UNSET   
    .word   _stack_top          
    .word   Reset_Handler       
    .size   __fw_header, . - __fw_header

/* ------------------------------------------------------------------------ */
/*  2) Reset_Handler: "De 0 a main" (diapos 5 y 6)                          */
/* ------------------------------------------------------------------------ */
    .section .text.Reset_Handler, "ax"
    .balign 4                       /* contrato: reset_handler alineado a 4 */
    .globl  Reset_Handler
    .type   Reset_Handler, @function
Reset_Handler:
    la      sp, _stack_top

    /* pasamos las direcciones de los simbolos a t0, t1 y t2 */
    la      t0, _sidata
    la      t1, _sdata 
    la      t2, _edata 

.Lcopy_data:
    bgeu    t1, t2, .Ldata_done /* comparar limite antes de acceder a memoria */

    lw      t3, 0(t0)
    sw      t3, 0(t1)

    addi    t0, t0, 4
    addi    t1, t1, 4
    j       .Lcopy_data

.Ldata_done:
    la      t0, _sbss 
    la      t1, _ebss 

.Lzero_bss:
    bgeu    t0, t1, .Lbss_done

    sw      zero, 0(t0)

    addi    t0, t0, 4
    j       .Lzero_bss

.Lbss_done:
    call    main

    /* main retorno: comprobar la señal de finalizacion del TP. */
    li      t0, REG_FW_STATUS_ADDR
    lw      t1, 0(t0)
    li      t2, FW_STATUS_PASS

    beq     t1, t2, .Lapp_halt
    ebreak

.Lapp_halt:
    j       .Lapp_halt

    .size   Reset_Handler, . - Reset_Handler

    /*   
     *     sp = _stack_top. ¿Hace falta si el BL ya lo cargó desde la vector table?
     *     El bootloader carga el stack indicado por la cabecera antes de transferir el control. 
     *     La aplicación vuelve a cargar sp para establecer explícitamente su entorno de ejecución. 
     *     Aunque ambas cargas producen el mismo valor en este flujo, 
     *     la segunda evita depender exclusivamente del estado recibido del bootloader.
     *
     *     si main volvió y REG_FW_STATUS no quedó en FW_STATUS_PASS: ebreak.
     *     (¿qué pasa con la simulación si no lo hacen?)
     *     Si no lo hacemos podríamos quedarnos en un bucle y la simulacion
     *     podria seguir esperando una señal que no va a llegar
     */
