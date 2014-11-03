package anh.trinh.ble_demo.data;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;

import android.bluetooth.BluetoothGattCharacteristic;
import android.content.Intent;
import android.util.Log;
import anh.trinh.ble_demo.BluetoothLeService;
import anh.trinh.ble_demo.HomeActivity;

public class ProcessMsgPacket {
	private static HomeActivity mContext;
	
	public ProcessMsgPacket(HomeActivity mContext){
		ProcessMsgPacket.mContext = mContext;
	}

	/**
     * Get BLE device data
     * 
     * @param intent
     * @return
     */
    public static BluetoothMessage getBLEMessage(Intent intent){
    	byte[] 				recBuf = intent.getByteArrayExtra(BluetoothLeService.EXTRA_DATA);
    	BluetoothMessage 	msg	   = parseBTMessage(recBuf);
		return msg;
    
    }
    
    /**
     * Set BLE device data
     * 
     * @param characteristic
     * @param msg
     * @return
     */
    public static boolean putBLEMessage(BluetoothGattCharacteristic characteristic, BluetoothMessage msg){
    	ByteBuffer sendBuf = ByteBuffer.allocate(msg.getLength()+3);
    	sendBuf.put(msg.getLength());
    	sendBuf.put(msg.getCmdIdH());
    	sendBuf.put(msg.getCmdIdL());
    	sendBuf.put(msg.getPayload());
    	characteristic.setValue(sendBuf.array());
    	mContext.mBluetoothLeService.writeCharacteristic(characteristic);
    	return true;
    }
    
    /**
     * Parse Data Buffer receive to Bluetooth Message
     * 
     * @param message
     * @return
     */
    public static BluetoothMessage parseBTMessage(byte[] msg){
    	BluetoothMessage BTMsg;
    	byte  	len 	= msg[0];
    	byte  	cmdIdH	= msg[1];
    	byte  	cmdIdL	= msg[2];
    	byte[]	payload = new byte[len];
    	for(int i = 0; i < len; i++){
    		payload[i] = msg[i+3];
    	}
    
    	BTMsg = new BluetoothMessage(len, cmdIdH, cmdIdL, payload);							
    	
    	return BTMsg;
    }
    
    
    /**
     * Process Message from BLE device
     * 
     * @param msgQueue
     */
    public static void processBTMessageQueue(ArrayList<BluetoothMessage> msgQueue){
    	
    	DeviceInfo          mDevice = null;
    	ByteBuffer 			devIdx 	= null;
    	ByteBuffer 			devID  	= null;
    	ByteBuffer 			devVal 	= null;
    	int					len 	= 0;
    	
    	for(BluetoothMessage msg : msgQueue){
    		len +=  msg.getLength();
    	}
    	ByteBuffer dataBuf = ByteBuffer.allocate(len);
    	
    	for (BluetoothMessage msg : msgQueue){
    		
    		// Consider type of messages
    		switch (msg.getCmdIdL()) {
			case NodeInfoDefination.NUM_OF_DEVS:
				// number of devices in index = 2; 
//				numOfDev = (byte) Arrays.binarySearch(payload.get(0), (byte) 2);
				// after receive num of dev, Mobile want to get info of all of devices
				ByteBuffer getAllDev = ByteBuffer.allocate(4);
				getAllDev.putInt(0xffffffff);
				putBLEMessage(mContext.mWriteCharacteristic, new BluetoothMessage( 	(byte) 	1,
																					(byte) 	BTMsgTypeDef.GET,
																					(byte) 	NodeInfoDefination.DEV_WITH_INDEX,
																							getAllDev.array()) );
				break;
			case NodeInfoDefination.DEV_WITH_INDEX:
				dataBuf.put(msg.getPayload());
				break;

			case NodeInfoDefination.DEV_VAL:
				
				break;
				
			default:
				break;
			}
    		
    	}
    	
    	/*
    	 * Parse payload content device information to List of device object
    	 * 
    	 */
    	for(int i = 0; i < dataBuf.array().length; i++){
    		switch (i%10) {
			case 0:
				mDevice = new DeviceInfo();
				devIdx  = ByteBuffer.allocate(4);
				devIdx.put(dataBuf.get());
				break;
			case 1:
			case 2:
				devIdx.put(dataBuf.get());
				break;
			case 3:
				devIdx.put(dataBuf.get());
				mDevice.setDevIdx(devIdx.getInt());
				devIdx.clear();
				break;
			case 4:
				devID  = ByteBuffer.allocate(4);
				devID.put(dataBuf.get());
				break;
			case 5:
			case 6:
				devID.put(dataBuf.get());
				break;
			case 7:
				devID.put(dataBuf.get());
				mDevice.setDevID(devID.getInt());
				devID.clear();
				break;
			case 8:
				devVal = ByteBuffer.allocate(2);
				devVal.put(dataBuf.get());
				break;
			case 9:
				devVal.put(dataBuf.get());
				mDevice.setDevVal(devVal.getShort());
				devVal.clear();
				mContext.mDevInfoList.add(mDevice);
				mDevice = null;
				break;
			default:
				Log.i("DataInfo", "error");
				break;
			}
    	}
    }
    

    
    
}
