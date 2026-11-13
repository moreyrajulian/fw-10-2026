/*
 * bl_start.S — TP4 (Bloque 4): vector table + arranque mínimo del BOOTLOADER
 *
 * Vector table, arranque y transferencia al firmware.
 *
 * Datos de la plataforma (platform/rtl/top.v, parámetros de picorv32):
 *   - PROGADDR_RESET = 0x0000_0000: al salir de reset el core EJECUTA lo que hay ahí.
 *   - PROGADDR_IRQ   = 0x0000_0010 (ENABLE_IRQ = 0: no hay IRQs).
 *   - STACKADDR      = 0xFFFF_FFFF: el core NO inicializa sp.
 */

#define REG_FW_STATUS_ADDR  0x00040000      /* project_config.def: REG_FW_STATUS */
#define FW_STATUS_FAIL      0xBAD           /* convención de los tests de platform/fw */

/* ------------------------------------------------------------------------ */
/*  Vector table del BL (bl.ld la tiene que ubicar en 0x0000_0000)          */
/* ------------------------------------------------------------------------ */
    .section .vectors, "ax"
    .option push
    .option norvc                   /* instrucciones de 4 bytes: los offsets importan */
    .globl  _bl_vectors
    .type   _bl_vectors, @function
_bl_vectors:
    j       bl_reset_handler
    j       bl_unexpected
    j       bl_unexpected
    j       bl_unexpected
    j       bl_unexpected

    .size   _bl_vectors, . - _bl_vectors
    .option pop

    /*
     *   - 0x04, 0x08 y 0x0C no son vectores en PicoRV32: ¿qué conviene poner?
     *   Conviene poner un salto hacia algún handler bl_unexpected que informa un error y detiene la ejecución
     *   Pregunta: ¿por qué acá la vector table son INSTRUCCIONES y en Cortex-M
     *   son DIRECCIONES? Se basa en lo que hace el hardware al arrancar, el PicoRV32
     *   busca y ejecuta una instrucción en la dirección de reset. Mientras que el 
     *   Cortex-M interpreta las primeras entradas como el valor inicial del stack y la dirección del handler de reset.
     */

/* ------------------------------------------------------------------------ */
/*  Reset: init mínimo del BL                                               */
/* ------------------------------------------------------------------------ */
    .section .text.bl_reset_handler, "ax"
    .globl  bl_reset_handler
    .type   bl_reset_handler, @function
bl_reset_handler:
    la sp, _bl_stack_top
    call bl_main
    j       bl_unexpected          
    .size   bl_reset_handler, . - bl_reset_handler


    /* 
    *  Cargar sp con _bl_stack_top (lo define bl.ld). ¿Por qué lo tiene que
    *  hacer el software?
    *  El software prepara sp porque el core no lo inicializa

    *  data/.bss del BL: si el BL no usa variables globales, no hay nada
    *  que copiar ni poner en cero (¿cómo garantizan que no las use?)
    *  los ASSERT del linker impiden datos estáticos modificables que necesiten copia o borrado
    *   
    */

/* ------------------------------------------------------------------------ */
/*  void bl_jump(uint32_t sp, uint32_t entry)  __attribute__((noreturn))    */
/* ------------------------------------------------------------------------ */
    .section .text.bl_jump, "ax"
    .globl  bl_jump
    .type   bl_jump, @function
bl_jump:
    mv      sp, a0
    la      ra, bl_unexpected
    jr      a1

    .size   bl_jump, . - bl_jump

    /*
     * La ABI pasa el primer argumento en a0 y el segundo en a1.
     * Cargamos sp desde a0 y transferimos el control a la direccion de a1.
     * Usamos jr porque la aplicacion no debe retornar al llamador del BL.
     * Dejamos bl_unexpected en ra como destino de un retorno inesperado.
    */

/* ------------------------------------------------------------------------ */
/*  bl_unexpected: vector inesperado -> FAIL + trap   (YA ESTÁ HECHO)       */
/*  void bl_halt(void): corta la simulación           (YA ESTÁ HECHO)       */
/* ------------------------------------------------------------------------ */
    .section .text.bl_halt, "ax"
    .globl  bl_unexpected
    .type   bl_unexpected, @function
bl_unexpected:
    li      t0, REG_FW_STATUS_ADDR
    li      t1, FW_STATUS_FAIL
    sw      t1, 0(t0)
    /* sigue en bl_halt */
    .globl  bl_halt
    .type   bl_halt, @function
bl_halt:
    ebreak                          /* Pregunta: ¿por qué hace falta el ebreak si ya */
                                    /* dejamos 0xBAD en REG_FW_STATUS?               */
                                    /*
                                     * El BL no escribe 0xFF porque el testbench lo interpreta como
                                     * finalización exitosa de la prueba. Validar la imagen todavía
                                     * no demuestra que la aplicación se haya ejecutado correctamente.
                                    */
1:  j       1b
    .size   bl_halt, . - bl_halt
    .size   bl_unexpected, . - bl_unexpected
