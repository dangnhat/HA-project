package anh.trinh.ble_demo.data;

/***********************************************************************************************
 * 						Packet format:
 *  ____________________________________________________________
 * | Length | CmdId(1) | CmdId(2) | Payload
 * |____1___|____1_____|_____1____|_____________________________
 * 
 * Length: Length of payload
 * CmdId1:
 * 		- GET
 * 		- SET
 * CmdId2:
 * 		- NUM_OF_DEVS
 * 		- DEV_WITH_INDEX
 * 		- DEV_VAL	
 * 		- ...
 * 
 **********************************************************************************************/

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;

import android.bluetooth.BluetoothGattCharacteristic;
import android.content.Intent;
import android.os.Message;
import android.util.Log;
import anh.trinh.ble_demo.BluetoothLeService;
import anh.trinh.ble_demo.DeviceControlFragment;
import anh.trinh.ble_demo.HomeActivity;

public class ProcessBTMsg {
	private HomeActivity mContext;
	private final static String TAG = "ProcessMessage";
	
	public ProcessBTMsg(HomeActivity mContext){
		this.mContext = mContext;
	}

	/**
     * Get BLE device data
     * 
     * @param intent
     * @return
     */
    public BluetoothMessage getBLEMessage(Intent intent){
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
    public void putBLEMessage(BluetoothGattCharacteristic characteristic, BluetoothMessage msg){
    	ByteBuffer sendBuf = ByteBuffer.allocate(msg.getLength()+3);
    	sendBuf.put(msg.getLength());
    	sendBuf.put(msg.getCmdIdH());
    	sendBuf.put(msg.getCmdIdL());
    	if(msg.getLength() != 0){
    		sendBuf.put(msg.getPayload());
    	}
    	characteristic.setValue(sendBuf.array());
    	sendBuf.clear();
    	mContext.mBluetoothLeService.writeCharacteristic(characteristic);
    }
    
    /**
     * Parse Data Buffer receive to Bluetooth Message
     * 
     * @param message
     * @return
     */
    public BluetoothMessage parseBTMessage(byte[] msg){
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
    public void processBTMessageQueue(ArrayList<BluetoothMessage> msgQueue){
    	
    	int	len = 0;
    	ByteBuffer devID  = ByteBuffer.allocate(4);
    	ByteBuffer devVal = ByteBuffer.allocate(2);
    	Message handlerMsg;
    	//Get length of payload
    	for(BluetoothMessage msg : msgQueue){
    		len +=  msg.getLength();
    	}
    	ByteBuffer dataBuf = ByteBuffer.allocate(len);
    	
    	//Get data of payload
    	for (BluetoothMessage msg : msgQueue){
    		dataBuf.put(msg.getPayload());
    	}
    	
//    	Log.i(TAG, "DataBufSize = " + dataBuf.array().length);
    	
    	//Analyze kind of massage
    	switch (msgQueue.get(0).getCmdIdL()) {
    	
		case CommandID.NUM_OF_DEVS:
			mContext.mNumOfDev = dataBuf.get(0);
			handlerMsg = mContext.mMsgHandler.obtainMessage(CommandID.NUM_OF_DEVS);
			mContext.mMsgHandler.sendMessage(handlerMsg);
			break;
		case CommandID.DEV_WITH_INDEX:
			mContext.mDevInfoList = getDeviceList(dataBuf);
			
			//send message to handler
			handlerMsg = mContext.mMsgHandler.obtainMessage(CommandID.DEV_WITH_INDEX);
			mContext.mMsgHandler.sendMessage(handlerMsg);
			break;
		case CommandID.DEV_VAL:
			//update device value from CC
			mContext.mDevInfoList.get(0).setDevVal((short) 1);
			DeviceControlFragment mDeviceFrag = (DeviceControlFragment)mContext.getSupportFragmentManager()
					.getFragments().get(0);
			mDeviceFrag.updateUI(mContext.mDevInfoList);
			devID.clear();
			devVal.clear();
			break;

		default:
			break;
		}
    	//Clear buffer
    	dataBuf.clear();
    	
    }
    
    
    /**
     * Get List of Devices from payload of Message queue
     * 
     * @param dataBuf
     * @return mListOfDev
     */
    public ArrayList<DeviceInfo> getDeviceList(ByteBuffer dataBuf){
    	ArrayList<DeviceInfo> mListOfDev = new ArrayList<DeviceInfo>();
    	DeviceInfo mDevice = null;
		ByteBuffer devIdx = null;
		ByteBuffer devID = null;
		ByteBuffer devVal = null;
      	// Parse payload content device information to List of device object
//    	for(int i = 0; i < 40; i++){
    	for(int i = 0; i < dataBuf.array().length; i++){
			switch (i%10) {
			case 0:
				mDevice = new DeviceInfo();
				devIdx  = ByteBuffer.allocate(4);
				devIdx.put(dataBuf.get(i));
				break;
			case 1:
			case 2:
				devIdx.put(dataBuf.get(i));
				break;
			case 3:
				devIdx.put(dataBuf.get(i));
				mDevice.setDevIdx(devIdx.getInt(0));
				devIdx.clear();
				break;
			case 4:
				devID  = ByteBuffer.allocate(4);
				devID.put(dataBuf.get(i));
				break;
			case 5:
			case 6:
				devID.put(dataBuf.get(i));
				break;
			case 7:
				devID.put(dataBuf.get(i));
				mDevice.setDevID(devID.getInt(0));
				devID.clear();
				break;
			case 8:
				devVal = ByteBuffer.allocate(2);
				devVal.put(dataBuf.get(i));
				break;
			case 9:
				devVal.put(dataBuf.get(i));
				mDevice.setDevVal(devVal.getShort(0));
				devVal.clear();
				mListOfDev.add(mDevice);
				mDevice = null;
				break;
			default:
				Log.i("DataInfo", "error");
				break;
			}
    	}
		return mListOfDev;
    }
    /**
	 * Create Message for testing in Mobile
	 *
	 */
	public ArrayList<BluetoothMessage> createMessageQueue(){
		ArrayList<BluetoothMessage> mMsgList = new ArrayList<BluetoothMessage>();
		BluetoothMessage 			mBLEMsg;
		byte[] 						msgArray = new byte[43];
		
		msgArray[0] = 40;
		msgArray[1] = CommandID.SET;
		msgArray[2] = CommandID.NUM_OF_DEVS;
		
		// LEVEL BULB
		msgArray[3] = 0x00;							// device index = 0
		msgArray[4] = 0x00;
		msgArray[5] = 0x00;
		msgArray[6] = 0x00;
		
		msgArray[7] = 0x00;							// zone ID = 0
		msgArray[8] = 0x00;							// node ID = 0
		msgArray[9] = 0x00;							// endpoint ID = 0
		msgArray[10] = DeviceTypeDef.LEVEL_BULB; 	// Device ID = LEVEL BULB
		msgArray[11] = 0x02;						// Device Value;
		msgArray[12] = 0x00;						//
		
		// DIMMER
		msgArray[13] = 0x00;						// device index = 1
		msgArray[14] = 0x00;
		msgArray[15] = 0x00;
		msgArray[16] = 0x01;
		
		msgArray[17] = 0x01;						// zone ID = 1
		msgArray[18] = 0x00;						// node ID = 0
		msgArray[19] = 0x00;						// endpoint ID = 0
		msgArray[20] = DeviceTypeDef.DIMMER; 		// Device ID = DIMMER
		msgArray[21] = 0x02;
		msgArray[22] = 0x00;
		
		// TEMP SENSOR
		msgArray[23] = 0x00;						// device index = 2
		msgArray[24] = 0x00;
		msgArray[25] = 0x00;
		msgArray[26] = 0x02;
		
		msgArray[27] = 0x00;						// zone ID = 0
		msgArray[28] = 0x00;						// node ID = 0
		msgArray[29] = 0x00;						// endpoint ID = 0
		msgArray[30] = DeviceTypeDef.LINEAR_SENSOR; 	// Device ID = TEMP SENSOR
		msgArray[31] = 0x20;						// Device Value
		msgArray[32] = 0x13;
		
		// BUTTON
		msgArray[33] = 0x00;						// device index = 3
		msgArray[34] = 0x00;
		msgArray[35] = 0x00;
		msgArray[36] = 0x03;
		
		msgArray[37] = 0x01;						// zone ID = 1
		msgArray[38] = 0x00;						// node ID = 0
		msgArray[39] = 0x00;						// endpoint ID = 0
		msgArray[40] = DeviceTypeDef.BUTTON; 		// Device ID = BUTTON
		msgArray[41] = 0x00;						// Device Value
		msgArray[42] = 0x01;
		
		mBLEMsg =  parseBTMessage(msgArray);
		mMsgList.add(mBLEMsg);
		
		return mMsgList;
		
	}
    
    
}
