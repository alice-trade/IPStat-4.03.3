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
static GList	*st_group_limit = NULL;

static char	*id 		= "401nti";
static char	*name 		= "system_group_limit";
static char	*version 	= "0.1";
static char	*description	= "System module for IPStat Group Limit";
static char	*author		= "Kukuev Dmitry";
static char	*homepage	= "http://ipstat.code-art.ru/";


typedef struct NODE_GROUP_LIMIT
{
	IP_id		id;
	IP_id		id_user;
	IP_id		id_group;
	IP_id		type;
	IP_bigsize	limitation;
	IP_bigsize	traffic;
	IP_bigsize	alpha;
	IP_flag		onblocked;
	
	IP_flag		status;
	char		md5[33];
} NODE_GROUP_LIMIT;

static gboolean		my_autz(IP_id service, void *data, void *ret);
static gboolean		my_acct(IP_id service, void *data, void *ret);

static gboolean		sync_limits(void *data);
static gboolean		sync_traffic(void *data);
static gboolean		mark_delete();

static gboolean		server_job (int fd, int argc, char **argv, void *data);

static gboolean		check_group_limit_tables(STORE_API *store, LOG_API *log);

static gboolean		init_limits(void);

static gboolean		free_limits(void);

static void			traverse_free(gpointer data, gpointer user_data);
static void			traverse_print(gpointer data, gpointer user_data);
static void			traverse_mark(gpointer data, gpointer user_data);
static void			traverse_check (gpointer data, gpointer user_data);

static gint			compare_limit(gconstpointer a, gconstpointer b);
static gint			compare_group_traffic(gconstpointer a, gconstpointer b);

static gboolean		limits_print(int fd);

static gboolean		insert_limit(IP_id id);
static gboolean		update_limit(IP_id id);
static gboolean		delete_limit(IP_id id);
static gboolean		limits_delete(void);

static gboolean		gl_task_every_day(void *data);
static gboolean		gl_task_every_week(void *data);
static gboolean		gl_task_every_month(void *data);

static NODE_GROUP_LIMIT*			find_limit(IP_id id);
static NODE_GROUP_LIMIT*			find_group_traffic(IP_id id_user, IP_id id_group);

static pthread_mutex_t 	limit_mutex = PTHREAD_MUTEX_INITIALIZER;

#define LIMIT_LOCK 		pthread_mutex_lock(&limit_mutex)
#define LIMIT_UNLOCK 	pthread_mutex_unlock(&limit_mutex)

static int	sync_group_limit_timeout = 1;
static int	secondary_daemon = 0;


static CONF_PARSER module_config[] = {
        { "sync_group_limit_timeout", CF_TYPE_INT, 0, &sync_group_limit_timeout, "1" },
        { "secondary_daemon", CF_TYPE_INT, 0, &secondary_daemon, "0" },
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
    module_info.api_version = 1;
    module_info.type		= IPSTAT_PLUGIN_SERVICE;
    module_info.flags		= 0;
    module_info.priority	= IPSTAT_PRIORITY_DEFAULT;
    module_info.id 			= id;
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
    log->print_log(IP_INFO, "Starting System Module for IPStat Group Limits");
	
	check_group_limit_tables(store, log);
	
	init_limits();
	
    core->register_autz_hook((void*)&my_autz);
    core->register_acct_hook((void*)&my_acct);
    
	sprintf(str, "*/%d * * * *", sync_group_limit_timeout);

	if (server) server->registred("group_limits", (void*)&server_job, NULL);
	
	if (secondary_daemon == 0) {  
		if (sched) sched->registred("sync_group_limit", str, (void*)&sync_limits, NULL);
   		if (sched) sched->registred("gl_every_day", "0 0 * * *", (void*)&gl_task_every_day, api);
   		if (sched) sched->registred("gl_every_week", "0 0 * * 1", (void*)&gl_task_every_week, api);
   		if (sched) sched->registred("gl_every_month", "0 0 1 * *", (void*)&gl_task_every_month, api);
	}	
    return TRUE;
}

gboolean
unload(void *data)
{
    CORE_API		*core = NULL;
    LOG_API			*log   = NULL;
	SCHEDULER_API	*sched = NULL;
	SERVER_API		*server= NULL;
	
    core 	= (CORE_API*) api->core;
    log	 	= (LOG_API*)  api->log;
    sched	= (SCHEDULER_API*) api->sched;
	server	= (SERVER_API*) api->server;
	
	if (secondary_daemon == 0) {  
		if (sched) sched->unregistred("sync_group_limit");
   		if (sched) sched->unregistred("gl_every_day");
   		if (sched) sched->unregistred("gl_every_week");
   		if (sched) sched->unregistred("gl_every_month");
	}	


	if (server) server->unregistred("group_limit");

    core->unregister_acct_hook((void*)&my_acct);
    core->unregister_autz_hook((void*)&my_autz);

    free_limits();
    log->print_log(IP_INFO, "Stoping System Module for IPStat Group Limits");

    return TRUE;
}

