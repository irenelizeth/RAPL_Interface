TARGET = librapl_utils.so rapl_util.o arch_spec.o
CFLAGS = -fPIC
#JAVA_HOME = $(shell readlink -f /usr/bin/javac | sed "s:bin/javac::") 
#JAVA_INCLUDE = $(JAVA_HOME)/include
JAVA_INCLUDE = "/usr/lib/jvm/java-1.7.0-openjdk-amd64/include/"

all: librapl_utils

librapl_utils:
	gcc -c $(CFLAGS) -I $(JAVA_INCLUDE) rapl_util.c arch_spec.c
	gcc -shared $(CFLAGS) -o librapl_utils.so -I $(JAVA_INCLUDE) rapl_util.o arch_spec.o

clean:
	rm -rf librapl_utils $(TARGET)


