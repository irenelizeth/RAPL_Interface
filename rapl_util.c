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

#define MSR_RAPL_POWER_UNIT	0x606 // units for power, energy and time

#define MSR_PKG_ENERGY_STATUS	0x611 // socket power plane
#define MSR_PP0_ENERGY_STATUS	0x639 // core power plane - all cores on socket
#define MSR_PP1_ENERGY_STATUS	0x641 // graphics power plane - gpu (client only)
#define MSR_DRAM_ENERGY_STATUS	0x619 // dram power plane (server only)


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
JNIEXPORT jdoubleArray JNICALL Java_util_EnergyMetric_getRAPLEnergyStatus(JNIEnv *env, jclass class){

	double result = 0.0;
	jdouble energy_val[num_pkg*4];

	// Energy Domains
	double package[num_pkg];	// entire socket
	double pp0[num_pkg];		// power plane 0 (processor cores)
	double pp1[num_pkg];		// power plane 1 (client: uncore devices e.g., gpu)
	double dram[num_pkg];		// dram (server)

	int i,k;
	int offset = 0;
	k=0;

	for (i=0; i< num_pkg; i++){
	  
	  //read value for MSR PP0 domain
	  result = read_msr(fd[i], MSR_PP0_ENERGY_STATUS);
	  pp0[i] = (double) result * rapl_units.energy;
	  //printf("\t msr_pp0_energy_status: %lf \n", pp0[i]);  
	  energy_val[k] = pp0[i];
	  k++;

	  //depending on the cpu model some MSR are available
	  switch(cpu_model){
	      
	   case SANDYBRIDGE_EP:
	      	result = 0.0;
              	//read value for MSR DRAM domain
     	      	result = read_msr(fd[i], MSR_DRAM_ENERGY_STATUS);
              	dram[i] = (double)result*rapl_units.energy;
              	//printf("\t msr_dram_energy_status: %lf \n", dram[i]);
		energy_val[k] = dram[i];	      	
		k++;
	
		pp1[i] = 0.0; energy_val[k]= 0.0; k++;
		break;

	   case IVYBRIDGE: 
	   case SANDYBRIDGE:
		result = 0.0;	
		// read value for MSR PP1 domain
          	result = read_msr(fd[i], MSR_PP1_ENERGY_STATUS);
          	pp1[i] = (double) result * rapl_units.energy;
          	//printf("\t msr_pp1_energy_status: %lf \n", pp1[i]);
		energy_val[k] = pp1[i];	
		k++;

		dram[i] = 0.0; energy_val[k] = 0.0; k++;
		break;

	  }
 
	  // read value for PKG domain
          result = read_msr(fd[i], MSR_PKG_ENERGY_STATUS);
          package[i] = (double) result * rapl_units.energy;
          //printf("\t msr_pkg_energy_status: %lf \n", package[i]);
   	  energy_val[k] = package[i];
	  k++;
	
	 //TODO: fix indexx for dram depending on architecture could be 1 or 2
	 //printf("\n \t Printing Energy Estimation Values:\n ");
	 //printf("\t\t [package_num: %g] \n\t\t [cpu_num: %g] \n\t\t [gpu_num: %g] \n\t\t [dram_num: %g] \n", energy_val[3],energy_val[0], energy_val[1], energy_val[2]); 	

	jdoubleArray jEnergyArray = (*env)->NewDoubleArray(env,num_pkg*4);
        (*env)->SetDoubleArrayRegion(env,jEnergyArray,0,num_pkg*4,energy_val);

	return jEnergyArray;
     }
}

