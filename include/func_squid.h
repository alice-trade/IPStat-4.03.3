
#ifndef _FUNC_SQUID_H
#define _FUNC_SQUID_H

#include "types.h"
#include "plugin.h"
#include <stdio.h>
#include "gfunc.h"

typedef struct ACCT_SQUID_NODE
{
	IP_id		id_user;
	char		url[1024];
	IP_size		traffic;
} ACCT_SQUID_NODE;

gboolean			fifo_create(PLUGIN_API*, char*);
FILE				*fifo_open(PLUGIN_API*, char*);
gboolean			fifo_close(PLUGIN_API*, FILE*);
int		 			fifo_read(FILE*, void*, size_t, int);

gboolean			squid_init(PLUGIN_API *api);
gboolean			squid_free(void);
gboolean 			squid_acct(char *url, SQUID_DATA_INFO *squid_data);


#endif /* include/func_squid.h */
