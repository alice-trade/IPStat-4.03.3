/*
 * ipstat_system_time_police.c	Function for works with system.
 *
 * Version:	$Id: main.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $
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
 *   (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@code-art.ru]
*/

static const char rcsid[] = "$Id: main.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $";

#include "../include/core.h"
#include "../include/system.h"
#include "../include/misc.h"
#include "../include/tasks.h"
#include "../include/types.h"
#include "../include/plugin.h"
#include "../include/sched.h"
#include "../include/server.h"
#include "../include/log.h"
#include "../include/glist.h"

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

static PLUGIN_INFO	module_info;

static PLUGIN_API	*api;

int debug = 0;
static GList	*st_policy_time = NULL;
static GList	*st_policy_users = NULL;
static GList	*st_policy_group_users = NULL;
static GList	*st_policy_type_traffic = NULL;
static GList	*st_policy_group_traffic = NULL;

static time_t	st_begin_period = 0;
static guint8	st_week_day = 0;

static char	*id 		= "401nti";
static char	*name 		= "system_time_police";
static char	*version 	= "0.1";
static char	*description	= "System module for IPStat Time Police";
static char	*author		= "Kukuev Dmitry";
static char	*homepage	= "http://ipstat.code-art.ru/";


typedef struct NODE_POLICY_TIME
{
	IP_id	id;
	IP_id	id_policy;
	guint8	week_day;
	guint	time_begin;
	guint	time_end;
	guint8	target;
	char	md5[33];
	time_t	time_live;
} NODE_POLICY_TIME;

typedef struct NODE_POLICY_USER
{
	IP_id	id_user;
	IP_id	id_policy;
	guint8	def;
	char	md5[33];
	time_t	time_live;
} NODE_POLICY_USER;

typedef struct NODE_POLICY_GROUP_USER
{
	IP_id	id_group_user;
	IP_id	id_policy;
	guint8	def;
	char	md5[33];
	time_t	time_live;
} NODE_POLICY_GROUP_USER;

typedef struct NODE_POLICY_TYPE_TRAFFIC
{
	IP_id	id_type_traffic;
	IP_id	id_policy;
	guint8	def;
	char	md5[33];
	time_t	time_live;
} NODE_POLICY_TYPE_TRAFFIC;

typedef struct NODE_POLICY_GROUP_TRAFFIC
{
	IP_id	id_group_traffic;
	IP_id	id_policy;
	guint8	def;
	char	md5[33];
	time_t	time_live;
} NODE_POLICY_GROUP_TRAFFIC;

static gboolean		autz(IP_id service, void *data, void *ret);

static gboolean		sync_users_list(void *data);
static gboolean		sync_group_users_list(void *data);
static gboolean		sync_type_traffic_list(void *data);
static gboolean		sync_group_traffic_list(void *data);
static gboolean		sync_time_list(void *data);

static gboolean		policy_time_job (int fd, int argc, char **argv, void *data);
static gboolean		every_day(void *data);
static gboolean		check_policy_time_tables(STORE_API *store, LOG_API *log);

static gboolean		init_users_list(void);
static gboolean		init_group_users_list(void);
static gboolean		init_type_traffic_list(void);
static gboolean		init_group_traffic_list(void);
static gboolean		init_policy_list(void);

static gboolean		free_policy_list(void);
static gboolean		free_group_traffic_list(void);
static gboolean		free_type_traffic_list(void);
static gboolean		free_group_users_list(void);
static gboolean		free_users_list(void);

static gboolean		check_user(IP_id id_user, IP_id *id_policy, guint8 *def);
static gboolean		check_group_user(IP_id id_group_user, IP_id *id_policy, guint8 *def);
static gboolean		check_type_traffic(IP_id id_type_traffic, IP_id *id_policy, guint8 *def);
static gboolean		check_group_traffic(IP_id id_group_traffic, IP_id *id_policy, guint8 *def);
static gboolean		check_policy(IP_id id_policy, guint8 def);

static void			policy_traverse_free (gpointer data, gpointer user_data);
static gint			policy_compare_users(gconstpointer a, gconstpointer b);
static gint			policy_compare_group_users(gconstpointer a, gconstpointer b);
static gint			policy_compare_type_traffic(gconstpointer a, gconstpointer b);
static gint			policy_compare_group_traffic(gconstpointer a, gconstpointer b);
static gint 		policy_compare_time(gconstpointer a, gconstpointer b);

static void			policy_traverse_users_print (gpointer data, gpointer user_data);
static void			policy_traverse_group_users_print (gpointer data, gpointer user_data);
static void			policy_traverse_type_traffic_print (gpointer data, gpointer user_data);
static void			policy_traverse_group_traffic_print (gpointer data, gpointer user_data);
static void			policy_id_traverse_print (gpointer data, gpointer user_data);

static time_t		get_begin_period(void);
static gboolean		policy_time_users_print(int fd);
static gboolean		policy_time_group_users_print(int fd);
static gboolean		policy_time_type_traffic_print(int fd);
static gboolean		policy_time_group_traffic_print(int fd);
static gboolean		policy_time_print(int fd);


static gboolean		insert_policy_user(IP_id id_user);
static gboolean		insert_policy_group_user(IP_id id_group_user);
static gboolean		insert_policy_type_traffic(IP_id id_type_traffic);
static gboolean		insert_policy_group_traffic(IP_id id_group_traffic);
static gboolean		insert_policy_time(IP_id id);

static gboolean		update_policy_user(IP_id id_user);
static gboolean		update_policy_group_user(IP_id id_group_user);
static gboolean		update_policy_type_traffic(IP_id id_type_traffic);
static gboolean		update_policy_group_traffic(IP_id id_group_traffic);
static gboolean		update_policy_time(IP_id id);

static gboolean		delete_policy_user(IP_id id_user);
static gboolean		delete_policy_group_user(IP_id id_group_user);
static gboolean		delete_policy_type_traffic(IP_id id_type_traffic);
static gboolean		delete_policy_group_traffic(IP_id id_group_traffic);
static gboolean		delete_policy_time(IP_id id);

static NODE_POLICY_USER*			user_find(IP_id id_user);
static NODE_POLICY_GROUP_USER*		group_user_find(IP_id id_group_user);
static NODE_POLICY_TYPE_TRAFFIC*	type_traffic_find(IP_id id_type_traffic);
static NODE_POLICY_GROUP_TRAFFIC*	group_traffic_find(IP_id id_group_traffic);
static NODE_POLICY_TIME*			time_find(IP_id id);


static pthread_mutex_t 	policy_users_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t 	policy_group_users_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t 	policy_type_traffic_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t 	policy_group_traffic_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t 	policy_time_mutex = PTHREAD_MUTEX_INITIALIZER;

#define POLICY_USERS_LOCK 			pthread_mutex_lock(&policy_users_mutex)
#define POLICY_USERS_UNLOCK 		pthread_mutex_unlock(&policy_users_mutex)

#define POLICY_GROUP_USERS_LOCK 	pthread_mutex_lock(&policy_group_users_mutex)
#define POLICY_GROUP_USERS_UNLOCK 	pthread_mutex_unlock(&policy_group_users_mutex)

#define POLICY_TYPE_TRAFFIC_LOCK 	pthread_mutex_lock(&policy_type_traffic_mutex)
#define POLICY_TYPE_TRAFFIC_UNLOCK 	pthread_mutex_unlock(&policy_type_traffic_mutex)

