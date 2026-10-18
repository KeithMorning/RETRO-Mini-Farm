ENTRY(reset_handler)

MEMORY
{
    rom (rx)  : ORIGIN = 0x08010000, LENGTH = 448K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
}

_estack = ORIGIN(ram) + LENGTH(ram);

SECTIONS
{
    .vectors : {
        KEEP(*(.vectors))
    } > rom

    .text : {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > rom

    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > rom

    .ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > rom

    .data : {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > ram AT > rom
    _sidata = LOADADDR(.data);

    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > ram
}
