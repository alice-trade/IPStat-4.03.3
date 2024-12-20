
#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "types.h"
#include "plugin.h"

#include "gfunc.h"

#define SYSTEM_API_VERSION	1

typedef struct SYSTEM
{
	void					*library;
	char					*name;
	PLUGIN_INFO*			(*info)(void);
	gboolean				(*load)(void*);
	gboolean				(*unload)(void*);
	gboolean				status;
} SYSTEM;

gboolean 	system_init(PLUGIN_API*);
gboolean	system_load_all(void);
gboolean	system_unload_all(void);
gboolean	system_free(void);

#endif /* include/system.h */
