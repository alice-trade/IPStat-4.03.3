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

#include "../include/collectors.h"
#include "../include/log.h"
#include "../include/conf.h"
#include "../include/misc.h"
#include "../include/server.h"
#include "../include/glist.h"

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <libgen.h>

extern int debug;

static	PLUGIN_API	*plugin_api = NULL;
static	pthread_mutex_t collect_mutex = PTHREAD_MUTEX_INITIALIZER;

static	CONF_API		*st_conf	= NULL;
static	LOG_API			*st_log		= NULL;

static 	GList		*st_collectors 	= NULL;
static	char		dir[FILENAME_MAX];

static	gint			collect_traverse(gpointer data, gpointer user_data);
static	gboolean		collect_print(int fd);
static	gboolean		collect_job (int fd, int argc, char **argv, void *data);
static	gint			compare(gconstpointer a, gconstpointer b);

#define COLLECT_LOCK	pthread_mutex_lock(&collect_mutex)
#define COLLECT_UNLOCK	pthread_mutex_unlock(&collect_mutex)

static CONF_PARSER module_config[] = {
        { "plugin_dir", CF_TYPE_STRING, FILENAME_MAX, dir, "/opt/IPStat/lib" },
        { NULL, -1, 0, NULL, NULL }
};

gboolean		col_init(void);
gboolean		col_free(void);

gboolean 	
collectors_init(PLUGIN_API *api)
{
	gboolean	res = FALSE;
	SERVER_API	*server = NULL;

	st_conf = (CONF_API*) api->conf;
	st_log	= (LOG_API*)  api->log;
	
	if ((plugin_api = api)==NULL) {
		st_log->print_log(IP_ERR, "[collectors.c][collectors_init]: not correct collector api");
		return FALSE;
	}
	if (st_conf) st_conf->parse(module_config);

	res = col_init();
	
	if (plugin_api) server = (SERVER_API*) plugin_api->server;

	if (server) server->registred("collectors", (void*)&collect_job, NULL);
	return res;
}


gboolean	
collectors_free(void)
{
	gboolean	res;
	SERVER_API	*server = NULL;
	
	if (plugin_api) server = (SERVER_API*) plugin_api->server;
	if (server) server->unregistred("collectors");
	res = col_free();
	
	plugin_api = NULL;
	
	return res;
}

