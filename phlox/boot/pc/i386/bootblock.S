/**
 ** Copyright 1998 Brian J. Swetland
 ** All rights reserved.
 **
 ** Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions
 ** are met:
 ** 1. Redistributions of source code must retain the above copyright
 **    notice, this list of conditions, and the following disclaimer.
 ** 2. Redistributions in binary form must reproduce the above copyright
 **    notice, this list of conditions, and the following disclaimer in the
 **    documentation and/or other materials provided with the distribution.
 ** 3. The name of the author may not be used to endorse or promote products
 **    derived from this software without specific prior written permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 ** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 ** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 ** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 ** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 ** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 ** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

/*
 ** Reviewed, documented and some minor modifications and bug-fixes
 ** applied by Michael Noisternig on 2001-09-02.
 */

/*
 ** Moved to gas syntax 2005, TG
 */

/* Default parameters patched from outside */
#define VESA                  1    /* 1 - enable VESA, 0 - disable VESA */
#define VESA_X_TARGET         640  /* screen width                      */
#define VESA_Y_TARGET         480  /* screen height                     */
#define VESA_BIT_DEPTH_TARGET 16   /* color depth                       */
#define READ_RETR_COUNT       3    /* read retry count on error         */

/* other parameters */
#define TARGET                0x11000  /* target where image be loaded */
#define BUFFER                0x8000   /* read buffer                  */
#define BOOTSEG               0x7c00   /* first part of this boot      */
#define BOOTSEG2              0x7e00   /* its second part              */
#define STACK                 0x10000  /* stack                        */
#define MEMINFO               0x7000   /* target for memory info data  */
#define VESAINFO              0x10000  /* target for vesainfo data     */

/* org is 0x7c00
 */
.code16
.globl _start
_start:
    jmp     start

sectors:
    .word 800 /* this is interpreted as data (bytes 3 and 4)
                 and patched from outside to the size of the bootcode (in sectors)
               */
start:
    cli                      /* no interrupts please */
    cld
    xor     %ax, %ax
    mov     %ax, %ss         /* setup stack from 0 - 0x7c00 */
    mov     $BOOTSEG, %sp    /* stack pointer to top of stack */
    call    enable_a20       /* enable a20 gate */
    lgdt    %ss:gdt          /* load global descriptor table */
    mov     %cr0, %eax       /* switch to protected mode */
    or      $0x1, %al        /* by setting 'protected mode enable' bit */
    mov     %eax, %cr0       /* and writing back */
.code32
    .byte 0x66
    ljmp    $0x8,$(unreal32)  /* do the actual switch into protected mode */
    /*
      The switch into protected mode and back is to get the processor into 'unreal' mode
      where it is in 16-bit mode but with segments that are larger than 64k.
      this way, the floppy code can load the image directly into memory >1Mb.
     */
.code16
unreal32:
    mov     $0x10, %bx        /* load all of the data segments with large 32-bit segment */
    mov     %bx, %ds
    mov     %bx, %es
    mov     %bx, %ss
    and     $0xfe, %al        /* switch back to real mode */
    mov     %eax, %cr0
    ljmp    $0x0,$(unreal)    /* actually go back to 16-bit */

