/*
 * device_info.h
 *
 *  Created on: Nov 10, 2014
 *      Author: dangnhat
 */

#ifndef DEVICE_INFO_H_
#define DEVICE_INFO_H_


#define	DEV_BuildDevID(zoneID, nodeID, enpID, devID)  ( (uint32_t)(devID) | \
		(uint32_t)(enpID<<8) | (uint32_t)(nodeID<<16) | (uint32_t)(zoneID<<24) )

namespace dev_id_ns{
	enum:uint8_t{
		SWITCH			=	0x01,
		BUTTON			=	0x02,
		DIMMER			=	0x03,
		LINEAR_SENSOR 	=	0x04,
		EVENT_SENSOR	=	0x05,
		ON_OFF_BULB		=	0x41,
		LEVEL_BULB		=	0x42,
		RGB_LED			=	0x43,
		SERVO_SG90		=	0x44
	};
}


#endif /* DEVICE_INFO_H_ */
