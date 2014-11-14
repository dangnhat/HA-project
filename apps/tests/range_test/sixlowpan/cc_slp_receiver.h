/**
 * @file cc_slp_receiver.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 11-Nov-2014
 * @brief This is header file for CC's 6lowpan receiver thread.
 */

#ifndef CC_SLP_RECEIVER_H_
#define CC_SLP_RECEIVER_H_

extern "C" {
#include "thread.h"
}

/*--------------------- Global variable --------------------------------------*/
namespace ha_cc_ns {
extern kernel_pid_t slp_receiver_pid;
}

/**
 * @brief   Create and start 6lowpan receiver thread.
 */
void cc_slp_receiver_start(void);

#endif /* CC_SLP_RECEIVER_H_ */
