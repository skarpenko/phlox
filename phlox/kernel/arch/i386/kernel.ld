OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(_phlox_kernel_entry)
SEARCH_DIR("libgcc");
SECTIONS
{
    . = 0xC0000000;

    /* text/read-only data */
    .text : { *(.text .gnu.linkonce.t.*) } = 0x90909090

    __ctor_list = .;
    .ctors : { *(.ctors) }
    __ctor_end = .;

    .rodata : { *(.rodata .rodata.*) }

    /* writable data  */
    . = ALIGN(0x1000);
    __data_start = .;
    .data : { *(.data .gnu.linkonce.d.*) }
    __data_end = .;

    /* unintialized data (in same segment as writable data) */
    __bss_start = .;
    .bss : { *(.bss) }
    __bss_end = .;

    . = ALIGN(0x1000);
    _end = .;
    _global_bootfs_vaddress = .;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame .dtors .stab .stabstr .debug*) }
}
