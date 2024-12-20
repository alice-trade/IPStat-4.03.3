
#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "types.h"
#include "conf.h"
#include "log.h"

#include "gfunc.h"

#include <stdio.h>


typedef struct SCHEDULER_API
{
	gboolean	(*set_server)(void*);
	gboolean	(*registred)(char *name, char *crontab, gboolean (*func)(const void*), void *data);
	gboolean	(*unregistred)(char *name);
	gboolean	(*print)(int fd);
} SCHEDULER_API;


SCHEDULER_API		*scheduler_init(CONF_API*, LOG_API*);
gboolean		scheduler_free(SCHEDULER_API*);

#endif /* include/sched.h */