#define POLICY_GROUP_TRAFFIC_LOCK 	pthread_mutex_lock(&policy_group_traffic_mutex)
#define POLICY_GROUP_TRAFFIC_UNLOCK pthread_mutex_unlock(&policy_group_traffic_mutex)

#define POLICY_TIME_LOCK 			pthread_mutex_lock(&policy_time_mutex)
#define POLICY_TIME_UNLOCK 			pthread_mutex_unlock(&policy_time_mutex)

static int	sync_time_policy_timeout = 1;

static CONF_PARSER module_config[] = {
        { "sync_time_policy_timeout", CF_TYPE_INT, 0, &sync_time_policy_timeout, "1" },
        { NULL, -1, 0, NULL, NULL }
};

__attribute__ ((constructor)) void init()
{

}
	                                                                                
__attribute__ ((destructor)) void fini()
{
	                                                                                
}
	
//----------------------------------------------------------------------------
//				info
//----------------------------------------------------------------------------
PLUGIN_INFO
*info(void)
{
    module_info.api_version 	= 1;
    module_info.type		= IPSTAT_PLUGIN_SERVICE;
    module_info.flags		= 0;
    module_info.priority	= IPSTAT_PRIORITY_DEFAULT;
    module_info.id 		= id;
    module_info.name		= name;
    module_info.version		= version;
    module_info.summary		= NULL;
    module_info.description	= description;
    module_info.author		= author;
    module_info.homepage	= homepage;
    return &module_info;
}



gboolean
load(void *data)
{
    CORE_API		*core  = NULL;
    LOG_API			*log   = NULL;
	SCHEDULER_API	*sched = NULL;
	SERVER_API		*server= NULL;
	STORE_API		*store = NULL;
	CONF_API		*conf 	= NULL;
	char			str[20];
	
    api = (PLUGIN_API*) data;

    core  	= (CORE_API*) api->core;
    log		= (LOG_API*)  api->log;
    sched	= (SCHEDULER_API*) api->sched;
	server	= (SERVER_API*) api->server;
	store	= (STORE_API*) api->store;
	conf 	= (CONF_API*) api->conf;
	
	if (conf) conf->parse(module_config);
		
    fprintf(stderr, "Start System module...\n");    
    log->print_log(IP_INFO, "Starting System Module for IPStat Time Police");
	
	st_begin_period = get_begin_period();
	log->print_log(IP_DEBUG, "begin_period=%d", st_begin_period);
	
	check_policy_time_tables(store, log);
	
	init_users_list();
	init_group_users_list();
	init_type_traffic_list();
	init_group_traffic_list();
    init_policy_list();
	
    core->register_autz_hook((void*)&autz);
    
	sprintf(str, "*/%d * * * *", sync_time_policy_timeout);
	if (sched) sched->registred("sync_policy_users", str, (void*)&sync_users_list, NULL);
	if (sched) sched->registred("sync_policy_group_users", str, (void*)&sync_group_users_list, NULL);
	if (sched) sched->registred("sync_policy_type_traffic", str, (void*)&sync_type_traffic_list, NULL);
	if (sched) sched->registred("sync_policy_group_traffic", str, (void*)&sync_group_traffic_list, NULL);
	if (sched) sched->registred("sync_policy_time", str, (void*)&sync_time_list, NULL);
	if (server) server->registred("policy_time", (void*)&policy_time_job, NULL);
	if (sched) sched->registred("policy_time_every_day", "0 0 * * *", (void*)&every_day, NULL);
		
    return TRUE;
}

gboolean
unload(void *data)
{
    CORE_API	*core = NULL;
    LOG_API	*log   = NULL;
	SCHEDULER_API	*sched = NULL;
	SERVER_API		*server= NULL;
	
    core = (CORE_API*) api->core;
    log	 = (LOG_API*)  api->log;
    sched	= (SCHEDULER_API*) api->sched;
	server	= (SERVER_API*) api->server;
	
	if (sched) sched->unregistred("policy_time_every_day");
	if (server) server->unregistred("policy_time");
	if (sched) sched->unregistred("sync_policy_time");
	if (sched) sched->unregistred("sync_policy_group_traffic");
	if (sched) sched->unregistred("sync_policy_type_traffic");
	if (sched) sched->unregistred("sync_policy_group_users");
	if (sched) sched->unregistred("sync_policy_users");		
    core->unregister_autz_hook((void*)&autz);

    free_policy_list();
	free_group_traffic_list();
	free_type_traffic_list();
	free_group_users_list();
	free_users_list();
    log->print_log(IP_INFO, "Stoping System Module for IPStat Time Police");

    return TRUE;
}

static gboolean
autz(IP_id service, void *data, void *ret)
{
    gboolean	res;
	IP_id		id_policy = 0;
	IP_id		id_user = 0;
	IP_id		id_group_user = 0;
	IP_id		id_type_traffic = 0;
	IP_id		id_group_traffic = 0;
	guint8		def;
	
	if (service == SERVICE_TRAFFIC) {
		PACKET_DATA_INFO	*packet_data = (PACKET_DATA_INFO*) ret;
		
		id_user 		= packet_data->id_user;
		id_group_user 	= packet_data->id_group_user;
		id_type_traffic = packet_data->id_traffic_type;
		id_group_traffic= packet_data->id_traffic_group;
	}else if (service == SERVICE_SQUID) {
		SQUID_DATA_INFO	*squid_data = (SQUID_DATA_INFO*) ret;
		id_user = squid_data->id_user;
		id_group_user 	= squid_data->id_group_user;
		id_type_traffic = squid_data->id_traffic_type;
		id_group_traffic= squid_data->id_traffic_group;
	}else return FALSE;
    
	res = check_user(id_user, &id_policy, &def);
	if (res) {											// Если пользователь найден, если нет, то разрешаем
		res = check_policy(id_policy, def);				// Проверяем политику
		if (!res) return FALSE;							// Если запрещён, то отбрасываем.
	}

	res = check_group_user(id_group_user, &id_policy, &def);
	if (res) {
		res = check_policy(id_policy, def);
		if (!res) return FALSE;	
	}
	
	res = check_type_traffic(id_type_traffic, &id_policy, &def);
	if (res) {
		res = check_policy(id_policy, def);
		if (!res) return FALSE;	
	}

	res = check_group_traffic(id_group_traffic, &id_policy, &def);
	if (res) {
		res = check_policy(id_policy, def);
		if (!res) return FALSE;	
	}

    return TRUE;
}

static gboolean
check_user(IP_id id_user, IP_id *id_policy, guint8 *def)
{
	GList 				*ptr = NULL;
	NODE_POLICY_USER	node, *tmp;
	
	node.id_user = id_user;
	
	POLICY_USERS_LOCK;
		ptr = g_list_find_custom(st_policy_users, &node, (void*)&policy_compare_users);
	
		if (ptr == NULL) {
			POLICY_USERS_UNLOCK;
			return FALSE;
		}
	
		if (ptr->data == NULL) {
			POLICY_USERS_UNLOCK;
			return FALSE;
		}
		
		tmp = (NODE_POLICY_USER*) ptr->data;

		*id_policy	= tmp->id_policy;
		*def		= tmp->def;
	POLICY_USERS_UNLOCK;
	
	return TRUE;
}

