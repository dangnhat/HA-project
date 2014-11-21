/**
 * @file slp_receiver.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.1
 * @date 14-Nov-2014
 * @brief This is header file for 6lowpan receiver thread.
 */

#ifndef SLP_RECEIVER_H_
#define SLP_RECEIVER_H_

extern "C" {
#include "thread.h"
}

/**
 * @brief   Create and start 6lowpan receiver thread.
 */
void slp_receiver_start(void);

/**
 * @brief   GFF message handler. This function needs to be implemented on node, or cc.
 *          For cc, it just need to forward GFF frame to controller cir_queue.
 *          For node, to save space, node don't have controller and cir_queues for end points
 *          and only SET_DEV_VAL message is available.
 *          Thus, SET_DEV_VAL will be sent directly to suitable thread of an end point with
 *          value in RIOT's IPC message.
 *
 * @param[in]   GFF_buffer, buffer holding GFF frame.
 */
void slp_received_GFF_handler(uint8_t *GFF_buffer);

#endif /* CC_SLP_RECEIVER_H_ */
