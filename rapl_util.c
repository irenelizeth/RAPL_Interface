#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "arch_spec.h"
#include "rapl_util.h"

//RAPL registers

#define MSR_RAPL_POWER_UNIT	0x606

#define MSR_PKG_ENERGY_STATUS	0x611
#define MSR_PP0_ENERGY_STATUS	0x639
#define MSR_PP1_ENERGY_STATUS	0x641
#define MSR_DRAM_ENERGY_STATUS	0x619


int *fd; // file descriptor
double WRAPAROUND_VALUE;
rapl_msr_unit rapl_units;


jint JNICALL Java_util_EnergyMetric_open_1msr(JNIEnv *env, jclass class, jint core){

 char msr_filename[BUFSIZ];
  int fd;

  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDONLY);
  if ( fd < 0 ) {
    if ( errno == ENXIO ) {
      fprintf(stderr, "rdmsr: No CPU %d\n", core);
      exit(2);
    } else if ( errno == EIO ) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr,"Trying to open %s\n",msr_filename);
      exit(127);
    }
  }

  return fd; 

}

JNIEXPORT jlong JNICALL Java_util_EnergyMetric_read_1msr(JNIEnv *env, jclass class, jint msrId, jint fd){
  uint64_t data;

  //int fd = open_msr(core);
  
  if( fd>0 && pread(fd, &data, sizeof data, msrId) != sizeof data){
    perror("Error readind file descriptor - rdmsr:pread");
    exit(127);
  }

  return (long long)data;

}

uint64_t read_msr(int fd, uint64_t msrId) {

  uint64_t data = 0;

  if ( pread(fd, &data, sizeof data, msrId) != sizeof data ) {
    printf("pread error!\n");
  }

  return data;
}

JNIEXPORT jint JNICALL Java_util_EnergyMetric_profileInit(JNIEnv *env, jclass class){

  jintArray results;
  char msr_filename[BUFSIZ];

  get_cpu_model();
  get_socket_number();

  switch(cpu_model) {
	case SANDYBRIDGE_EP:
	  printf("Sandrybridge_ep cpu model\n");
	  break;
	
	case SANDYBRIDGE:
	  printf("Sandrybridge cpu model\n");
	  break;

	case IVYBRIDGE:
	  printf("Ivybridge cpu model\n");
	  break;
  }

 // printf("socket number is: %" PRIu64 "\n",num_pkg); 
 
  jint wraparound_energy;
  int i; 
  // TODO: confirm that the energy estimates are per socket and not per core
  // so it makes sense just to query one core instead of all of them

  fd = (int *) malloc(num_pkg * sizeof(int));
  // for each socket (pkg)
  for(i=0; i < num_pkg; i++){
	if(i>0){
	  core+= num_pkg_thread/2;
	}

	sprintf(msr_filename, "/dev/cpu/%d/msr", core);
	fd[i] = open(msr_filename, O_RDWR);
  }

  uint64_t unit_info = read_msr(fd[0],MSR_RAPL_POWER_UNIT);
  
  //printf("value read from msr_rapl_power_unit: %" PRIu64 "\n", unit_info);  
  
  get_msr_units(&rapl_units, unit_info);
  WRAPAROUND_VALUE = 1.0/ (rapl_units.energy);
  wraparound_energy = (int)WRAPAROUND_VALUE;

  return wraparound_energy;
}

/*Get the values of the Energy Status RAPL MSR for all the domains available in the
current architecture (e.g., PKG, PP0, PP1) */
JNIEXPORT jstring JNICALL Java_util_EnergyMetric_getRAPLEnergyStatus(JNIEnv *env, jclass class){

	jstring energy_info;
	char *energy_values;
	double result = 0.0;
	// Energy Domains
	double package[num_pkg];	// entire socket
	double pp0[num_pkg];		// power plane 0 (processor cores)
	double pp1[num_pkg];		// power plane 1 (client: uncore devices e.g., gpu)
	double dram[num_pkg];		// dram (server)

	long dram_num = 0L;
	long cpu_num = 0L;
	long gpu_num = 0L;
	long package_num = 0L;
	int info_size;
	int i;
	int offset = 0;

	char package_buffer[60];
        char cpu_buffer[60];
        char gpu_buffer[60];

	for (i=0; i< num_pkg; i++){
	 // char package_buffer[60];
	 // char cpu_buffer[60];
	 // char gpu_buffer[60];
	  
	  // read value for PKG domain
	  result = read_msr(fd[i], MSR_PKG_ENERGY_STATUS);	
	  package[i] = (double) result * rapl_units.energy;
	  printf("\t msr_pkg_energy_status: %lf \n", package[i]);
	 

	  result = 0.0;
	  //read value for PP0 domain
	  result = read_msr(fd[i], MSR_PP0_ENERGY_STATUS);
	  pp0[i] = (double) result * rapl_units.energy;
	  printf("\t msr_pp0_energy_status: %lf \n", pp0[i]);
	  
	  sprintf(package_buffer, "%f", package[i]);
	  sprintf(cpu_buffer, "%f", pp0[i]);

	  package_num = strlen(package_buffer);
	  cpu_num = strlen(cpu_buffer);
	  printf("\t Number of bits [package_num: %ld] \t [cpu_num: %ld] \n", package_num, cpu_num);	

	  if(i==0){
	    info_size = num_pkg * (cpu_num + package_num + 1);
	    energy_values = (char*)malloc(info_size);
	  }

	 /* printf("\n \t Printing Energy Estimation Values: ");
	   // copy the energy estimation from cpu
	   memcpy(energy_values + offset, &cpu_buffer, cpu_num);
	   // add separator to string
	   energy_values[offset + cpu_num]= '#';
	   // copy the energy estimation from the socket
	   memcpy(energy_values + offset + cpu_num + 1, &package_buffer, package_num);
	  
	   energy_info = (*env)->NewStringUTF(env,energy_values);
	   free(energy_values);
	*/

	}	

	 printf("\n \t Printing Energy Estimation Values: ");
         // copy the energy estimation from cpu
         memcpy(energy_values + offset, &cpu_buffer, cpu_num);
         // add separator to string
         energy_values[offset + cpu_num]= '#';
         // copy the energy estimation from the socket
         memcpy(energy_values + offset + cpu_num + 1, &package_buffer, package_num);

         energy_info = (*env)->NewStringUTF(env,energy_values);
         free(energy_values);
	
	return energy_info;
}