static gboolean
check_group_user(IP_id id_group_user, IP_id *id_policy, guint8 *def)
{
	GList 					*ptr = NULL;
	NODE_POLICY_GROUP_USER	node, *tmp;
	
	node.id_group_user = id_group_user;
	
	POLICY_GROUP_USERS_LOCK;
		ptr = g_list_find_custom(st_policy_group_users, &node, (void*)&policy_compare_group_users);
	
		if (ptr == NULL) {
			POLICY_GROUP_USERS_UNLOCK;
			return FALSE;
		}
	
		if (ptr->data == NULL) {
			POLICY_GROUP_USERS_UNLOCK;
			return FALSE;
		}
		
		tmp = (NODE_POLICY_GROUP_USER*) ptr->data;

		*id_policy	= tmp->id_policy;
		*def		= tmp->def;
	POLICY_GROUP_USERS_UNLOCK;
	
	return TRUE;
}

static gboolean
check_type_traffic(IP_id id_type_traffic, IP_id *id_policy, guint8 *def)
{
	GList 						*ptr = NULL;
	NODE_POLICY_TYPE_TRAFFIC	node, *tmp;
	
	node.id_type_traffic = id_type_traffic;
	
	POLICY_TYPE_TRAFFIC_LOCK;
		ptr = g_list_find_custom(st_policy_type_traffic, &node, (void*)&policy_compare_type_traffic);
	
		if (ptr == NULL) {
			POLICY_TYPE_TRAFFIC_UNLOCK;
			return FALSE;
		}
	
		if (ptr->data == NULL) {
			POLICY_TYPE_TRAFFIC_UNLOCK;
			return FALSE;
		}
		
		tmp = (NODE_POLICY_TYPE_TRAFFIC*) ptr->data;

		*id_policy	= tmp->id_policy;
		*def		= tmp->def;
	POLICY_TYPE_TRAFFIC_UNLOCK;
	
	return TRUE;
}

static gboolean
check_group_traffic(IP_id id_group_traffic, IP_id *id_policy, guint8 *def)
{
	GList 						*ptr = NULL;
	NODE_POLICY_GROUP_TRAFFIC	node, *tmp;
	
	node.id_group_traffic = id_group_traffic;
	
	POLICY_GROUP_TRAFFIC_LOCK;
		ptr = g_list_find_custom(st_policy_group_traffic, &node, (void*)&policy_compare_group_traffic);
	
		if (ptr == NULL) {
			POLICY_GROUP_TRAFFIC_UNLOCK;
			return FALSE;
		}
	
		if (ptr->data == NULL) {
			POLICY_GROUP_TRAFFIC_UNLOCK;
			return FALSE;
		}
		
		tmp = (NODE_POLICY_GROUP_TRAFFIC*) ptr->data;

		*id_policy	= tmp->id_policy;
		*def		= tmp->def;
	POLICY_GROUP_TRAFFIC_UNLOCK;
	
	return TRUE;
}

static gboolean
check_policy(IP_id id_policy, guint8 def)
{
	GList				*t_tek, *t_tmp;
	NODE_POLICY_TIME	*node;
	guint8				res;
	time_t				tek_time;
//	LOG_API			*log = NULL;
	
//	if ((log = api->log) == NULL) return FALSE;	
//**********************************
// Выход: 1 - разрешён, 0 - запрещён
//**********************************
	res = def;
	POLICY_TIME_LOCK;
		t_tmp = st_policy_time;
//	log->print_log(IP_DEBUG, "tek_date=%u new_day=%u st_week_day=%d delta=%u", time(NULL), st_begin_period, st_week_day, (time(NULL) - st_begin_period));
		while ((t_tek = t_tmp) != NULL) {
			t_tmp = g_list_next(t_tmp);
			node = (NODE_POLICY_TIME*) t_tek->data;
			tek_time = time(NULL) - st_begin_period;

			if ((node->id_policy == id_policy) && (st_week_day == node->week_day) && (tek_time >= node->time_begin) && (tek_time <= node->time_end)) {
				res = node->target;
				break;
			}
		}
	POLICY_TIME_UNLOCK;
	if (res == 0) return FALSE;
	return TRUE;	
}

//******************************************************************************
// Init Policy List
//******************************************************************************
static gboolean
init_policy_list(void)
{
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	SQL_SESSION		*sid;
	SQL_ROW			row;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		if (!store->query(sid, "select id, id_policy, Week_Day, Time_Begin, Time_End, Target, md5(concat(id_policy, Week_Day, Time_Begin, Time_End, Target)) from Policy_Time_Def order by id")) {
			log->print_log(IP_ERR, "[policy_time.c][init_users_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			NODE_POLICY_TIME	*node;
			
			if ((node = g_malloc0(sizeof(NODE_POLICY_TIME))) == NULL) {
				log->print_log(IP_ERR, "[policy_time]: error allocate memory for NODE_POLICY_TIME, skiping");
				continue;
			}
			
			node->id 			= atoi(row[0]);
			node->id_policy 	= atoi(row[1]);
			node->week_day		= atoi(row[2]);
			node->time_begin	= atol(row[3]);
			node->time_end		= atol(row[4]);
			node->target		= atoi(row[5]);
			strncpy(node->md5, row[6], 32);
			node->time_live		= time(NULL);

			st_policy_time = g_list_append(st_policy_time, node);
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time]: Load Time Policy (id=%d, id_policy=%d, md5=%s)",  node->id, node->id_policy, node->md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	} else {
		log->print_log(IP_ERR, "[policy_time][init_policy_list]: error connect to mysql");
		return FALSE;
	}		

	return TRUE;
}

static gboolean
init_users_list(void)
{
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	SQL_SESSION		*sid;
	SQL_ROW			row;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		if (!store->query(sid, "select a.id_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_Users a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][init_users_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			NODE_POLICY_USER	*node;
			
			if ((node = g_malloc0(sizeof(NODE_POLICY_USER))) == NULL) {
				log->print_log(IP_ERR, "[policy_time]: error allocate memory for NODE_POLICY_USER, skiping");
				continue;
			}
			
			node->id_user 		= atoi(row[0]);
			node->id_policy 	= atoi(row[1]);
			node->def			= atoi(row[2]);
			strncpy(node->md5, row[3], 32);
			node->time_live		= time(NULL);

			st_policy_users = g_list_append(st_policy_users, node);
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time]: Load Policy User (id_user=%d, id_policy=%d, Def=%d, md5=%s)",  node->id_user, node->id_policy, node->def, node->md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	} else {
		log->print_log(IP_ERR, "[policy_time][init_users_list]: error connect to mysql");
		return FALSE;
	}		

	return TRUE;
}

static gboolean
init_group_users_list(void)
{
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	SQL_SESSION		*sid;
	SQL_ROW			row;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		if (!store->query(sid, "select a.id_group_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupUsers a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][init_group_users_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			NODE_POLICY_GROUP_USER	*node;
			
			if ((node = g_malloc0(sizeof(NODE_POLICY_GROUP_USER))) == NULL) {
				log->print_log(IP_ERR, "[policy_time]: error allocate memory for NODE_POLICY_GROUP_USER, skiping");
				continue;
			}
			
			node->id_group_user 	= atoi(row[0]);
			node->id_policy 		= atoi(row[1]);
			node->def				= atoi(row[2]);
			strncpy(node->md5, row[3], 32);
			node->time_live			= time(NULL);

			st_policy_group_users = g_list_append(st_policy_group_users, node);
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time]: Load Policy GroupUser (id_group_user=%d, id_policy=%d, Def=%d, md5=%s)",  node->id_group_user, node->id_policy, node->def, node->md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	} else {
		log->print_log(IP_ERR, "[policy_time][init_group_users_list]: error connect to mysql");
		return FALSE;
	}		

	return TRUE;
}

