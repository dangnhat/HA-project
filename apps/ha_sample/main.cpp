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
}

#include "MB1_System.h"
#include "cc110x_reconfig.h"

using namespace Btn_ns;

using namespace std;

/*******************************************************************************
 * Variables & Data Buffer
 *
 ******************************************************************************/
uint8_t 			rxBuf[MAX_BUF_SIZE]		= "\0";	// hold usart3 rx data
uint8_t 			idxBuf					= 0;
ble_serv_stt_s 		BTFlags;
uint8_t 			msgBuf[]				=  "This is default data of BLE112A";
uint8_t				attBuf[MAX_MSGBUF_SIZE];


/*******************************************************************************
 * Private Functions
 *
 ******************************************************************************/



/* main */
int main() {
	/* Initial MBoard-1 system */
	MB1_system_init();
	/* Initial BLE interface */
	ble_init();


//	while(BTFlags.ready != 0x01){
//
//	}
//	ble_cmd_attributes_write(0x000b, 0, 31, msgBuf);
	while(1){

		if(newKey == MB1_usrBtn0.pressedKey_get()){
			MB1_Led_green.toggle();	//DEBUG
			/* Set BLE device discoverable */
			ble_cmd_gap_set_mode(gap_general_discoverable, gap_undirected_connectable);
		}

		if(newKey == MB1_usrBtn1.pressedKey_get()){
			ble_cmd_attributes_write(0x000b, 0, 31, msgBuf);
		}

/* recv_handler queue */
msg_t msg_q[RCV_BUFFER_SIZE];

void* recv_handler(void*);

static bool blink_led = false;
void* taskled(void*);

int cal_rssi_dbm(uint8_t rssi_dec);

int main(void)
{
    int recv_handler_pid;
    uint8_t color_id = 0;

    MB1_system_init();

    recv_handler_pid = thread_create(recv_handler_stack_buffer,
    RECV_HANDLER_STACK_SIZE, PRIORITY_MAIN - 2, 0, recv_handler,
    NULL, "Receiving handler");
    transceiver_init(TRANSCEIVER_CC1100);
    transceiver_start();
    cc110x_reconfig();
    transceiver_register(TRANSCEIVER_CC1100, recv_handler_pid);

    taskled_pid = thread_create(taskled_stack, KERNEL_CONF_STACKSIZE_PRINTF,
    PRIORITY_MAIN - 1, CREATE_WOUT_YIELD, taskled, NULL, "Task blink led");

    /* get btn and send data */
    msg_t rep;
    while (1) {
        if (MB1_usrBtn0.pressedKey_get() == Btn_ns::newKey) {
            printf("UsrBtn0::newKey\n");

            /* send data */
            mesg.type = SND_PKT;
            mesg.content.ptr = (char*) &tcmd;

            tcmd.transceivers = TRANSCEIVER_CC1100;
            tcmd.data = &p;

            p.length = 1;
            p.dst = 0;

#if (NODE_ID == 1)
            send_data_buffer[0] = toggle_rgb;
#else
            send_data_buffer[0] = toggle_ledw;
#endif
            p.data = send_data_buffer;

            msg_send_receive(&mesg, &rep, transceiver_pid);
            printf("rep, transceiver_id %d, type %d\n", rep.sender_pid,
                    rep.type);
        }

        if (MB1_usrBtn0.pressedKey_get() == Btn_ns::newLongKey) {
            printf("UsrBtn0::newLongKey\n");
            /* send data */
            mesg.type = SND_PKT;
            mesg.content.ptr = (char*) &tcmd;

            tcmd.transceivers = TRANSCEIVER_CC1100;
            tcmd.data = &p;

            p.length = 1;
            p.dst = 0;

#if (NODE_ID == 1)
            send_data_buffer[0] = blink_rgb;
#else
            send_data_buffer[0] = blink_ledw;
#endif
            p.data = send_data_buffer;

            msg_send_receive(&mesg, &rep, transceiver_pid);
            printf("rep, transceiver_id %d, type %d\n", rep.sender_pid,
                    rep.type);

            while (MB1_usrBtn0.isStillLongPressed() == true) {
                ;
            }

            /* send off command */
#if (NODE_ID == 1)
            send_data_buffer[0] = set_rgb_off;
#else
            send_data_buffer[0] = set_ledw_off;
#endif

            msg_send_receive(&mesg, &rep, transceiver_pid);
            printf("rep, transceiver_id %d, type %d\n", rep.sender_pid,
                    rep.type);
        }

        if (MB1_usrBtn1.pressedKey_get() == Btn_ns::newKey) {
            printf("UsrBtn1::newKey\n");
            /* send data */
            mesg.type = SND_PKT;
            mesg.content.ptr = (char*) &tcmd;

            tcmd.transceivers = TRANSCEIVER_CC1100;
            tcmd.data = &p;

            p.dst = 0;

#if (NODE_ID == 1)
            p.length = 4;
            send_data_buffer[0] = set_rgb_value;
            send_data_buffer[1] = RGBColors[color_id][0];
            send_data_buffer[2] = RGBColors[color_id][1];
            send_data_buffer[3] = RGBColors[color_id][2];
            color_id = (color_id + 1) % 15;
            printf("%d\n", color_id);

#else
            p.length = 2;
            send_data_buffer[0] = set_ledw_value;
            send_data_buffer[1] = ledw_value;
            ledw_value = 1;
#endif

            p.data = send_data_buffer;

            msg_send_receive(&mesg, &rep, transceiver_pid);
            printf("rep, transceiver_id %d, type %d\n", rep.sender_pid,
                    rep.type);
        }
    } // end while
}

void* recv_handler(void*)
{
    msg_t m;
    radio_packet_t *p;

    printf("RecvHandler\n");

    ledw_duty = 0;

    msg_init_queue(msg_q, RCV_BUFFER_SIZE);

    while (1) {
        msg_receive(&m);
        printf("m.type: %d\n", m.type);
        if (m.type == PKT_PENDING) {
            p = (radio_packet_t*) m.content.ptr;
            printf("Packet waiting, process %p...\n", p);
            printf("\tLength:\t%u\n", p->length);
            printf("\tSrc:\t%u\n", p->src);
            printf("\tDst:\t%u\n", p->dst);
            printf("\tLQI:\t%u\n", p->lqi);
            printf("\tRSSI:\t%d dBm\n", cal_rssi_dbm(p->rssi));

            p->processing--;

            /* process command */
            printf("Received command %s\n", command_string[p->data[0]]);

            switch (p->data[0]) {
            case set_rgb_on:
                printf("set_rgb_on\n");
                rgb_state = 1;
                break;

            case set_rgb_off:
                printf("set_rgb_off\n");
                rgb_state = 0;

                blink_led = false;
                break;

            case set_rgb_value:
                printf("Values %d %d %d\n", p->data[1], p->data[2], p->data[3]);
                break;

            case blink_rgb:
                printf("blink_rgb\n");
                blink_led = true;
                thread_wakeup(taskled_pid);
                break;

            case toggle_rgb:
                printf("toggle_rgb\n");
                if (rgb_state == 0) {
                    rgb_state = 1;
                } else {
                    rgb_state = 0;
                }
                break;

            case set_ledw_on:
                printf("set_ledw_on\n");
                break;

            case set_ledw_off:
                printf("set_ledw_off\n");
                blink_led = false;
                break;

            case set_ledw_value:
                printf("Inc Value %d\n", p->data[1]);
                break;

            case blink_ledw:
                printf("blink_ledw\n");
                blink_led = true;
                thread_wakeup(taskled_pid);
                break;

            case toggle_ledw:
                printf("toggle_ledw\n");
                break;

            }
        } else if (m.type == ENOBUFFER) {
            puts("Transceiver buffer full");
        } else {
            puts("Unknown packet received");
        }
    }

    return NULL;
}

void* taskled(void*)
{
    printf("taskled\n");

    while (1) {
        if (blink_led == true) {
        } else {
            thread_sleep();
        }
    }

    return NULL;
}

int cal_rssi_dbm(uint8_t rssi_dec)
{
    int rssi_dBm;

    if (rssi_dec >= 128) {
        rssi_dBm = (int) ((int) (rssi_dec - 256) / 2) - RSSI_OFFSET;
    } else {
        rssi_dBm = (rssi_dec / 2) - RSSI_OFFSET;
    }

    return rssi_dBm;

}
