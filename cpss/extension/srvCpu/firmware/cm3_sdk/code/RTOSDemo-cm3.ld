MEMORY
{
    SRAM (rwx) : ORIGIN = 0x00000000 LENGTH = 126K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.exceptions_area))
        *(.text*)
        *(.rodata*)
    } > SRAM

    /* for exception handling/unwind - some Newlib functions (in common with C++ and STDC++) use this. */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > SRAM

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > SRAM

    _etext = .;

    .ARM.attributes :
    {
        *(.ARM.attributes* .gnu.linkonce.armattributes.*)
    } > SRAM

    .data :
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM

    stackPtr = 126K;
}
