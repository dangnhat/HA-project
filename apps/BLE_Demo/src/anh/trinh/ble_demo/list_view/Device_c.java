package anh.trinh.ble_demo.list_view;

import android.R;
import anh.trinh.ble_demo.data.DeviceTypeDef;

public class Device_c {
	
	private String name;
	private short	val;
	private byte childID;
	
	public Device_c(String name, short val){
		this.name = name;
		this.val  = val;
	}
	
	public Device_c() {
		// TODO Auto-generated constructor stub
	}

	public String getName(){
		return name;
	}
	
	public short getVal(){
		return val;
	}
	
	public void setName(byte devID){
		this.childID = devID;
		switch (devID) {
		case DeviceTypeDef.SWITCH:
			this.name = "Switch";
			break;
		case DeviceTypeDef.BUTTON:
			this.name = "Button";
			break;
		case DeviceTypeDef.DIMMER:
			this.name = "Dimmer";
			break;	
		case DeviceTypeDef.EVENT_SENSOR:
			this.name = "Gas Sensor";
			break;
		case DeviceTypeDef.LEVEL_BULB:
			this.name = "Level Bulb";
			break;
		case DeviceTypeDef.LINEAR_SENSOR:
			this.name = "Light Sensor";
			break;
		case DeviceTypeDef.SERVO_SG90:
			this.name = "Servo SG90";
			break;
		case DeviceTypeDef.ON_OFF_BULB:
			this.name = "ON/OFF Bulb";
			break;
		case DeviceTypeDef.RGB_LED:
			this.name = "RGB Led";
			break;
		default:
			this.name = "Unknown Device";
			break;
		}

	}
	
	public void setVal(short val){
		this.val = val;
	}
	
	public byte getID(){
		return childID;
	}
}