unreal:
    xor     %ax, %ax          /* load NULL-descriptor (base 0) into ds, es, ss */
    mov     %ax, %es          /* the processor doesn't clear the internal high size bits on these descriptors */
    mov     %ax, %ds          /* so the descriptors can now reference 4Gb of memory, with size extensions */
    mov     %ax, %ss

    /* read in the second half of this stage of the bootloader */
    xor     %dx, %dx          /* start at head 0 */
    mov     $0x2, %bx         /* start at sector 2 for the second half of this loader */
    mov     $0x1, %cx         /* one sector */
    mov     $BOOTSEG2, %edi   /* right after this one */
    sti
    mov     $loadmsg, %si
    call    print             /* print message on loading */
    call    load_floppy

    /* read in the rest of the disk */
    mov     $TARGET, %edi     /* destination buffer for sector reading in load_floppy */
    mov     $0x3, %bx         /* start at sector 3 (and cylinder 0) */
    mov     sectors, %cx      /* read that much sectors */
    xor     %dx, %dx          /* start at head 0 */
    sti
    call    load_floppy       /* read remaining sectors at address edi */
    call    disable_floppy_motor
    mov     $okmsg, %si
    call    print

    /* use int 15, ax=e820 to find the memory map */
    call  find_mem_size_real

    /* try to probe vesa if the shift key is held */
    xor     %al, %al          /* check for vesa support requested */
    cmp     in_vesa, %al
    je      no_vesa
    in      $0x60, %al        /* read data from keyboard */
    and     $0x7f, %al
    cmp     $42, %al          /* try to set VESA mode if left shift key was pressed */
    mov     $0, %al
    jne     no_vesa
    call    enable_vesa
no_vesa:
    mov     %al, in_vesa

    cli

    /* switch back to protected mode and jump into our loaded image */
    mov     $0xcf, %al
    mov     %al, (gdt + 14)   /* set desc. 1 and 2 */
    mov     %al, (gdt + 22)   /* to 32-bit segment */
    lgdt    gdt               /* load global descriptor table */

    mov     %cr0, %eax
    or      $1, %al
    mov     %eax,%cr0         /* enter protected mode */

.code32
    .byte 0x66
    ljmp    $0x8,$(code32)    /* flush prefetch queue and enter 32-bit mode */

code32:
    /* load descriptors and set up the stack */
    mov     $0x10, %eax        /* load descriptor 2 in all segment selectors (except cs) */
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    mov     %ax, %ss
    mov     $STACK,%ebp
    mov     %ebp, %esp
    /* NOTE: After loading selectors only %al is not zero */

    /* push some arguments to the boot image */
    pushl   vesa_info

    mov     in_vesa, %al
    push    %eax

    mov     ext_mem_count, %al
    push    %eax

    mov     ext_mem_info, %edx
    push    %edx

    /* if we didn't find any info in the extended memory search, do a memory probe */
    xor     %eax, %eax        /* default memsize = 0 */
    or      %edx, %edx
    jnz     no_probe_mem      /* if ext_mem_info is a real pointer, we don't need to probe */

    call    find_mem_size_probe
no_probe_mem:
    push    %eax

    mov     $TARGET, %ebx
    call    *%ebx             /* jump to stage1 entry */
infinite:
    jmp     infinite

/* find memory size by testing
 * OUT: eax = memory size
 */
find_mem_size_probe:
    mov     $0x31323738, %eax /* test value */
    mov     $0x100ff0, %esi   /* start above conventional mem + HMA = 1 MB + 1024 Byte */
_fms_loop:
    mov     (%esi), %edx      /* read value */
    mov     %eax, (%esi)      /* write test value */
    mov     (%esi), %ecx      /* read it again */
    mov     %edx, (%esi)      /* write back old value */
    cmp     %eax, %ecx
    jnz     _fms_loop_out     /* read value != test value -> above mem limit */
    add     $0x1000, %esi     /* test next page (4 K) */
    jmp     _fms_loop
_fms_loop_out:
    mov     %esi, %eax
    sub     $0x1000, %eax
    add     $0x10, %eax
    ret

.code16
/* read sectors into memory
 * IN: bx = sector # to start with: should be 2 as sector 1 (bootsector) was read by BIOS
 *     cx = # of sectors to read
 *     edi = buffer
 */
load_floppy:
    push    %bx
    push    %cx
tryagain:
    mov     $0x13, %al        /* read a maximum of 18 sectors */
    sub     %bl, %al          /* substract first sector (to prevent wrap-around ???) */

    xor     %ah, %ah          /* TK: don't read more then required, VMWare doesn't like that */
    cmp     %cx, %ax
    jl      shorten
    mov     %cx, %ax
