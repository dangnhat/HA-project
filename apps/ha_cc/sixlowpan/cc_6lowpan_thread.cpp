/**
 * @file cc_6lowpan_thread.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 4-Oct-2014
 * @brief This is implementations of CC's 6lowpan thread.
 */

#include "ha_system.h"
#include "cc_6lowpan_thread.h"

/*--------------------- Config interface -------------------------------------*/
static const uint16_t cc_6lowpan_thread_stack_size = 1024;
static char cc_6lowpan_thread_stack[cc_6lowpan_thread_stack_size];
static const char cc_6lowpan_thread_name[] = "CC 6lowpan thread";

static const char config_file[] = "slp_conf";

/*--------------------- Global variable --------------------------------------*/
namespace ha_cc_ns {
kernel_pid_t sixlowpan_pid;
}


