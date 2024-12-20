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

#include "../include/system.h"
#include "../include/log.h"
#include "../include/misc.h"
#include "../include/conf.h"
#include "../include/server.h"
#include "../include/glist.h"

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <libgen.h>

extern int debug;

static	PLUGIN_API		*system_api = NULL;
static	pthread_mutex_t system_mutex = PTHREAD_MUTEX_INITIALIZER;

static	CONF_API		*st_conf	= NULL;
static	LOG_API			*st_log		= NULL;

static 	GList			*st_system 	= NULL;
static	char			dir[FILENAME_MAX];

static	gint			system_traverse(gpointer data, gpointer user_data);
static	gboolean		system_print(int fd);
static	gboolean		system_job (int fd, int argc, char **argv, void *data);
static	gint			compare(gconstpointer a, gconstpointer b);

#define SYSTEM_LOCK 	pthread_mutex_lock(&system_mutex)
#define SYSTEM_UNLOCK 	pthread_mutex_unlock(&system_mutex)

static CONF_PARSER module_config[] = {
        { "plugin_dir", CF_TYPE_STRING, FILENAME_MAX, dir, "/opt/IPStat/lib" },
        { NULL, -1, 0, NULL, NULL }
};

gboolean		sys_init(void);
gboolean		sys_free(void);

gboolean 	
system_init(PLUGIN_API *api)
{
	gboolean	res = FALSE;
	SERVER_API	*server = NULL;

	st_conf = (CONF_API*) api->conf;
	st_log	= (LOG_API*)  api->log;
	
	if ((system_api = api)==NULL) {
		st_log->print_log(IP_ERR, "[system.c][system_init]: not correct system api");
		return FALSE;
	}
	if (st_conf) st_conf->parse(module_config);

	res = sys_init();
	
	if (system_api) server = (SERVER_API*) system_api->server;

	if (server) server->registred("system", (void*)&system_job, NULL);
	return res;
}


gboolean	
system_free(void)
{
	gboolean	res;
	SERVER_API	*server = NULL;
	
	if (system_api) server = (SERVER_API*) system_api->server;
	if (server) server->unregistred("system");
	res = sys_free();
	
	system_api = NULL;
	
	return res;
}

