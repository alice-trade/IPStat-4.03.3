
#ifndef _LOG_H
#define _LOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "types.h"
#include "conf.h"


#include "gfunc.h"

#define IP_DEBUG	7
#define IP_INFO		6
#define IP_NOTICE	5
#define IP_WARNING	4
#define IP_ERR		3
#define IP_CRIT		2
#define	IP_ALERT	1

typedef struct LOG_API
{
	gboolean 	(*print_log) (int lvl, const char *msg, ...);
} LOG_API;

LOG_API			*log_init(CONF_API*);
gboolean		log_free(LOG_API*);

#endif /* include/log.h */