static gboolean
init_type_traffic_list(void)
{
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	SQL_SESSION		*sid;
	SQL_ROW			row;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		if (!store->query(sid, "select a.id_type_traffic, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_TypeTraffic a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][init_type_traffic_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			NODE_POLICY_TYPE_TRAFFIC	*node;
			
			if ((node = g_malloc0(sizeof(NODE_POLICY_TYPE_TRAFFIC))) == NULL) {
				log->print_log(IP_ERR, "[policy_time]: error allocate memory for NODE_POLICY_TYPE_TRAFFIC, skiping");
				continue;
			}
			
			node->id_type_traffic	= atoi(row[0]);
			node->id_policy 		= atoi(row[1]);
			node->def				= atoi(row[2]);
			strncpy(node->md5, row[3], 32);
			node->time_live			= time(NULL);

			st_policy_type_traffic = g_list_append(st_policy_type_traffic, node);
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time]: Load Policy TypeTraffic (id_type_traffic=%d, id_policy=%d, Def=%d, md5=%s)",  node->id_type_traffic, node->id_policy, node->def, node->md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	} else {
		log->print_log(IP_ERR, "[policy_time][init_type_traffic_list]: error connect to mysql");
		return FALSE;
	}		

	return TRUE;
}

static gboolean
init_group_traffic_list(void)
{
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	SQL_SESSION		*sid;
	SQL_ROW			row;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		if (!store->query(sid, "select a.id_group_traffic, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupTraffic a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][init_group_traffic_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			NODE_POLICY_GROUP_TRAFFIC	*node;
			
			if ((node = g_malloc0(sizeof(NODE_POLICY_GROUP_TRAFFIC))) == NULL) {
				log->print_log(IP_ERR, "[policy_time]: error allocate memory for NODE_POLICY_GROUP_TRAFFIC, skiping");
				continue;
			}
			
			node->id_group_traffic	= atoi(row[0]);
			node->id_policy 		= atoi(row[1]);
			node->def				= atoi(row[2]);
			strncpy(node->md5, row[3], 32);
			node->time_live			= time(NULL);

			st_policy_group_traffic = g_list_append(st_policy_group_traffic, node);
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time]: Load Policy GroupTraffic (id_group_traffic=%d, id_policy=%d, Def=%d, md5=%s)",  node->id_group_traffic, node->id_policy, node->def, node->md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	} else {
		log->print_log(IP_ERR, "[policy_time][init_group_traffic_list]: error connect to mysql");
		return FALSE;
	}		

	return TRUE;
}

static gboolean
free_policy_list(void)
{
	g_list_foreach(st_policy_time, (void*)&policy_traverse_free, NULL);
	g_list_free(st_policy_time);
	st_policy_time = NULL;
	return TRUE;
}

static gboolean
free_users_list(void)
{
	g_list_foreach(st_policy_users, (void*)&policy_traverse_free, NULL);
	g_list_free(st_policy_users);
	st_policy_users = NULL;
	return TRUE;

}

static gboolean
free_group_users_list(void)
{
	g_list_foreach(st_policy_group_users, (void*)&policy_traverse_free, NULL);
	g_list_free(st_policy_group_users);
	st_policy_group_users = NULL;
	return TRUE;

}

static gboolean
free_type_traffic_list(void)
{
	g_list_foreach(st_policy_type_traffic, (void*)&policy_traverse_free, NULL);
	g_list_free(st_policy_type_traffic);
	st_policy_type_traffic = NULL;
	return TRUE;

}

static gboolean
free_group_traffic_list(void)
{
	g_list_foreach(st_policy_group_traffic, (void*)&policy_traverse_free, NULL);
	g_list_free(st_policy_group_traffic);
	st_policy_group_traffic = NULL;
	return TRUE;

}

static gboolean
sync_users_list(void *data)
{
	SQL_SESSION		*sid;
	SQL_ROW			row;
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	
	GList			 *t_tmp, *t_tek;
	NODE_POLICY_USER *node;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		if (!store->query(sid, "select a.id_user, md5(concat(a.id_policy, b.Def)) from Policy_Time_Users a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][sync_users_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id				id_user;
 			char				md5[33];
			NODE_POLICY_USER	*node;
			
			id_user = atoi(row[0]);
			strncpy(md5, row[1], 32);
			
			POLICY_USERS_LOCK;			
   			if ((node = user_find(id_user))!=NULL) {
    			if (memcmp(node->md5, md5, 32)!=0) 	update_policy_user(id_user);
    			node->time_live = time(NULL);    
   			}else insert_policy_user(id_user);
   			POLICY_USERS_UNLOCK;
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time][sync_users_list]: SYNC (id=%d, md5=%s)",  id_user, md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
		
		POLICY_USERS_LOCK;
		t_tmp = st_policy_users;
		while ((t_tek = t_tmp) !=NULL) {
			t_tmp = g_list_next(t_tmp);
		
			node = (NODE_POLICY_USER*) t_tek->data;
			if ((time(NULL) - node->time_live) > 90) {
				delete_policy_user(node->id_user);
			}
		}
		POLICY_USERS_UNLOCK;
 	}else{
		log->print_log(IP_ERR, "[policy_time][sync_users_list]: error connect to mysql");
	}			
	return TRUE;	
}

static gboolean
sync_group_users_list(void *data)
{
	SQL_SESSION		*sid;
	SQL_ROW			row;
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	
	GList			 *t_tmp, *t_tek;
	NODE_POLICY_GROUP_USER *node;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		if (!store->query(sid, "select a.id_group_user, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupUsers a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][sync_group_users_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id				id_group_user;
 			char				md5[33];
			NODE_POLICY_GROUP_USER	*node;
			
			id_group_user = atoi(row[0]);
			strncpy(md5, row[1], 32);
			
			POLICY_GROUP_USERS_LOCK;			
   			if ((node = group_user_find(id_group_user))!=NULL) {
    			if (memcmp(node->md5, md5, 32)!=0) 	update_policy_group_user(id_group_user);
    			node->time_live = time(NULL);    
   			}else insert_policy_group_user(id_group_user);
   			POLICY_GROUP_USERS_UNLOCK;
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time][sync_group_users_list]: SYNC (id=%d, md5=%s)",  id_group_user, md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
		
		POLICY_GROUP_USERS_LOCK;
			t_tmp = st_policy_group_users;
			while ((t_tek = t_tmp) !=NULL) {
				t_tmp = g_list_next(t_tmp);
		
				node = (NODE_POLICY_GROUP_USER*) t_tek->data;
				if ((time(NULL) - node->time_live) > 90) {
					delete_policy_group_user(node->id_group_user);
				}
			}
		POLICY_GROUP_USERS_UNLOCK;
 	}else{
		log->print_log(IP_ERR, "[policy_time][sync_group_users_list]: error connect to mysql");
	}			
	return TRUE;	
}

