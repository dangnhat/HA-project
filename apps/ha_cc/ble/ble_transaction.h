/**************************************************************
 * ble_transaction.h
 *
 *  Created on: Sep 29, 2014
 *  Author: AnhTrinh
 *************************************************************/

#ifndef BLE_TRANSACTION_H_
#define BLE_TRANSACTION_H_

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)

#include "MB1_System.h"
#include "apitypes.h"
#include "cmd_def.h"
#include "cir_queue.h"
#include "ha_debug.h"
#include "cc_msg_id.h"

#define MAX_BUF_SIZE		(256)
#define MAX_MSGBUF_SIZE		(1024)
#define ATT_WRITE_ADDR    	(0x0B)

namespace ble_thread_ns {
extern int16_t ble_thread_pid;
extern uint16_t idxBuf;
extern uint8_t  usart3_rec_buf[MAX_BUF_SIZE];
/*controller message queue */
extern const uint16_t controller_to_ble_msg_queue_size;
extern uint8_t controller_to_ble_msg_queue_buf[MAX_MSGBUF_SIZE];
extern cir_queue controller_to_ble_msg_queue;
}

/* initial usart3 interrupt */
void USART3_RxInit(void);

/* BLE device transaction initialation */
void ble_init(void);

/* usart3 interrupt receive */
void usart3_receive(void);

/* send Message to BLE device */
void sendBTMessage(uint8_t len1, uint8_t* data1, uint16_t len2, uint8_t* data2);

/* receive Message from BLE device */
void receiveBTMessage(void);

/* start ble thread */
void ble_thread_start(void);

#endif /* BLE_TRANSACTION_H_ */
