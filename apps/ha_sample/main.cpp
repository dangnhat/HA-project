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
//#include "rgb.h"
//#include "ledw.h"

/* definitions */
#define RECV_HANDLER_STACK_SIZE    (KERNEL_CONF_STACKSIZE_DEFAULT + KERNEL_CONF_STACKSIZE_PRINTF)
#define RCV_BUFFER_SIZE     (64)

#define RSSI_OFFSET	(74)

#define NODE_ID	(1)

typedef enum
    : uint8_t {
        set_rgb_on = 0x01,
    set_rgb_off = 0x02,
    set_rgb_value = 0x03, // 3 values follow
    blink_rgb = 0x04,
    toggle_rgb = 0x05,
    set_ledw_on = 0x06,
    set_ledw_off = 0x07,
    set_ledw_value = 0x08, // 1 value follows
    blink_ledw = 0x09,
    toggle_ledw = 0x0A,
} command_t;

const char command_string[][128] { "", "set_rgb_on", "set_rgb_off",
        "set_rgb_value", "blink_rgb", "toggle_rgb", "set_ledw_on",
        "set_ledw_off", "set_ledw_value", "blink_ledw", "toggle_ledw", };

/* RGB color */
const uint8_t RGBColors[15][3] = { { 0, 0, 0 }, // black
        { 255, 0, 0 }, // red
        { 0, 255, 0 }, // green
        { 0, 0, 255 }, // blue
        { 255, 255, 0 }, // yellow
        { 0, 255, 255 }, // aqua
        { 255, 0, 255 }, // magenta
        { 192, 192, 192 }, // silver
        { 128, 128, 128 }, // gray
        { 128, 0, 0 }, // maroon
        { 128, 128, 0 }, // olive
        { 0, 128, 0 }, // green
        { 128, 0, 128 }, //purple
        { 0, 128, 128 }, // teal
        { 0, 0, 128 } // navy
};

/* global vars */
static msg_t mesg;
static transceiver_command_t tcmd;
static radio_packet_t p;

//static RGB_c led_rgb;
//static LEDW_c ledw;

static bool rgb_state = 0;
static uint8_t ledw_duty = 0;

static int taskled_pid;

/* Send data buffer */
uint8_t send_data_buffer[CC1100_MAX_DATA_LENGTH];

/* recv_handler stack */
char recv_handler_stack_buffer[RECV_HANDLER_STACK_SIZE];