static gboolean
sync_type_traffic_list(void *data)
{
	SQL_SESSION		*sid;
	SQL_ROW			row;
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	
	GList			 *t_tmp, *t_tek;
	NODE_POLICY_TYPE_TRAFFIC *node;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		if (!store->query(sid, "select a.id_type_traffic, md5(concat(a.id_policy, b.Def)) from Policy_Time_TypeTraffic a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][sync_type_traffic_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id						id_type_traffic;
 			char						md5[33];
			NODE_POLICY_TYPE_TRAFFIC	*node;
			
			id_type_traffic = atoi(row[0]);
			strncpy(md5, row[1], 32);
			
			POLICY_TYPE_TRAFFIC_LOCK;			
   			if ((node = type_traffic_find(id_type_traffic))!=NULL) {
    			if (memcmp(node->md5, md5, 32)!=0) 	update_policy_type_traffic(id_type_traffic);
    			node->time_live = time(NULL);    
   			}else insert_policy_type_traffic(id_type_traffic);
   			POLICY_TYPE_TRAFFIC_UNLOCK;
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time][sync_type_traffic_list]: SYNC (id=%d, md5=%s)",  id_type_traffic, md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
		
		POLICY_TYPE_TRAFFIC_LOCK;
		t_tmp = st_policy_type_traffic;
		while ((t_tek = t_tmp) !=NULL) {
			t_tmp = g_list_next(t_tmp);
		
			node = (NODE_POLICY_TYPE_TRAFFIC*) t_tek->data;
			if ((time(NULL) - node->time_live) > 90) {
				delete_policy_type_traffic(node->id_type_traffic);
			}
		}
		POLICY_TYPE_TRAFFIC_UNLOCK;
 	}else{
		log->print_log(IP_ERR, "[policy_time][sync_type_traffic_list]: error connect to mysql");
	}			
	return TRUE;	
}

static gboolean
sync_group_traffic_list(void *data)
{
	SQL_SESSION		*sid;
	SQL_ROW			row;
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	
	GList			 *t_tmp, *t_tek;
	NODE_POLICY_GROUP_TRAFFIC *node;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		if (!store->query(sid, "select a.id_group_traffic, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupTraffic a, Policy_Time b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_time][sync_group_traffic_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id						id_group_traffic;
 			char						md5[33];
			NODE_POLICY_GROUP_TRAFFIC	*node;
			
			id_group_traffic = atoi(row[0]);
			strncpy(md5, row[1], 32);
			
			POLICY_GROUP_TRAFFIC_LOCK;			
   			if ((node = group_traffic_find(id_group_traffic))!=NULL) {
    			if (memcmp(node->md5, md5, 32)!=0) 	update_policy_group_traffic(id_group_traffic);
    			node->time_live = time(NULL);    
   			}else insert_policy_group_traffic(id_group_traffic);
   			POLICY_GROUP_TRAFFIC_UNLOCK;
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time][sync_group_traffic_list]: SYNC (id=%d, md5=%s)",  id_group_traffic, md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
		
		POLICY_GROUP_TRAFFIC_LOCK;
		t_tmp = st_policy_group_traffic;
		while ((t_tek = t_tmp) !=NULL) {
			t_tmp = g_list_next(t_tmp);
		
			node = (NODE_POLICY_GROUP_TRAFFIC*) t_tek->data;
			if ((time(NULL) - node->time_live) > 90) {
				delete_policy_group_traffic(node->id_group_traffic);
			}
		}
		POLICY_GROUP_TRAFFIC_UNLOCK;
 	}else{
		log->print_log(IP_ERR, "[policy_time][sync_group_traffic_list]: error connect to mysql");
	}			
	return TRUE;	
}

static gboolean
sync_time_list(void *data)
{
	SQL_SESSION		*sid;
	SQL_ROW			row;
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	
	GList			 *t_tmp, *t_tek;
	NODE_POLICY_TIME *node;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		if (!store->query(sid, "select id, md5(concat(id_policy, Week_Day, Time_Begin, Time_End, Target)) from Policy_Time_Def")) {
			log->print_log(IP_ERR, "[policy_time][sync_users_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id				id;
 			char				md5[33];
			NODE_POLICY_TIME	*node;
			
			id = atoi(row[0]);
			strncpy(md5, row[1], 32);
			
			POLICY_TIME_LOCK;			
   			if ((node = time_find(id))!=NULL) {
    			if (memcmp(node->md5, md5, 32)!=0) 	update_policy_time(id);
    			node->time_live = time(NULL);    
   			}else insert_policy_time(id);
   			POLICY_TIME_UNLOCK;
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time][sync_time_list]: SYNC (id=%d, md5=%s)",  id, md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
		
		POLICY_TIME_LOCK;
		t_tmp = st_policy_time;
		while ((t_tek = t_tmp) !=NULL) {
			t_tmp = g_list_next(t_tmp);
		
			node = (NODE_POLICY_TIME*) t_tek->data;
			if ((time(NULL) - node->time_live) > 90) {
				delete_policy_time(node->id);
			}
		}
		POLICY_TIME_UNLOCK;
 	}else{
		log->print_log(IP_ERR, "[policy_time][sync_time_list]: error connect to mysql");
	}			
	return TRUE;	
}

//******************************************************************************
// Private functions
//******************************************************************************
static void			
policy_traverse_free (gpointer data, gpointer user_data)
{
	if (data) free(data);
}
static gint 
policy_compare_users(gconstpointer a, gconstpointer b)
{
	NODE_POLICY_USER *node1 = (NODE_POLICY_USER*) a;
	NODE_POLICY_USER *node2 = (NODE_POLICY_USER*) b;
	
	return memcmp(&node1->id_user, &node2->id_user, sizeof(IP_id));
}

static gint 
policy_compare_group_users(gconstpointer a, gconstpointer b)
{
	NODE_POLICY_GROUP_USER *node1 = (NODE_POLICY_GROUP_USER*) a;
	NODE_POLICY_GROUP_USER *node2 = (NODE_POLICY_GROUP_USER*) b;
	
	return memcmp(&node1->id_group_user, &node2->id_group_user, sizeof(IP_id));
}

static gint 
policy_compare_type_traffic(gconstpointer a, gconstpointer b)
{
	NODE_POLICY_TYPE_TRAFFIC *node1 = (NODE_POLICY_TYPE_TRAFFIC*) a;
	NODE_POLICY_TYPE_TRAFFIC *node2 = (NODE_POLICY_TYPE_TRAFFIC*) b;
	
	return memcmp(&node1->id_type_traffic, &node2->id_type_traffic, sizeof(IP_id));
}

static gint 
policy_compare_group_traffic(gconstpointer a, gconstpointer b)
{
	NODE_POLICY_GROUP_TRAFFIC *node1 = (NODE_POLICY_GROUP_TRAFFIC*) a;
	NODE_POLICY_GROUP_TRAFFIC *node2 = (NODE_POLICY_GROUP_TRAFFIC*) b;
	
	return memcmp(&node1->id_group_traffic, &node2->id_group_traffic, sizeof(IP_id));
}

static gint 
policy_compare_time(gconstpointer a, gconstpointer b)
{
	NODE_POLICY_TIME *node1 = (NODE_POLICY_TIME*) a;
	NODE_POLICY_TIME *node2 = (NODE_POLICY_TIME*) b;
	
	return memcmp(&node1->id, &node2->id, sizeof(IP_id));
}

static time_t
get_begin_period(void)
{
	struct tm 	*t;
	time_t		t1;
	
	t1 = time(NULL);
	t = localtime(&t1);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	
	st_week_day = t->tm_wday;
	
	return mktime(t);
}