shorten:

    mov     %bx, %cx          /*   -> sector/cylinder # to read from */
    mov     $BUFFER, %bx      /* buffer address */
    mov     $0x2, %ah         /* command 'read sectors' */
    push    %ax
    int     $0x13             /* call BIOS */
    pop     %ax               /* TK: should return number of transferred sectors in al */
                              /* but VMWare 3 clobbers it, so we (re-)load al manually */
    jnc     okok              /* no error -> proceed as usual */
    decb    retrycnt
    jz      fail
    xor     %ah, %ah          /* reset disk controller */
    int     $0x13
    jmp     tryagain          /* retry */
okok:
    movb    $3, retrycnt      /* reload retrycnt */
    movw    $dot, %si
    call    print
    mov     $BUFFER, %esi     /* source */
    xor     %ecx, %ecx
    mov     %al, %cl          /* copy # of read sectors (al) */
    shl     $0x7, %cx         /* of size 128*4 bytes */
    rep addr32 movsd          /* to destination (edi) setup before func3 was called */
    pop     %cx
    pop     %bx
    xor     $0x1, %dh         /* read: next head */
    jnz     bar6
    inc     %bh               /* read: next cylinder */
bar6:
    mov     $0x1, %bl         /* read: sector 1 */
    xor     %ah, %ah
    sub     %ax, %cx          /* substract # of read sectors */
    jg      load_floppy       /* sectors left to read ? */
    ret

disable_floppy_motor:
    xor     %al, %al
    mov     $0x3f2, %dx       /* disable floppy motor */
    out     %al, %dx
    ret

/* prints message in reg. si */
print:
    pusha
_n:
    lodsb
    or      %al, %al
    jz      _e
    mov     $0x0e, %ah
    mov     $7, %bx
    int     $0x10
    jmp     _n
_e:
    popa
    ret

/* print errormsg, wait for keypress and reboot */
fail:
    mov     $errormsg, %si
    call    print
    xor     %ax, %ax
    int     $0x16
    int     $0x19

/* enables the a20 gate
 * the usual keyboard-enable-a20-gate-stuff
 */
enable_a20:
    call    _a20_loop
    jnz     _enable_a20_done
    mov     $0xd1, %al
    out     %al, $0x64
    call    _a20_loop
    jnz     _enable_a20_done
    mov     $0xdf, %al
    out     %al, $0x60
_a20_loop:
    mov     $0x20000, %ecx
_loop2:
    jmp     _c
_c:
    in      $0x64, %al
    test    $0x2, %al
    loopne  _loop2
_enable_a20_done:
    ret

loadmsg:
    .ascii  "Booting Phlox \0"
errormsg:
    .ascii  "\n\rDisk read error.\n\r\0"
okmsg:
    .ascii  "OK\n\r\0"
dot:
    .ascii  "#\0"
gdt:
    /* the first entry serves 2 purposes: as the GDT header and as the first descriptor
     * note that the first descriptor (descriptor 0) is always a NULL-descriptor
     */
    .word 0xffff      /* full size of GDT used */
    .word gdt         /* address of GDT (dword) */
    .long 0
    /* descriptor 1: */
    .long 0x0000ffff  /* base - limit: 0 - 0xfffff * 4K */
    .long 0x008f9a00  /* type: 16 bit, exec-only conforming, <present>, privilege 0 */
    /* descriptor 2: */
    .long 0x0000ffff  /* base - limit: 0 - 0xfffff * 4K */
    .long 0x008f9200  /* type: 16 bit, data read/write, <present>, privilege 0 */

/***************/
.skip (_start + 510 - .)
    .word   0xaa55    /* magic number for boot sector */


/******** second sector of the boot code *********/

.code16

/* data block of second sector */

in_vesa:
    .byte VESA                  /* this is interpreted as data (byte 0 of second sector)
                                   and patched to 0 (not enable vesa) or 1 (enable vesa)
                                 */
