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
#include "cir_queue.h"
#include "cc_msg_id.h"

#define ATT_WRITE_ADDR    	(0x08)

namespace ha_ble_ns {

enum ble_msg_type_id
    : uint8_t {
        BLE_MSG_ACK,
    BLE_MSG_DATA,
};
}

namespace ble_thread_ns {
extern int16_t ble_thread_pid;
/*controller message queue */
extern cir_queue controller_to_ble_msg_queue;
}

struct ble_ack_s {
    bool need_to_wait_ack = false;
    uint16_t packet_index = 0;
};

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

/* start interrupt thread */
//void usart_int_thread_start(void);

/* attach header to ble message */
void add_hdr_to_ble_msg(uint8_t msgType, uint8_t* ack_idx_buf,
        uint8_t* payload);

/* send ack to mobile */
void send_ack_to_mobile(uint8_t* ack_idx_buf);
#endif /* BLE_TRANSACTION_H_ */
