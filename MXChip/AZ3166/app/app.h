#ifndef _APP_H
#define _APP_H
   
#include "nx_api.h"
#include "nxd_dns.h"
#include "tx_api.h"

//UINT app_entry(NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

VOID app_thread_entry(ULONG parameter);
#endif