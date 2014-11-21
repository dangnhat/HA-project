package anh.trinh.ble_demo.data;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

import anh.trinh.ble_demo.list_view.Device_c;

/**
 * @version 1.0
 * @author Anh Trinh
 * @category Data Type
 *
 */


public class DataConversion {
	
	/**
	 * Convert byte to unsigned byte
	 * 
	 * @param number
	 * @return
	 */
	 public static int byteType(byte number){
    	int result;
    	ByteBuffer mTempBuf = ByteBuffer.allocate(4);
    	if( (number & 0x80) != 0){
    		result = number*0x000000ff;
    		result = mTempBuf.put(new byte[]{0x00, 0x00, 0x00, number}).getInt(0);
    		mTempBuf.clear();
    	}
    	else{
    		result = number;
    	}
    	return result;
	 }
	 
	 /**
	  * Convert integer to byte[] array
	  * 
	  * @param inNum
	  * @return
	  */
	 public static byte[] int2ByteArr(int inNum){
		 byte[] retBuf = new byte[4];
		 retBuf[0] = (byte)(inNum >> 24);
		 retBuf[1] = (byte)(inNum >> 16);
		 retBuf[2] = (byte)(inNum >> 8);
		 retBuf[3] = (byte)(inNum );
		 return retBuf;
		 
	 }
	 
	 /**
	  * Convert short to byte[] array
	  * 
	  * @param inNum
	  * @return
	  */
	 public static byte[] short2ByteArr(short inNum){
		 byte[] retBuf = new byte[2];
		 retBuf[0] = (byte)(inNum >> 8);
		 retBuf[1] = (byte)(inNum);
		 return retBuf;
	 }
	 
	 
	 /**
	  * Convert device info to byte[] array
	  * 
	  * @param devInfo
	  * @return
	  */
	 public static byte[] devInfo2ByteArr(Device_c devInfo){
		 ByteBuffer tempBuf = ByteBuffer.allocate(6);
		 tempBuf.put(int2ByteArr(devInfo.getID()) );
		 tempBuf.put(short2ByteArr(devInfo.getVal()));
		return tempBuf.array(); 
	 }


}
