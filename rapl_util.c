#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>
#include "arch_spec.h"
#include "rapl_util.h"

#define MSR_RAPL_POWER_UNIT	0x606

int *fd; // file descriptor
double WRAPAROUND_VALUE;

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
  
}
