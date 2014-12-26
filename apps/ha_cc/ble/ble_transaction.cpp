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
extern "C" {
#include "msg.h"
#include "thread.h"
}


#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

using namespace ble_thread_ns;


/* USART3 receive buffer */
static const uint16_t usart3_buf_size = 256;
static uint16_t idxBuf = 0;
static uint8_t usart3_rec_buf[usart3_buf_size];

void USART3_RxInit()
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}

void ble_init()
{
    USART3_RxInit();
    bglib_output = &sendBTMessage;
    MB1_ISRs.subISR_assign(ISRMgr_ns::ISRMgr_USART3, usart3_receive);
}

void usart3_receive()
{
    USART_ClearFlag(USART3, USART_FLAG_RXNE);
//	printf("%02x \n", MB1_USART3.Get_ISR());            //DEBUG
    usart3_rec_buf[idxBuf++] = MB1_USART3.Get_ISR();

    if ((idxBuf > 1) && (idxBuf == (usart3_rec_buf[1] + 4))) {	//END of packet
        idxBuf = 0;
        receiveBTMessage();							    // Parse data from packet
    }
}

void sendBTMessage(uint8_t len1, uint8_t* data1, uint16_t len2, uint8_t* data2)
{
    //this line assumes the BLE module is in packet mode, meaning the
    //length of the packet must be specified immediately before sending
    //the packet; this line does that

    uint8_t len = len1 + (uint8_t) len2;
    MB1_USART3.Out(len);

    //this loop sends the header of the BLE Message
    for (uint8_t i = 0; i < len1; i++) {
        MB1_USART3.Out(data1[i]);
    }

    //this loop sends the payload of the BLE Message
    for (uint8_t i = 0; i < len2; i++) {
        MB1_USART3.Out(data2[i]);
    }
}

void receiveBTMessage()
{
    const struct ble_msg *BTMessage;         //holds BLE message
    struct ble_header BTHeader;				 //holds header of message
    uint8_t data[256] = "\0";	             //holds payload of message

    //read BLE message header
    BTHeader.type_hilen = usart3_rec_buf[0];
    BTHeader.lolen = usart3_rec_buf[1];
    BTHeader.cls = usart3_rec_buf[2];
    BTHeader.command = usart3_rec_buf[3];

    //read the payload of the BLE Message

    for (uint8_t i = 0; i < BTHeader.lolen; i++) {
        data[i] = usart3_rec_buf[i + 4];
    }

    //find the appropriate message based on the header, which allows
    //the ble112 library to call the appropriate handler
    BTMessage = ble_get_msg_hdr(BTHeader);

    //print error if the header doesn't match any known message header
    if (!BTMessage) {
        //handle error here
        return;
    }
    //call the handler for the received message, passing in the received payload data
    BTMessage->handler(data);

}
