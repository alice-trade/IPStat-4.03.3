/*
  Function for Core
  
  (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _CORE_H
#define _CORE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "core_users.h"
#include "conf.h"
#include "log.h"
#include "gfunc.h"

#include <unistd.h>
#include <stdio.h>


typedef struct CORE_API
{
	gboolean		(*auth)(IP_id service, void *data, gboolean flag_extern_auth, void *ret);			// аутентификация
	gboolean		(*autz)(IP_id ervice, void *data, gboolean flag_extern_autz, void *ret);			// авторизация
	gboolean		(*acct)(IP_id service, void *data, gboolean flag_extern_acct, void *ret);			// акаунтинг
	gboolean		(*sync)(void);						// 
	gboolean		(*set_store)(void*);				// установка api хранилища
	gboolean		(*set_server)(void*);				// Server API
	gboolean		(*set_sched)(void*);				// Scheduler API
	gboolean		(*register_auth_hook)(gboolean (*func_auth)(IP_id service, void *data, void *ret));
	gboolean		(*register_autz_hook)(gboolean (*func_autz)(IP_id service, void *data, void *ret));
	gboolean		(*register_acct_hook)(gboolean (*func_acct)(IP_id service, void *data, void *ret));
	gboolean		(*unregister_auth_hook)(gboolean (*func_auth)(IP_id service, void *data, void *ret));
	gboolean		(*unregister_autz_hook)(gboolean (*func_autz)(IP_id service, void *data, void *ret));
	gboolean		(*unregister_acct_hook)(gboolean (*func_acct)(IP_id service, void *data, void *ret));
} CORE_API;

//-----------------------------------------------------------------------------
//                      Public functions
//-----------------------------------------------------------------------------
CORE_API				*core_init(CONF_API*, LOG_API*);
gboolean				core_free(CORE_API*);

#endif /* include/core.h */
