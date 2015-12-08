
package util;


public class EnergyMetric {

	static final int MSR_PKG_ENERGY_STATUS = 0X611;

	static{
	  System.loadLibrary("rapl_utils");
	}


	public native static int open_msr(int core);
	public native static long read_msr(int msr, int fd);
	public native static int profileInit();
	//public native static String getRAPLEnergyStatus();
	public native static double[] getRAPLEnergyStatus();

	public static void main(String[] args){
		
	  // check msr module is loaded (modprobe msr) and read access is allow

	  if(!ShellCommand.execute("lsmod | grep msr").contains("msr"))
		ShellCommand.execute("modprobe msr"); 
	   
	  //check perf_event_paranoid < 1

	  // open MSR for reading
	  int val = EnergyMetric.open_msr(0);
	  
	  if(val<0){
	    System.err.println("Error reading file descriptor");  
	  }else{
	    System.out.println("File Descriptor is: "+val);
	  }

	  // read MSR register for the specified core
	  double result = read_msr(MSR_PKG_ENERGY_STATUS, val);
	  System.out.println("Value of Energy Status MSR: "+result);

	  System.out.println("Testing profileInit()");
	  System.out.println("Wraparound energy value is: "+ profileInit());  
	  
	  //String eStatus = getRAPLEnergyStatus();
	  double eStatus[] = getRAPLEnergyStatus();
	  //int cpu_model = getCPUModel();
	  // if Sandybridge_ep --> dram is second value
	  // if Sandybridge/ivybridge --> pp1/gpu is second value
	  //String list[] = eStatus.split("#");

	  System.out.println("energy Status: pp0->" + eStatus[0]+"; pp1-> "+eStatus[1]+"; pkg->"+ eStatus[3]+"\n");	

	}

}
