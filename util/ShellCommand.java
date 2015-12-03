package util;

import java.io.BufferedReader;
import java.io.*;

public class ShellCommand{

	public static String execute(String command){
	  
	  Process p;
	  StringBuffer output = new StringBuffer();

	  try{
	    p = Runtime.getRuntime().exec(command);
	    p.waitFor();
	    BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()));
	    String line = "";
	    
	    while((line = reader.readLine())!=null){
		output.append(line + "\n");
	    }
	  }catch(Exception e){
	    e.printStackTrace();
	  }

	  return output.toString();
	
	}
}
