OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(_start)
SECTIONS
{
    . = 0x11000;

    /* startup entry code */
    .startup_entry :    { *(.startup_entry) }

    /* text/read-only data */
    .text :    { *(.text .gnu.linkonce.t.*) }

    __ctor_list = .;
    .ctors : { *(.ctors) }
    __ctor_end = .;

    .rodata : { *(.rodata .rodata.*) }

    /* writable data  */
    . = ALIGN(0x1000);
    __data_start = .;
    .data : { *(.data .gnu.linkonce.d.* .bss) }

    /* . = ALIGN(0x1000); */
    _end = . ;
    _compressed_image = . ;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame .dtors .stab .stabstr .debug*) }
}
