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

#include "../include/func_time.h"
#include "../include/sched.h"
#include "../include/store.h"
#include "../include/server.h"
#include "../include/log.h"
#include "../include/misc.h"
#include "../include/gtree.h"

#include <pthread.h>
#include <stdio.h>

static gint			time_traverse_print (gpointer key, gpointer value, gpointer data);
static gint			time_traverse_free (gpointer key, gpointer value, gpointer data);
static gint			time_traverse_sync (gpointer key, gpointer value, gpointer data);
static gboolean		time_export(SQL_SESSION *sid, IP_id id_user, IP_size size);
static gboolean     time_print(int fd);
static gint			time_compare (gconstpointer  a, gconstpointer  b);
static gboolean		time_clean(ITree *tree);
static gboolean		time_job (int fd, int argc, char **argv, void *data);
static gboolean		time_sync(void *data);
static gboolean		time_group(void *data);

static pthread_mutex_t	time_mutex = PTHREAD_MUTEX_INITIALIZER;
static ITree			*ag_time = NULL;
static PLUGIN_API	*t_api;

#define TIME_LOCK 		pthread_mutex_lock(&time_mutex)
#define TIME_UNLOCK 	pthread_mutex_unlock(&time_mutex)

static int	time_aggregation_timeout = 1;
static int	time_group_timeout = 30;

static CONF_PARSER module_config[] = {
        { "time_aggregation_timeout", CF_TYPE_INT, 0, &time_aggregation_timeout, "1" },
        { "time_group_timeout", CF_TYPE_INT, 0, &time_group_timeout, "30" },
        { NULL, -1, 0, NULL, NULL }
};

gboolean
time_init(PLUGIN_API *api)
{
	SERVER_API		*server = NULL;
	SCHEDULER_API	*sched 	= NULL;
	LOG_API			*log 	= api->log;
	CONF_API		*conf 	= NULL;
	char			str[20];
	
	t_api = api;
	
	server 	= (SERVER_API*) api->server;
	sched 	= (SCHEDULER_API*) api->sched;
	conf 	= (CONF_API*) api->conf;
	
	if (conf) conf->parse(module_config);
		
	if ((ag_time = i_tree_new((void*)&time_compare))==NULL) {
		log->print_log(IP_ERR, "Error create g_tree");
		return FALSE;
	}
	
	sprintf(str, "*/%d * * * *", time_aggregation_timeout);
	
	if (server) server->registred("time_aggregation", (void*)&time_job, NULL);
	if (sched) sched->registred("time_aggregation", str, (void*)&time_sync, NULL);
		
	sprintf(str, "*/%d * * * *", time_group_timeout);
	if (time_group_timeout != 0) if (sched) sched->registred("time_group", str, (void*)&time_group, NULL);
		
	return TRUE;
}

gboolean
time_free(void)
{
	SERVER_API	*server = NULL;
	SCHEDULER_API	*sched = NULL;
	
	if (!ag_time) return FALSE;
		
	server = t_api->server;
	sched = t_api->sched;
	
	if (time_group_timeout != 0) if (sched) sched->unregistred("time_group");
	if (sched) sched->unregistred("time_aggregation");
	if (server) server->unregistred("time_aggregation");

		
	time_sync(NULL);	
	TIME_LOCK;
		time_clean(ag_time);
		i_tree_destroy(ag_time);
		ag_time = NULL;
	TIME_UNLOCK;
	return TRUE;
}

gboolean
time_acct(PACKET_DATA_INFO *packet_data)
{
	gpointer			ptr;
	IP_size				*val;
	IP_id				*key;
	IP_size				*value;
	LOG_API				*log = NULL;
	IP_id				id_user = 0;
	
	log		= (LOG_API*) t_api->log;
	
	id_user = packet_data->id_user;
	
	if (!ag_time) return FALSE;
		
	if ((ptr = i_tree_lookup(ag_time, (gpointer)&id_user))!=NULL) {
		TIME_LOCK;
			val = (IP_size*) ptr;
			*val = *val + packet_data->len;
		TIME_UNLOCK;
	} else {
		if ((key = g_malloc0(sizeof(IP_id)))==NULL) {
			log->print_log(IP_ERR, "error allocate memofy for key");
			return FALSE;
		}
		if ((value = g_malloc0(sizeof(IP_size)))==NULL) {
			log->print_log(IP_ERR, "error allocate memofy for value");
			g_free(key);
			return FALSE;
		}
		*key = id_user;
		*value = packet_data->len;
		TIME_LOCK;
			i_tree_insert(ag_time, key, value);
		TIME_UNLOCK;
	}
	return TRUE;
}