gboolean
col_init(void)
{
	COLLECTOR		*node;
	PLUGIN_INFO		*info;
	char			*error;
	DIR				*dd;
	struct dirent	*dp;
	char			path[200], *t_path;
	
	
	if ((dd = opendir(dir))!=NULL) {
		while ((dp = readdir(dd))!= NULL) {
			char    *cp;
			
			if ((node = (COLLECTOR*) g_malloc0(sizeof(COLLECTOR))) == NULL) {
				st_log->print_log(IP_ALERT, "[collectors.c][col_init]: error allocate memory for COLLECTOR");
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
				
				node->info = dlsym(node->library,"info");
				if ((error = dlerror()) != NULL) {
					st_log->print_log(IP_ALERT, "[collectors.c][col_init]: error symlink to 'info'. Check library");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
				
				if ((info = node->info())==NULL) {
					st_log->print_log(IP_ALERT, "[collectors.c][col_info]: error execute 'info'");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
	
				if (info->type != IPSTAT_PLUGIN_COLLECTOR) {
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}

				if (info->api_version<COLLECTORS_API_VERSION) {
					st_log->print_log(IP_ALERT, "[collectors.c][col_init]: error not correct version API module");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
				
				node->name = info->name;
				node->load = dlsym(node->library, "load");
				if ((error = dlerror()) != NULL) {
					st_log->print_log(IP_ALERT, "[collectors.c][col_init]: error symlink to 'load'");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}

				node->unload = dlsym(node->library, "unload");
				if ((error = dlerror()) != NULL) {
					st_log->print_log(IP_ALERT, "[collectors.c][col_init]: error symlink to 'unload'");
					dlclose(node->library);
					g_free(node);
					free(t_path);
					continue;
				}
				st_log->print_log(IP_INFO, "[collectors.c][col_init]: %s", dp->d_name);
				if (memcmp(dp->d_name, ".off.", 5)==0)  node->status = FALSE;
				else					node->status = TRUE;

				COLLECT_LOCK;
					st_collectors = g_list_prepend(st_collectors, node);
				COLLECT_UNLOCK;
				st_log->print_log(IP_INFO, "*****************************************************");
				st_log->print_log(IP_INFO, "*** Loading collector module:                       *");
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
col_free(void)
{
	GList	*t_tek, *t_tmp;
	
	COLLECT_LOCK;	
	t_tmp = st_collectors;
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		
		if (t_tek->data) g_free(t_tek->data);
	}
	g_list_free(st_collectors);
	st_collectors = NULL;
	COLLECT_UNLOCK;
	return TRUE;
}

gboolean
collectors_load_all(void)
{
	GList		*t_tek, *t_tmp;
	COLLECTOR 	*collect;
	
	if (debug) st_log->print_log(IP_DEBUG, "Start load collectors");
	t_tmp = st_collectors;
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		
		collect = (COLLECTOR*) t_tek->data;
		
		if (collect->load(plugin_api) == TRUE) {
//			collect->status = TRUE;
			st_log->print_log(IP_INFO, "Collector module '%s' loaded", collect->name);
		}
	}

	return TRUE;
}

gboolean
collectors_unload_all(void)
{
	GList		*t_tek, *t_tmp;
	COLLECTOR 	*collect;

	if (debug) st_log->print_log(IP_DEBUG, "Start unload collectors");
	
	t_tmp = st_collectors;
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		
		collect = (COLLECTOR*) t_tek->data;
		
		if (collect->unload(NULL) == TRUE) {
			collect->status = FALSE;
			st_log->print_log(IP_INFO, "Collector module '%s' unloaded", collect->name);
		}
	}
	return TRUE;
}

gboolean
collectors_load(char *name)
{
	COLLECTOR 	collect, *col;
	GList		*ptr;
	gboolean	res = FALSE;
	
	collect.name = name;
	
	COLLECT_LOCK;
		ptr = g_list_find_custom(st_collectors, &collect, compare);
	
		if (ptr) {
			col = (COLLECTOR*) ptr->data;
			if (col->status==FALSE) {
				if (col->load(plugin_api)==TRUE) {
					col->status = TRUE;
					st_log->print_log(IP_INFO, "Collector module '%s' loaded", col->name);
					res = TRUE;
				}else{
					st_log->print_log(IP_INFO, "Collector module '%s' failed load", col->name);
					res = FALSE;
				}
			}
		}
	COLLECT_UNLOCK;	

	return res;
}

gboolean
collectors_unload(char *name)
{
	COLLECTOR 	collect, *col;
	GList		*ptr;
	gboolean	res = FALSE;
	
	collect.name = name;
	
	COLLECT_LOCK;
		ptr = g_list_find_custom(st_collectors, &collect, compare);

		if (ptr) {
			col = (COLLECTOR*) ptr->data;
			if (col->status==TRUE) {
				if (col->unload(plugin_api)==TRUE) {
					col->status = FALSE;
					st_log->print_log(IP_INFO, "Collector module '%s' unloaded", col->name);
					res = TRUE;
				}else{
					st_log->print_log(IP_INFO, "Collector module '%s' failed unload", col->name);
					res = FALSE;
				}
			}
		}
	COLLECT_UNLOCK;	

	return res;
}

static gboolean              
collect_print(int fd) 
{
	send_to_socket(fd, "List Collectors:\n\r");
	send_to_socket(fd, "------------------------------------------------------------------------------\n\r");
	send_to_socket(fd, "| %5s  | %18s | %10s | %20s | %8s |\n\r", "id", "Name", "Version","Author", "Status");
	send_to_socket(fd, "------------------------------------------------------------------------------\n\r");
	COLLECT_LOCK;
		g_list_foreach(st_collectors, (void*)&collect_traverse, &fd);
	COLLECT_UNLOCK;
	send_to_socket(fd, "-----------------------------------------------------------------------------\n\r");
 	return TRUE;
}

static gint
collect_traverse(gpointer data, gpointer user_data)
{
	int             *fd = (int*) user_data;
	COLLECTOR      	*collect = (COLLECTOR*) data;
	PLUGIN_INFO		*info;
	char			tmp[5];

	info = collect->info();
	if (collect->status) 	sprintf(tmp, "ON");
	else					sprintf(tmp, "OFF");
		
	send_to_socket(*fd, "| %5s | %18s | %10s | %20s | %8s |\n\r", info->id, info->name, info->version, info->author, tmp);
	return FALSE;
}

static gboolean		
collect_job (int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - show collect    - Print list collectors  *\n\r");
		send_to_socket(fd, "*  - on collect    - On Collector             *\n\r");
		send_to_socket(fd, "*  - off collect   - Off Collector            *\n\r");
		return FALSE;
    }else if ((argc == 2) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "collect", 6) == 0)) {
		collect_print(fd);
		return TRUE;
	}else if ((argc == 3) && (strncmp(argv[0], "on", 4) == 0) && (strncmp(argv[1], "collect", 6) == 0)) {
//		collectors_load(argv[2]);
		return TRUE;
	}else if ((argc == 3) && (strncmp(argv[0], "off", 6) == 0) && (strncmp(argv[1], "collect", 6) == 0)) {
//		collectors_unload(argv[2]);
		return TRUE;
	}
	return FALSE;
}
static gint 
compare(gconstpointer a, gconstpointer b)
{
	COLLECTOR	*node1 = (COLLECTOR*) a;
	COLLECTOR	*node2 = (COLLECTOR*) b;

	return (gint) strncmp(node1->name, node2->name, 50);
}