static gboolean
my_autz(IP_id service, void *data, void *ret)
{
	int					onactive = 1;
	IP_flag				flag = TRUE;
	
	if (service == SERVICE_TRAFFIC) {
		PACKET_DATA_INFO	*packet_data = (PACKET_DATA_INFO*) ret;
		NODE_GROUP_LIMIT	*node;
		
		LIMIT_LOCK;
			if ((node = find_group_traffic(packet_data->id_user, packet_data->id_traffic_group)) != NULL) {
				onactive = 1;
				if (node->onblocked) onactive = 0; else
					if (node->limitation != -1)
				if (node->traffic >= node->limitation) onactive = 0;
					
				if (onactive)	flag = TRUE;
				else			flag = FALSE;
			}
		LIMIT_UNLOCK;
		return flag;
	}else if (service == SERVICE_SQUID) {
		SQUID_DATA_INFO	*squid_data = (SQUID_DATA_INFO*) ret;
		NODE_GROUP_LIMIT	*node;
		
		LIMIT_LOCK;
			if ((node = find_group_traffic(squid_data->id_user, squid_data->id_traffic_group)) != NULL) {
				onactive = 1;
				if (node->onblocked) onactive = 0; else
					if (node->limitation != -1)
				if (node->traffic >= node->limitation) onactive = 0;
					
				if (onactive)	flag = TRUE;
				else			flag = FALSE;
			}
		LIMIT_UNLOCK;
		return flag;
	}
    return FALSE;
}

static gboolean
my_acct(IP_id service, void *data, void *ret)
{
	if (service == SERVICE_TRAFFIC) {
		PACKET_DATA_INFO	*packet_data = (PACKET_DATA_INFO*) ret;
		NODE_GROUP_LIMIT	*node;
		
		if (packet_data->not_sum == 0) {
			LIMIT_LOCK;
				if ((node = find_group_traffic(packet_data->id_user, packet_data->id_traffic_group)) != NULL) {
					node->traffic+=packet_data->len;
					node->alpha+=packet_data->len;
				}
			LIMIT_UNLOCK;
		}
	}else if (service == SERVICE_SQUID) {
		SQUID_DATA_INFO	*squid_data = (SQUID_DATA_INFO*) ret;
		NODE_GROUP_LIMIT	*node;
		
		if (squid_data->not_sum == 0) {
			LIMIT_LOCK;
				if ((node = find_group_traffic(squid_data->id_user, squid_data->id_traffic_group)) != NULL) {
					node->traffic+=squid_data->len;
					node->alpha+=squid_data->len;
				}
			LIMIT_UNLOCK;
		}
	}else return FALSE;
    return TRUE;
}

//******************************************************************************
// Init Group Limits
//******************************************************************************
static gboolean
init_limits(void)
{
	sync_limits(api);
	
	return TRUE;
}

static gboolean
free_limits(void)
{
	g_list_foreach(st_group_limit, (void*)&traverse_free, NULL);
	g_list_free(st_group_limit);
	st_group_limit = NULL;
	return TRUE;
}

static gboolean
sync_limits(void *data)
{
	SQL_SESSION			*sid;
	SQL_ROW				row;
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		if (!store->query(sid, "select id, md5(concat(id_user, id_group, Type, Limitation, onBlocked)) from Rules_GroupTraffic")) {
			log->print_log(IP_ERR, "[group_limit][sync_limits]: error execute query");
			store->close(sid);
			return FALSE;
		}
		
		mark_delete();
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id				id;
 			char				md5[33];
			NODE_GROUP_LIMIT	*node;
			
			id = atoi(row[0]);
			strncpy(md5, row[1], 32);
			
			LIMIT_LOCK;
   				if ((node = find_limit(id))!=NULL) {
					node->status = FALSE;
    				if (memcmp(node->md5, md5, 32)!=0) 	update_limit(id);
   				}else insert_limit(id);
   			LIMIT_UNLOCK;
			
			if (debug) log->print_log(IP_DEBUG, "[group_limit][sync_limits]: SYNC (id=%d, md5=%s)",  id, md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
		
		limits_delete();
		
		sync_traffic(data);
 	}else{
		log->print_log(IP_ERR, "[policy_time][sync_users_list]: error connect to mysql");
	}			
	return TRUE;	
}

