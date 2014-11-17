/**
 * @file cc_slp_receiver.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 11-Nov-2014
 * @brief This is source file for CC's 6lowpan receiver thread.
 */

extern "C" {
#include "thread.h"
#include "msg.h"
#include "socket_base/socket.h"
}

#include "ha_sixlowpan.h"
#include "ha_gff_misc.h"

#include "cc_msg_id.h"
#include "cc_slp_receiver.h"

#include "MB1_System.h" /* for range test */

/*--------------------- Global variable --------------------------------------*/
namespace ha_cc_ns {
kernel_pid_t slp_receiver_pid;
}

/*--------------------- Configurations ---------------------------------------*/
#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (1)
#include "ha_debug.h"

static const char slp_receiver_prio = PRIORITY_MAIN - 2;
static const uint16_t slp_receiver_stacksize = 2048;
static char slp_receiver_stack[slp_receiver_stacksize];
static void *slp_receiver_func(void *arg);

static const char slp_receiver_msgqueue_size = 128;
static msg_t slp_receiver_msgqueue[slp_receiver_msgqueue_size];

/*--------------------- Public functions -------------------------------------*/
/**
 * @brief   Create and start 6lowpan receiver thread.
 */
static void *slp_receiver_func(void *arg);

void cc_slp_receiver_start(void)
{
    /* Create 6lp_receiver thread */
    ha_cc_ns::slp_receiver_pid = thread_create(slp_receiver_stack, slp_receiver_stacksize,
            slp_receiver_prio, CREATE_STACKTEST, slp_receiver_func, NULL, "CC_6LoWPAN_receiver");
    if (ha_cc_ns::slp_receiver_pid > 0) {
        HA_NOTIFY("CC 6LoWPAN receiver thread created.\n");
    }
    else {
        HA_NOTIFY("Can't create CC 6LoWPAN receiver thread.\n");
    }
}

/*--------------------- Static functions -------------------------------------*/
/* Prototypes */
static int16_t filter_node_id(uint16_t node_id, uint8_t* payload_buffer, int32_t &recsize);
static void start_receiver_loop(void);

/**
 * @brief   6lowpan receiver thread's function.
 */
static void *slp_receiver_func(void *arg)
{
    msg_t mesg;

    /* Init message queue */
    msg_init_queue(slp_receiver_msgqueue, slp_receiver_msgqueue_size);

    while (1) {
        /* wait for message */
        msg_receive(&mesg);

        switch (mesg.type) {
        case ha_ns::SIXLOWPAN_RESTART:
            HA_DEBUG("slp_receiver: Received SIXLOWPAN_RESTART.\n");
            start_receiver_loop();
            break;

        default:
            HA_DEBUG("slp_receiver: Unknown message.\n");
            break;
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
static void start_receiver_loop(void)
{
    int sock;
    sockaddr6_t server_addr, from_addr;
    int32_t recsize;
    uint8_t payload_buffer[ha_ns::sixlowpan_payload_maxsize];
    uint32_t from_len;
    uint16_t count;
    char addr_str[IPV6_MAX_ADDR_STR_LEN];

    /* open socket and bind to addr */
    sock = socket_base_socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET;
    server_addr.sin6_port = HTONS(ha_ns::sixlowpan_receiving_port);

    if (socket_base_bind(sock, &server_addr, sizeof(server_addr)) == -1) {
        HA_DEBUG("start_receiver: bind failed!\n");
        socket_base_close(sock);
        return;
    }

    while (1) {
        recsize = socket_base_recvfrom(sock, (void *)payload_buffer,
                ha_ns::sixlowpan_payload_maxsize, 0, &from_addr, &from_len);
        HA_DEBUG("start_receiver: %ld bytes received from %s\n", recsize,
                ipv6_addr_to_str(addr_str, IPV6_MAX_ADDR_STR_LEN, &(from_addr.sin6_addr)));

        /* filter address */
        filter_node_id(ha_ns::sixlowpan_node_id, payload_buffer, recsize);
        if (recsize < 0) {
            continue;
        }
        else {
            HA_DEBUG("start_receiver: received data:\n");
            for (count = 0; count < recsize; count++) {
                HA_DEBUG("%x ", payload_buffer[count]);
            }
            HA_DEBUG("\n");
            /* for range test */
            MB1_Led_green.toggle();
        }
    }

    socket_base_close(sock);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   Filter node id from payload.
 *
 * @param[in]       node_id, node_id of this node.
 * @param[in/out]   payload_buffer,
 * @param[in/out]   recsize, received size before/after filtering.
 *
 * @return  -1 if message is not for this node (recsize will be set to 0), 0 is successful.
 */
static int16_t filter_node_id(uint16_t node_id, uint8_t* payload_buffer, int32_t &recsize)
{
    uint16_t recv_node_id;

    /* get node_id */
    recv_node_id = buf2uint16(payload_buffer);

    if (recv_node_id != node_id) {
        HA_DEBUG("filter_node_id: This message is not mine (my node_id %u, recv_node_id %u).\n",
                node_id, recv_node_id);
        recsize = -1;
        return -1;
    }

    memmove(&payload_buffer[0], &payload_buffer[2], recsize);
    recsize = recsize - 2;

    return 0;
}
