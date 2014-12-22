/**
 * @file slp_sender.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.1
 * @date 14-Nov-2014
 * @brief This is source file for 6lowpan sender thread.
 */

extern "C" {
#include "thread.h"
#include "msg.h"
#include "socket_base/socket.h"
}

#include "ha_sixlowpan.h"
#include "ha_gff_misc.h"
#include "gff_mesg_id.h"

#include "slp_sender.h"

#include "cir_queue.h"
#include "ff.h"

/*--------------------- Global variable --------------------------------------*/
namespace ha_ns {
kernel_pid_t sixlowpan_sender_pid;

const uint16_t sixlowpan_sender_gff_queue_size =
#ifdef HA_HOST
        500;
#endif
#ifdef HA_CC
        1024;
#endif
uint8_t sixlowpan_sender_gff_queue_buf[sixlowpan_sender_gff_queue_size];
cir_queue sixlowpan_sender_gff_queue(
        sixlowpan_sender_gff_queue_buf,
        sixlowpan_sender_gff_queue_size);
    /* This queue will hold data in GFF format from */
    /* controller thread to 6lowpan sender thread */
}

/*--------------------- Configurations ---------------------------------------*/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

static const char slp_sender_prio = PRIORITY_MAIN-2;
static const uint16_t slp_sender_stacksize = 1536;
static char slp_sender_stack[slp_sender_stacksize];
static void *slp_sender_func(void *arg);

static const char slp_sender_msgqueue_size = 32;
static msg_t slp_sender_msgqueue[slp_sender_msgqueue_size];

/*--------------------- Public functions -------------------------------------*/
/**
 * @brief   Create and start 6lowpan sender thread.
 */
void slp_sender_start(void)
{
    /* Create 6lp_sender thread */
    ha_ns::sixlowpan_sender_pid = thread_create(slp_sender_stack, slp_sender_stacksize,
            slp_sender_prio, CREATE_STACKTEST, slp_sender_func, NULL, "6LoWPAN_sender");
    if (ha_ns::sixlowpan_sender_pid > 0) {
        HA_NOTIFY("6LoWPAN sender thread created.\n");
    }
    else {
        HA_NOTIFY("Can't create 6LoWPAN sender thread.\n");
    }
}

/*--------------------- Static functions -------------------------------------*/
/* Prototypes */
static int16_t restart_sixlowpan(void);
static int16_t send_data_gff(cir_queue *gff_cir_queue);
static void insert_node_id(uint8_t* payload_buffer, uint16_t node_id);

/**
 * @brief   6lowpan sender thread's function.
 */
