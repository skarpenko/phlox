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

void arch_processor_set_init(arch_processor_set_t *aps, kernel_args_t *kargs, uint32 curr_cpu) {
}

void arch_processor_init(arch_processor_t *ap, kernel_args_t *kargs, uint32 curr_cpu) {
    uint32 cpu;
    uint32 a, b, c, d;
    uint32 i;
    
    /* init FPU */
    asm("fninit");
    /* NOTE: So, on 80386 and 80486 FPU must be present! */

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
       ap->family   = (a >> 8) & 0x0f;
       ap->model    = (a >> 4) & 0x0f;
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
}
