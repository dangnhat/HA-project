#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

extern "C" {
#include "transceiver.h"
#include "board_uart0.h"
#include "thread.h"
#include "board.h"
#include "hwtimer.h"
#include "vtimer.h"
#include "msg.h"
#include "cc110x_ng.h"

#include "cc110x_reconfig.h"
}

#include "MB1_System.h"
#include "ble_transaction.h" //used by ANH
#include "cir_queue.h"

using namespace ble_message_ns;

/*******************************************************************************
 * Variables & Data Buffer
 *
 ******************************************************************************/
const uint16_t ble_thread_stack_size = 2048;
char ble_thread_stack[ble_thread_stack_size];

uint8_t rxBuf[MAX_BUF_SIZE] = "\0";	// hold usart3 rx data
uint8_t idxBuf = 0;

uint8_t attBuf[MAX_MSGBUF_SIZE];
cir_queue ble_msg_queue = cir_queue(attBuf, MAX_MSGBUF_SIZE);

//test data

uint8_t tempBuf1[] = {0x06, 0x01, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};
uint8_t tempBuf2[] = {0x06, 0x01, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01};

/*******************************************************************************
 * Private Functions
 *
 ******************************************************************************/

void *ble_transaction(void *arg);

/*******************************************************************************
 * Main Function
 *
 ******************************************************************************/

/* main */
int main()
{
    /* Initial MBoard-1 system */
    MB1_system_init();
    /* Initial BLE interface */
    ble_init();
//    vtimer_init();

    ble_thread_ns::ble_thread_pid = thread_create(ble_thread_stack,
                                    ble_thread_stack_size,
                                    PRIORITY_MAIN - 2,
                                    0,
                                    ble_transaction,
                                    NULL, "ble thread");
//	printf("%x\n", ble_thread_ns::ble_thread_pid);

    while (1) {

		if(Btn_ns::newKey == MB1_usrBtn0.pressedKey_get()){
			/* Set BLE device discoverable */
//		    msg_t msg;
//		    msg.type = ble_cmd_id_ns::DEV_VAL;
//
//		    msg.content.ptr =
		    ble_cmd_attributes_write(ATT_WRITE_ADDR, 0, sizeof(tempBuf1), tempBuf1);
		}
//		else{
//		    ble_cmd_attributes_write(ATT_WRITE_ADDR, 0, sizeof(tempBuf2), tempBuf2);
//		}

//		vtimer_usleep(1000);


    }
}

/*******************************************************************************
 * Functions implementation
 *
 ******************************************************************************/
/* ble_thread_func */
void *ble_transaction(void *arg)
{

    msg_t msg;
    uint8_t numOfMsg = 0;
    uint8array* msgPtr;
    uint8_t msgLen;
    uint8_t msgBuf[256];
    uint8_t sumOfMsgLen = 0;
    const uint8_t msg_queue_size = 64;
    msg_t msg_queue[msg_queue_size];

    msg_init_queue(msg_queue, msg_queue_size);
    while (1) {

        msg_receive(&msg);
        switch (msg.type) {

        case BLE_SERVER_RESET:
            printf("--- server reset ---\n");
            // Make BLE device discoverable
            ble_cmd_gap_set_mode(gap_general_discoverable,
                    gap_undirected_connectable);
            ble_cmd_sm_set_bondable_mode(1);
            break;
        case BLE_USART_REC:
//				printf("--- usart rec ---\n");
            //determine type of ble message
            receiveBTMessage();
            break;
        case BLE_CLIENT_WRITE:
            printf("--- client write ---\n");
            //get bluetooth message
            msgPtr = reinterpret_cast<uint8array*>(msg.content.ptr);
            numOfMsg++;
            sumOfMsgLen += msgPtr->len;
            printf("cnt = %d\n", numOfMsg);
            printf("clen = %d\n", msgPtr->len);
            if (1 == numOfMsg) {
                //the first byte of message is length of data
                msgLen = msgPtr->data[0] + 3;       //plus 3 bytes of header
//					msg_len = client_msg->data[0] + 1;	//test
                printf("len= %d\n", msgLen);
            }

            if (sumOfMsgLen == msgLen) {
                printf("end of packet\n");
                numOfMsg = 0;
                sumOfMsgLen = 0;
                ble_msg_queue.get_data(msgBuf, msgLen);
                parse_ble_msg(msgBuf, msgLen);
            }

            break;
        default:
            printf("tao lao\n");
            break;
        }
    }

    return NULL;
}
