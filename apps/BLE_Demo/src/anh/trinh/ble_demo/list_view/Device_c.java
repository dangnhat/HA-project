package anh.trinh.ble_demo.list_view;

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
	
	public int getVal(){
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
		case DeviceTypeDef.GAS_SENSOR:
			this.name = "Gas Sensor";
			break;
		case DeviceTypeDef.LEVEL_BULB:
			this.name = "Level Bulb";
			break;
		case DeviceTypeDef.LIGHT_SENSOR:
			this.name = "Light Sensor";
			break;
		case DeviceTypeDef.MOTOR:
			this.name = "Motor";
			break;
		case DeviceTypeDef.ON_OFF_BULB:
			this.name = "On/Off Bulb";
			break;
		case DeviceTypeDef.PIR_SENSOR:
			this.name = "PIR Sensor";
			break;
		case DeviceTypeDef.RGB_LED:
			this.name = "RGB Led";
			break;
		case DeviceTypeDef.TEMP_SENSOR:
			this.name = "Temp Sensor";
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