static gboolean		
time_sync(void *data)
{
	STORE_API	*store = NULL;
	SQL_SESSION	*sid = NULL;
	
	if (!(store = t_api->store)) {
		fprintf(stderr, "netfilter_time: error find store api\n");
		return FALSE;
	}
	if ((sid = store->open())!=NULL) {

		i_tree_traverse(ag_time, (void*)&time_traverse_sync, I_IN_ORDER, sid);
		
		store->close(sid);
	}else fprintf(stderr, "netfilter_time: error open connection\n");
	return TRUE;	
}

static gboolean              
time_print(int fd) 
{
	send_to_socket(fd, "Time Aggregations:\n\r");
	send_to_socket(fd, "-----------------------------------------------------------------------------\n\r");
	send_to_socket(fd, "| %8s | %20s |\n\r", "id_user", "Traffic");
	send_to_socket(fd, "-----------------------------------------------------------------------------\n\r");
	TIME_LOCK;
		i_tree_traverse(ag_time, (void*)&time_traverse_print, I_IN_ORDER, &fd);
	TIME_UNLOCK;
	send_to_socket(fd, "-----------------------------------------------------------------------------\n\r");
 	return TRUE;
}

static gint 
time_traverse_print (gpointer key, gpointer value, gpointer data)
{
	int            *fd = (int*) data;
	IP_id      		*t_key = (IP_id*) key;
	IP_size			*t_value = (IP_size*) value;

	send_to_socket(*fd, "| %8d | %20u |\n\r", *t_key, *t_value);
	return FALSE;
}

static gint 
time_traverse_sync (gpointer key, gpointer value, gpointer data)
{
	SQL_SESSION *sid = (SQL_SESSION*) data;
	IP_id		*id_user = (IP_id*) key;
	IP_size		tmp, *size = (IP_size*) value;
	
	TIME_LOCK;
		tmp = *size;
		*size-=tmp;
	TIME_UNLOCK;
	if (time_export(sid, *id_user, tmp)!=TRUE) {
		fprintf(stderr, "netfilter_time: error export record\n");
		TIME_LOCK;
			*size+=tmp;
		TIME_UNLOCK;
	}		
	
	return FALSE;
}

static gint 
time_traverse_free (gpointer key, gpointer value, gpointer data)
{
	if (key) g_free(key);
	if (value) g_free(value);
		
	return FALSE;
}

static gboolean
time_export(SQL_SESSION *sid, IP_id id_user, IP_size size)
{
    SQL_ROW     row;
    char        *buffer;
    IP_flag     flag_tmp = 0;
	STORE_API	*store = NULL;
	LOG_API		*log = NULL;

	log = (LOG_API*) t_api->log;
	
	if (!(store = t_api->store))	return FALSE;

    if ((buffer = (char*) g_malloc0(1024))==NULL) {
        log->print_log(IP_ERR, "error allocate memory for buffer");
        return FALSE;
    }

    sprintf(buffer, "select COUNT(id) from Log_Temp where id = %u", id_user);
    if (!store->query(sid, buffer)) goto error;
	
	
    row = store->fetch_row(sid);
    flag_tmp = atol(row[0]);
    store->free(sid);

    if (flag_tmp){
        sprintf(buffer, "update Log_Temp SET Traffic = Traffic + %u where id=%u", size, id_user);
    }else{
        sprintf(buffer, "insert into Log_Temp values (%u, %u)",id_user, size);
    }
    
	if (!store->exec(sid, buffer)) goto error;

    g_free(buffer);
    return TRUE;
error:
    g_free(buffer);
    return FALSE;
}



static gint
time_compare (gconstpointer  a, gconstpointer  b)
{
	IP_id	*id_1	= (IP_id*) a;
	IP_id	*id_2	= (IP_id*) b;
	
	return memcmp(id_1, id_2, sizeof(IP_id));
}



static gboolean
time_clean(ITree *tree)
{
	i_tree_traverse(tree, (void*)&time_traverse_free, I_IN_ORDER, NULL);
	return TRUE;
}
static gboolean		
time_job (int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - show time      - Show time`s aggregation *\n\r");
		return FALSE;
    }else if ((argc == 2) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "time", 4) == 0)) {
		time_print(fd);
		return TRUE;
	}
	return FALSE;
}

static gboolean
time_group(void *data)
{
    SQL_SESSION *sid;
    STORE_API	*store = NULL;
	
	if ((store = (STORE_API*) t_api->store) == NULL) return FALSE;

    if ((sid = store->open())!=NULL){
        if (!store->exec(sid, "set autocommit=0")) goto error;
        if (!store->exec(sid, "start transaction")) goto error;
        if (!store->exec(sid, "insert into Log_Time (Time, id_user, Traffic) select NOW(), id, Traffic from Log_Temp where Traffic!=0")) goto error;
        if (!store->exec(sid, "update Log_Temp SET Traffic = 0")) goto error;
        if (!store->exec(sid, "commit")) goto error;
        goto ok;
error:
        store->exec(sid, "rollback");
		store->close(sid);
		return FALSE;
ok:
        store->close(sid);
		return TRUE;
    }
	return TRUE;
}
