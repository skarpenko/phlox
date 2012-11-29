/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <arch/arch_bits.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/processor.h>


/* cpu vendor info */
struct i386_cpu_vendor_info {
    const char *vendor_name;      /* short vendor's name */
    const char *ident_string[3];  /* list of identification strings */
};

static const struct i386_cpu_vendor_info cpu_vendor_info[I386_VENDORS_COUNT] = {
    { "Unknown",   { } },                 /* unknown vendor */
    { "Intel",     { "GenuineIntel" } },  /* Intel Corporation */
    { "AMD",       { "AuthenticAMD",      /* Advanced Micro Devices, Inc. */
                     "AMDisbetter!",
                     "AMD ISBETTER" } },
    { "Cyrix",     { "CyrixInstead" } },  /* Cyrix */
    { "Centaur",   { "CentaurHauls" } },  /* Centaur Technology */
    { "SiS",       { "SiS SiS SiS" } },   /* Silicon Integrated Systems */
    { "NexGen",    { "NexGenDriven" } },  /* NexGen */
    { "Transmeta", { "GenuineTMx86",      /* Transmeta Corporation */
                     "TransmetaCPU" } },
    { "Rise",      { "RiseRiseRise" } },  /* Rise Technology */
    { "UMC",       { "UMC UMC UMC" } },   /* United Microelectronics Corporation */
    { "NSC",       { "Geode by NSC" } }   /* National Semiconductor */
};


/*** Externals need to be initialized ***/

#if CPU_i386
/* CPU-dependend MMU routines */
extern uint32 __arch_invalidate_TLB_entry_link;
void cpu_i386_arch_invalidate_TLB_entry(addr_t virt_addr);
void cpu_i486_arch_invalidate_TLB_entry(addr_t virt_addr);
    
extern uint32 __arch_invalidate_TLB_range_link;
void cpu_i386_arch_invalidate_TLB_range(addr_t start, size_t size);
void cpu_i486_arch_invalidate_TLB_range(addr_t start, size_t size);

extern uint32 __arch_invalidate_TLB_list_link;
void cpu_i386_arch_invalidate_TLB_list(addr_t pages[], size_t count);
void cpu_i486_arch_invalidate_TLB_list(addr_t pages[], size_t count);
#endif

/* architecture specific processor module init */
void arch_processor_mod_init(arch_processor_t *bsp) {
#if CPU_i386
    /* init MMU routines */
    if(bsp->family<4) {
       /* init CPU i386 compatible links */
       __arch_invalidate_TLB_entry_link = (uint32)&cpu_i386_arch_invalidate_TLB_entry;
       __arch_invalidate_TLB_range_link = (uint32)&cpu_i386_arch_invalidate_TLB_range;
       __arch_invalidate_TLB_list_link  = (uint32)&cpu_i386_arch_invalidate_TLB_list;
    } else {
       /* init CPU i486+ compatible links */
       __arch_invalidate_TLB_entry_link = (uint32)&cpu_i486_arch_invalidate_TLB_entry;
       __arch_invalidate_TLB_range_link = (uint32)&cpu_i486_arch_invalidate_TLB_range;
       __arch_invalidate_TLB_list_link  = (uint32)&cpu_i486_arch_invalidate_TLB_list;
    }
#endif
}

/* architecture specific processor set init */
void arch_processor_set_init(arch_processor_set_t *aps, kernel_args_t *kargs, uint32 curr_cpu) {
    /* do nothing for now */
}

