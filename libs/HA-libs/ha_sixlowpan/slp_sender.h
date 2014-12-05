/**
 * @file slp_sender.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.1
 * @date 14-Nov-2014
 * @brief This is header file for 6lowpan sender thread.
 *
 * NOTE: sender thread will configure network stack prefix, address, channel when
 * SIXLOWPAN_RESTART message have been sent from shell. After network stack has been
 * initialized sender thread will forward SIXLOWPAN_RESTART message to receiver thread.
 */

#ifndef SLP_SENDER_H_
#define SLP_SENDER_H_

extern "C" {
#include "thread.h"
}

#include "cir_queue.h"

/**
 * @brief   Create and start 6lowpan thread.
 */
void slp_sender_start(void);

#endif /* CC_SLP_SENDER_H_ */
