
package anh.trinh.ble_demo.data;

/**
 * @version 1.0
 * @author Anh Trinh
 * 
 *
 */

public class DeviceInfo {
	
	private int devIdx;
	private int devID;
	private short devVal;
	
	public DeviceInfo() {
		// TODO Auto-generated constructor stub
	}
	
	public DeviceInfo(int devIdx, int devID, short devVal){
		this.devIdx = devIdx;
		this.devID 	= devID;
		this.devVal = devVal;
	}
	
	/**
	 * Set device index value
	 * 
	 * @param devIdx
	 */
	public void setDevIdx(int devIdx){
		this.devIdx = devIdx;
	}
	
	/**
	 * Get device index value
	 * 
	 * @return devIdx
	 */
	public int getDevIdx(){
		return devIdx;
	}
	
	public void setDevID(int devID){
		this.devID = devID;
	}
	
	public int getDevID(){
		return devID;
	}
	
	public void setDevVal(short devVal){
		this.devVal = devVal;
	}
	
	public short getDevVal(){
		return devVal;
	}
	

}
