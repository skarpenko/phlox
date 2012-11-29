/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/ktypes.h>
#include <phlox/elf32.h>
#include <phlox/kargs.h>
#include <boot/bootfs.h>
#include "vesa_vbe.h"
#include "kinit_private.h"

/* controls additional info output */
#define PRINT_BIOS_MEM_MAP  1
#define PRINT_KINIT_SUMMARY 1

/* need for console stuff */
static unsigned short *screenBase = (unsigned short *) 0xb8000;
static unsigned int    screenOffset = 0;
static unsigned int    line = 0;

/* address of temporary page with kernel arguments */
kernel_args_t *kargs = (kernel_args_t *)0x11000;

/* page directory and page table */
static mmu_pde *pgdir   = NULL;
static mmu_pte *pgtable = NULL;

/* address of temporary MMU storage */
#define MMU_TMPDATA  0x12000 /* next page after kernel args */

/* MMU operations (stolen from NewOS's stage2) */
static int mmu_init(kernel_args_t *ka, uint32 *next_physaddr);
static void mmu_map_page(uint32 virt_addr, uint32 phys_addr);

/* ELF */
static void load_elf_image(void *elf_data, uint32 *next_physaddr, addr_range_t *range, uint32 *entry_addr);

/* Miscellaneous */
static void sort_addr_ranges(addr_range_t *ranges, uint32 count);


