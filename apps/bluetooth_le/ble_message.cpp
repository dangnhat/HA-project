/*
 * ble_message.cpp
 *
 *  Created on: Nov 12, 2014
 *      Author: AnhTrinh
 */

#include "MB1_System.h"
#include <stdio.h>
#include "ble_message.h"
#include "ble_transaction.h"
#include "device_info.h"

using namespace ble_cmd_id_ns;
using namespace ble_message_ns;
using namespace dev_id_ns;


void parse_ble_msg(uint8_t dataBuf[]){
	bleMsg bt_msg;

	switch (dataBuf[2]) {
		case NUM_OF_DEVS:
			// send number of devices to mobile
			bt_msg.len	 	= 4;
			bt_msg.cmdIDh 	= BLE_MESSAGE_SET;
			bt_msg.cmdIDl	= NUM_OF_DEVS;
			bt_msg.data		= intToUint8(4);
			send_ble_msg(bt_msg);
			break;

		case DEV_WITH_INDEX:
			// send device info to mobile
			devInfo devList[] 	= init_ble_database();
			bt_msg.len 			= 10*sizeof(devList);
			bt_msg.cmdIDh		= BLE_MESSAGE_SET;
			bt_msg.cmdIDl		= DEV_WITH_INDEX;
			bt_msg.data			= getMsgData(devList);
			send_ble_msg(bt_msg);
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
	devInfo devList[dev_list_size];

	for(uint8_t i = 0; i < sizeof(devList); i++){
		devList[i].devIdx = i;
	}

	devList[0].devID = buildDevID(0, 0, 0, BUTTON);
	devList[0].devVal = 0;

	devList[1].devID = buildDevID(1, 0, 0, ON_OFF_BULB);
	devList[1].devVal = 1;

	devList[2].devID = buildDevID(0, 0, 0, EVENT_SENSOR);
	devList[2].devVal = 1;

	devList[3].devID = buildDevID(1, 0, 0, SERVO_SG90);
	devList[3].devVal = 0xfa;


	return devList;
}

void send_ble_msg(bleMsg msg){
	uint8_t sendDataBuf[] = bleMsgToArray(msg);
	ble_cmd_attributes_write(ATT_WRITE_ADDR, 0x00, sizeof(sendDataBuf), sendDataBuf);
}

bleMsg arrToBTMsg(uint8_t dataBuf[]){
	bleMsg msg;
	msg.len 	= dataBuf[0];
	msg.cmdIDh 	= dataBuf[1];
	msg.cmdIDl	= dataBuf[2];
	for(uint8_t i = 0; i < msg.len; i++){
		msg.data[i] = dataBuf[i+3];
	}
	return msg;
}

uint8_t* 	bleMsgToArray(bleMsg BTMsg){
	uint8_t dataBuf[BTMsg.len+3];
	dataBuf[0]	= BTMsg.len;
	dataBuf[1]	= BTMsg.cmdIDh;
	dataBuf[2]	= BTMsg.cmdIDl;
	for(uint8_t i = 3; i < sizeof(dataBuf); i++){
		dataBuf[i] = BTMsg.data[i-3];
	}
	return dataBuf;
}

uint8_t*    getMsgData(devInfo devList[]){
	uint8_t dataBuf[sizeof(devList)*10];
	uint8_t devIdx[4];
	uint8_t devID[4];
	uint8_t devVal[2];
	for(uint8_t i = 0; i < sizeof(dataBuf); i++){
			switch (i%10) {
				// Get devIdx;
				case 0:
				case 1:
				case 2:
				case 3:
					devIdx = (intToUint8(devList[i/10].devIdx));
					dataBuf[i] = devIdx[i%10];
					break;
				// Get devId
				case 4:
				case 5:
				case 6:
				case 7:
					devID = (intToUint8(devList[i/10].devID));
					dataBuf[i] = devID[i%10 - 4];
					break;
				// Get val
				case 8:
				case 9:
					devVal = (convert16bitTo8bit(devList[i/10].devVal));
					dataBuf[i] = devVal[i%10 - 8];
					break;

				default:
					printf("invalid \n");
					break;
			}
	}
	return dataBuf;
}

uint8_t*    intToUint8(int input){
	uint8_t out[4];
	for(uint8_t i = 0; i < 4; i++){
		out[i] = (uint8_t)(input>>((3-i)*8));
	}
	return out;
}

uint8_t*  convert16bitTo8bit(int16_t input){
	uint8_t out[2];
	out[0]	= (uint8_t)(input>>8);
	out[1]	= (uint8_t)input;
	return out;

}
uint32_t   buildDevID(uint8_t zoneID, uint8_t nodeID, uint8_t enpID, uint8_t devID){
	uint32_t devId = ( ((uint32_t)zoneID)<<24 ) | ( ((uint32_t)nodeID)<<16 ) | \
					 ( ((uint32_t)enpID)<<8)    | ( ((uint32_t)devID));
	return devId;
}
