/*
 * Version:	$Id: conf.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   (C) 2001-2004 Dmitry Kukuev aka Kornet [kornet@code-art.ru]
*/

#include "../include/core.h"
#include "../include/log.h"

#include <stdlib.h>
#include <string.h>


CORE_API 	
*core_init(CONF_API *conf, LOG_API *log)
{
	CORE_API	*core;
	
	if ((core = (CORE_API*) g_malloc0(sizeof(CORE_API))) == NULL) {
		log->print_log(IP_ALERT, "[core.c][core_init]: error allocate memory for core_api");
		return NULL;
	}

	core->auth 					= users_auth;
	core->autz 					= users_autz;
	core->acct 					= users_acct;
	core->sync					= users_sync;
	core->set_store				= core_set_store;
	core->set_server			= core_set_server;
	core->set_sched				= core_set_sched;
	
	core->register_auth_hook	= register_auth;
	core->register_autz_hook	= register_autz;
	core->register_acct_hook	= register_acct;

	core->unregister_auth_hook	= unregister_auth;
	core->unregister_autz_hook	= unregister_autz;
	core->unregister_acct_hook	= unregister_acct;	
	
	if (users_init(conf, log) != TRUE) {
		free(core);
		core = NULL;
	}
	return core;
}

gboolean	
core_free(CORE_API *core)
{
	users_free();
	if (core) g_free(core);
	core = NULL;

	return TRUE;
}
