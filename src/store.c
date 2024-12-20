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
 *   (C) 2001-2004 Dmirty Kukuev aka Kornet [kornet@code-art.ru]
*/

#include "../include/store.h"
#include "../include/log.h"
#include "../include/conf.h"

#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

static SQL_SESSION* (*sql_open_lib)(const char*, const char*, const char*, const char*);
static SQL_SESSION* sql_open_my(void);

static STORE_API *st_store = NULL;
static CONF_API	 *st_conf  = NULL;
static LOG_API   *st_log   = NULL;

static char			store_plugin[FILENAME_MAX];
static char			host_name[200];
static char			user_name[200];
static char			password[200];
static char			db_name[200];

extern int debug;

static CONF_PARSER module_config[] = {
        { "store_plugin", CF_TYPE_STRING, FILENAME_MAX, store_plugin, "/opt/IPStat/lib/libipstat_mysql.so" },
        { "name_server", CF_TYPE_STRING, 200, host_name, "127.0.0.1" },
        { "name_user", CF_TYPE_STRING, 200, user_name, "root" },
        { "password", CF_TYPE_STRING, 200, password, "" },
        { "db_name", CF_TYPE_STRING, 200, db_name, "IPStat" },
        { NULL, -1, 0, NULL, NULL }
};

STORE_API 	
*store_init(CONF_API *conf, LOG_API *log)
{
	STORE_API	*api;
	PLUGIN_INFO	*info;
	char		*error;
	
	st_conf = conf;
	st_log  = log;
	
	if ((st_store = api = g_malloc0(sizeof(STORE_API))) == NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error allocate memory for store_api");
		return NULL;
	}
	
	if (conf) conf->parse(module_config);

	if (debug) st_log->print_log(IP_DEBUG, "open store_plugin='%s'", store_plugin);
	if ((api->library = dlopen(store_plugin, RTLD_LAZY))==NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error open library");
		g_free(api);
		return NULL;
	}
	
	api->info = dlsym(api->library, "info");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'info'. Check library");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	if ((info = api->info())==NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_info]: error execute 'info'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	
	if (info->type != IPSTAT_PLUGIN_STORE) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error not correct type API module");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	if (info->api_version<STORE_API_VERSION) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error not correct version API module");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	sql_open_lib = dlsym(api->library, "sql_open");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_open'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	api->open = sql_open_my;

	api->close = dlsym(api->library, "sql_close");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_close'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	api->exec = dlsym(api->library, "sql_exec");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_exec'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	api->query = dlsym(api->library, "sql_query");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_query'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}

	api->list_tables = dlsym(api->library, "sql_list_tables");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_list_tables'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}

	api->free = dlsym(api->library, "sql_free");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_free'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	api->fetch_row = dlsym(api->library, "sql_fetch_row");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_fetch_row'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	api->num_fields = dlsym(api->library, "sql_num_fields");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_num_fields'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	api->check_error = dlsym(api->library, "sql_check_error");
	if ((error = dlerror()) != NULL) {
		st_log->print_log(IP_ALERT, "[store.c][store_init]: error symlink to 'sql_check_error'");
		dlclose(api->library);
		g_free(api);
		return NULL;
	}
	st_log->print_log(IP_INFO, "*****************************************************");
	st_log->print_log(IP_INFO, "*** Loading store module:                           *");
	st_log->print_log(IP_INFO, "*****************************************************");
	st_log->print_log(IP_INFO, "* Name - %s", info->name);
	st_log->print_log(IP_INFO, "* Version - %s", info->version);
	st_log->print_log(IP_INFO, "* Author - %s [%s]", info->author, info->homepage);
	st_log->print_log(IP_INFO, "* Desc - %s", info->description);
	st_log->print_log(IP_INFO, "*****************************************************");
	
	return api;
}

gboolean
store_free(STORE_API *api)
{
	if (api->library) dlclose(api->library);
	if (api) free(api);
	api = NULL;
	
	return TRUE;
}

/*
	Private functions
*/

static SQL_SESSION
*sql_open_my(void)
{
	return sql_open_lib(host_name, user_name, password, db_name);
}
