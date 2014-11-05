/**
 * @file cc_6lowpan_thread.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 4-Oct-2014
 * @brief This is header file for CC's 6lowpan thread.
 */

#ifndef CC_6LOWPAN_THREAD_H_
#define CC_6LOWPAN_THREAD_H_

/*--------------------- Global variable --------------------------------------*/
namespace ha_cc_ns {
extern kernel_pid_t sixlowpan_pid;
}

/**
 * @brief   Create and start 6lowpan thread.
 */
void cc_6lowpan_thread_start(void);

#endif /* CC_6LOWPAN_THREAD_H_ */
