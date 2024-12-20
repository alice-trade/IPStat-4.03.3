
#ifndef _FUNC_IP_H
#define _FUNC_IP_H

#include "types.h"
#include "collectors.h"
#include "gfunc.h"

gboolean	ip_init(PLUGIN_API *api);
gboolean	ip_free(void);
gboolean	ip_acct(PACKET_INFO *packet_info, PACKET_DATA_INFO *packet_data);

#endif /* include/func_ip.h  */