static gboolean
sync_traffic(void *data)
{
	SQL_SESSION			*sid;
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	GList				*t_tek, *t_tmp;
	NODE_GROUP_LIMIT	node, *ptr;
	char				buffer[2024];
	int					onactive;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		t_tmp = st_group_limit;
		while ((t_tek = t_tmp) !=NULL) {
			LIMIT_LOCK;
				t_tmp = g_list_next(t_tmp);
				ptr = (NODE_GROUP_LIMIT*) t_tek->data;
				memcpy(&node, ptr, sizeof(NODE_GROUP_LIMIT));
				ptr->alpha = 0;
			LIMIT_UNLOCK;
			onactive = 1;
			if (node.onblocked) onactive = 0; else
				if (node.limitation != -1)
			if (node.traffic >= node.limitation) onactive = 0;
				
			sprintf(buffer, "update Rules_GroupTraffic SET Traffic=%llu, TDay=TDay+%u, TMonth=TMonth+%u, onActive='%d' where id=%d", node.traffic, (int)node.alpha, (int)node.alpha, onactive, node.id);
			if (debug) log->print_log(IP_DEBUG, "query=(%s)", buffer);
			if (!store->exec(sid, buffer)) {
				LIMIT_LOCK;
					ptr->alpha+=node.alpha;
				LIMIT_UNLOCK;
				log->print_log(IP_ERR, "[group_limit][sync_traffic]: error query (%s)", buffer);
			}
		}
		store->close(sid);
	}
	return TRUE;	
}

static gboolean
mark_delete()
{
	LIMIT_LOCK;
		g_list_foreach(st_group_limit, (void*)&traverse_mark, NULL);
	LIMIT_UNLOCK;
	return TRUE;
}

static gboolean
limits_delete(void)
{
	GList				*t_tmp, *t_tek;
	NODE_GROUP_LIMIT	*node;

	t_tmp = st_group_limit;
	while ((t_tek = t_tmp) !=NULL) {
		t_tmp = g_list_next(t_tmp);

		node = (NODE_GROUP_LIMIT*) t_tek->data;
		if (node->status == TRUE) delete_limit(node->id);
	}

	return TRUE;
}

//******************************************************************************
// Private functions
//******************************************************************************
static void			
traverse_free (gpointer data, gpointer user_data)
{
	if (data) free(data);
}

static void			
traverse_check (gpointer data, gpointer user_data)
{
	int					*type = (int*) user_data;
	NODE_GROUP_LIMIT	*node = (NODE_GROUP_LIMIT*) data;
	
	if (node->type == *type) node->traffic = 0; 
}

static void
traverse_print (gpointer data, gpointer user_data)
{
	int					*fd = (int*) user_data;
	NODE_GROUP_LIMIT	*node = (NODE_GROUP_LIMIT*) data;
	int					onactive = 1;

	if (node->onblocked) onactive = 0; else
		if (node->limitation != -1)
	if (node->traffic >= node->limitation) onactive = 0;

	send_to_socket(*fd, "| %3d | %3d | %3d | %3d | %10lld | %10llu |\n\r", node->id_user, node->id_group, onactive, node->type, node->limitation, node->traffic);
}

static void
traverse_mark(gpointer data, gpointer user_data)
{
	NODE_GROUP_LIMIT	*node = (NODE_GROUP_LIMIT*) data;

	node->status = TRUE;
}

static gint 
compare_limit(gconstpointer a, gconstpointer b)
{
	NODE_GROUP_LIMIT *node1 = (NODE_GROUP_LIMIT*) a;
	NODE_GROUP_LIMIT *node2 = (NODE_GROUP_LIMIT*) b;
	
	return memcmp(&node1->id, &node2->id, sizeof(IP_id));
}

static gint 
compare_group_traffic(gconstpointer a, gconstpointer b)
{
	NODE_GROUP_LIMIT *node1 = (NODE_GROUP_LIMIT*) a;
	NODE_GROUP_LIMIT *node2 = (NODE_GROUP_LIMIT*) b;
	
	return memcmp(&node1->id_user, &node2->id_user, sizeof(IP_id)) || memcmp(&node1->id_group, &node2->id_group, sizeof(IP_id));
}

