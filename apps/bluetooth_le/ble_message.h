/*
 * ble_message.h
 *
 *  Created on: Nov 10, 2014
 *      Author: AnhTrinh
 */

#ifndef BLE_MESSAGE_H_
#define BLE_MESSAGE_H_

#include "MB1_System.h"

namespace ble_message_ns{
	enum:uint16_t{
		BLE_MESSAGE_GET		=	0x0000,
		BLE_MESSAGE_SET 	=	0x0001,

		BLE_USART_REC   	=	0x0010,
		BLE_SERVER_RESET	=	0x0011,

		BLE_CLIENT_WRITE	=	0x0020
	};
}	// ble_message_ns

namespace ble_cmd_id_ns{
	enum:uint8_t{
		//device def
		NUM_OF_DEVS				= 	0x00,
		DEV_WITH_INDEX  		=  	0x01,
		DEV_VAL					=	0x02,
		//scene def
		NUM_OF_SCENES			= 	0x10,
		ACT_SCENE_WITH_INDEX	=	0x11,
		INACT_SCENE_WITH_INDEX	=	0x12,
		//rule def
		NUM_OF_RULES			=	0X20,
		RULE_WITH_INDEX			=	0x21,
		//zone
		ZONE_NAME				=	0x30
	};
}	// ble_cmd_id_ns

typedef struct ble_msg_t{
	uint8_t len;
	uint8_t cmdIDh;
	uint8_t cmdIDl;
	uint8_t data[];
}bleMsg;


typedef struct devInfo_t{
	uint32_t  devIdx;
	uint32_t  devID;
	int16_t	  devVal;

}devInfo;


/**
 * Initial list of devices
 */
void 	init_ble_database(devInfo* devList, uint8_t len);

/**
 *  Create Bluetooth Message
 */

bleMsg create_ble_msg();

/**
 * Receive message from Mobile
 */
bleMsg	arrToBTMsg(uint8_t dataBuf[]);

/**
 * Send message to Mobile
 */
void        send_ble_msg(bleMsg msg);

/**
 *
 */
void  parse_ble_msg(uint8_t dataBuf[]);

/**
 *  Convert Bluetooth message to Array of bytes
 */
void bleMsgToArray(bleMsg BTMsg, uint8_t* buffer, uint8_t bufLen);

/**
 * Convert list of device to byte array
 */
void getMsgData(devInfo devList[], uint8_t* buffer, uint8_t bufLen);

/**
 * Convert 32bit to array 8bit
 */
void   intToUint8(int inNum, uint8_t* buffer);

/**
 * Convert 16bit to array 8bit
 */
void   convert16bitTo8bit(int16_t inNum, uint8_t* buffer);

/**
 * Create device ID
 */
uint32_t   buildDevID(uint8_t zoneID, uint8_t nodeID, uint8_t enpID, uint8_t devID);

#endif /* BLE_MESSAGE_H_ */
