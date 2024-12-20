
#ifndef _COLLECTORS_H
#define _COLLECTORS_H

#include "types.h"
#include "plugin.h"
#include "gfunc.h"

#define COLLECTORS_API_VERSION	1

typedef struct COLLECTOR
{
	void					*library;
	char					*name;
	PLUGIN_INFO*			(*info)(void);
	gboolean				(*load)(void*);
	gboolean				(*unload)(void*);
	gboolean				status;
} COLLECTOR;

gboolean 	collectors_init(PLUGIN_API*);
gboolean	collectors_load_all(void);
gboolean	collectors_unload_all(void);
gboolean	collectors_free(void);

#endif /* include/store.h */