static void *slp_sender_func(void *arg)
{
    msg_t mesg;

    /* Init message queue */
    msg_init_queue(slp_sender_msgqueue, slp_sender_msgqueue_size);

    while (1) {
        /* wait for message */
        msg_receive(&mesg);

        switch (mesg.type) {
        case ha_ns::SIXLOWPAN_RESTART:
            HA_DEBUG("slp_sender: Received SIXLOWPAN_RESTART.\n");
            restart_sixlowpan();
            /* restart receiver */
            msg_send(&mesg, ha_ns::sixlowpan_receiver_pid, false);
            break;

        case ha_ns::GFF_PENDING:
            HA_DEBUG("slp_sender: Received GFF_PENDING.\n");
            send_data_gff((cir_queue*)mesg.content.ptr);
            break;

        default:
            HA_DEBUG("slp_sender: Unknown message.\n");
            break;
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   6lowpan read configurations from file and restart network stack.
 *          Using following global variables:
 *          - sixlowpan_config_file,
 *          - sixlowpan_config_pattern,
 *          - sixlowpan_default_interface
 *          in ha_sixlowpan.h
 *
 * @return  -1 if error. Error will occur when file doesn't exist, node id or prefixes are 0.
 */
static int16_t restart_sixlowpan(void)
{
    uint16_t prefixes[4];
    uint16_t node_id;
    char netdev_type;
    uint16_t channel;
    int16_t retval;

    HA_NOTIFY("Restarting 6LoWPAN network stack...\n");

    /* read configurations */
    retval = ha_slp_readconfig(ha_ns::sixlowpan_config_file, ha_ns::sixlowpan_config_pattern,
            sizeof(ha_ns::sixlowpan_config_pattern),
            prefixes, node_id, netdev_type, channel);
    if (retval < 0) {
        HA_NOTIFY("rst_slp: Error when reading configurations from file.\n");
        return -1;
    }

    HA_DEBUG("Configurations:\n");
    HA_DEBUG(ha_ns::sixlowpan_config_pattern,
            (uint32_t)prefixes[3], (uint32_t)prefixes[2],
            (uint32_t)prefixes[1], (uint32_t)prefixes[0],
            (uint32_t)node_id, netdev_type, (uint32_t)channel);

    /* All configurations are ok, we're good to go */
    retval = ha_slp_init(ha_ns::sixlowpan_default_interface, ha_ns::sixlowpan_default_transceiver,
            prefixes, node_id, netdev_type, channel);
    if(retval < 0) {
        HA_NOTIFY("rst_slp: Error when initializing 6LoWPAN stack.\n");
        return -1;
    }

    HA_NOTIFY("6LoWPAN stack restarted.\n");

    return 0;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   Send data in GFF format to 6lowpan.
 *          Using following global variables:
 *          - sixlowpan_payload_maxsize
 *          - sixlowpan_default_interface
 *          in ha_sixlowpan.h
 *
 * @param[in]   gff_cir_queue, pointer to cir_queue object holding GFF frames.
 *
 * @return  -1 if error.
 */
static int16_t send_data_gff(cir_queue *gff_cir_queue)
{
    uint8_t payload_buffer[ha_ns::sixlowpan_payload_maxsize];
    uint8_t gff_data_size;
    uint16_t node_id, gff_cmd_id;
    ipv6_addr_t ipaddr;
    sockaddr6_t saddr;
    int sock;

    int32_t bytes_sent;

    /* get data from queue */
    gff_data_size = gff_cir_queue->preview_data(false);
    if (gff_cir_queue->get_size() < (gff_data_size + 2 + 1)) {
        HA_DEBUG("send_data_gff: Size of GFF frame in queue(%ld) < gff_data_size(%hu) + 3\n",
                gff_cir_queue->get_size(), gff_data_size);
        return -1;
    }

    gff_cir_queue->get_data(payload_buffer, gff_data_size + 3);

    /* check kind of message */
    gff_cmd_id = buf2uint16(&payload_buffer[1]);

    switch (gff_cmd_id) {
    case ha_ns::SET_DEV_VAL:
        HA_DEBUG("send_data_gff: SET_DEV_VAL message (%hu, %lx, %hd).\n",
                payload_buffer[0], buf2uint32(&payload_buffer[3]),
                (int16_t)buf2uint16(&payload_buffer[7]));
#ifdef HA_CC
        node_id = parse_node_deviceid(buf2uint32(&payload_buffer[3]));
#endif
#ifdef HA_HOST
        node_id = ha_ns::sixlowpan_ha_cc_node_id;
#endif
        break;
    case ha_ns::ALIVE:
        HA_DEBUG("send_data_gff: ALIVE message.\n");
        node_id = ha_ns::sixlowpan_ha_cc_node_id;
        break;
    default:
        HA_DEBUG("send_data_gff: unknow GFF command id %x\n", gff_cmd_id);
        return -1;
    }

    /* insert node id */
    insert_node_id(payload_buffer, node_id);

    /* Set address to send data */
    ipv6_addr_set_all_nodes_addr(&ipaddr);

    /* open a socket and send data */
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin6_family = AF_INET6;
    memcpy(&saddr.sin6_addr, &ipaddr, 16);
    saddr.sin6_port = HTONS(ha_ns::sixlowpan_receiving_port);

    sock = socket_base_socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        HA_DEBUG("send_data_gff: Error Creating Socket.\n");
        return -1;
    }

    bytes_sent = socket_base_sendto(sock, payload_buffer, gff_data_size + 3 + 2, 0,
            &saddr, sizeof(saddr));
    if (bytes_sent >= 0) {
        HA_DEBUG("send_data_gff: %ld bytes sent to %hu\n", bytes_sent, node_id);
    }
    else {
        HA_NOTIFY("send_data_gff: Error when send data to %hu\n", node_id);
    }

    socket_base_close(sock);

    /* WORKAROUND: sleep to 10ms so it will not make receiver buffer overflow
     * TODO(later): apply CoAP with flow control will resolve this issue */
    vtimer_usleep(10000);

    return 0;
}

/*----------------------------------------------------------------------------*/
static void insert_node_id(uint8_t* payload_buffer, uint16_t node_id)
{
    /* get node_id */
    memmove(&payload_buffer[2], &payload_buffer[0], payload_buffer[0] + 3);

    payload_buffer[0] = (uint8_t)(node_id >> 8);
    payload_buffer[1] = (uint8_t)(node_id);
}
