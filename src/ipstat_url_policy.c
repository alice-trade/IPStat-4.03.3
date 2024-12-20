/*
 * ipstat_system_url_police.c	Function for works with system.
 *
 * Version:	$Id: ipstat_url_policy.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $
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

static const char rcsid[] = "$Id: ipstat_url_policy.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $";

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
#include <regex.h>

static GList	*st_policy_pattern = NULL;
static GList	*st_policy_users = NULL;
static PLUGIN_API	*api = NULL;
static int		debug = 0;
typedef struct NODE_POLICY_PATTERN
{
	IP_id	id;
	IP_id	id_policy;
	char	pattern[100];
	regex_t	reg;
	guint8	target;
	char	md5[33];
	time_t	time_live;
} NODE_POLICY_PATTERN;

typedef struct NODE_POLICY_USER
{
	IP_id	id_user;
	IP_id	id_policy;
	guint8	def;
	char	md5[33];
	time_t	time_live;
} NODE_POLICY_USER;

gboolean			autz_url_policy(IP_id id_user, char *url);
static gboolean		sync_users_list(void *data);
static gboolean		sync_pattern_list(void *data);

static gboolean		check_policy_url_tables(STORE_API *store, LOG_API *log);

static gboolean		init_users_list(void);
static gboolean		init_pattern_list(void);
static gboolean		free_users_list(void);
static gboolean		free_pattern_list(void);

static gboolean		check_user(IP_id id_user, IP_id *id_policy, guint8 *def);
static gboolean		check_policy(IP_id id_policy, char *url, guint8 def);

static void			policy_traverse_free (gpointer data, gpointer user_data);
static gint			policy_compare_users(gconstpointer a, gconstpointer b);
static gint 		policy_compare_pattern(gconstpointer a, gconstpointer b);

static gboolean		insert_policy_user(IP_id id_user);
static gboolean		insert_policy_pattern(IP_id id);
static gboolean		update_policy_user(IP_id id_user);
static gboolean		update_policy_pattern(IP_id id);
static gboolean		delete_policy_user(IP_id id_user);
static gboolean		delete_policy_pattern(IP_id id);

static NODE_POLICY_USER*	user_find(IP_id id_user);
static NODE_POLICY_PATTERN*	pattern_find(IP_id id);


static pthread_mutex_t 	policy_users_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t 	policy_pattern_mutex = PTHREAD_MUTEX_INITIALIZER;

#define POLICY_USERS_LOCK 	pthread_mutex_lock(&policy_users_mutex)
#define POLICY_USERS_UNLOCK pthread_mutex_unlock(&policy_users_mutex)
#define POLICY_PATTERN_LOCK 	pthread_mutex_lock(&policy_pattern_mutex)
#define POLICY_PATTERN_UNLOCK 	pthread_mutex_unlock(&policy_pattern_mutex)

static int	sync_url_policy_timeout = 1;

static CONF_PARSER module_config[] = {
        { "sync_url_policy_timeout", CF_TYPE_INT, 0, &sync_url_policy_timeout, "1" },
        { NULL, -1, 0, NULL, NULL }
};

gboolean
load_url_policy(void *data)
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
		
    fprintf(stderr, "Start IPStat_URL_Policy module...\n");    
    log->print_log(IP_INFO, "Starting IPStat_URL_Policy");
	
	check_policy_url_tables(store, log);
	
	init_users_list();
    init_pattern_list();
	
	sprintf(str, "*/%d * * * *", sync_url_policy_timeout);
	
	if (sched) sched->registred("sync_url_policy_users", str, (void*)&sync_users_list, NULL);
	if (sched) sched->registred("sync_url_policy_pattern", str, (void*)&sync_pattern_list, NULL);
		
    return TRUE;
}

gboolean
unload_url_policy(void *data)
{
    CORE_API	*core = NULL;
    LOG_API	*log   = NULL;
	SCHEDULER_API	*sched = NULL;
	SERVER_API		*server= NULL;
	
    core = (CORE_API*) api->core;
    log	 = (LOG_API*)  api->log;
    sched	= (SCHEDULER_API*) api->sched;
	server	= (SERVER_API*) api->server;
	
	if (sched) sched->unregistred("sync_url_policy_pattern");
	if (sched) sched->unregistred("sync_url_policy_users");
		
    free_pattern_list();
	free_users_list();
    log->print_log(IP_INFO, "Stoping Module IPStat_URL_Policy");

    return TRUE;
}

