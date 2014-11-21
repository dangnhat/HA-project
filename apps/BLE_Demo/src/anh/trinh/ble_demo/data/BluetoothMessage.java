package anh.trinh.ble_demo.data;

public class BluetoothMessage {
	private byte 	length;
	private byte	cmdIdH;
	private byte    cmdIdL;
	private byte[]  payload;
	
	public BluetoothMessage(byte length, byte cmdIdH, byte cmdIdL, byte[] payload){
		this.length 	= length;
		this.cmdIdH 	= cmdIdH;
		this.cmdIdL 	= cmdIdL;
		this.payload 	= payload;
	}
	
	public BluetoothMessage(){
		
	}
	
	public void setLength(byte length){
		this.length = length;
	}
	
	public byte getLength(){
		return length;
	}
	
	public void setCmdIdH(byte cmdIdH){
		this.cmdIdH = cmdIdH;
	}
	
	public byte getCmdIdH(){
		return cmdIdH;
	}
	
	public void setCmdIdL(byte cmdIdL){
		this.cmdIdL = cmdIdL;
	}
	
	public byte getCmdIdL(){
		return cmdIdL;
	}
	
	public void setPayload(byte[] payload){
		this.payload = payload;
	}
	
	public byte[] getPayload(){
		return payload;
	}
	
}