static NODE_GROUP_LIMIT
*find_limit(IP_id id)
{
	GList 				*ptr = NULL;
	NODE_GROUP_LIMIT	node, *tmp;
	
	node.id = id;
	
	ptr = g_list_find_custom(st_group_limit, &node, (void*)&compare_limit);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_GROUP_LIMIT*) ptr->data;
	
	return tmp;
}


static NODE_GROUP_LIMIT
*find_group_traffic(IP_id id_user, IP_id id_group)
{
	GList 				*ptr = NULL;
	NODE_GROUP_LIMIT	node, *tmp;
	
	node.id_user	= id_user;
	node.id_group 	= id_group;
	
	ptr = g_list_find_custom(st_group_limit, &node, (void*)&compare_group_traffic);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_GROUP_LIMIT*) ptr->data;
	
	return tmp;
}

static gboolean
insert_limit(IP_id id)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_GROUP_LIMIT	*node;
	char				buffer[2024];
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select id_group, Type, Limitation, Traffic, onBlocked, id_user, md5(concat(id_user, id_group, Type, Limitation, onBlocked)) from Rules_GroupTraffic where id = %d", id);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[group_limit][insert_limit]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		if ((node = g_malloc0(sizeof(NODE_GROUP_LIMIT))) == NULL) {
			log->print_log(IP_ERR, "[group_limit][insert_limit]: error allocate memory for NODE_GROUP_LIMIT, skiping");
			store->close(sid); 
			return FALSE;
		}
			
		node->id			= id;
		node->id_user		= atoi(row[5]);
		node->id_group 		= atoi(row[0]);
		node->type			= atoi(row[1]);
		node->limitation	= atoll(row[2]);
		node->traffic		= atoll(row[3]);
		node->onblocked		= atoi(row[4]);
		node->status		= FALSE;
		node->alpha			= 0;
		strncpy(node->md5, row[6], 32);

		st_group_limit = g_list_append(st_group_limit, node);
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[group_limit][insert_limit]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}


static gboolean
update_limit(IP_id id)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_GROUP_LIMIT	*node, tmp_node;
	char				buffer[2024];
	GList				*ptr = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;

	tmp_node.id = id;
	
	ptr = g_list_find_custom(st_group_limit, &tmp_node, (void*)&compare_limit);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
		
	node = (NODE_GROUP_LIMIT*) ptr->data;			

		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select id_group, Type, Limitation, onBlocked, id_user, md5(concat(id_user, id_group, Type, Limitation, onBlocked)) from Rules_GroupTraffic where id = %d", id);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[group_limit][update_limit]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		node->id_user		= atoi(row[4]);
		node->id_group 		= atoi(row[0]);
		node->type 			= atoi(row[1]);
		node->limitation	= atoll(row[2]);
		node->onblocked		= atoi(row[3]);
		strncpy(node->md5, row[5], 32);
		
		store->close(sid); 
	} else {
		log->print_log(IP_ERR, "[group_limit][update_limit]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}


static gboolean
delete_limit(IP_id id)
{
	NODE_GROUP_LIMIT	tmp_node;
	GList				*ptr = NULL;
	
	tmp_node.id = id;
	
	ptr = g_list_find_custom(st_group_limit, &tmp_node, (void*)&compare_limit);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
	
	st_group_limit = g_list_remove(st_group_limit, ptr->data);

	return TRUE;
}


static gboolean		
server_job (int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - show group limit- Show policy for users *\n\r");
		return FALSE;
    }else if ((argc == 3) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "group", 5) == 0) && (strncmp(argv[2], "limit", 5) == 0)) {
		limits_print(fd);
		return TRUE;
    }
	return FALSE;
}

static gboolean              
limits_print(int fd) 
{
	send_to_socket(fd, "Limit to Group Traffic:\n\r");

	send_to_socket(fd, "---------------------------------------------------\n\r");
    send_to_socket(fd, "| %3s | %3s | %3s | %3s | %10s | %10s |\n\r", "U", "G", "S", "T", "Limit", "Traffic");
    send_to_socket(fd, "---------------------------------------------------\n\r");
	LIMIT_LOCK;
		g_list_foreach(st_group_limit, (void*)&traverse_print, (gpointer) &fd);
	LIMIT_UNLOCK;
    send_to_socket(fd, "---------------------------------------------------\n\r");
 	return TRUE;
}