gboolean
autz_url_policy(IP_id id_user, char *url)
{
    gboolean	res;
	IP_id		id_policy;
	guint8		def;
    
	res = check_user(id_user, &id_policy, &def);
	if (res) {											// Если пользователь найден, если нет, то разрешаем
		res = check_policy(id_policy, url, def);				// Проверяем политику
		if (!res) return FALSE;							// Если запрещён, то отбрасываем.
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
check_policy(IP_id id_policy, char *url, guint8 def)
{
	GList				*t_tek, *t_tmp;
	NODE_POLICY_PATTERN	*node;
	guint8				res;
	LOG_API				*log;
	
	
	log		= (LOG_API*)  api->log;
//**********************************
// Выход: 1 - разрешён, 0 - запрещён
//**********************************
	res = def;
	POLICY_PATTERN_LOCK;
	t_tmp = st_policy_pattern;
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		node = (NODE_POLICY_PATTERN*) t_tek->data;
		
		if ((node->id_policy == id_policy) && (regexec(&node->reg, url, 0, NULL, 0)==0)) {
			log->print_log(IP_DEBUG, "pattern='%s' url='%s' - OK", node->pattern, url);
			res = node->target;
			break;
		}
	}
	POLICY_PATTERN_UNLOCK;
	if (res == 0) return FALSE;
	return TRUE;	
}

//******************************************************************************
// Init Policy List
//******************************************************************************
static gboolean
init_pattern_list(void)
{
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	SQL_SESSION		*sid;
	SQL_ROW			row;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		if (!store->query(sid, "select id, id_policy, Pattern, Target, md5(concat(id_policy, Pattern, Target)) from Policy_URL_Def order by id")) {
			log->print_log(IP_ERR, "[policy_url][init_pattern_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			NODE_POLICY_PATTERN	*node;
			
			if ((node = g_malloc0(sizeof(NODE_POLICY_PATTERN))) == NULL) {
				log->print_log(IP_ERR, "[policy_time]: error allocate memory for NODE_POLICY_PATTERN, skiping");
				continue;
			}
			
			node->id 			= atoi(row[0]);
			node->id_policy 	= atoi(row[1]);
			strncpy(node->pattern, row[2], 99);
			node->target		= atoi(row[3]);
			strncpy(node->md5, row[4], 32);
			node->time_live		= time(NULL);
			if(regcomp(&node->reg, node->pattern, REG_EXTENDED+REG_ICASE+REG_NOSUB) != 0) {
				free(node);
				continue;
			}

			st_policy_pattern = g_list_append(st_policy_pattern, node);
			
			if (debug) log->print_log(IP_DEBUG, "[policy_time]: Load Time Pattern (id=%d, id_policy=%d, md5=%s)",  node->id, node->id_policy, node->md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	} else {
		log->print_log(IP_ERR, "[policy_url][init_pattern_list]: error connect to mysql");
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
  		
		if (!store->query(sid, "select a.id_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_URL_Users a, Policy_URL b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_url][init_users_list]: error execute query");
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
			
			if (debug) log->print_log(IP_DEBUG, "[policy_url]: Load Policy User (id_user=%d, id_policy=%d, Def=%d, md5=%s)",  node->id_user, node->id_policy, node->def, node->md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	} else {
		log->print_log(IP_ERR, "[policy_url][init_users_list]: error connect to mysql");
		return FALSE;
	}		

	return TRUE;
}

static gboolean
free_pattern_list(void)
{
	g_list_foreach(st_policy_pattern, (void*)&policy_traverse_free, NULL);
	g_list_free(st_policy_pattern);
	st_policy_pattern = NULL;
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
		if (!store->query(sid, "select a.id_user, md5(concat(a.id_policy, b.Def)) from Policy_URL_Users a, Policy_URL b where b.id = a.id_policy")) {
			log->print_log(IP_ERR, "[policy_url][sync_users_list]: error execute query");
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
			
			if (debug) log->print_log(IP_DEBUG, "[policy_url][sync_users_list]: SYNC (id=%d, md5=%s)",  id_user, md5);
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
		log->print_log(IP_ERR, "[policy_url][sync_users_list]: error connect to mysql");
	}			
	return TRUE;	
}

static gboolean
sync_pattern_list(void *data)
{
	SQL_SESSION		*sid;
	SQL_ROW			row;
	STORE_API		*store = NULL;
	LOG_API			*log = NULL;
	
	GList			 *t_tmp, *t_tek;
	NODE_POLICY_PATTERN *node;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;	
	
	
	if ((sid = store->open())!=NULL) {
		if (!store->query(sid, "select id, md5(concat(id_policy, Pattern, Target)) from Policy_URL_Def")) {
			log->print_log(IP_ERR, "[policy_url][sync_pattern_list]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id				id;
 			char				md5[33];
			NODE_POLICY_PATTERN	*node;
			
			id = atoi(row[0]);
			strncpy(md5, row[1], 32);
			
			POLICY_PATTERN_LOCK;			
   			if ((node = pattern_find(id))!=NULL) {
    			if (memcmp(node->md5, md5, 32)!=0) 	update_policy_pattern(id);
    			node->time_live = time(NULL);    
   			}else insert_policy_pattern(id);
   			POLICY_PATTERN_UNLOCK;
			
			if (debug) log->print_log(IP_DEBUG, "[policy_url][sync_pattern_list]: SYNC (id=%d, md5=%s)",  id, md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
		
		POLICY_PATTERN_LOCK;
		t_tmp = st_policy_pattern;
		while ((t_tek = t_tmp) !=NULL) {
			t_tmp = g_list_next(t_tmp);
		
			node = (NODE_POLICY_PATTERN*) t_tek->data;
			if ((time(NULL) - node->time_live) > 90) {
				delete_policy_pattern(node->id);
			}
		}
		POLICY_PATTERN_UNLOCK;
 	}else{
		log->print_log(IP_ERR, "[policy_url][sync_pattrn_list]: error connect to mysql");
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
policy_compare_pattern(gconstpointer a, gconstpointer b)
{
	NODE_POLICY_PATTERN *node1 = (NODE_POLICY_PATTERN*) a;
	NODE_POLICY_PATTERN *node2 = (NODE_POLICY_PATTERN*) b;
	
	return memcmp(&node1->id, &node2->id, sizeof(IP_id));
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

static NODE_POLICY_PATTERN
*pattern_find(IP_id id)
{
	GList 				*ptr = NULL;
	NODE_POLICY_PATTERN	node, *tmp;
	
	node.id = id;
	
	ptr = g_list_find_custom(st_policy_pattern, &node, (void*)&policy_compare_pattern);
	
	if (ptr == NULL) return NULL;
	
	if (ptr->data == NULL) return NULL;
		
	tmp = (NODE_POLICY_PATTERN*) ptr->data;
	
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
  		
		sprintf(buffer, "select a.id_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_URL_Users a, Policy_URL b where b.id = a.id_policy and a.id_user = %d", id_user);
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
insert_policy_pattern(IP_id id)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_PATTERN	*node;
	char				buffer[2024];
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;
		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select id, id_policy, Pattern, Target, md5(concat(id_policy, Pattern, Target)) from Policy_URL_Def where id = %d", id);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_url][insert_policy_pattern]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		if ((node = g_malloc0(sizeof(NODE_POLICY_PATTERN))) == NULL) {
			log->print_log(IP_ERR, "[policy_url][insert_policy_pattern]: error allocate memory for NODE_POLICY_PATTERN, skiping");
			store->close(sid);
			return FALSE;
		}
			
		node->id 			= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		strncpy(node->pattern, row[2], 99);
		node->target		= atoi(row[3]);
		strncpy(node->md5, row[4], 32);
		node->time_live		= time(NULL);
		if(regcomp(&node->reg, node->pattern, REG_EXTENDED+REG_ICASE+REG_NOSUB) != 0) {
			free(node);
			store->close(sid);
			log->print_log(IP_ERR, "[policy_url][insert_policy_pattern]: error compile reg, skiping");
			return FALSE;
		}		
		st_policy_pattern = g_list_append(st_policy_pattern, node);
		store->close(sid);
	} else {
		log->print_log(IP_ERR, "[policy_url][insert_policy_pattern]: error connect to mysql server");	
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
  		
		sprintf(buffer, "select a.id_user, a.id_policy, b.Def, md5(concat(a.id_policy, b.Def)) from Policy_URL_Users a, Policy_URL b where b.id = a.id_policy and a.id_user = %d", id_user);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_url][update_policy_user]: error execute query");
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
		log->print_log(IP_ERR, "[policy_url][update_policy_user]: error connect to mysql server");	
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
update_policy_pattern(IP_id id)
{
	STORE_API			*store = NULL;
	LOG_API				*log = NULL;
	SQL_SESSION			*sid;
	SQL_ROW				row;
	NODE_POLICY_PATTERN	*node, tmp_node;
	char				buffer[2024];
	GList				*ptr = NULL;
	
	if ((store = api->store) == NULL) return FALSE;
	if ((log = api->log) == NULL) return FALSE;

	tmp_node.id = id;
	
	ptr = g_list_find_custom(st_policy_pattern, &tmp_node, (void*)&policy_compare_pattern);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
		
	node = (NODE_POLICY_PATTERN*) ptr->data;			

		
 	if ((sid = store->open())!=NULL) {
  		
		sprintf(buffer, "select id, id_policy, Pattern, Target, md5(concat(id_policy, Pattern, Target)) from Policy_URL_Def where id = %d", id);
		if (!store->query(sid, buffer)) {
			log->print_log(IP_ERR, "[policy_url][update_policy_pattern]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		row = store->fetch_row(sid);
		
		node->id 			= atoi(row[0]);
		node->id_policy 	= atoi(row[1]);
		strncpy(node->pattern, row[2], 99);
		node->target		= atoi(row[3]);
		strncpy(node->md5, row[4], 32);
		node->time_live		= time(NULL);
		if(regcomp(&node->reg, node->pattern, REG_EXTENDED+REG_ICASE+REG_NOSUB) != 0) {
			store->close(sid);
			log->print_log(IP_ERR, "[policy_url][update_policy_pattern]: error compile reg, skiping");
			return FALSE;
		}	
		store->close(sid);
	} else {
		log->print_log(IP_ERR, "[policy_url][update_policy_pattern]: error connect to mysql server");	
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
delete_policy_pattern(IP_id id)
{
	NODE_POLICY_PATTERN	tmp_node;
	GList				*ptr = NULL;
	
	tmp_node.id = id;
	
	ptr = g_list_find_custom(st_policy_pattern, &tmp_node, (void*)&policy_compare_pattern);
	
	if (ptr == NULL) return FALSE;
	
	if (ptr->data == NULL) return FALSE;
	
	st_policy_pattern = g_list_remove(st_policy_pattern, ptr->data);

	return TRUE;
}

static gboolean		
check_policy_url_tables(STORE_API *store, LOG_API *log)
{
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	res = TRUE;
	
	if ((sid = store->open())!=NULL) {
		if ((buffer = (char*) g_malloc0(2048))==NULL) {
			store->close(sid);
			return FALSE;
		}
		if (store->list_tables(sid, "Policy_URL")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_URL (id int(11) NOT NULL auto_increment, Name varchar(50) NOT NULL default '', Def enum('0','1') NOT NULL default '0', PRIMARY KEY  (id)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_URL'");
			}
		}
		if (store->list_tables(sid, "Policy_URL_Users")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_URL_Users (id_user int(11) NOT NULL default '0', id_policy int(11)  NOT NULL default '0', PRIMARY KEY  (id_user)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_URL_Users'");
			}
		}

		if (store->list_tables(sid, "Policy_URL_Def")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Policy_URL_Def (id int(11) NOT NULL auto_increment, id_policy int(11) NOT NULL default '0', Pattern char(100) NOT NULL, Target enum('0','1') NOT NULL default '0', PRIMARY KEY  (id), KEY id_policy (id_policy)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Policy_URL_Def'");
			}
		}
		
		g_free(buffer);
		store->close(sid);
	}
	return TRUE;
}
