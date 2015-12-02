#include <stdint.h>
#include <unistd.h>
#ifndef _ARCH_SPEC_H
#define _ARCH_SPEC_H

#define SANDYBRIDGE		0x2AU
#define SANDYBRIDGE_EP		0x2DU
#define IVYBRIDGE 		0x3AU

// CPUID opcode, supplementary instruction for x86
// the value in the EAX or ECX register specifies what information to return
#define CPUID                              \
    __asm__ volatile ("cpuid"                             \
                        : "=a" (eax),     \
                        "=b" (ebx),     \
                        "=c" (ecx),     \
                        "=d" (edx)      \
                        : "0" (eax), "2" (ecx))

#define _2POW(num)        \
((num == 0) ? 1 : (2 << (num - 1)))


// APIC ID is a unique ID assigned to each logical processor in an Intel 64 or IA-32 platform
typedef struct APIC_ID_t {
        uint64_t smt_id;
        uint64_t core_id;
        uint64_t pkg_id;
        uint64_t os_id;
} APIC_ID_t;

//x86 registers
typedef struct cpuid_info_t {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
} cpuid_info_t;

extern uint32_t eax, ebx, ecx, edx;
extern uint32_t cpu_model;
extern int core;
extern uint64_t num_core_thread; //number of physical threads per core
extern uint64_t num_pkg_thread; //number of physical threads per package (socket)
extern uint64_t num_pkg_core; //number of cores per package (socket)
extern uint64_t num_pkg; //number of packages for current machine

// RAPL MSR Abstractions

/*represent MSR_RAPL_POWER_UNIT register*/
typedef struct rapl_msr_unit {
        double power; // power units
        double energy; // energy units
        double time; // time units
} rapl_msr_unit;

extern rapl_msr_unit rapl_units;

#endif
