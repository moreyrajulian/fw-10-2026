ENTRY(Reset_Handler)

INCLUDE memory_map.ld

SECTIONS
{
  .text ORIGIN(APP_ROM) : {
    /* __fw_start y el header + vector table del FW (seccion .fw_header
     * de app_startup.S) PRIMERO, al inicio de APP_ROM. ¿Por que KEEP? */
      /*
      * La imagen comienza en ORIGIN(APP_ROM), en 0x1000.
      * __fw_start registra esa direccion.
      *
      * Recogemos .fw_header antes del codigo para que el bootloader
      * encuentre la cabecera y los datos de entrada en posiciones conocidas.
      * KEEP evita que --gc-sections descarte esa seccion aunque no exista
      * una referencia desde el codigo de la aplicacion.
      */
    /*  ¿que otra seccion de solo lectura genera GCC en RISC-V? (hito 1) */
      /*
      * Incluimos .srodata y .srodata.* para recoger tambien las secciones
      * de datos pequenos de solo lectura que puede generar GCC para RISC-V.
      * Asi quedan contempladas explicitamente junto con .rodata.
      */
    __fw_start = .;

    KEEP(*(.fw_header))

    *(.text .text.*)
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)

    . = ALIGN(4);
  } > APP_ROM

  /*
  * .data reside en RAM durante la ejecucion y conserva sus valores
  * iniciales en APP_ROM. El startup copiara desde _sidata hacia
  * el intervalo [_sdata, _edata).
  */

  .data : ALIGN(4) {
    _sdata = .;

    *(.data .data.*)
    *(.sdata .sdata.*)

    . = ALIGN(4);
    _edata = .;
  } > RAM AT > APP_ROM

  _sidata = LOADADDR(.data);

  /*
  * .bss reserva espacio en RAM sin almacenar valores iniciales
  * en la imagen. El startup pondra en cero [_sbss, _ebss).
  * Incluimos .sbss y COMMON para abarcar esas variantes de entrada.
  */

  .bss (NOLOAD) : ALIGN(4) {
    _sbss = .;

    *(.bss .bss.*)
    *(.sbss .sbss.*)

    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
  } > RAM

  /DISCARD/ : { *(.eh_frame .eh_frame_hdr) }
}

/*
 * La imagen cargable termina despues de los valores iniciales de .data.
 * Su tamaño no incluye .bss ni el stack.
 *
 * El stack comienza en el limite superior de RAM. El ASSERT exige
 * que los datos estaticos dejen al menos 1 KiB disponible para el.
 * Esta comprobacion no detecta desbordamientos durante la ejecucion.
 */

__fw_end   = LOADADDR(.data) + SIZEOF(.data); 
__fw_size  = __fw_end - __fw_start;
_stack_top = ORIGIN(RAM) + LENGTH(RAM);

_stack_min_size = 1024;

ASSERT(_ebss <= (_stack_top - _stack_min_size),
       "La aplicacion no deja al menos 1 KiB para el stack")
