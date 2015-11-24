#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include "EnergyMetric.h"

//implementation of native method getCPUEnergy() of EnergyMetric class
JNIEXPORT jdouble JNICALL Java_EnergyMetric_getCPUEnergy(JNIEnv *env, jclass class){

  int r = rand()*1000;
  return (double)r;

}
