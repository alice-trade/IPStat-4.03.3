
#ifndef _STORE_H
#define _STORE_H

#include "mysql.h"
#include "types.h"
#include "plugin.h"

#include "conf.h"
#include "log.h"

#include "gfunc.h"

#define STORE_API_VERSION	1

typedef struct STORE_API
{
	PLUGIN_INFO*	(*info)(void);
	SQL_SESSION*	(*open)(void);
	gboolean		(*close)(SQL_SESSION*);
	gboolean  		(*exec)(SQL_SESSION*, const char*);
	gboolean		(*query)(SQL_SESSION*, const char*);
	gboolean		(*list_tables)(SQL_SESSION*, const char*);
	gboolean		(*free)(SQL_SESSION*);
	SQL_ROW			(*fetch_row)(const SQL_SESSION*);
	gboolean		(*num_fields)(const SQL_SESSION*);
	gboolean		(*check_error)(SQL_SESSION*);	
	
	void			*library;
} STORE_API;


STORE_API 	*store_init(CONF_API*, LOG_API*);
gboolean	store_free(STORE_API*);

#endif /* include/store.h */
