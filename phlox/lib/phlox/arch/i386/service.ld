OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(_start)
STARTUP(arch_start.o)
SEARCH_DIR("libgcc")


SECTIONS
{
    /* User space starts at second page */
    . = 0x1000;

    /* text */
    .text : { *(.text .text.* .gnu.linkonce.t.*) }

    . = ALIGN(0x1000);

    /* global constructors list */
    __ctor_list = .;
    .ctors : { CONSTRUCTORS *(.ctors .ctors.*) LONG(0) }
    __ctor_end = .;

    /* global destructors list */
    __dtor_list = .;
    .dtors : { *(.dtors .dtors.*) LONG(0) }
    __dtor_end = .;

    /* read only data  */
    . = ALIGN(0x1000);
    .rodata : { *(.rodata .rodata.*) }

    /* writable data  */
    . = ALIGN(0x1000);
    __data_start = .;
    .data : { *(.data .data.* .gnu.linkonce.d.*) }
    __data_end = .;

    /* unintialized data (in same segment as writable data) */
    . = ALIGN(0x1000);
    __bss_start = .;
    .bss : { *(.bss .bss.*) }
    __bss_end = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame .dtors .stab .stabstr .debug*) }
}
