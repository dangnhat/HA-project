/**
 * @file cc_slp_sender.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 8-Nov-2014
 * @brief This is header file for CC's 6lowpan sender thread.
 *
 * NOTE: sender thread will configure network stack prefix, address, channel when
 * SIXLOWPAN_RESTART message have been sent from shell. After network stack has been
 * initialized sender thread will forward SIXLOWPAN_RESTART message to receiver thread.
 */

#ifndef CC_SLP_SENDER_H_
#define CC_SLP_SENDER_H_

extern "C" {
#include "thread.h"
}

#include "cir_queue.h"

/*--------------------- Global variable --------------------------------------*/
namespace ha_cc_ns {
extern kernel_pid_t slp_sender_pid;

extern cir_queue slp_sender_gff_queue;  /* This queue will hold data in GFF format from */
                                        /* controller thread to 6lowpan sender thread */
}

/**
 * @brief   Create and start 6lowpan thread.
 */
void cc_slp_sender_start(void);

#endif /* CC_SLP_SENDER_H_ */