/* architecture specific processor init */
void arch_processor_init(arch_processor_t *ap, kernel_args_t *kargs, uint32 curr_cpu) {
    uint32 cpu, fpu;
    uint16 fpu_tmp;
    uint32 a, b, c, d;
    uint32 i;
    char feature_str[I386_CPU_FEATURE_STR_MAX];


    /*
     * This asm part returns 3 - for 80386 CPU,
     * 4 - for 80486 CPU without CPUID support
     * and 0 - if CPUID supported and it is possible to
     * obtain extended CPU information.
     * (stolen from Mach)
     */
    asm(
     "   pushfl                      ;"  /* Fetch flags ... */
     "   popl    %%eax               ;"  /*  ... into eax */
     "   movl    %%eax, %%ecx        ;"  /* Save original flags for return */
     "   xorl    $0x00240000, %%eax  ;"  /* Attempt to toggle ID and AC bits */
     "   pushl   %%eax               ;"  /* Save flags... */
     "   popfl                       ;"  /*  ... In EFLAGS */
     "   pushfl                      ;"  /* Fetch flags back ... */
     "   popl    %%eax               ;"  /*  ... into eax */

     "   xorl    %%ecx, %%eax        ;"  /* See if any bits didn't change */
     "   testl   $0x00040000, %%eax  ;"  /* Test AC bit */
     "   jnz 0f                      ;"  /* Skip next bit if AC toggled */
     "   movl    $3, %%eax           ;"  /*   Return value is 386 */
     "   jmp 9f                      ;"  /*   And FINISH */

     "0: testl   $0x00200000, %%eax  ;"  /* Test ID bit */
     "   jnz 0f                      ;"  /* Skip next bit if ID toggled */
     "   movl    $4, %%eax           ;"  /*   Return value is 486 */
     "   jmp 9f                      ;"  /*   And FINISH */

     /* We are a modern enough processor to have the CPUID instruction;
        use it to find out what we are. */
     "0: movl $0, %%eax              ;"  /* Return 0 */
    
     "9: pushl   %%ecx               ;"  /* From ecx... */
     "   popfl                       ;"  /* ... restore original flags */
     : "=a" (cpu) : : "%ecx");

    /* if CPUID instruction is not supported */
    if(cpu) {
       ap->vendor = I386_VENDOR_UNKNOWN;
       ap->vendor_name = cpu_vendor_info[I386_VENDOR_UNKNOWN].vendor_name;
       ap->family = cpu;
       strcpy(ap->model_name, "Unknown");
    } else { /* if CPUID instruction supported */
       ap->cpuid = 1; /* set CPUID supported flag */

       /* Get vendor's identification string */
       i386_cpuid(0, &a, &b, &c, &d);
       *(uint32 *)&ap->vendor_str[0] = b;
       *(uint32 *)&ap->vendor_str[4] = d;
       *(uint32 *)&ap->vendor_str[8] = c;
       
       /* Figure out what vendor we have here */
       ap->vendor_name = cpu_vendor_info[I386_VENDOR_UNKNOWN].vendor_name;
       for(i=1; i<I386_VENDORS_COUNT; i++) {
           if(!strcmp(ap->vendor_str, cpu_vendor_info[i].ident_string[0])) {
             ap->vendor = i;
             ap->vendor_name = cpu_vendor_info[i].vendor_name;
             break;
           }
           if(!strcmp(ap->vendor_str, cpu_vendor_info[i].ident_string[1])) {
             ap->vendor = i;
             ap->vendor_name = cpu_vendor_info[i].vendor_name;
             break;
           }
           if(!strcmp(ap->vendor_str, cpu_vendor_info[i].ident_string[2])) {
             ap->vendor = i;
             ap->vendor_name = cpu_vendor_info[i].vendor_name;
             break;
           }
       }

       /* Get processor family, model and stepping */
       a = i386_cpuid_eax(1);
       ap->family   = ((a >> 20) & 0xff) + ((a >> 8) & 0x0f); /* Ext.family + family */
       ap->model    = ((a >> 12) & 0xf0) + ((a >> 4) & 0x0f); /* Ext.model<<4 + model */
       ap->stepping = a & 0x0f;

       /* try to get processor's model name */
       a = i386_cpuid_eax(0x80000000);
       if(a >= 0x80000004) { /* if it is possible to obtain model name */
           /* build the model string */
           i386_cpuid(0x80000002, &a, &b, &c, &d);
             *(uint32 *)&ap->model_name[0]  = a;
             *(uint32 *)&ap->model_name[4]  = b;
             *(uint32 *)&ap->model_name[8]  = c;
             *(uint32 *)&ap->model_name[12] = d;
           i386_cpuid(0x80000003, &a, &b, &c, &d);
             *(uint32 *)&ap->model_name[16] = a;
             *(uint32 *)&ap->model_name[20] = b;
             *(uint32 *)&ap->model_name[24] = c;
             *(uint32 *)&ap->model_name[28] = d;
           i386_cpuid(0x80000004, &a, &b, &c, &d);
             *(uint32 *)&ap->model_name[32] = a;
             *(uint32 *)&ap->model_name[36] = b;
             *(uint32 *)&ap->model_name[40] = c;
             *(uint32 *)&ap->model_name[44] = d;

           /* some cpus return a right-justified string */
           for(i = 0; ap->model_name[i] == ' '; i++)
               ;
           if(i > 0)
               memmove(ap->model_name, &ap->model_name[i], strlen(&ap->model_name[i]) + 1);
       } else { /* it is not possible to obtain model name */
           strcpy(ap->model_name, "Unknown");
       }
       
       /* Load feature bits */
       i386_cpuid(1, &a, &b, &c, &d);
       ap->features[I386_FEATURE_C] = c;  /* ecx register */
       ap->features[I386_FEATURE_D] = d;  /* edx register */
       /* try to get extended feature bits */
       a = i386_cpuid_eax(0x80000000);
       if(a>=0x80000001) {
          i386_cpuid(0x80000001, &a, &b, &c, &d);
          ap->features[I386_AMD_FEATURE_C] = c;  /* ecx register */
          ap->features[I386_AMD_FEATURE_D] = d;  /* edx register */
       }

       /* Set special flags */
       if(ap->features[I386_FEATURE_D] & X86_CPUID_TSC)
           ap->rdtsc = 1; /* RDTSC instruction supported */
     } /* else */


    /*
     *  Detect and initialize FPU.
     *  Returns 1 if FPU present, else returns 0.
     */
    asm(
     "    movl     $0, %0        ;"   /* init variables */
     "    movw     $0x5252, %1   ;"
     "    clts                   ;"   /* clear TS flag */
     /* check fpu status word */
     "    fninit                 ;"   /* reset fpu status word */
     "    fnstsw   %%ax          ;"   /* save fpu status word */
     "    cmpb     $0, %%al      ;"   /* was correct status word? */
     "    jne 1f                 ;"   /* no fpu present */
     /* check fpu control word */
     "    fnstcw   %1            ;"   /* save fpu control word */
     "    movw     %1, %%ax      ;"   /* copy control word to %ax */
     "    andw     $0x103f, %%ax ;"   /* selected parts to examine */
     "    cmpw     $0x3f, %%ax   ;"   /* was correct control word? */
     "    jne 1f                 ;"   /* incorrect control word, no FPU */
     /* fpu present */
     "    movl     $1, %0        ;"   /* fpu present */
     "    .byte 0xDB, 0xE4       ;"   /* fsetpm for 287, ignored by 387 */
     " 1:                        "    /* finish */
     : : "m" (fpu), "m" (fpu_tmp) : "%eax");
     ap->fpu = fpu;


     /* print processor info */
     kprint("CPU #%d info:\n", curr_cpu);
     kprint("  FPU present: "); (ap->fpu)?kprint("yes\n"):kprint("no\n");
     kprint("  CPUID support: "); (ap->cpuid)?kprint("yes\n"):kprint("no\n");
     kprint("  Vendor: %s (%s)\n", ap->vendor_name, ap->vendor_str);
     kprint("  CPU: Family %d, Model %d, SteppingID %d\n", ap->family, ap->model, ap->stepping);
     kprint("  CPU model name: %s\n", ap->model_name);
     kprint("  RDTSC support: "); (ap->rdtsc)?kprint("yes\n"):kprint("no\n");
     i386_cpu_feature_str(ap, feature_str);
     kprint("  CPU feature string: %s\n", feature_str);


     /* Ensure that it is possible to run kernel on this processor */
     if(curr_cpu && ap->family == 3) {
        kprint("\n\nSorry, but SMP currently is not supported on i386 chips.\n\n");
        kprint("system stopped.\n");
        while(1);
     }
     if(!ap->fpu) {
        kprint("\n\nSorry, but FPU emulation currently is not supported.\n\n");
        kprint("system stopped.\n");
        while(1);
     }

     i = 0;
     #if CPU_i386
       i = 3; /* optimized for 80386 */
     #endif

     #if CPU_i486
       i = 4; /* optimized for 80486 */
     #endif

     #if CPU_i586 || CPU_Pentium || CPU_PentiumMMX || CPU_K6 || CPU_K6_2 || CPU_K6_3
       i = 5; /* optimized for Pentium-compatible chips */
     #endif

     #if CPU_i686 || CPU_PentiumPro || CPU_Pentium2 || CPU_Pentium3 || CPU_Pentium4 || \
         CPU_K7 || CPU_Athlon || CPU_Athlon_Tbird || CPU_Athlon4 || CPU_AthlonXP || CPU_AthlonMP
       i = 6; /* optimized for PentiumPro-compatible chips */
     #endif

     if(ap->family < i) {
        kprint("\n\nSorry, but Phlox kernel cannot run on this computer\n");
        kprint("due to lacking of some features.\n\n");
        kprint("system stopped.\n");
        while(1);
     }
}