static NODE_POLICY_USER
*user_find(IP_id id_user)
{
	GList 				*ptr = NULL;
	NODE_POLICY_USER	node, *tmp;
	
	node.id_user = id_user;
	
	ptr = g_list_find_custom(st_policy_users, &node, (void*)&policy_compare_users);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_POLICY_USER*) ptr->data;
	
	return tmp;
}

static NODE_POLICY_GROUP_USER
*group_user_find(IP_id id_group_user)
{
	GList 					*ptr = NULL;
	NODE_POLICY_GROUP_USER	node, *tmp;
	
	node.id_group_user = id_group_user;
	
	ptr = g_list_find_custom(st_policy_group_users, &node, (void*)&policy_compare_group_users);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_POLICY_GROUP_USER*) ptr->data;
	
	return tmp;
}

static NODE_POLICY_TYPE_TRAFFIC
*type_traffic_find(IP_id id_type_traffic)
{
	GList 						*ptr = NULL;
	NODE_POLICY_TYPE_TRAFFIC	node, *tmp;
	
	node.id_type_traffic = id_type_traffic;
	
	ptr = g_list_find_custom(st_policy_type_traffic, &node, (void*)&policy_compare_type_traffic);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_POLICY_TYPE_TRAFFIC*) ptr->data;
	
	return tmp;
}

static NODE_POLICY_GROUP_TRAFFIC
*group_traffic_find(IP_id id_group_traffic)
{
	GList 						*ptr = NULL;
	NODE_POLICY_GROUP_TRAFFIC	node, *tmp;
	
	node.id_group_traffic = id_group_traffic;
	
	ptr = g_list_find_custom(st_policy_group_traffic, &node, (void*)&policy_compare_group_traffic);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_POLICY_GROUP_TRAFFIC*) ptr->data;
	
	return tmp;
}

static NODE_POLICY_TIME
*time_find(IP_id id)
{
	GList 				*ptr = NULL;
	NODE_POLICY_TIME	node, *tmp;
	
	node.id = id;
	
	ptr = g_list_find_custom(st_policy_time, &node, (void*)&policy_compare_time);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_POLICY_TIME*) ptr->data;
	
	return tmp;
}

static gboolean
insert_policy_user(IP_id id_user)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_USER	*node;
	char				buffer[2024];
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_Users a, Policy_Time b where b.id = a.id_policy and a.id_user = %d", id_user);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][insert_policy_user]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		if ((node = g_malloc0(sizeof(NODE_POLICY_USER))) == NULL) {
			log->print_log(IP_ERR, "[policy_time][insert_policy_user]: error allocate memory for NODE_POLICY_USER, skiping");
			store->close(sid); 
			return FALSE;
		}
			
		node->id_user 		= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		node->def			= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live		= time(NULL);

		st_policy_users = g_list_append(st_policy_users, node);
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][insert_policy_user]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
insert_policy_group_user(IP_id id_group_user)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_GROUP_USER	*node;
	char				buffer[2024];
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_group_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupUsers a, Policy_Time b where b.id = a.id_policy and a.id_group_user = %d", id_group_user);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][insert_policy_group_user]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		if ((node = g_malloc0(sizeof(NODE_POLICY_GROUP_USER))) == NULL) {
			log->print_log(IP_ERR, "[policy_time][insert_policy_group_user]: error allocate memory for NODE_POLICY_GROUP_USER, skiping");
			store->close(sid); 
			return FALSE;
		}
			
		node->id_group_user 	= atoi(row[0]);
		node->id_policy 		= atoi(row[1]);
		node->def				= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live			= time(NULL);

		st_policy_group_users = g_list_append(st_policy_group_users, node);
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][insert_policy_group_user]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
insert_policy_type_traffic(IP_id id_type_traffic)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_TYPE_TRAFFIC	*node;
	char				buffer[2024];
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_type_traffic, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_TypeTraffic a, Policy_Time b where b.id = a.id_policy and a.id_type_traffic = %d", id_type_traffic);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][insert_policy_type_traffic]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		if ((node = g_malloc0(sizeof(NODE_POLICY_TYPE_TRAFFIC))) == NULL) {
			log->print_log(IP_ERR, "[policy_time][insert_policy_type_traffic]: error allocate memory for NODE_POLICY_TYPE_TRAFFIC, skiping");
			store->close(sid); 
			return FALSE;
		}
			
		node->id_type_traffic	= atoi(row[0]);
		node->id_policy 		= atoi(row[1]);
		node->def				= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live			= time(NULL);

		st_policy_type_traffic = g_list_append(st_policy_type_traffic, node);
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][insert_policy_type_traffic]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
insert_policy_group_traffic(IP_id id_group_traffic)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_GROUP_TRAFFIC	*node;
	char				buffer[2024];
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_group_traffic, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupTraffic a, Policy_Time b where b.id = a.id_policy and a.id_group_traffic = %d", id_group_traffic);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][insert_policy_group_traffic]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		if ((node = g_malloc0(sizeof(NODE_POLICY_GROUP_TRAFFIC))) == NULL) {
			log->print_log(IP_ERR, "[policy_time][insert_policy_group_traffic]: error allocate memory for NODE_POLICY_GROUP_TRAFFIC, skiping");
			store->close(sid); 
			return FALSE;
		}
			
		node->id_group_traffic	= atoi(row[0]);
		node->id_policy 		= atoi(row[1]);
		node->def				= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live			= time(NULL);

		st_policy_group_traffic = g_list_append(st_policy_group_traffic, node);
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][insert_policy_group_traffic]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
insert_policy_time(IP_id id)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_TIME	*node;
	char				buffer[2024];
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select id, id_policy, Week_Day, Time_Begin, Time_End, Target, md5(concat(id_policy, Week_Day, Time_Begin, Time_End, Target)) from Policy_Time_Def where id = %d", id);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][insert_policy_time]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		if ((node = g_malloc0(sizeof(NODE_POLICY_TIME))) == NULL) {
			log->print_log(IP_ERR, "[policy_time][insert_policy_time]: error allocate memory for NODE_POLICY_TIME, skiping");
			store->close(sid); 
			return FALSE;
		}
			
		node->id 			= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		node->week_day		= atoi(row[2]);
		node->time_begin	= atol(row[3]);
		node->time_end		= atol(row[4]);
		node->target		= atoi(row[5]);
		strncpy(node->md5, row[6], 32);
		node->time_live		= time(NULL);
		
		st_policy_time = g_list_append(st_policy_time, node);
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][insert_policy_time]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
update_policy_user(IP_id id_user)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_USER	*node, tmp_node;
	char				buffer[2024];
	GList				*ptr = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;

	tmp_node.id_user = id_user;
	
	ptr = g_list_find_custom(st_policy_users, &tmp_node, (void*)&policy_compare_users);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
		
	node = (NODE_POLICY_USER*) ptr->data;			

		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_Users a, Policy_Time b where b.id = a.id_policy and a.id_user = %d", id_user);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][update_policy_user]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		node->id_user 		= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		node->def			= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live		= time(NULL);
		
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][update_policy_user]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
update_policy_group_user(IP_id id_group_user)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_GROUP_USER	*node, tmp_node;
	char				buffer[2024];
	GList				*ptr = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;

	tmp_node.id_group_user = id_group_user;
	
	ptr = g_list_find_custom(st_policy_group_users, &tmp_node, (void*)&policy_compare_group_users);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
		
	node = (NODE_POLICY_GROUP_USER*) ptr->data;			

		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_group_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupUsers a, Policy_Time b where b.id = a.id_policy and a.id_group_user = %d", id_group_user);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][update_policy_group_user]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		node->id_group_user 	= atoi(row[0]);
		node->id_policy 		= atoi(row[1]);
		node->def				= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live			= time(NULL);
		
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][update_policy_group_user]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
update_policy_type_traffic(IP_id id_type_traffic)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_TYPE_TRAFFIC	*node, tmp_node;
	char				buffer[2024];
	GList				*ptr = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;

	tmp_node.id_type_traffic = id_type_traffic;
	
	ptr = g_list_find_custom(st_policy_type_traffic, &tmp_node, (void*)&policy_compare_type_traffic);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
		
	node = (NODE_POLICY_TYPE_TRAFFIC*) ptr->data;			

		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_type_traffic, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_TypeTraffic a, Policy_Time b where b.id = a.id_policy and a.id_type_traffic = %d", id_type_traffic);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][update_policy_type_traffic]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		node->id_type_traffic 		= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		node->def			= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live		= time(NULL);
		
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][update_policy_type_traffic]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
update_policy_group_traffic(IP_id id_group_traffic)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_GROUP_TRAFFIC	*node, tmp_node;
	char				buffer[2024];
	GList				*ptr = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;

	tmp_node.id_group_traffic = id_group_traffic;
	
	ptr = g_list_find_custom(st_policy_group_traffic, &tmp_node, (void*)&policy_compare_group_traffic);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
		
	node = (NODE_POLICY_GROUP_TRAFFIC*) ptr->data;			

		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select a.id_group_traffic, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_Time_GroupTraffic a, Policy_Time b where b.id = a.id_policy and a.id_group_traffic = %d", id_group_traffic);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][update_policy_group_traffic]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		node->id_group_traffic 		= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		node->def			= atoi(row[2]);
		strncpy(node->md5, row[3], 32);
		node->time_live		= time(NULL);
		
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][update_policy_group_traffic]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
update_policy_time(IP_id id)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_TIME	*node, tmp_node;
	char				buffer[2024];
	GList				*ptr = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;

	tmp_node.id = id;
	
	ptr = g_list_find_custom(st_policy_time, &tmp_node, (void*)&policy_compare_time);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
		
	node = (NODE_POLICY_TIME*) ptr->data;			

		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select id, id_policy, Week_Day, Time_Begin, Time_End, Target, md5(concat(id_policy, Week_Day, Time_Begin, Time_End, Target)) from Policy_Time_Def where id = %d", id);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_time.c][update_policy_time]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		node->id 			= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		node->week_day		= atoi(row[2]);
		node->time_begin	= atol(row[3]);
		node->time_end		= atol(row[4]);
		node->target		= atoi(row[5]);
		strncpy(node->md5, row[6], 32);
		node->time_live		= time(NULL);

		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[policy_time.c][update_policy_time]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