/*
 * State:
 * 32-bit mode
 * Supervisor mode
 * Stack somewhere below 1Mb
 * Paging disabled
 *
*/
void _start(uint32 memsize, void *ext_mem_block, uint32 ext_mem_count, int in_vesa, uint32 vesa_ptr, uint32 console_ptr) {
    bootfs_t btfs;
    btfs_dir_entry *en;
    uint32 next_physaddr;
    uint32 next_virtaddr;
    cpu_seg_desc  gdt_desc;
    cpu_seg_desc  *gdt;  /* pointer to Global Descriptor Table */
    cpu_gate_desc idt_desc;
    cpu_gate_desc *idt;  /* pointer to Interrupt Descriptor Table */
    uint32 i;
    uint32 kentry;                    /* kernel entry point */
    uint32 kstack_start, kstack_size; /* kernel stack       */
    addr_range_t krange;              /* virtual address range
                                       * occupied by kernel
                                       */

    asm("cld");  /* Ain't nothing but a GCC thang. (from NewOS's stage2) */
    asm("cli");  /* ensure that interrupts is still masked. */
    
    screenOffset = console_ptr;

    memset(kargs, 0, PAGE_SIZE); /* prepare kernel args page */

    dprintf("\n= kinit started =\n");
    dprintf("remounting bootfs...");
    if( btfs_mount(BTFS_IMAGE, &btfs) )
      panic("error remounting bootfs.");
    else {
      if( btfs.hdr->psize != PAGE_SIZE )
        panic("bootfs image damaged.");
      else
        dprintf("OK\n");
    }

    /* next free physical address is right after bootfs image */
    next_physaddr = (uint32)BTFS_IMAGE + btfs.hdr->fsize*PAGE_SIZE;
    /* store image allocation info into kernel args */
    kargs->btfs_image_addr.start = (uint32)BTFS_IMAGE;
    kargs->btfs_image_addr.size  = btfs.hdr->fsize*PAGE_SIZE;

    /* reserve a page for idt */
    idt = (cpu_gate_desc *)next_physaddr;
    next_physaddr += PAGE_SIZE;
    memset(idt, 0, PAGE_SIZE); /* init page to 0 */
    kargs->arch_args.phys_idt = (uint32)idt;

    /* reserve a page for gdt */
    gdt = (cpu_seg_desc *)next_physaddr;
    next_physaddr += PAGE_SIZE;
    memset(gdt, 0, PAGE_SIZE); /* init page to 0 */
    kargs->arch_args.phys_gdt = (uint32)gdt;

    /* init memory management unit */
    if(mmu_init(kargs, &next_physaddr))
        panic("mmu init failed!\n");

    /* locate kernel binary */
    en = btfs_locate("phloxk");
    if(!en)
       panic("kernel binary not found.\n");

    /* load and map kernel image */
    load_elf_image(btfs_objaddr(en), &next_physaddr, &krange, &kentry);

    /* store kernel image info into kernel args */
    kargs->phys_kernel_addr.start = ROUNDOWN(next_physaddr - krange.size, PAGE_SIZE);
    kargs->phys_kernel_addr.size  = krange.size;
    kargs->virt_kernel_addr.start = krange.start;
    kargs->virt_kernel_addr.size  = krange.size;

    /* next free virtual address is right after kernel */
    next_virtaddr = ROUNDUP(krange.start + krange.size, PAGE_SIZE);

    dprintf("Kernel entry point address: 0x%X\n", kentry);

    /* map kernel stack into virtual space */
    kstack_start = next_virtaddr;
    kargs->phys_cpu_kstack[0].start = next_physaddr;
    kargs->virt_cpu_kstack[0].start = next_virtaddr;
    for(i=0; i<KERNEL_STACK_SIZE; i++) {
      mmu_map_page(next_virtaddr, next_physaddr);
      next_physaddr += PAGE_SIZE;
      next_virtaddr += PAGE_SIZE;
    }
    kstack_size = next_virtaddr - kstack_start;
    kargs->phys_cpu_kstack[0].size = kstack_size;
    kargs->virt_cpu_kstack[0].size = kstack_size;

    /* map idt into virtual space. it will be filled by kernel later. */
    mmu_map_page(next_virtaddr, (uint32)idt);
    kargs->arch_args.virt_idt = next_virtaddr;
    next_virtaddr += PAGE_SIZE;
    /* load new idt */
    idt_desc.raw.dword0 = MAX_IDT_LIMIT-1;
    idt_desc.raw.dword1 = kargs->arch_args.virt_idt;
    asm("lidt   %0; "
        : : "m" (idt_desc));

    /* build and map new gdt into virtual space */
    gdt[0].raw.dword0 = 0;           /* NULL descriptor            */
    gdt[0].raw.dword1 = 0;
    gdt[1].raw.dword0 = 0x0000ffff;  /* kernel 4Gb code (seg 0x8)  */
    gdt[1].raw.dword1 = 0x00cf9a00;
    gdt[2].raw.dword0 = 0x0000ffff;  /* kernel 4Gb data (seg 0x10) */
    gdt[2].raw.dword1 = 0x00cf9200;
    gdt[3].raw.dword0 = 0x0000ffff;  /* user 4Gb code (seg 0x18)   */
    gdt[3].raw.dword1 = 0x00cffa00;
    gdt[4].raw.dword0 = 0x0000ffff;  /* user 4Gb data (seg 0x20)   */
    gdt[4].raw.dword1 = 0x00cff200;
    /* map into virtual space */
    mmu_map_page(next_virtaddr, (uint32)gdt);
    kargs->arch_args.virt_gdt = next_virtaddr;
    next_virtaddr += PAGE_SIZE;
    /* load new gdt */
    gdt_desc.raw.dword0 = MAX_GDT_LIMIT-1;
    gdt_desc.raw.dword1 = kargs->arch_args.virt_gdt;
    asm("lgdt   %0; "
        : : "m" (gdt_desc));

    /* map page directory into virtual space */
    mmu_map_page(next_virtaddr, (uint32)pgdir);
    kargs->arch_args.virt_pgdir = next_virtaddr;
    next_virtaddr += PAGE_SIZE;

    /* store VESA VBE params in kargs (for more details refer to VBE standard) */
    if(in_vesa) {
        ModeInfoBlock_t *mode_info = (ModeInfoBlock_t *)(vesa_ptr + 0x200);

        kargs->fb.enabled                 = 1;
        kargs->fb.x_size                  = mode_info->XResolution;
        kargs->fb.y_size                  = mode_info->YResolution;
        kargs->fb.bit_depth               = mode_info->BitsPerPixel;
        kargs->fb.red_mask_size           = mode_info->RedMaskSize;
        kargs->fb.red_field_position      = mode_info->RedFieldPosition;
        kargs->fb.green_mask_size         = mode_info->GreenMaskSize;
        kargs->fb.green_field_position    = mode_info->GreenFieldPosition;
        kargs->fb.blue_mask_size          = mode_info->BlueMaskSize;
        kargs->fb.blue_field_position     = mode_info->BlueFieldPosition;
        kargs->fb.reserved_mask_size      = mode_info->RsvdMaskSize;
        kargs->fb.reserved_field_position = mode_info->RsvdFieldPosition;
        kargs->fb.mapping.start           = mode_info->PhysBasePtr;
        kargs->fb.mapping.size            = kargs->fb.x_size *
                                            kargs->fb.y_size *
                                            ((kargs->fb.bit_depth+7)/8);
        kargs->fb.already_mapped          = 0;
    } else {
        kargs->fb.enabled = 0;
    }

    /** Prepare memory map to pass into kernel (this section completely taken from NewOS's Stage2)**/
    /* mark memory that we know is used */
    kargs->phys_alloc_range[0].start = (uint32)BTFS_IMAGE;
    kargs->phys_alloc_range[0].size = next_physaddr - (uint32)BTFS_IMAGE;
    kargs->num_phys_alloc_ranges = 1;

    /* figure out the memory map */
    if(ext_mem_count > 0) {
        struct ext_mem_struct *buf = (struct ext_mem_struct *)ext_mem_block;

      #if PRINT_BIOS_MEM_MAP
        /* print RAM map provided by BIOS */
        dprintf("BIOS-provided physical RAM map:\n");
        for(i = 0; i < ext_mem_count; i++) {
          dprintf(" BIOS-e820: 0x%016LX - 0x%016LX", buf[i].base_addr, buf[i].length);
          if(buf[i].type == 1)
            dprintf(" (usable)\n");
          else
            dprintf(" (reserved)\n");
        }
        dprintf("===\n");
      #endif

        kargs->num_phys_mem_ranges = 0;

        for(i = 0; i < ext_mem_count; i++) {
          if(buf[i].type == 1) {
             /* round everything up to page boundaries, exclusive of pages it partially occupies */
             buf[i].length   -= (buf[i].base_addr % PAGE_SIZE) ? (PAGE_SIZE - (buf[i].base_addr % PAGE_SIZE)) : 0;
             buf[i].base_addr = ROUNDUP(buf[i].base_addr, PAGE_SIZE);
             buf[i].length    = ROUNDOWN(buf[i].length, PAGE_SIZE);

             /* this is mem we can use */
             if(kargs->num_phys_mem_ranges == 0) {
                 kargs->phys_mem_range[0].start = (addr_t)buf[i].base_addr;
                 kargs->phys_mem_range[0].size  = (addr_t)buf[i].length;
                 kargs->num_phys_mem_ranges++;
             } else {
                 /* we might have to extend the previous hole */
                 addr_t previous_end = kargs->phys_mem_range[kargs->num_phys_mem_ranges-1].start +
                                       kargs->phys_mem_range[kargs->num_phys_mem_ranges-1].size;
                 if(previous_end <= buf[i].base_addr &&
                   ((buf[i].base_addr - previous_end) < 0x100000)) {
                     /* extend the previous buffer */
                     kargs->phys_mem_range[kargs->num_phys_mem_ranges-1].size += (buf[i].base_addr - previous_end) +
                                                                                 buf[i].length;
                         
                     /* mark the gap between the two allocated ranges in use */
                     kargs->phys_alloc_range[kargs->num_phys_alloc_ranges].start = previous_end;
                     kargs->phys_alloc_range[kargs->num_phys_alloc_ranges].size  = buf[i].base_addr - previous_end;
                     kargs->num_phys_alloc_ranges++;
                 }
             }
        }
    }
    } else {
        /* we dont have an extended map, assume memory is contiguously mapped at 0x0 */
        kargs->phys_mem_range[0].start = 0;
        kargs->phys_mem_range[0].size = memsize;
        kargs->num_phys_mem_ranges = 1;

        /* mark the bios area allocated */
        kargs->phys_alloc_range[kargs->num_phys_alloc_ranges].start = 0x9f000; /* 640k - 1 page */
        kargs->phys_alloc_range[kargs->num_phys_alloc_ranges].size  = 0x61000;
        kargs->num_phys_alloc_ranges++;
    }

    /* save the memory we've virtually allocated (for the kernel and other stuff) */
    kargs->virt_alloc_range[0].start = KERNEL_BASE;
    kargs->virt_alloc_range[0].size = next_virtaddr - KERNEL_BASE;
    kargs->num_virt_alloc_ranges = 1;

    /* sort the address ranges */
    sort_addr_ranges(kargs->phys_mem_range,   kargs->num_phys_mem_ranges);
    sort_addr_ranges(kargs->phys_alloc_range, kargs->num_phys_alloc_ranges);
    sort_addr_ranges(kargs->virt_alloc_range, kargs->num_virt_alloc_ranges);

    /* write total memory count into kernel args */
    kargs->memsize = kargs->phys_mem_range[kargs->num_phys_mem_ranges-1].size;

  #if PRINT_KINIT_SUMMARY
    /* print out kinit summary */
    dprintf("\nkinit summary:\n");
    dprintf(" BootFS image phys.addr.: 0x%08X - 0x%08X\n", kargs->btfs_image_addr.start,
                                                           kargs->btfs_image_addr.start +
                                                           kargs->btfs_image_addr.size - 1);
    dprintf(" Kernel image phys.addr.: 0x%08X - 0x%08X\n", kargs->phys_kernel_addr.start,
                                                           kargs->phys_kernel_addr.start +
                                                           kargs->phys_kernel_addr.size - 1);
    dprintf(" Kernel image virt.addr.: 0x%08X - 0x%08X\n", kargs->virt_kernel_addr.start,
                                                           kargs->virt_kernel_addr.start +
                                                           kargs->virt_kernel_addr.size - 1);
    dprintf(" Kernel stack phys.addr.: 0x%08X - 0x%08X\n", kargs->phys_cpu_kstack[0].start,
                                                           kargs->phys_cpu_kstack[0].start +
                                                           kargs->phys_cpu_kstack[0].size - 1);
    dprintf(" Kernel stack virt.addr.: 0x%08X - 0x%08X\n", kargs->virt_cpu_kstack[0].start,
                                                           kargs->virt_cpu_kstack[0].start +
                                                           kargs->virt_cpu_kstack[0].size - 1);
    dprintf(" IDT phys.addr.: 0x%08X\n", kargs->arch_args.phys_idt);
    dprintf(" IDT virt.addr.: 0x%08X\n", kargs->arch_args.virt_idt);
    dprintf(" GDT phys.addr.: 0x%08X\n", kargs->arch_args.phys_gdt);
    dprintf(" GDT virt.addr.: 0x%08X\n", kargs->arch_args.virt_gdt);
    dprintf(" Page Directory phys.addr.: 0x%08X\n", kargs->arch_args.phys_pgdir);
    dprintf(" Page Directory virt.addr.: 0x%08X\n", kargs->arch_args.virt_pgdir);
    dprintf(" VESA: ");
    if(kargs->fb.enabled)
       dprintf("%dx%dx%d\n", kargs->fb.x_size, kargs->fb.y_size, kargs->fb.bit_depth);
    else dprintf("not enabled\n");
    dprintf(" RAM Size: %dMB\n", kargs->memsize/1024/1024);
    dprintf(" Zones of physical memory:\n");
    for(i=0; i < kargs->num_phys_mem_ranges; i++)
      dprintf("   0x%08X - 0x%08X\n", kargs->phys_mem_range[i].start,
                                      kargs->phys_mem_range[i].start +
                                      kargs->phys_mem_range[i].size - 1);
    dprintf(" Zones of allocated physical memory:\n");
    for(i=0; i < kargs->num_phys_alloc_ranges; i++)
      dprintf("   0x%08X - 0x%08X\n", kargs->phys_alloc_range[i].start,
                                      kargs->phys_alloc_range[i].start +
                                      kargs->phys_alloc_range[i].size - 1);
    dprintf(" Zones of allocated virtual memory:\n");
    for(i=0; i < kargs->num_virt_alloc_ranges; i++)
      dprintf("   0x%08X - 0x%08X\n", kargs->virt_alloc_range[i].start,
                                      kargs->virt_alloc_range[i].start +
                                      kargs->virt_alloc_range[i].size - 1);
    dprintf("===\n");
  #endif

    /* save remaining kernel args */
    kargs->num_cpus = 1;
    kargs->cons_line = screenOffset / SCREEN_WIDTH;

    /* switch to new stack */
    asm(" movl %0,    %%eax; "
        " movl %%eax, %%esp; "
        :: "m" (kstack_start+kstack_size) );
    /* jump to kernel */
    asm(" pushl $0x0; " /* we are bootstrap CPU (0) */
        " pushl %0;   " /* kernel args */
        " pushl $0x0; " /* dummy return value for call to kernel main() */
        " pushl %1;   " /* push return address */
        " ret;        " /* return */
        :: "g" (kargs), "g" (kentry) );
}

