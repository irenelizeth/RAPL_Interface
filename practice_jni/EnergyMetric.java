import java.lang.reflect.Field;

public class EnergyMetric {
	
	//load native library at runtime rapl_read.so
	static{
	// System.setProperty("java.library.path", System.getProperty("user.dir"));
	  
	/*  try {
			Field fieldSysPath = ClassLoader.class.getDeclaredField("sys_paths");
			fieldSysPath.setAccessible(true);
			fieldSysPath.set(null, null);
	  } catch (Exception e) { }
	 */
	  //System.loadLibrary("/Users/irene/Documents/GreenProject/Projects/RAPL_Interface/EnergyMetric");
	  System.loadLibrary("EnergyMetric");
	}
	
	//native method declared in rapl_read.so
	//private native static int rapl_perf(int core);
	
	//native method declared in rapl_utils.so
	public native static double getCPUEnergy();

	public static void main(String[] args){
		
	  double energyStats = getCPUEnergy();

	  try{
	    Thread.sleep(10000);
          }catch(Exception e){
	    System.err.println("Error in sleep method :" + e);
          }

          double afterEnergyStats = getCPUEnergy();
          
	  System.out.println("total energy: "+ (energyStats-afterEnergyStats)/10.0);
	} 
	
}

