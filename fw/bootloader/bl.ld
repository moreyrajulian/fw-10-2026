ENTRY(_bl_vectors)

INCLUDE memory_map.ld

SECTIONS
{
  .text ORIGIN(BL_ROM) : {
    KEEP(*(.vectors))
    *(.text .text.*)
    *(.rodata .rodata.* .srodata .srodata.*)
    . = ALIGN(4);
  } > BL_ROM

  .data : {
    KEEP(*(.data .data.* .sdata .sdata.*))
} > RAM

  .bss (NOLOAD) : {
    KEEP(*(.bss .bss.* .sbss .sbss.*))
    KEEP(*(COMMON))
} > RAM

  /DISCARD/ : { *(.eh_frame .eh_frame_hdr) }
}

_bl_stack_top = ORIGIN(RAM) + LENGTH(RAM);

__fw_slot_start = ORIGIN(APP_ROM);
__fw_slot_size  = LENGTH(APP_ROM);

ASSERT(_bl_vectors == ORIGIN(BL_ROM),
       "Los vectores del BL no quedaron al inicio de BL_ROM")

ASSERT(SIZEOF(.data) == 0,
       "El BL contiene datos modificables inicializados")

ASSERT(SIZEOF(.bss) == 0,
       "El BL contiene datos modificables que requieren cero")



