#ifndef __SNTP_H
#define __SNTP_H

#include "cmsis_os2.h"                  // CMSIS RTOS definitions
#include "rl_net.h"                     // Network definitions

extern const NET_ADDR4 ntp_server ;
void get_time (void);
static void time_callback (uint32_t seconds, uint32_t seconds_fraction);

extern osThreadId_t TID_SNTP; 
#endif
