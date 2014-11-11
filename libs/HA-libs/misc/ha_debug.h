/**
 * @file ha_debug.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 6-Nov-2014
 * @brief This contains marco for debug and notify.
 */

#ifndef HA_DEBUG_H_
#define HA_DEBUG_H_

#include <stdio.h>

#if HA_NOTIFICATION
#define HA_NOTIFY(...) printf(__VA_ARGS__)
#else
#define HA_NOTIFY(...)
#endif

#if HA_DEBUG_EN
#define HA_DEBUG(...) printf(__VA_ARGS__)
#else
#define HA_DEBUG(...)
#endif

#endif /* HA_DEBUG_H_ */
