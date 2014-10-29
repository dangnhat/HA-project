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

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_BUF_SIZE		512
#define MAX_MSGBUF_SIZE		256

extern uint8_t rxBuf[MAX_BUF_SIZE];
extern uint8_t idxBuf;
extern uint8_t attBuf[MAX_MSGBUF_SIZE];

struct ble_serv_stt_s{
	uint8_t	ready;
	uint8_t connected;
};

void	USART3_RxInit(void);
void	ble_init(void);
void	isr_usart3(void);
void	sendBTMessage(uint8_t len1, uint8_t* data1, uint16_t len2, uint8_t* data2);
void	receiveBTMessage(void);


extern ble_serv_stt_s BTFlags;

#ifdef __cplusplus
}
#endif

#endif /* BLE_TRANSACTION_H_ */