/* build cpu features string */
void i386_cpu_feature_str(arch_processor_t *p, char *str) {
    str[0] = 0;

    /* standard features from edx */
    if(p->features[I386_FEATURE_D] & X86_CPUID_FPU)
        strcat(str, "fpu ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_VME)
        strcat(str, "vme ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_DE)
        strcat(str, "de ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_PSE)
        strcat(str, "pse ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_TSC)
        strcat(str, "tsc ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_MSR)
        strcat(str, "msr ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_PAE)
        strcat(str, "pae ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_MCE)
        strcat(str, "mce ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_CX8)
        strcat(str, "cx8 ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_APIC)
        strcat(str, "apic ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_SEP)
        strcat(str, "sep ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_MTRR)
        strcat(str, "mtrr ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_PGE)
        strcat(str, "pge ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_MCA)
        strcat(str, "mca ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_CMOV)
        strcat(str, "cmov ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_PAT)
        strcat(str, "pat ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_PSE36)
        strcat(str, "pse36 ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_PSN)
        strcat(str, "psn ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_CLFSH)
        strcat(str, "clfsh ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_DS)
        strcat(str, "ds ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_ACPI)
        strcat(str, "acpi ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_MMX)
        strcat(str, "mmx ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_FXSR)
        strcat(str, "fxsr ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_SSE)
        strcat(str, "sse ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_SSE2)
        strcat(str, "sse2 ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_SS)
        strcat(str, "ss ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_HTT)
        strcat(str, "htt ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_TM1)
        strcat(str, "tm1 ");
    if(p->features[I386_FEATURE_D] & X86_CPUID_PBE)
        strcat(str, "pbe ");

    /* standard features from ecx */
    if(p->features[I386_FEATURE_C] & X86_CPUID_SSE3)
        strcat(str, "sse3 ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_MON)
        strcat(str, "mon ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_DSCPL)
        strcat(str, "dscpl ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_VMX)
        strcat(str, "vmx ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_SMX)
        strcat(str, "smx ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_EST)
        strcat(str, "est ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_TM2)
        strcat(str, "tm2 ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_SSSE3)
        strcat(str, "ssse3 ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_CID)
        strcat(str, "cid ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_CX16)
        strcat(str, "cx16 ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_XTPR)
        strcat(str, "xtpr ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_PDCM)
        strcat(str, "pdcm ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_DCA)
        strcat(str, "dca ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_SSE41)
        strcat(str, "sse4.1 ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_SSE42)
        strcat(str, "sse4.2 ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_X2APIC)
        strcat(str, "x2apic ");
    if(p->features[I386_FEATURE_C] & X86_CPUID_POPCNT)
        strcat(str, "popcnt ");
    
    /* extended AMD features from edx */
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_MP)
        strcat(str, "mp ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_NX)
        strcat(str, "nx ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_MMXEXT)
        strcat(str, "mmx+ ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_FFXSR)
        strcat(str, "ffxsr ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_PG1G)
        strcat(str, "pg1g ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_TSCP)
        strcat(str, "tscp ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_LM)
        strcat(str, "lm ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_3DNOWEXT)
        strcat(str, "3dnow!+ ");
    if(p->features[I386_AMD_FEATURE_D] & X86_CPUID_AMD_3DNOW)
        strcat(str, "3dnow! ");
        
    /* extended AMD features from ecx */
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_AHF64)
        strcat(str, "ahf64 ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_CMP)
        strcat(str, "cmp ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_SVM)
        strcat(str, "svm ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_EAS)
        strcat(str, "eas ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_CR8D)
        strcat(str, "cr8d ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_ABM)
        strcat(str, "abm ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_SSE4A)
        strcat(str, "sse4a ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_MSSE)
        strcat(str, "msse ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_3DNOWPREF)
        strcat(str, "3dnow!- ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_OSVM)
        strcat(str, "osvm ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_IBS)
        strcat(str, "ibs ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_SSE5A)
        strcat(str, "sse5a ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_SKINIT)
        strcat(str, "skinit ");
    if(p->features[I386_AMD_FEATURE_C] & X86_CPUID_AMD_WDT)
        strcat(str, "wdt ");
}

/* invalidate TLB entries refered to given address range */
#if CPU_i386
void cpu_i486_arch_invalidate_TLB_range(addr_t start, size_t size) {
#else
void arch_invalidate_TLB_range(addr_t start, size_t size) {
#endif
    int32 num_pages = (start+size)/PAGE_SIZE - start/PAGE_SIZE;

    while(num_pages-- >= 0) {
       arch_invalidate_TLB_entry(start); /* invalidate */
       start += PAGE_SIZE;               /* switch to next page */
    }
}

/* invalidate list of TLB entries */
#if CPU_i386
void cpu_i486_arch_invalidate_TLB_list(addr_t pages[], size_t count) {
#else
void arch_invalidate_TLB_list(addr_t pages[], size_t count) {
#endif
    uint32 i;

    for(i=0; i<count; i++)
       arch_invalidate_TLB_entry(pages[i]);
}