delete_policy_user(IP_id id_user)
{
	NODE_POLICY_USER	tmp_node;
	GList				*ptr = NULL;
	
	tmp_node.id_user = id_user;
	
	ptr = g_list_find_custom(st_policy_users, &tmp_node, (void*)&policy_compare_users);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
	
	st_policy_users = g_list_remove(st_policy_users, ptr->data);
	
	return TRUE;
}

static gboolean
delete_policy_group_user(IP_id id_group_user)
{
	NODE_POLICY_GROUP_USER	tmp_node;
	GList				*ptr = NULL;
	
	tmp_node.id_group_user = id_group_user;
	
	ptr = g_list_find_custom(st_policy_group_users, &tmp_node, (void*)&policy_compare_group_users);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
	
	st_policy_group_users = g_list_remove(st_policy_group_users, ptr->data);

	return TRUE;
}

static gboolean
delete_policy_type_traffic(IP_id id_type_traffic)
{
	NODE_POLICY_TYPE_TRAFFIC	tmp_node;
	GList				*ptr = NULL;
	
	tmp_node.id_type_traffic = id_type_traffic;
	
	ptr = g_list_find_custom(st_policy_type_traffic, &tmp_node, (void*)&policy_compare_type_traffic);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
	
	st_policy_type_traffic = g_list_remove(st_policy_type_traffic, ptr->data);

	return TRUE;
}

static gboolean
delete_policy_group_traffic(IP_id id_group_traffic)
{
	NODE_POLICY_GROUP_TRAFFIC	tmp_node;
	GList				*ptr = NULL;
	
	tmp_node.id_group_traffic = id_group_traffic;
	
	ptr = g_list_find_custom(st_policy_group_traffic, &tmp_node, (void*)&policy_compare_group_traffic);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
	
	st_policy_group_traffic = g_list_remove(st_policy_group_traffic, ptr->data);

	return TRUE;
}

static gboolean
delete_policy_time(IP_id id)
{
	NODE_POLICY_TIME	tmp_node;
	GList				*ptr = NULL;
	
	tmp_node.id = id;
	
	ptr = g_list_find_custom(st_policy_time, &tmp_node, (void*)&policy_compare_time);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
	
	st_policy_time = g_list_remove(st_policy_time, ptr->data);

	return TRUE;
}

static gboolean		
policy_time_job (int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - policy time users- Show policy for users *\n\r");
		send_to_socket(fd, "*  - policy time group users - Show policy for group users *\n\r");
		send_to_socket(fd, "*  - policy time type traffic - Show policy for type traffic *\n\r");
		send_to_socket(fd, "*  - policy time group traffic - Show policy for group traffic *\n\r");

		send_to_socket(fd, "*  - policy time list - Show policy list *\n\r");
		return FALSE;
    }else if ((argc == 3) && (strncmp(argv[0], "policy", 6) == 0) && (strncmp(argv[1], "time", 4) == 0) && (strncmp(argv[2], "users", 5) == 0)) {
		policy_time_users_print(fd);
		return TRUE;
    }else if ((argc == 4) && (strncmp(argv[0], "policy", 6) == 0) && (strncmp(argv[1], "time", 4) == 0) && (strncmp(argv[2], "group", 5) == 0) && (strncmp(argv[3], "users", 5) == 0)) {
		policy_time_group_users_print(fd);
		return TRUE;
    }else if ((argc == 4) && (strncmp(argv[0], "policy", 6) == 0) && (strncmp(argv[1], "time", 4) == 0) && (strncmp(argv[2], "type", 4) == 0) && (strncmp(argv[3], "traffic", 7) == 0)) {
		policy_time_type_traffic_print(fd);
		return TRUE;
    }else if ((argc == 4) && (strncmp(argv[0], "policy", 6) == 0) && (strncmp(argv[1], "time", 4) == 0) && (strncmp(argv[2], "group", 5) == 0) && (strncmp(argv[3], "traffic", 7) == 0)) {
		policy_time_group_traffic_print(fd);
		return TRUE;
	}else if ((argc == 3) && (strncmp(argv[0], "policy", 6) == 0) && (strncmp(argv[1], "time", 4) == 0) && (strncmp(argv[2], "list", 4) == 0)) {
		policy_time_print(fd);
		return TRUE;
	}
	return FALSE;
}

static gboolean              
policy_time_users_print(int fd) 
{
	send_to_socket(fd, "Policy for Users:\n\r");
	
	send_to_socket(fd, "--------------------------------\n\r");
    send_to_socket(fd, "| %7s | %10s | %5s |\n\r", "id_user", "id_policy", "Def");
    send_to_socket(fd, "--------------------------------\n\r");
	POLICY_USERS_LOCK;
		g_list_foreach(st_policy_users, (void*)&policy_traverse_users_print, (gpointer) &fd);
	POLICY_USERS_UNLOCK;
    send_to_socket(fd, "--------------------------------\n\r");
 	return TRUE;
}

