/**************************************************************
 * ble_transaction.h
 *
 *  Created on: Sep 29, 2014
 *  Author: AnhTrinh
 *************************************************************/

#ifndef BLE_TRANSACTION_H_
#define BLE_TRANSACTION_H_

#include "MB1_System.h"
#include "apitypes.h"
#include "cmd_def.h"
#include "ble_message.h"
#include "cir_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BUF_SIZE		(512)
#define MAX_MSGBUF_SIZE		(1024)
#define ATT_WRITE_ADDR    	(0x0B)

extern uint8_t rxBuf[MAX_BUF_SIZE];
extern uint8_t idxBuf;
extern uint8_t attBuf[MAX_MSGBUF_SIZE];
extern cir_queue ble_msg_queue;

namespace ble_thread_ns {
extern int16_t ble_thread_pid;
}

struct ble_serv_stt_s {
    uint8_t ready;
    uint8_t connected;
};

void USART3_RxInit(void);
void ble_init(void);
void usart3_receive(void);
void sendBTMessage(uint8_t len1, uint8_t* data1, uint16_t len2, uint8_t* data2);
void receiveBTMessage(void);

extern ble_serv_stt_s BTFlags;

#ifdef __cplusplus
}
#endif

#endif /* BLE_TRANSACTION_H_ */
