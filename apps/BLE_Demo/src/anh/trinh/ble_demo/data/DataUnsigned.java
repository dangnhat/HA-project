package anh.trinh.ble_demo.data;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

/**
 * @version 1.0
 * @author Anh Trinh
 * @category Data Type
 *
 */


public class DataUnsigned {
	
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


}