static gboolean              
policy_time_group_users_print(int fd) 
{
	send_to_socket(fd, "Policy for Group Users:\n\r");
	
	send_to_socket(fd, "--------------------------------\n\r");
    send_to_socket(fd, "| %7s | %10s | %5s |\n\r", "id_group", "id_policy", "Def");
    send_to_socket(fd, "--------------------------------\n\r");
	POLICY_GROUP_USERS_LOCK;
		g_list_foreach(st_policy_group_users, (void*)&policy_traverse_group_users_print, (gpointer) &fd);
	POLICY_GROUP_USERS_UNLOCK;
    send_to_socket(fd, "--------------------------------\n\r");
 	return TRUE;
}

static gboolean              
policy_time_type_traffic_print(int fd) 
{
	send_to_socket(fd, "Policy for Type Traffic:\n\r");
	
	send_to_socket(fd, "--------------------------------\n\r");
    send_to_socket(fd, "| %7s | %10s | %5s |\n\r", "id_type", "id_policy", "Def");
    send_to_socket(fd, "--------------------------------\n\r");
	POLICY_TYPE_TRAFFIC_LOCK;
		g_list_foreach(st_policy_type_traffic, (void*)&policy_traverse_type_traffic_print, (gpointer) &fd);
	POLICY_TYPE_TRAFFIC_UNLOCK;
    send_to_socket(fd, "--------------------------------\n\r");
 	return TRUE;
}

static gboolean              
policy_time_group_traffic_print(int fd) 
{
	send_to_socket(fd, "Policy for Group Traffic:\n\r");
	
	send_to_socket(fd, "--------------------------------\n\r");
    send_to_socket(fd, "| %7s | %10s | %5s |\n\r", "id_group", "id_policy", "Def");
    send_to_socket(fd, "--------------------------------\n\r");
	POLICY_GROUP_TRAFFIC_LOCK;
		g_list_foreach(st_policy_group_traffic, (void*)&policy_traverse_group_traffic_print, (gpointer) &fd);
	POLICY_GROUP_TRAFFIC_UNLOCK;
    send_to_socket(fd, "--------------------------------\n\r");
 	return TRUE;
}

static gboolean              
policy_time_print(int fd) 
{
	send_to_socket(fd, "Policy Time:\n\r");
	
	send_to_socket(fd, "------------------------------------------------------\n\r");
    send_to_socket(fd, "| %5s | %10s | %4s | %6s | %6s | %4s |\n\r", "id", "id_policy", "WD", "TB", "TE", "T");
    send_to_socket(fd, "------------------------------------------------------\n\r");
	POLICY_TIME_LOCK;
		g_list_foreach(st_policy_time, (void*)&policy_id_traverse_print, (gpointer) &fd);
	POLICY_TIME_UNLOCK;
    send_to_socket(fd, "------------------------------------------------------\n\r");
 	return TRUE;
}

static void
policy_traverse_users_print (gpointer data, gpointer user_data)
{
	int					*fd = (int*) user_data;
	NODE_POLICY_USER	*node = (NODE_POLICY_USER*) data;

	send_to_socket(*fd, "| %7d | %10d | %5d |\n\r", node->id_user, node->id_policy, node->def);
}

static void
policy_traverse_group_users_print (gpointer data, gpointer user_data)
{
	int					*fd = (int*) user_data;
	NODE_POLICY_GROUP_USER	*node = (NODE_POLICY_GROUP_USER*) data;

	send_to_socket(*fd, "| %7d | %10d | %5d |\n\r", node->id_group_user, node->id_policy, node->def);
}
static void
policy_traverse_type_traffic_print (gpointer data, gpointer user_data)
{
	int					*fd = (int*) user_data;
	NODE_POLICY_TYPE_TRAFFIC	*node = (NODE_POLICY_TYPE_TRAFFIC*) data;

	send_to_socket(*fd, "| %7d | %10d | %5d |\n\r", node->id_type_traffic, node->id_policy, node->def);
}

static void
policy_traverse_group_traffic_print (gpointer data, gpointer user_data)
{
	int					*fd = (int*) user_data;
	NODE_POLICY_GROUP_TRAFFIC	*node = (NODE_POLICY_GROUP_TRAFFIC*) data;

	send_to_socket(*fd, "| %7d | %10d | %5d |\n\r", node->id_group_traffic, node->id_policy, node->def);
}
static void
policy_id_traverse_print (gpointer data, gpointer user_data)
{
	int					*fd = (int*) user_data;
	NODE_POLICY_TIME	*node = (NODE_POLICY_TIME*) data;

	send_to_socket(*fd, "| %5d | %10d | %4d | %6d | %6d | %4d |\n\r", node->id, node->id_policy, node->week_day, node->time_begin, node->time_end, node->target);
}

static gboolean
every_day(void *data)
{
	st_begin_period = get_begin_period();
	return TRUE;
}

static gboolean		
check_policy_time_tables(STORE_API *store, LOG_API *log)
{
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	res = TRUE;
	
	if ((sid = store->open())!=NULL) {
		if ((buffer = (char*) g_malloc0(2048))==NULL) {
			store->close(sid);
			return FALSE;
		}
		if (store->list_tables(sid, "Policy_Time")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_Time (id int(11) NOT NULL auto_increment, Name varchar(50) NOT NULL default '', Def enum('0','1') NOT NULL default '0', PRIMARY KEY  (id)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_Time'");
			}
		}
		if (store->list_tables(sid, "Policy_Time_Users")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_Time_Users (id_user int(11) NOT NULL default '0', id_policy int(11)  NOT NULL default '0', PRIMARY KEY  (id_user)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_Time_Users'");
			}
		}

		if (store->list_tables(sid, "Policy_Time_GroupUsers")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_Time_GroupUsers (id_group_user int(11) NOT NULL default '0', id_policy int(11)  NOT NULL default '0', PRIMARY KEY  (id_group_user)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_Time_GroupUsers'");
			}
		}

		if (store->list_tables(sid, "Policy_Time_TypeTraffic")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_Time_TypeTraffic (id_type_traffic int(11) NOT NULL default '0', id_policy int(11)  NOT NULL default '0', PRIMARY KEY  (id_type_traffic)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_Time_TypeTraffic'");
			}
		}

		if (store->list_tables(sid, "Policy_Time_GroupTraffic")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_Time_GroupTraffic (id_group_traffic int(11) NOT NULL default '0', id_policy int(11)  NOT NULL default '0', PRIMARY KEY  (id_group_traffic)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_Time_GroupTraffic'");
			}
		}

		if (store->list_tables(sid, "Policy_Time_Def")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_Time_Def (id int(11) NOT NULL auto_increment, id_policy int(11) NOT NULL default '0', Week_Day tinyint(3) unsigned NOT NULL default '0', Time_Begin int(10) unsigned NOT NULL default '0', Time_End int(10) unsigned NOT NULL default '0', Target enum('0','1') NOT NULL default '0', PRIMARY KEY  (id), KEY id_policy (id_policy)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_Time_Def'");
			}
		}
		
		g_free(buffer);
		store->close(sid);
	}
	return TRUE;
}
