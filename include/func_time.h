
#ifndef _FUNC_TIME_H
#define _FUNC_TIME_H

#include "types.h"
#include "collectors.h"
#include "gfunc.h"

gboolean	time_init(PLUGIN_API *api);
gboolean	time_free(void);
gboolean	time_acct(PACKET_DATA_INFO *packet_data);

#endif /* include/func_time.h  */