vesa_x:
    .word VESA_X_TARGET         /* this is interpreted as data (bytes 1 and 2 of second sector)
                                   and patched to expected screen width
                                 */
vesa_y:
    .word VESA_Y_TARGET         /* this is interpreted as data (bytes 3 and 4 of second sector)
                                   and patched to expected screen height
                                 */

vesa_bit_depth:
    .byte VESA_BIT_DEPTH_TARGET /* this is interpreted as data (bytes 5 and 6 of second sector)
                                   and patched to expected color depth
                                 */
retrycnt:
    .byte READ_RETR_COUNT       /* this is interpreted as data (byte 7 of second sector)
                                   and patched to retry count on disk read error
                                 */
/* other data used in code */
vesa_info:
    .long 0

ext_mem_info:
    .long 0

ext_mem_count:
    .byte 0

/* end of data block */

    /* use int 0x15, EAX=0xe820 to test for memory
     * assumes es is null
     */
find_mem_size_real:
    mov     $0, %ebx
    mov     $MEMINFO, %edi      /* the extended memory structures will go at MEMINFO */
    mov     %edi, ext_mem_info

find_mem_next:
    mov     $0xe820, %eax
    mov     $0x534d4150, %edx   /* 'SMAP' in the correct order */
    mov     $0x20, %ecx

    int     $0x15
    jc      done_mem_real       /* if carry is set, it wasn't supported */

    incb    ext_mem_count       /* increment the count of the number */

    add     $0x20, %edi         /* increment the buffer by 0x20 */

    or      %ebx, %ebx          /* test if we're done */
    jne     find_mem_next

done_mem_real:
    ret

/* fool around with vesa mode */
enable_vesa:
    /* put the VBEInfo struct at VESAINFO address */
    mov     $VESAINFO, %eax
    mov     %eax, vesa_info
    mov     $0x3000, %dx
    mov     %dx, %es
    mov     $0x4f00, %ax
    mov     $0, %di
    int     $0x10

    /* check the return code */
    cmp     $0x4f, %al
    jne     done_vesa_bad
    cmp     $0x00, %ah
    jne     done_vesa_bad

    /* check the signature on the data structure */
    mov     %es:0, %eax
    cmp     $0x41534556, %eax   /* 'VESA' */
    je      vesa_sig_ok
    cmp     $0x32454256, %eax   /* 'VBE2' */
    jne     done_vesa_bad

vesa_sig_ok:
    /* scan through each mode and grab the info on them */
    les     %es:14, %bx /* calculate the pointer to the mode list */
    mov     $0x200, %di /* push the buffer up a little to be past the VBEInfo struct */

mode_loop:
    mov     %es:(%bx), %cx  /* grab the next mode in the list */
    cmp     $0xffff, %cx
    je      done_vesa_bad
    and     $0x01ff, %cx
    mov     $0x4f01, %ax
    int     $0x10

    /* if it matches what we're looking for, go for it */
    mov     %es:(%di), %ax
    test    $0x1, %ax        /* test the supported bit */
    jz      next_mode
    test    $0x08, %ax       /* test the linear frame mode bit */
    jz      next_mode
    mov     %es:18(%di), %ax
    cmp     vesa_x, %ax          /* x */
    jne     next_mode
    mov     %es:20(%di), %ax
    cmp     vesa_y, %ax          /* y */
    jne     next_mode
    mov     %es:25(%di), %al
    cmp     vesa_bit_depth, %al  /* bit_depth */
    jne     next_mode

    /* looks good, switch into it */
    mov     $0x4f02, %ax
    mov     %cx, %bx
    or      $0x4000, %bx                 /* add the linear mode bit */
    int     $0x10
    cmp     $0x004f, %ax
    je      done_vesa_good

next_mode:
    /* get ready to try the next mode */
    inc     %bx
    inc     %bx
    jmp     mode_loop

done_vesa_good:
    mov     $0x1, %ax
    ret

done_vesa_bad:
    xor     %ax, %ax
    ret

.skip (_start + 1024 - .)