gboolean
sys_init(void)
{
	SYSTEM			*node;
	PLUGIN_INFO		*info;
	char			*error;
	DIR				*dd;
	struct dirent	*dp;
	char			path[200], *t_path;
	
	
	if ((dd = opendir(dir))!=NULL) {
		while ((dp = readdir(dd))!= NULL) {
			char    *cp;
			
			if ((node = g_malloc0(sizeof(SYSTEM))) == NULL) {
				st_log->print_log(IP_ALERT, "[system.c][sys_init]: error allocate memory for SYSTEM");
				return FALSE;
			}			

			t_path = strdup(dp->d_name);
			cp = t_path+strlen(t_path)-3;
			if (!cp) {
				free(t_path);
				g_free(node);
				continue;
			}
			if ((strcmp(dp->d_name,"..")!=0)&&(strcmp(dp->d_name,".")!=0)&&(memcmp(dp->d_name, "libipstat_", 10)==0)&&(memcmp(cp, ".so", 3)==0)) {
				sprintf(path, "%s/%s", dir, dp->d_name);

				node->library = dlopen(path, RTLD_LAZY);

				if (!node->library){
					st_log->print_log(IP_ERR, "Loading plugin '%s' failed (%s)\n", basename(path), dlerror());
					free(t_path);
					dlclose(node->library);
					g_free(node);
					continue;
				}
				
				node->info = dlsym(node->library, "info");
				if ((error = dlerror()) != NULL) {
					st_log->print_log(IP_ALERT, "[system.c][sys_init]: error symlink to 'info'. Check library");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
				
				if ((info = node->info())==NULL) {
					st_log->print_log(IP_ALERT, "[system.c][sys_info]: error execute 'info'");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
	
				if (info->type != IPSTAT_PLUGIN_SERVICE) {
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}

				if (info->api_version<SYSTEM_API_VERSION) {
					st_log->print_log(IP_ALERT, "[system.c][sys_init]: error not correct version API module");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
				
				node->name = info->name;
				node->load = dlsym(node->library, "load");
				if ((error = dlerror()) != NULL) {
					st_log->print_log(IP_ALERT, "[system.c][sys_init]: error symlink to 'load'");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}

				node->unload = dlsym(node->library, "unload");
				if ((error = dlerror()) != NULL) {
					st_log->print_log(IP_ALERT, "[system.c][sys_init]: error symlink to 'unload'");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
				SYSTEM_LOCK;
					st_system = g_list_prepend(st_system, node);
				SYSTEM_UNLOCK;
				st_log->print_log(IP_INFO, "*****************************************************");
				st_log->print_log(IP_INFO, "*** Loading system module:                       *");
				st_log->print_log(IP_INFO, "*****************************************************");
				st_log->print_log(IP_INFO, "* Name - %s", info->name);
				st_log->print_log(IP_INFO, "* Version - %s", info->version);
				st_log->print_log(IP_INFO, "* Author - %s [%s]", info->author, info->homepage);
				st_log->print_log(IP_INFO, "* Desc - %s", info->description);
				st_log->print_log(IP_INFO, "*****************************************************");
			}
		}
	}
	return TRUE;
}

gboolean
sys_free(void)
{
	GList	*t_tek, *t_tmp;
	
	SYSTEM_LOCK;	
	t_tmp = st_system;
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		
		if (t_tek->data) g_free(t_tek->data);
	}
	g_list_free(st_system);
	st_system = NULL;
	SYSTEM_UNLOCK;
	return TRUE;
}

gboolean
system_load_all(void)
{
	GList		*t_tek, *t_tmp;
	SYSTEM 		*system;
	
	if (debug) st_log->print_log(IP_DEBUG, "Start load system module");
	t_tmp = st_system;
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		
		system = (SYSTEM*) t_tek->data;
		
		if (system->load(system_api) == TRUE) {
			system->status = TRUE;
			st_log->print_log(IP_INFO, "System module '%s' loaded", system->name);
		}
	}

	return TRUE;
}

gboolean
system_unload_all(void)
{
	GList		*t_tek, *t_tmp;
	SYSTEM 		*system;

	if (debug) st_log->print_log(IP_DEBUG, "Start unload system modules");
	
	t_tmp = st_system;
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		
		system = (SYSTEM*) t_tek->data;
		
		if (system->unload(NULL) == TRUE) {
			system->status = FALSE;
			st_log->print_log(IP_INFO, "System module '%s' unloaded", system->name);
		}
	}
	return TRUE;
}

gboolean
system_load(char *name)
{
	SYSTEM 		system, *col;
	GList		*ptr;
	gboolean	res = FALSE;
	
	system.name = name;
	
	SYSTEM_LOCK;
		ptr = g_list_find_custom(st_system, &system, compare);
	
		if (ptr) {
			col = (SYSTEM*) ptr->data;
			if (col->status==FALSE) {
				if (col->load(system_api)==TRUE) {
					col->status = TRUE;
					st_log->print_log(IP_INFO, "System module '%s' loaded", col->name);
					res = TRUE;
				}else{
					st_log->print_log(IP_INFO, "System module '%s' failed load", col->name);
					res = FALSE;
				}
			}
		}
	SYSTEM_UNLOCK;	

	return res;
}

gboolean
system_unload(char *name)
{
	SYSTEM 		system, *col;
	GList		*ptr;
	gboolean	res = FALSE;
	
	system.name = name;
	
	SYSTEM_LOCK;
		ptr = g_list_find_custom(st_system, &system, compare);

		if (ptr) {
			col = (SYSTEM*) ptr->data;
			if (col->status==TRUE) {
				if (col->unload(system_api)==TRUE) {
					col->status = FALSE;
					st_log->print_log(IP_INFO, "System module '%s' unloaded", col->name);
					res = TRUE;
				}else{
					st_log->print_log(IP_INFO, "System module '%s' failed unload", col->name);
					res = FALSE;
				}
			}
		}
	SYSTEM_UNLOCK;	

	return res;
}

static gboolean              
system_print(int fd) 
{
	send_to_socket(fd, "List System Modules:\n\r");
	send_to_socket(fd, "-----------------------------------------------------------------------------\n\r");
	send_to_socket(fd, "| %5s | %18s | %10s | %20s | %8s |\n\r", "id", "Name", "Version","Author", "Status");
	send_to_socket(fd, "-----------------------------------------------------------------------------\n\r");
	SYSTEM_LOCK;
		g_list_foreach(st_system, (void*)&system_traverse, &fd);
	SYSTEM_UNLOCK;
	send_to_socket(fd, "-----------------------------------------------------------------------------\n\r");
 	return TRUE;
}

static gint
system_traverse(gpointer data, gpointer user_data)
{
	int             *fd = (int*) user_data;
	SYSTEM      	*system = (SYSTEM*) data;
	PLUGIN_INFO		*info;
	char			tmp[5];

	info = system->info();
	if (system->status) 	sprintf(tmp, "ON");
	else					sprintf(tmp, "OFF");
		
	send_to_socket(*fd, "| %5s | %18s | %10s | %20s | %8s |\n\r", info->id, info->name, info->version, info->author, tmp);
	return FALSE;
}

static gboolean		
system_job (int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - show system    - Show system modules     *\n\r");
		return FALSE;
    }else if ((argc == 2) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "system", 6) == 0)) {
		system_print(fd);
		return TRUE;
	}
	return FALSE;
}
static gint 
compare(gconstpointer a, gconstpointer b)
{
	SYSTEM	*node1 = (SYSTEM*) a;
	SYSTEM	*node2 = (SYSTEM*) b;

	return (gint) strncmp(node1->name, node2->name, 50);
}
