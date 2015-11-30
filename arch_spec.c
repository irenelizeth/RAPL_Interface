#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include "inttypes.h"
#include "arch_spec.h"

uint32_t eax, ebx, ecx, edx;
uint32_t cpu_model;
int core = 0;
int core_num = 0;
uint64_t num_core_thread = 0;
uint64_t num_pkg_thread = 0;
uint64_t num_pkg_core = 0;
uint64_t num_pkg = 0;

void get_cpu_model(void){
    uint32_t eax = 0x01;
    CPUID;
    cpu_model = (((eax>>16)&0xFU)<<4) + ((eax>>4)&0xFU);
}

void cpuid(uint32_t eax_in, uint32_t ecx_in, cpuid_info_t *ci) {
      asm (
	#if defined(__LP64__)           /* 64-bit architecture */
             "cpuid;"                /* execute the cpuid instruction */
             "movl %%ebx, %[ebx];"   /* save ebx output */
	#else                           /* 32-bit architecture */
             "pushl %%ebx;"          /* save ebx */
             "cpuid;"                /* execute the cpuid instruction */
             "movl %%ebx, %[ebx];"   /* save ebx output */
             "popl %%ebx;"           /* restore ebx */
	#endif
             : "=a"(ci->eax), [ebx] "=r"(ci->ebx), "=c"(ci->ecx), "=d"(ci->edx)
             : "a"(eax_in), "c"(ecx_in)
     );
}

cpuid_info_t getProcessorTopology(uint32_t level) {
         cpuid_info_t info;
         cpuid(0xb, level, &info);
         return info;
}

int get_socket_number(){
	uint32_t l0 = 0; //level 1
	uint32_t l1 = 1; //level 2
	 
	cpuid_info_t info_l0;
	cpuid_info_t info_l1;

	core_num = get_core_num();

	//printf("core_num: %d\n ",core_num);

	info_l0 = getProcessorTopology(l0);
	info_l1 = getProcessorTopology(l1);

	num_core_thread = info_l0.ebx & 0xffff;
	num_pkg_thread = info_l1.ebx & 0xffff;

	printf("Number of threads per pkg: %" PRIu64 "\n", num_pkg_thread);
	printf("Number of threads per core: %" PRIu64 "\n", num_core_thread);

	num_pkg_core = num_pkg_thread / num_core_thread;
        num_pkg = core_num / num_pkg_thread;

	return num_pkg;
}


/* returns the number of processors available in the system */
int get_core_num() {
        return sysconf(_SC_NPROCESSORS_CONF);
	// sysconf (_SC_NPROCESSORS_ONLN) // number of processors online (available)
}
