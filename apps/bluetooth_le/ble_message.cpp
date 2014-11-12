/*
 * ble_message.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: AnhTrinh
 */

#include "ble_message.h"
#include "ble_transaction.h"
#include "device_info.h"

using namespace ble_cmd_id_ns;
using namespace ble_message_ns;
using namespace dev_id_ns;


void parse_ble_msg(uint8_t dataBuf[]){
	switch (dataBuf[2]) {
		case NUM_OF_DEVS:

			break;
		case DEV_WITH_INDEX:

			break;
		case DEV_VAL:

			break;
		case NUM_OF_SCENES:

			break;
		case ACT_SCENE_WITH_INDEX:

			break;
		case INACT_SCENE_WITH_INDEX:

			break;
		case NUM_OF_RULES:

			break;
		case RULE_WITH_INDEX:

			break;
		case ZONE_NAME:

			break;
		default:
			break;
	}
}

devInfo* 	init_ble_database(){
	uint8_t dev_list_size	= 4;
	devInfo devList[] = new devInfo[dev_list_size];

	for(uint8_t i = 0; i < sizeof(devList); i++){
		devList[i].devIdx = i;
	}

	devList[0].devID = DEV_BuildDevID(0, 0, 0, BUTTON);
	devList[0].devVal = 0;

	devList[1].devID = DEV_BuildDevID(1, 0, 0, ON_OFF_BULB);
	devList[1].devVal = 1;

	devList[2].devID = DEV_BuildDevID(0, 0, 0, EVENT_SENSOR);
	devList[2].devVal = 1;

	devList[3].devID = DEV_BuildDevID(1, 0, 0, SERVO_SG90);
	devList[3].devVal = 0xfa;


	return devList;
}

//
//void parse_ble_msg(uint8_t dataBuf[]){
//
//}
void send_ble_msg(ble_msg_t msg){
	ble_cmd_attributes_write(ATT_WRITE_ADDR, 0x00, msg.len+3, msg.data);
}

ble_msg_t receive_ble_msg(uint8_t dataBuf[]){
	ble_msg_t msg;
	msg.len 	= dataBuf[0];
	msg.cmdIDh 	= dataBuf[1];
	msg.cmdIDl	= dataBuf[2];
	for(uint8_t i = 0; i < msg.len; i++){
		msg.data[i] = dataBuf[i+3];
	}
}

