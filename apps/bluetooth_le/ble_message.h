/*
 * ble_message.h
 *
 *  Created on: Nov 10, 2014
 *      Author: dangnhat
 */

#ifndef BLE_MESSAGE_H_
#define BLE_MESSAGE_H_

namespace ble_message_ns{
	enum:uint16_t{
		BLE_MESSAGE_GET		=	0x0000,
		BLE_MESSAGE_SET 	=	0x0001,

		BLE_USART_REC   	=	0x0010,
		BLE_SERVER_RESET	=	0x0011,
		BLE_CLIENT_WRITE	=	0x0002
	};
}




#endif /* BLE_MESSAGE_H_ */