/* taskled_stack */
char taskled_stack[KERNEL_CONF_STACKSIZE_PRINTF];

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
//    uint8_t ledw_value = 0;
    printf("Main\n");
    MB1_system_init();

    recv_handler_pid = thread_create(recv_handler_stack_buffer, RECV_HANDLER_STACK_SIZE, PRIORITY_MAIN-2, CREATE_WOUT_YIELD, recv_handler, NULL, "Receiving handler");
    transceiver_init(TRANSCEIVER_CC1100);
    transceiver_start();
    cc110x_reconfig();
    printf("Main2\n");
    transceiver_register(TRANSCEIVER_CC1100, recv_handler_pid);
    printf("Main3\n");
    taskled_pid = thread_create(taskled_stack, KERNEL_CONF_STACKSIZE_PRINTF, PRIORITY_MAIN-1, CREATE_WOUT_YIELD, taskled, NULL, "Task blink led");
    printf("Main4\n");
    /* get btn and send data */
    uint8_t test1 = 0xfa;
    uint16_t test2 = 0xfafa;
    unsigned char test3[] = "DEA";
    unsigned char test4[] = "nvhien1992";
    unsigned char test5[] = "DEADBEEFDEADBEEFDEADBEEFDEADBEEF";

    uint32_t crc_val = 0;

    MB1_crc.crc_start();
    msg_t rep;
    printf("Main5\n");
    while (1) {
        if (MB1_usrBtn0.pressedKey_get() == Btn_ns::newKey) {
            printf("UsrBtn0::newKey\n");

            crc_val = MB1_crc.crc32_block_cal((uint8_t*) &test1, 1);
            printf("CRC32 1b: 0x%lX\n", crc_val);

            crc_val = MB1_crc.crc32_block_cal((uint8_t*) &test2, 2);
            printf("CRC32 2b: 0x%lX\n", crc_val);

            crc_val = MB1_crc.crc32_block_cal((uint8_t*) test3, 3);
            printf("CRC32 3b: 0x%lX\n", crc_val);

            crc_val = MB1_crc.crc32_block_cal((uint8_t*) test4, 10);
            printf("CRC32 block 10b: 0x%lX\n", crc_val);

            crc_val = MB1_crc.crc32_block_cal((uint8_t*) test5, 32);
            printf("CRC32 block 32b: 0x%lX\n", crc_val);

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
    uint8_t count;
    printf("RecvHandler\n");
    /* init rgb */
#if (NODE_ID == 0)
//    led_rgb.Start();
#else
//    ledw.Start();
//    ledw.Increase_Duty();
//    ledw.Increase_Duty();

    ledw_duty = (ledw_duty + 2) % 10;
#endif

//    delay_ms(300);

#if (NODE_ID == 0)
//    led_rgb.SetColor(0, 0, 0);
#else
    for (count = ledw_duty; count <= 10; count++) {
//    	ledw.Increase_Duty();
    }
    ledw_duty = 0;
#endif

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
//            	led_rgb.SetColor(255, 255, 255);
                rgb_state = 1;
                break;

            case set_rgb_off:
                printf("set_rgb_off\n");
//            	led_rgb.SetColor(0, 0, 0);
                rgb_state = 0;

                blink_led = false;
                break;

            case set_rgb_value:
                printf("Values %d %d %d\n", p->data[1], p->data[2], p->data[3]);
//            	led_rgb.SetColor(p->data[1], p->data[2], p->data[3]);
                break;

            case blink_rgb:
                printf("blink_rgb\n");
                blink_led = true;
                thread_wakeup(taskled_pid);
                break;

            case toggle_rgb:
                printf("toggle_rgb\n");
                if (rgb_state == 0) {
//            		led_rgb.SetColor(255, 255, 255);
                    rgb_state = 1;
                } else {
//            		led_rgb.SetColor(0, 0, 0);
                    rgb_state = 0;
                }
                break;

            case set_ledw_on:
                printf("set_ledw_on\n");
//            	ledw.Start();
//            	ledw_state = 1;
                break;

            case set_ledw_off:
                printf("set_ledw_off\n");
                for (count = ledw_duty; count <= 10; count++) {
//					ledw.Increase_Duty();
                }
                ledw_duty = 0;

                blink_led = false;
                break;

            case set_ledw_value:
                printf("Inc Value %d\n", p->data[1]);
                for (count = 0; count < p->data[1]; count++) {
//            		ledw.Increase_Duty();

                    ledw_duty = (ledw_duty + 1) % 10;
                }
                break;

            case blink_ledw:
                printf("blink_ledw\n");
                blink_led = true;
                thread_wakeup(taskled_pid);
                break;

            case toggle_ledw:
                printf("toggle_ledw\n");
                if (ledw_duty == 0) {

//            		ledw.Increase_Duty();
//					ledw.Increase_Duty();

                    ledw_duty = (ledw_duty + 2) % 10;
                } else {
                    for (count = ledw_duty; count <= 10; count++) {
//						ledw.Increase_Duty();
                    }
                    ledw_duty = 0;
                }
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
    uint16_t count;
    printf("taskled\n");
    while (1) {
        if (blink_led == true) {
#if (NODE_ID == 0)
            if (rgb_state == 0) {
//				led_rgb.SetColor(255, 255, 255);
                rgb_state = 1;
            }
            else {
//				led_rgb.SetColor(0, 0, 0);
                rgb_state = 0;
            }
#else
            if (ledw_duty == 0) {
//				ledw.Increase_Duty();
//				ledw.Increase_Duty();
                ledw_duty = (ledw_duty + 2) % 10;
            } else {
                for (count = ledw_duty; count <= 10; count++) {
//					ledw.Increase_Duty();
                }
                ledw_duty = 0;
            }
#endif

//			delay_ms(200);
        } else {
            thread_sleep();
        }
    }

    return NULL;
}

int cal_rssi_dbm(uint8_t rssi_dec)
{
    int rssi_dBm;

    if (rssi_dec >= 128)
        rssi_dBm = (int) ((int) (rssi_dec - 256) / 2) - RSSI_OFFSET;
    else
        rssi_dBm = (rssi_dec / 2) - RSSI_OFFSET;

    return rssi_dBm;
}