static gboolean		
check_group_limit_tables(STORE_API *store, LOG_API *log)
{
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	res = TRUE;
	
	if ((sid = store->open())!=NULL) {
		if ((buffer = (char*) g_malloc0(2048))==NULL) {
			store->close(sid);
			return FALSE;
		}
		if (store->list_tables(sid, "Rules_GroupTraffic")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Rules_GroupTraffic (id int(11) NOT NULL auto_increment, id_user int(11) NOT NULL default '0', id_group int(11) NOT NULL default '0', Type tinyint(3) default '0', Limitation bigint(20) default '-1', Traffic bigint(20) unsigned NOT NULL default '0', YDay bigint(20) unsigned NOT NULL default '0', TDay bigint(20) unsigned NOT NULL default '0', YMonth bigint(20) unsigned NOT NULL default '0', TMonth bigint(20) unsigned NOT NULL default '0', onActive enum('0','1') NOT NULL default '1', onBlocked enum('0','1') NOT NULL default '0', PRIMARY KEY  (id), KEY key0 (id_user, id_group), KEY `key1` (`Type`,`Limitation`,`Traffic`,`onActive`,`onBlocked`)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Rules_GroupTraffic'");
			}
		}
		
		g_free(buffer);
		store->close(sid);
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//		  						task_every_day
//----------------------------------------------------------------------------
static gboolean 
gl_task_every_day(void *data)
{
    PLUGIN_API	*plugin = (PLUGIN_API*) data;
    STORE_API	*store	= NULL;
    LOG_API		*log	= NULL;
	int			type	= 1;
    SQL_SESSION *sid;
    int			count = 4;
    
    if ((store = plugin->store) == NULL) return FALSE;
    log = plugin->log;
	
	LIMIT_LOCK;
		g_list_foreach(st_group_limit, (void*)&traverse_check, (gpointer) &type);
	LIMIT_UNLOCK;
	
    if ((sid = store->open())!=NULL) {
start_label:		
		if (!store->exec(sid, "set autocommit=0")) goto error;
		if (!store->exec(sid, "start transaction")) goto error;
  		if (!store->exec(sid, "update Rules_GroupTraffic SET YDay = TDay")) goto error;
		if (!store->exec(sid, "update Rules_GroupTraffic SET TDay = 0")) goto error;
		if (save_timestamp(sid, store, "gl_date_last_every_day")!=TRUE) goto error;
		if (!store->exec(sid, "commit")) goto error;
  		goto ok;
error:
  		store->exec(sid, "rollback");
		count--;
		sleep(10);
		if (count>0) goto start_label;
		log->print_log(IP_CRIT, "[group_limit][task_every_day]: error execute task_every_day!!!");
		return FALSE;
ok:
		log->print_log(IP_INFO, "[group_limit][task_every_day]: task_every_day - success");
		store->close(sid);
    }
    return TRUE;
}

//----------------------------------------------------------------------------
//		  						task_every_week
//----------------------------------------------------------------------------
static gboolean 
gl_task_every_week(void *data)
{
	int			type = 2;
	LIMIT_LOCK;
		g_list_foreach(st_group_limit, (void*)&traverse_check, (gpointer) &type);
	LIMIT_UNLOCK;

    return TRUE;
}

//----------------------------------------------------------------------------
//		  						task_every_month
//----------------------------------------------------------------------------
static gboolean 
gl_task_every_month(void *data)
{
    PLUGIN_API	*plugin = (PLUGIN_API*) data;
    STORE_API	*store	= NULL;
    LOG_API		*log	= NULL;
    SQL_SESSION *sid;
    int			count = 4;
	int			type = 3;
    
    if ((store = plugin->store) == NULL) return FALSE;
    log = plugin->log;

	LIMIT_LOCK;
		g_list_foreach(st_group_limit, (void*)&traverse_check, (gpointer) &type);
	LIMIT_UNLOCK;
	
    if ((sid = store->open())!=NULL){
start_label:
		if (!store->exec(sid, "set autocommit=0")) goto error;
  		if (!store->exec(sid, "start transaction")) goto error;
	  	
  		if (!store->exec(sid, "update Rules_GroupTraffic SET YMonth = TMonth")) goto error;
		if (!store->exec(sid, "update Rules_GroupTraffic SET TMonth = 0")) goto error;		
		if (save_timestamp(sid, store, "gl_date_last_every_month")!=TRUE) goto error;
		if (!store->exec(sid, "commit")) goto error;
	  	goto ok;
error:
  		store->exec(sid, "rollback");
		count--;
		sleep(10);
		if (count>0) goto start_label;
		log->print_log(IP_CRIT, "[group_limit][task_every_month]: error execute task_every_month!!!");
		return FALSE;
ok:
		log->print_log(IP_INFO, "[group_limit][task_every_month]: task_every_month - success");
		store->close(sid);
    }
    return TRUE;
}
