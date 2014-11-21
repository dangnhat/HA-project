/*************************************************************************
 *
 *
 *
 *	Author:	AnhTrinh
 *
 ************************************************************************/
#include "ble_transaction.h"
#include "MB1_System.h"
#include <stdio.h>

void USART3_RxInit()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel 						= USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 3;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}


void ble_init()
{
	USART3_RxInit();
	bglib_output = &sendBTMessage;
}

void isr_usart3()
{
	USART_ClearFlag(USART3, USART_FLAG_RXNE);
//	printf("%02x \n", MB1_USART3.Get_ISR()); //DEBUG
	rxBuf[idxBuf++] = MB1_USART3.Get_ISR();
	if(idxBuf == (rxBuf[1]+4)){	//END of packet
		idxBuf = 0;
//		printf("$\n");
		receiveBTMessage();		// Parse data from packet
	}
}

void sendBTMessage(uint8_t len1, uint8_t* data1, uint16_t len2, uint8_t* data2)
{
	//this line assumes the BLE module is in packet mode, meaning the
	//length of the packet must be specified immediately before sending
	//the packet; this line does that

	uint8_t len = len1 + (uint8_t)len2;
	MB1_USART3.Out(len);

	//this loop sends the header of the BLE Message
	for(uint8_t i = 0; i < len1; i++)
	{
		MB1_USART3.Out(data1[i]);
	}

	//this loop sends the payload of the BLE Message
	for(uint8_t i = 0; i < len2; i++)
	{
		MB1_USART3.Out(data2[i]);
	}
}


void receiveBTMessage()
{
	const struct	ble_msg		*BTMessage;				//holds BLE message
	struct			ble_header 	BTHeader;				//holds header of message
					uint8_t		data[256]	=	"\0";	//holds payload of message

//	rxBuf[index++] =	MB1_USART3.Get_ISR();

	//read BLE message header
	BTHeader.type_hilen 	= rxBuf[0];
	BTHeader.lolen 			= rxBuf[1];
	BTHeader.cls 			= rxBuf[2];
	BTHeader.command 		= rxBuf[3];

	//read the payload of the BLE Message
//	printf("len = %d \n", BTHeader.lolen);

	for(uint8_t i = 0; i < BTHeader.lolen; i++){
		data[i]	= rxBuf[i+4];
//		printf("%02x \n", data[i]);
	}

	//find the appropriate message based on the header, which allows
	//the ble112 library to call the appropriate handler
	BTMessage = ble_get_msg_hdr(BTHeader);

	//print error if the header doesn't match any known message header
	if(!BTMessage)
	{
		//handle error here
		printf("Don't match any msg header \n");
		//Disable interrupt usart3
		USART_ITConfig(USART3, USART_FLAG_RXNE, DISABLE);
		return;
	}
	//call the handler for the received message, passing in the received payload data
	BTMessage->handler(data);

}