void clearscreen() {
    int i;

    for(i=0; i< SCREEN_WIDTH*SCREEN_HEIGHT*2; i++) {
        screenBase[i] = 0xf20;
    }
}

static void screenup()
{
    int i;
    memcpy(screenBase, screenBase + SCREEN_WIDTH,
        SCREEN_WIDTH * SCREEN_HEIGHT * 2 - SCREEN_WIDTH * 2);
    screenOffset = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH;
    for(i=0; i<SCREEN_WIDTH; i++)
        screenBase[screenOffset + i] = 0x0720;
    line = SCREEN_HEIGHT - 1;
}

int puts(const char *str) {
    while (*str) {
        if (*str == '\n') {
            line++;
            if(line > SCREEN_HEIGHT - 1)
                screenup();
            else
                screenOffset += SCREEN_WIDTH - (screenOffset % SCREEN_WIDTH);
        } else {
            screenBase[screenOffset++] = 0xf00 | *str;
        }
        if (screenOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
            screenup();

        str++;
    }
    return 0;
}

int dprintf(const char *fmt, ...) {
    int ret;
    va_list args;
    char temp[256];

    va_start(args, fmt);
    ret = vsprintf(temp,fmt,args);
    va_end(args);

    puts(temp);
    return ret;
}

int panic(const char *fmt, ...) {
    int ret;
    va_list args;
    char temp[256];

    va_start(args, fmt);
    ret = vsprintf(temp,fmt,args);
    va_end(args);

    puts("\nKINIT PANIC: ");
    puts(temp);
    puts("\n");

    puts("system halted");
    for(;;);
    return ret;
}

/* allocate a page directory and page table to facilitate mapping
 * pages to the 0xC0000000 - 0xC0400000 region.
 * also identity maps the first 8MB of memory
*/
static int mmu_init(kernel_args_t *ka, uint32 *next_physaddr) {
    int i;

    /* allocate a new pgdir */
    pgdir = (mmu_pde *)*next_physaddr;
    (*next_physaddr) += PAGE_SIZE;

    /* store page directory address into kernel args */
    ka->arch_args.phys_pgdir = (uint32)pgdir;

    /* clear out the pgdir */
    for(i = 0; i < 1024; i++)
        pgdir[i].raw.dword0 = 0;

    /* make a new pagetable at this random spot */
    pgtable = (mmu_pte *)(MMU_TMPDATA);

    /* create page table for first 4MB region */
    for (i = 0; i < 1024; i++)
        pgtable[i].raw.dword0 = (i * 0x1000) | DEFAULT_PAGE_FLAGS;

    /* add to page directory */
    pgdir[0].raw.dword0 = (uint32)pgtable | DEFAULT_PAGE_FLAGS;

    /* make another pagetable at this random spot */
    pgtable = (mmu_pte *)(MMU_TMPDATA + PAGE_SIZE);

    /* create page table for second 4Mb region */
    for (i = 0; i < 1024; i++)
        pgtable[i].raw.dword0 = (i * 0x1000 + 0x400000) | DEFAULT_PAGE_FLAGS;

    /* add to page directory */
    pgdir[1].raw.dword0 = (uint32)pgtable | DEFAULT_PAGE_FLAGS;

    /* Get new page table and clear it out */
    pgtable = (mmu_pte *)*next_physaddr;
    (*next_physaddr) += PAGE_SIZE;

    /* store page info into kernel args */
    ka->arch_args.num_pgtables = 1;
    ka->arch_args.phys_pgtables[0] = (uint32)pgtable;

    for (i = 0; i < 1024; i++)
        pgtable[i].raw.dword0 = 0;

    /* put the new page table into the page directory
     * this maps the kernel at KERNEL_BASE
     */
    pgdir[KERNEL_BASE/(4*1024*1024)].raw.dword0 = (uint32)pgtable | DEFAULT_PAGE_FLAGS;

    /* switch to the new pgdir */
    asm("movl %0, %%eax;"
        "movl %%eax, %%cr3;" :: "m" (pgdir) : "eax");
    /* Important.  Make sure supervisor threads can fault on read only pages... */
    asm("movl %%eax, %%cr0" : : "a" (X86_CR0_PG | X86_CR0_WP | X86_CR0_NE | X86_CR0_PE));

    /** The paging turn-on here. **/

    return 0;
}

/* can only map the 4 meg region right after KERNEL_BASE, may fix this later
 * if need arises.
*/
static void mmu_map_page(uint32 virt_addr, uint32 phys_addr) {
    if(virt_addr < KERNEL_BASE || virt_addr >= (KERNEL_BASE + 4096*1024))
        panic("mmu_map_page: asked to map invalid page!\n");

    phys_addr &= ~(PAGE_SIZE-1);
    pgtable[(virt_addr % (PAGE_SIZE * 1024)) / PAGE_SIZE].raw.dword0 = phys_addr | DEFAULT_PAGE_FLAGS;
}

/* loads ELF image to next_physaddr and returns occupied virtual addresses range and
 * entry point address
 */
static void load_elf_image(void *elf_data, uint32 *next_physaddr, addr_range_t *range, uint32 *entry_addr) {
    Elf32_Ehdr_t *imageHeader = (Elf32_Ehdr_t *)elf_data;
    Elf32_Phdr_t *segments = (Elf32_Phdr_t *)(imageHeader->e_phoff + (unsigned)imageHeader);
    int segmentIndex;
    int foundSegmentIndex = 0;

    range->size = 0;
    range->size = 0;

    /* walk throught segments of ELF */
    for (segmentIndex = 0; segmentIndex < imageHeader->e_phnum; segmentIndex++) {
        Elf32_Phdr_t *segment = &segments[segmentIndex];
        unsigned segmentOffset;

        switch(segment->p_type) {
            case PT_LOAD:  /* load segment if loadable */
                break;
            default:
                continue;
        }

        /* map initialized portion */
        for (segmentOffset = 0;
             segmentOffset < ROUNDUP(segment->p_filesz, PAGE_SIZE);
             segmentOffset += PAGE_SIZE)
        {
            /* map */
            mmu_map_page(segment->p_vaddr + segmentOffset, *next_physaddr);
            /* copy data to mapped page */
            memcpy((void *)(segment->p_vaddr + segmentOffset),
                             (void *)((unsigned)elf_data + segment->p_offset +
                                      segmentOffset), PAGE_SIZE);
            /* switch to next physical address */
            (*next_physaddr) += PAGE_SIZE;
        }

        /* clean out the leftover part of the last page */
        if(segment->p_filesz % PAGE_SIZE > 0) {
            memset((void*)(segment->p_vaddr + segment->p_filesz), 0,
                            PAGE_SIZE - (segment->p_filesz % PAGE_SIZE) );
        }

        /* map uninitialized portion */
        for (; segmentOffset < ROUNDUP(segment->p_memsz, PAGE_SIZE); segmentOffset += PAGE_SIZE)
        {
            /* map */
            mmu_map_page(segment->p_vaddr + segmentOffset, *next_physaddr);
            /* init page to zeros */
            memset((void *)(segment->p_vaddr + segmentOffset), 0, PAGE_SIZE);
            /* switch to next physical address */
            (*next_physaddr) += PAGE_SIZE;
        }
        
        /* store segment ranges */
        switch(foundSegmentIndex) {
            case 0:
                range->start = segment->p_vaddr;
                range->size  = segment->p_memsz;
                break;
            default:
                ;
        }
        foundSegmentIndex++;
    }
    /* store entry point addres */
    *entry_addr = imageHeader->e_entry;
}

/* sorts array of address ranges by start address */
static void sort_addr_ranges(addr_range_t *ranges, uint32 count) {
    addr_range_t temp_range;
    uint32 i;
    bool done;

    /* simple bubble sort algorithm */
    do {
        done = true;
        for(i = 1; i < count; i++) {
            /* sort from lowest to highest adresses */
            if(ranges[i].start < ranges[i-1].start) {
                done = false;
                /* swap two entries */
                memcpy(&temp_range,  &ranges[i],   sizeof(temp_range));
                memcpy(&ranges[i],   &ranges[i-1], sizeof(temp_range));
                memcpy(&ranges[i-1], &temp_range,  sizeof(temp_range));
            }
        }
    } while(!done);
}
