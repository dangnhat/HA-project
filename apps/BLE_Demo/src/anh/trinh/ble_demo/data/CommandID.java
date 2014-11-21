package anh.trinh.ble_demo.data;

public final class CommandID {
	
	// Message type
	public static final int SET = 0x00;
	public static final int GET = 0x01;
	
	// Devices
	public static final int DEV_VAL 				= 0x00;
	public static final int NUM_OF_DEVS 			= 0x01;
	public static final int DEV_WITH_INDEX			= 0x02;
	
	//Scenes
	public static final int NUM_OF_SCENES 			= 0x03;
	public static final int ACT_SCENE_WITH_INDEX 	= 0x04;
	public static final int INACT_SCENE_WITH_INDEX 	= 0x05;
	
	//Rules
	public static final int NUM_OF_RULES 	= 0x06;
	public static final int RULE_WITH_INDEX = 0x07;
	
	//Zone
	public final static int ZONE_NAME = 0x08;
}
