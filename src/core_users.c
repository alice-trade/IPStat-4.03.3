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


#include "../include/core_users.h"
#include "../include/log.h"
#include "../include/inet.h"
#include "../include/sched.h"
#include "../include/store.h"
#include "../include/server.h"
#include "../include/misc.h"
#include "../include/glist.h"

#include <stdlib.h>

static USERS			*st_users 	 = NULL;
static CONF_API			*st_conf	 = NULL;
static LOG_API			*st_log		 = NULL;

static GList			*extern_auth = NULL;
static GList			*extern_autz = NULL;
static GList			*extern_acct = NULL;

static GList			*clean_list  = NULL;
static pthread_mutex_t 	users_mutex  = PTHREAD_MUTEX_INITIALIZER;


extern  int				debug;
static	int				cur_users = 0;

static  SCHEDULER_API	*sched = NULL;
static	SERVER_API		*server = NULL;
static  STORE_API		*store = NULL;


static gint 		users_traverse(gpointer data, gpointer user_data);
static gint 		rules_traverse(gpointer data, gpointer user_data);

static NODE_USER 	*users_find(IP_id key);
static NODE_USER 	*addr_find(IP_addr addr);

static NODE_USER 	*users_insert(NODE_USER *node);
static gboolean		users_delete(IP_id key);

static gboolean		users_print(int fd, int s_id);

static gint			users_compare_users_id(gconstpointer a, gconstpointer b);
static gint			users_compare_users_addr(gconstpointer a, gconstpointer b);

static gboolean		users_mysql_insert(const IP_id s_id);
static gboolean		users_mysql_update(const IP_id s_id);
static gboolean		users_mysql_delete(void);


static gboolean		users_sync_traffic(void);

static gboolean		users_mysql_export(SQL_SESSION *sid, IP_id id_sql, IP_size traffic);


static gboolean		rules_mysql_export(SQL_SESSION *sid, IP_id id_sql, IP_size traffic, IP_flag action);

static gboolean		rules_sync(NODE_USER *node, STORE_API *store);
static gboolean		rules_sync_traffic(NODE_USER *id, STORE_API *store);

static gboolean		rules_init(NODE_USER *id);
static gboolean		rules_insert(NODE_USER *id, NODE_RULE *node);
static gboolean		rules_clean(NODE_USER *id);
static gboolean		rules_free(NODE_USER *id);

static NODE_RULE	*rules_find(NODE_USER *id, const IP_id key);

static gint			users_compare_rules_id(gconstpointer a, gconstpointer b);

static gboolean		rules_mysql_load(NODE_USER *id, STORE_API *store);
static gboolean		rules_mysql_unload(NODE_USER *id, STORE_API *store);

static gboolean		core_job (int fd, int argc, char **argv, void *data);
static IP_result 	core_scheduler(const void *data);

#define CHECK_CORRECT_USERS	if (!st_users) return FALSE

#define USERS_LOCK	pthread_mutex_lock(&users_mutex)
#define USERS_UNLOCK	pthread_mutex_unlock(&users_mutex)

static pthread_mutex_t 	auth_hook_mutex  = PTHREAD_MUTEX_INITIALIZER;

#define AUTH_HOOK_LOCK		pthread_mutex_lock(&auth_hook_mutex)
#define AUTH_HOOK_UNLOCK	pthread_mutex_unlock(&auth_hook_mutex)

static pthread_mutex_t 	autz_hook_mutex  = PTHREAD_MUTEX_INITIALIZER;

#define AUTZ_HOOK_LOCK		pthread_mutex_lock(&autz_hook_mutex)
#define AUTZ_HOOK_UNLOCK	pthread_mutex_unlock(&autz_hook_mutex)

static pthread_mutex_t 	acct_hook_mutex  = PTHREAD_MUTEX_INITIALIZER;

#define ACCT_HOOK_LOCK		pthread_mutex_lock(&acct_hook_mutex)
#define ACCT_HOOK_UNLOCK	pthread_mutex_unlock(&acct_hook_mutex)

static int	core_sync_timeout = 1;

static CONF_PARSER module_config[] = {
        { "core_sync_timeout", CF_TYPE_INT, 0, &core_sync_timeout, "1" },
        { NULL, -1, 0, NULL, NULL }
};

gboolean
users_init(CONF_API *conf, LOG_API *log)
{
	USERS	*users;
	
	st_conf = conf;
	st_log	= log;
	
	if (st_conf) st_conf->parse(module_config);
		
	if ((st_users = users = (USERS*) g_malloc0(sizeof(USERS))) == NULL) {
		st_log->print_log(IP_ALERT, "[core_users.c][users_init]: error allocate memory for users");
		return FALSE;
	}
	
	st_users->id_users = NULL;
	

	if (debug) st_log->print_log(IP_DEBUG, "Create Users struct [addr=%x]", users);
	return TRUE;
}

gboolean		
users_free(void)
{
	NODE_USER			*node;
	GList  			*t_tek, *t_tmp;

	CHECK_CORRECT_USERS;	

	if (server) server->unregistred("core");
	if (sched) sched->unregistred("core");
		
	USERS_LOCK;
		t_tmp = st_users->id_users;
		while ((t_tek = t_tmp) != NULL) {
			t_tmp = g_list_next(t_tmp);

			node = (NODE_USER*) t_tek->data;
			rules_mysql_unload(node, store);
			if (node) g_free(node);
		}
		g_list_free(st_users->id_users);
		st_users->id_users = NULL;
		
		t_tmp = clean_list;
		while ((t_tek = t_tmp) != NULL) {
			t_tmp = g_list_next(t_tmp);

			node = (NODE_USER*) t_tek->data;
			rules_mysql_unload(node, store);
			if (node) g_free(node);
		}
		g_list_free(clean_list);
		clean_list = NULL;
	USERS_UNLOCK;

	g_free(st_users); st_users = NULL;
	
 	if (debug) st_log->print_log(IP_DEBUG, "[table_users.c]: free struct st_users");
 	return TRUE;
}


gboolean
users_auth(IP_id service, void *data, gboolean flag_extern_auth, void *ret)
{
	gboolean		flag = FALSE;
	
	CHECK_CORRECT_USERS;
	
	if (extern_auth && flag_extern_auth) {
		GList		*t_tek, *t_tmp;
		
		AUTH_HOOK_LOCK;
		
			t_tmp = extern_auth;
			while ((t_tek = t_tmp) != NULL) {
				gboolean	(*func_auth)(IP_id service, void *data, void *ret);
			
				t_tmp = g_list_next(t_tmp);
				func_auth = t_tek->data;
				if ((flag = func_auth(service, data, ret)) == TRUE) break;
			}
		AUTH_HOOK_UNLOCK;
		if (flag) return TRUE;
	}
	
	if (service == SERVICE_TRAFFIC) {
		PACKET_INFO			*packet_info = (PACKET_INFO*) data;
		PACKET_DATA_INFO	*packet_data = (PACKET_DATA_INFO*) ret;
		NODE_USER			*t_src = NULL, *t_dest = NULL;
		
		t_src   = addr_find(packet_info->src);
    	t_dest  = addr_find(packet_info->dest);
	
		if ((!packet_data) && (t_src || t_dest)) return TRUE; // если нам информация не нужна, то выходим с успехом
			
		packet_data->service 			= service;
		packet_data->id_traffic_group 	= 0;
		packet_data->id_traffic_type	= 0;
		
		if (t_src && !t_dest) {  		// Исходящий трафик
			packet_data->direct  		= TRAFFIC_OUT;
			packet_data->id_user		= t_src->id;
			packet_data->id_group_user	= t_src->id_group;
			flag = TRUE;
			
		}else if (!t_src && t_dest) {	// Входящий трафик
			packet_data->direct			= TRAFFIC_IN;
			packet_data->id_user		= t_dest->id;
			packet_data->id_group_user	= t_dest->id_group;
			flag = TRUE;
		}
//		st_log->print_log(IP_DEBUG, "user_id=%d, direct=%d", packet_data->id_user, packet_data->direct);
	}else if (service == SERVICE_SQUID) {
		SQUID_INFO			*squid_info = (SQUID_INFO*) data;
		SQUID_DATA_INFO		*squid_data = (SQUID_DATA_INFO*) ret;
		NODE_USER			*t_ip = NULL;
		
		t_ip   = addr_find(squid_info->ip);
		
		if (!squid_data && t_ip) 	return TRUE; // если нам информация не нужна, то выходим с успехом
		
		squid_data->service 			= service;
		squid_data->id_traffic_group 	= 0;
		squid_data->id_traffic_type		= 0;
		
		if (t_ip) {
			squid_data->id_user			= t_ip->id;
			squid_data->id_group_user	= t_ip->id_group;
			flag = TRUE;
		}
	}
	
	return flag;
}

gboolean
users_autz(IP_id service, void *data, gboolean flag_extern_autz, void *ret)
{
	GList			*t_tek = NULL, *t_tmp = NULL;
	NODE_RULE		*node_rule = NULL;
	NODE_USER		*node_user = NULL;
	gboolean		flag = FALSE;			// По дефолту запрещаем всё если нет ничего
	
	CHECK_CORRECT_USERS;

	if (service == SERVICE_TRAFFIC) {
		PACKET_INFO			*packet_info = (PACKET_INFO*) data;
		PACKET_DATA_INFO	*packet_data = (PACKET_DATA_INFO*) ret;
		IP_addr				addr = 0;
		IP_port				port = 0;
		
		node_user = users_find(packet_data->id_user);
		if (node_user == NULL) return FALSE;
		if (node_user->status == 0) return FALSE;	
		
		if ((packet_data->direct == TRAFFIC_OUT) && (*node_user->mac)) {
			if (strncasecmp(node_user->mac, packet_info->mac, 17)!=0) {
				st_log->print_log(IP_INFO, "User [%d][%s] change mac-address !!!", node_user->id, node_user->mac);
				return FALSE;
			}
		}
		
		if (packet_data->direct == TRAFFIC_IN) {
			addr = packet_info->src;
			port = packet_info->src_port;
		}else {
			addr = packet_info->dest;
			port = packet_info->dest_port;
		}
		
		t_tmp = node_user->id_rules;
		while ((t_tek = t_tmp) != NULL) {
			t_tmp = g_list_next(t_tmp);
			node_rule = (NODE_RULE*) t_tek->data;
			
			if (node_rule->service == SERVICE_TRAFFIC) {
				if (compare_with_mask(&addr, &node_rule->net, node_rule->mask)) {
					if (node_rule->proto == PROTO_ALL || node_rule->proto == packet_info->proto) {
						if (compare_ports(port, node_rule->ports)) {
							if ((node_rule->direct == TRAFFIC_ALL)||(packet_data->direct == node_rule->direct)) {
								if (node_rule->status == 1)	flag = TRUE;
								packet_data->id_traffic_group	= node_rule->id_group;
								packet_data->id_traffic_type	= node_rule->id_type;
								if (!((node_rule->action & CONST_SUM)==CONST_SUM))	packet_data->not_sum = 0;
								else												packet_data->not_sum = 1;									
								if (!((node_rule->action & CONST_NEXT) == CONST_NEXT)) break;
							}
						}
					}
				}
			}
		}
		
	}else if (service == SERVICE_SQUID) {
		SQUID_DATA_INFO		*squid_data = (SQUID_DATA_INFO*) ret;

		node_user = users_find(squid_data->id_user);
		
		if (node_user == NULL) return FALSE;
		if (node_user->status == 0) return FALSE;
				
		t_tmp = node_user->id_rules;
		while ((t_tek = t_tmp) != NULL) {
			t_tmp = g_list_next(t_tmp);
			node_rule = (NODE_RULE*) t_tek->data;
			
			if (node_rule->service == SERVICE_SQUID) {
				if (node_rule->status == 1)	{
					flag = TRUE;
					squid_data->id_traffic_group	= node_rule->id_group;
					squid_data->id_traffic_type		= node_rule->id_type;
					if (!((node_rule->action & CONST_SUM)==CONST_SUM))	squid_data->not_sum = 0;
					else												squid_data->not_sum = 1;
					break;
				}
			}
		}

	}

	if (!flag) return FALSE;
		
	if (extern_autz && flag_extern_autz) {
		
		AUTZ_HOOK_LOCK;
		
			t_tmp = extern_autz;
			while ((t_tek = t_tmp) != NULL) {
				gboolean	(*func_autz)(IP_id service, void *data, void *ret);
			
				t_tmp = g_list_next(t_tmp);
			
				func_autz = t_tek->data;
				if ((flag = func_autz(service, data, ret)) == FALSE) break;
			}
		AUTZ_HOOK_UNLOCK;
	}
	
	return flag;
}

gboolean
users_acct(IP_id service, void *data, gboolean flag_extern_acct, void *ret)
{
	GList					*t_tek = NULL, *t_tmp = NULL;
	NODE_RULE				*node_rule = NULL;
	NODE_USER				*node_user = NULL;
	gboolean				flag = TRUE;

	CHECK_CORRECT_USERS;
	
	if (service == SERVICE_TRAFFIC) {
		PACKET_INFO			*packet_info = (PACKET_INFO*) data;
		PACKET_DATA_INFO	*packet_data = (PACKET_DATA_INFO*) ret;
		IP_addr				addr = 0;
		IP_port				port = 0;
		
		node_user = users_find(packet_data->id_user);
		if (node_user == NULL) return FALSE;
		
		if (packet_data->direct == 1) {
			addr = packet_info->src;
			port = packet_info->src_port;
		}else {
			addr = packet_info->dest;
			port = packet_info->dest_port;
		}
		
		t_tmp = node_user->id_rules;
		while ((t_tek = t_tmp) != NULL) {
			t_tmp = g_list_next(t_tmp);
			node_rule = (NODE_RULE*) t_tek->data;
			
			if (node_rule->service == SERVICE_TRAFFIC) {
				if (compare_with_mask(&addr, &node_rule->net, node_rule->mask)) {
					if (node_rule->proto == PROTO_ALL || node_rule->proto == packet_info->proto) {
						if (compare_ports(port, node_rule->ports)) {
							if ((node_rule->direct == TRAFFIC_ALL)||(packet_data->direct == node_rule->direct)) {
								USERS_LOCK;
 									node_rule->traffic+=packet_data->len;
									if (!((node_rule->action & CONST_SUM)==CONST_SUM)) node_user->traffic+=packet_data->len;
								USERS_UNLOCK;
								if (!((node_rule->action & CONST_NEXT) == CONST_NEXT)) break;
							}
						}
					}
				}
			}
		}
	}else if (service == SERVICE_SQUID) {
		SQUID_DATA_INFO		*squid_data = (SQUID_DATA_INFO*) ret;

		node_user = users_find(squid_data->id_user);
		
		if (node_user == NULL) return FALSE;
				
		t_tmp = node_user->id_rules;
		while ((t_tek = t_tmp) != NULL) {
			t_tmp = g_list_next(t_tmp);
			node_rule = (NODE_RULE*) t_tek->data;
			
			if (node_rule->service == SERVICE_SQUID) {
				USERS_LOCK;
 					node_rule->traffic+=squid_data->len;
					if (!((node_rule->action & CONST_SUM)==CONST_SUM)) node_user->traffic+=squid_data->len;
				USERS_UNLOCK;
				break;
			}
		}
	}

	if (extern_autz && flag_extern_acct) {
		
		ACCT_HOOK_LOCK;
		
			t_tmp = extern_acct;
			while ((t_tek = t_tmp) != NULL) {
				gboolean	(*func_acct)(IP_id service, void *data, void *ret);
			
				t_tmp = g_list_next(t_tmp);
			
				func_acct = t_tek->data;
				if ((flag = func_acct(service, data, ret)) == FALSE) break;
			}
		ACCT_HOOK_UNLOCK;
	}
	
	return flag;
}

gboolean		
users_sync(void)
{
	SQL_SESSION		*sid;
	SQL_ROW			row;
	
	CHECK_CORRECT_USERS;

/* Sync data Users Table */	
 	if ((sid = store->open())!=NULL) {
  		
		if (!store->query(sid, "select id, Addr, md5(concat(id_group, Addr, Mac, onActive)) from Users order by id")) {
			st_log->print_log(IP_ERR, "[core_users.c]: error execute query");
			store->close(sid);
			return FALSE;
		}
  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_id		t_id;
 			char		t_md5[33];
			IP_addr		t_ip;
			NODE_USER	*node;
			
			t_id = atoi(row[0]);
			strncpy(t_md5, row[2], 32);
			hostbyip(row[1], &t_ip);
				
   			if ((node = users_find(t_id))!=NULL) {
    			if ((memcmp(node->md5, t_md5, 32)!=0) || (rules_sync(node, store)==TRUE)) 	users_mysql_update(t_id);
    			node->time_live = time(NULL);    
   			}else users_mysql_insert(t_id);
   
			if (debug) st_log->print_log(IP_DEBUG, "[core_users.c]: SYNC (id=%d, md5=%s)",  t_id, t_md5);
  		}  	
		if (row) free(row);
  		store->close(sid); 
 	}else{
		st_log->print_log(IP_ERR, "[core_users.c][users_sync]: error connect to mysql");
	}		

/* Delete time of live node */
	users_mysql_delete();

/* Sync traffic Users Table */
	users_sync_traffic();
	
	return TRUE;
}

static gboolean
users_sync_traffic(void)
{
	SQL_SESSION 	*sid;
	           
	CHECK_CORRECT_USERS;
		
 	if ((sid = store->open())!=NULL) {
		IP_size 			t_traffic = 0;
		GList				*t_tek, *t_tmp;
		NODE_USER			*node;
		
  		t_tmp = st_users->id_users;
		while ((t_tek = t_tmp) !=NULL) {
			t_tmp = g_list_next(t_tmp);
			
			node = (NODE_USER*) t_tek->data;
			
			if (node->traffic) {
				gboolean	err;
					
				t_traffic 		= node->traffic;
				node->traffic 	-= t_traffic;
    			if ((err = users_mysql_export(sid, node->id, t_traffic))!=TRUE) node->traffic += t_traffic;
				else rules_sync_traffic(node, store);
   			}
  		}
		
		save_timestamp(sid, store, "date_last_insert_users");
  		store->close(sid);
 	}else {
		st_log->print_log(IP_ERR, "[core_users.c][users_sync_traffic]: error open connect to mysql");
		return FALSE; 
	}

  	if (debug) st_log->print_log(IP_DEBUG, "[core_users.c][%d]: users_sync_traffic - OK");
 	return TRUE;
}

/*******************************************************************************
 Private functions
*******************************************************************************/
static gint
users_traverse(gpointer data, gpointer user_data)
{
	int            *fd = (int*) user_data;
	NODE_USER      *node = (NODE_USER*) data;
	char           addr[16];
                                                                                                                                                             
	ipbyhost(node->addr, addr);
	send_to_socket(*fd, "| %4d | %18s | %18s | %10u | %1d |\n\r", node->id, addr, node->mac, node->traffic, (int)node->status);
	return FALSE;
}

static gint
rules_traverse(gpointer data, gpointer user_data)
{
	int            *fd = (int*) user_data;
	NODE_RULE      *node = (NODE_RULE*) data;
	char           addr[16];

	ipbyhost(node->net, addr);
	send_to_socket(*fd, "| %4d | %4d | %1d | %18s | %5d | %10d | %5.2f |\n\r", node->id, node->status, node->direct, addr, node->mask, (int) node->traffic, node->price);
	
	return FALSE;
}

gboolean	
core_set_store(void *data)
{
	store = (STORE_API*) data;
	
	return TRUE;
}

gboolean	
core_set_server(void *data)
{
	server = (SERVER_API*) data;

	if (server) server->registred("core", (void*)&core_job, NULL);

	return TRUE;
}

gboolean	
core_set_sched(void *data)
{
	char	str[20];
	sched = (SCHEDULER_API*) data;
	
	sprintf(str, "*/%d * * * *", core_sync_timeout);
	if (sched) sched->registred("core", str, (void*)&core_scheduler, NULL);
	return TRUE;
}

gboolean		
register_auth(gboolean (*func_auth)(IP_id service, void *data, void *ret))
{
	AUTH_HOOK_LOCK;
		extern_auth = g_list_append(extern_auth, func_auth);
	AUTH_HOOK_UNLOCK;
	if (debug) st_log->print_log(IP_DEBUG, "CORE: append extern auth hook (%x)", (int) func_auth);
	return TRUE;
}

gboolean		
register_autz(gboolean (*func_autz)(IP_id service, void *data, void *ret))
{
	AUTZ_HOOK_LOCK;
		extern_autz = g_list_append(extern_autz, func_autz);
	AUTZ_HOOK_UNLOCK;
	if (debug) st_log->print_log(IP_DEBUG, "CORE: append extern autz hook (%x)", (int) func_autz);
	return TRUE;
}

gboolean
register_acct(gboolean (*func_acct)(IP_id service, void *data, void *ret))
{
	ACCT_HOOK_LOCK;
		extern_acct = g_list_append(extern_acct, func_acct);
	ACCT_HOOK_UNLOCK;
	if (debug) st_log->print_log(IP_DEBUG, "CORE: append extern acct hook (%x)", (int) func_acct);
	
	return TRUE;
}

gboolean		
unregister_auth(gboolean (*func_auth)(IP_id service, void *data, void *ret))
{
	AUTH_HOOK_LOCK;
		extern_auth = g_list_remove(extern_auth, func_auth);
	AUTH_HOOK_UNLOCK;
	if (debug) st_log->print_log(IP_DEBUG, "CORE: remove extern auth hook (%x)", (int) func_auth);
	return TRUE;
}

gboolean		
unregister_autz(gboolean (*func_autz)(IP_id service, void *data, void *ret))
{
	AUTZ_HOOK_LOCK;
		extern_autz = g_list_remove(extern_autz, func_autz);
	AUTZ_HOOK_UNLOCK;
	if (debug) st_log->print_log(IP_DEBUG, "CORE: remove extern autz hook (%x)", (int) func_autz);
	return TRUE;
}

gboolean
unregister_acct(gboolean (*func_acct)(IP_id service, void *data, void *ret))
{
	ACCT_HOOK_LOCK;
		extern_acct = g_list_remove(extern_acct, func_acct);
	ACCT_HOOK_UNLOCK;
	if (debug) st_log->print_log(IP_DEBUG, "CORE: remove extern acct hook (%x)", (int) func_acct);
	return TRUE;
}

static gboolean	
core_job(int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - show users    - Print users tables       *\n\r");
		send_to_socket(fd, "*  - show user N   - Print rules for N user   *\n\r");
		send_to_socket(fd, "*  - show core     - Show CORE information    *\n\r");
		send_to_socket(fd, "*                                             *\n\r");
		return FALSE;
    }else if ((argc == 2) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "users", 5) == 0)) {
		users_print(fd, 0);
		return TRUE;
	}else if ((argc == 3) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "user", 4) == 0)) {
		users_print(fd, atoi(argv[2]));
		return TRUE;
	}

	return FALSE;
}

static IP_result 	
core_scheduler(const void *data)
{
	users_sync();
	return TRUE;
}


/*
 								USERS TABLE
*/

//-----------------------------------------------------------------------------
//                                users_find
//-----------------------------------------------------------------------------
static NODE_USER		
*users_find(IP_id key) 
{
	NODE_USER			node;
	GList				*ptr;
 
	CHECK_CORRECT_USERS;
	
	node.id = key;
	USERS_LOCK;
		ptr = g_list_find_custom(st_users->id_users, &node, users_compare_users_id);
 	USERS_UNLOCK;
 	if (ptr)  return (NODE_USER*) ptr->data;
 	else	   return NULL;
}

//-----------------------------------------------------------------------------
//                                addr_find
//-----------------------------------------------------------------------------
static NODE_USER 
*addr_find(IP_addr addr)
{
	NODE_USER			node;
	GList				*ptr;

	CHECK_CORRECT_USERS;
	
	if (addr == 0) return NULL;

	node.addr = addr;
	USERS_LOCK;
 		ptr = g_list_find_custom(st_users->id_users, &node, users_compare_users_addr);
	USERS_UNLOCK;
	
 	if (ptr) return (NODE_USER*) ptr->data;
 	else	 return NULL;
}

//-----------------------------------------------------------------------------
//                                users_insert
//-----------------------------------------------------------------------------
static NODE_USER		
*users_insert(NODE_USER *node) 
{
	NODE_USER	*t_node;
	NODE_USER	*res;
	GList		*ptr = NULL;
	
	CHECK_CORRECT_USERS;
	
	if ((t_node = (NODE_USER*) g_malloc0(sizeof(NODE_USER)))==NULL) {
		st_log->print_log(IP_ALERT, "[core_users.c][users_insert]: error allocate memory for NODE_USER");
		return NULL;
	}
	memcpy(t_node, node, sizeof(NODE_USER));
	
	USERS_LOCK;
	    st_users->id_users = g_list_append (st_users->id_users, t_node);
	    cur_users++;
	    if (debug) st_log->print_log(IP_DEBUG, "Users %d insert", node->id);
	
	    ptr = g_list_find(st_users->id_users, t_node);
	USERS_UNLOCK;
	
	if (ptr) {
		res = (NODE_USER*) ptr->data;
	}else res = NULL;
	
 	return  res;
}

//-----------------------------------------------------------------------------
//                                users_delete
//-----------------------------------------------------------------------------
static gboolean		
users_delete(IP_id key) 
{
	NODE_USER	t_node, *node;
	GList		*ptr;
	IP_result	res = FALSE;

	CHECK_CORRECT_USERS;
	
	t_node.id = key;
	
	USERS_LOCK;
		ptr = g_list_find_custom(st_users->id_users, &t_node, users_compare_users_id);	// ищем заданную ноду
		if (ptr) {			
			node = (NODE_USER*) ptr->data;
			node->time_live = time(NULL);											// Присваиваем текущее время, окончательное удаление будет после 60 секунд
			clean_list = g_list_append(clean_list, ptr->data);						// переносим в массив на удаление

			// если ноду нашли, то
			st_users->id_users = g_list_remove(st_users->id_users, ptr->data);			// удаляем из таблицы Users

			if (debug) st_log->print_log(IP_DEBUG, "Users %d delete", key);			// вывод дебаг сообщения
			cur_users--;															// уменьшаем количество текущих пользователей
			res = TRUE;
		}
 	USERS_UNLOCK;
	return res;
}

//-----------------------------------------------------------------------------
//									users_print
//-----------------------------------------------------------------------------
static gboolean              
users_print(int fd, int s_id) 
{
	CHECK_CORRECT_USERS;
	
	if (s_id == 0) {
		send_to_socket(fd, "Users statistic`s:\n\r");

		send_to_socket(fd, "-------------------------------------------------------------------\n\r");
		send_to_socket(fd, "| %4s | %18s | %18s | %10s | %1s |\n\r", "id", "Addr", "Mac","S_Traffic", "S");
		send_to_socket(fd, "-------------------------------------------------------------------\n\r");
			g_list_foreach(st_users->id_users, (void*)&users_traverse, &fd);
		send_to_socket(fd, "-------------------------------------------------------------------\n\r");
 	}else{
		NODE_USER     		*node_user;
		
		node_user = users_find(s_id);
	
		if (node_user==NULL) return FALSE;
		
		send_to_socket(fd, "---------------------------------------------------------------------\n\r");
		send_to_socket(fd, "| %4s | %4s | %1s | %18s | %5s | %10s | %5s |\n\r", "id_r", "S", "D", "Net", "Mask", "Traffic", "Price");
		send_to_socket(fd, "---------------------------------------------------------------------\n\r");
		g_list_foreach(node_user->id_rules, (void*)&rules_traverse, &fd);
		send_to_socket(fd, "---------------------------------------------------------------------\n\r");
	}

 	return TRUE;
}


//-----------------------------------------------------------------------------
//							users_compare_users_id
//-----------------------------------------------------------------------------
static gint 
users_compare_users_id(gconstpointer a, gconstpointer b) 
{
	register NODE_USER *node1 = (NODE_USER*) a;
	register NODE_USER *node2 = (NODE_USER*) b;
                                                                                
 	return (gint) memcmp(&node1->id, &node2->id, sizeof(IP_id));
}

//-----------------------------------------------------------------------------
//							users_compare_users_addr
//-----------------------------------------------------------------------------
static gint 
users_compare_users_addr(gconstpointer a, gconstpointer b) 
{
	register NODE_USER *node1 = (NODE_USER*) a;
	register NODE_USER *node2 = (NODE_USER*) b;
                                                                                
 	return (gint) memcmp(&node1->addr, &node2->addr, sizeof(IP_addr));
}

/*
								STORE API
*/
//-----------------------------------------------------------------------------
//                                users_mysql_insert
//-----------------------------------------------------------------------------
static gboolean		
users_mysql_insert(const IP_id s_id) 
{
	SQL_ROW			row;
 	SQL_SESSION 	*sid;
   
	CHECK_CORRECT_USERS;
	
	if (!store) {
		st_log->print_log(IP_CRIT, "not found store api");
		return FALSE;
	}

 	if ((sid = store->open())!=NULL) {
	 	char	buffer[1024];

  		sprintf(buffer, "select Addr, Mac, onActive, id_group, md5(concat(id_group, Addr, Mac, onActive)) from Users where id = %u", s_id);
  		if (!store->query(sid, buffer)) {
			st_log->print_log(IP_ERR, "[table_users.c][users_store_insert]: error execute query");
			store->close(sid);
			return FALSE;
		}

  		while ((row = store->fetch_row(sid))!=NULL) {
			IP_addr		t_ip;
			NODE_USER	node, *ptr;
			
   			hostbyip(row[0], &t_ip);
			
			if (t_ip == 0) continue;
				
			bzero(&node, sizeof(NODE_USER));
			node.id 		= s_id;
   			node.addr 		= t_ip;
   		    node.traffic	= 0;
			node.id_group	= atoi(row[3]);
   			node.status		= atoi(row[2]);
			node.id_rules	= NULL;
			
   			strncpy(node.md5, row[4], 32);
			strncpy(node.mac, row[1], 18);				

			node.time_live 	= time(NULL);							// время жизни?
				
   			if ((ptr = users_insert(&node))!=NULL) {
				rules_mysql_load(ptr, store);	
			}else st_log->print_log(IP_WARNING, "[table_users.c]: Users %d not correct", s_id);
  		}  
		if (row) free(row);
  		store->close(sid); 
 	} 
 	if (debug) st_log->print_log(IP_DEBUG, "[table_users.c]: insert node id = %d", s_id);
 	return TRUE;
}

//-----------------------------------------------------------------------------
//                                users_mysql_update
//-----------------------------------------------------------------------------
static gboolean
users_mysql_update(const IP_id s_id) 
{
	NODE_USER		*node;
 	SQL_SESSION		*sid;
 	SQL_ROW			row;

	CHECK_CORRECT_USERS;
	
	if (!store) {
		st_log->print_log(IP_CRIT, "not found store api");
		return FALSE;
	}
	
 	if ((node = users_find(s_id))!=NULL) {
  		if ((sid = store->open())!=NULL) {
			char 	buffer[1024];
			
			rules_mysql_unload(node, store);
			
   			sprintf(buffer, "select Addr, Mac, onActive, id_group, md5(concat(id_group, Addr, Mac, onActive)) from Users where id = %u", s_id);
   			if (!store->query(sid, buffer)) {
				st_log->print_log(IP_ERR, "[table_users.c][users_store_update]: error execute query");
				store->close(sid);
				return FALSE;
			}
 
   			while ((row = store->fetch_row(sid))!=NULL) {
 				IP_addr		t_ip;
	   			
				hostbyip(row[0], &t_ip);
				
				if (t_ip == 0) continue;
				node->id 		= s_id;
   				node->addr 		= t_ip;
   			    node->status	= atoi(row[2]);
				node->id_group	= atoi(row[3]);
   				strncpy(node->md5, row[4], 32);
				strncpy(node->mac, row[1], 18);
   			}
			rules_mysql_load(node, store);	// загружаем его правила
			
			if (row) free(row);
   			store->close(sid);
  		}
 	}else st_log->print_log(IP_ERR, "[table_user.c][users_mysql_update]: error find user %d", s_id);
 	if (debug) st_log->print_log(IP_DEBUG, "[table_users.c]: update node id = %d", s_id);
	return TRUE;
}

//-----------------------------------------------------------------------------
//                                users_mysql_delete
//-----------------------------------------------------------------------------
static gboolean		
users_mysql_delete(void) 
{
	GList	*t_tmp, *t_tek;
	NODE_USER	*node;

	CHECK_CORRECT_USERS;
	
	t_tmp = st_users->id_users;
	while ((t_tek = t_tmp) !=NULL) {
		t_tmp = g_list_next(t_tmp);
		
		node = (NODE_USER*) t_tek->data;
		if ((time(NULL) - node->time_live) > 90) {
			users_delete(node->id);
		}
	}

 	return TRUE;
}


//-----------------------------------------------------------------------------
//                                users_mysql_export
//-----------------------------------------------------------------------------
static gboolean
users_mysql_export(SQL_SESSION *sid, IP_id id_sql, IP_size traffic)
{
	char            buffer[1024];

    sprintf(buffer, "update Users SET Traffic = Traffic + %u, TDay = TDay + %u, TMonth = TMonth + %u where id = %u", traffic, traffic, traffic, id_sql);
    if (!store->exec(sid, buffer)) return FALSE;
    if (debug) st_log->print_log(IP_DEBUG, "[system.c]: row %d insert to dbase", id_sql);

    return TRUE;
}

/*
								RULES FUNCTION
*/

//-----------------------------------------------------------------------------
//                                rules_sync
//-----------------------------------------------------------------------------
static gboolean
rules_sync(NODE_USER *node, STORE_API *store) 
{
 	SQL_ROW					row;
 	SQL_SESSION 			*sid;

	IP_result				flag_update = FALSE;
 
	if ((sid = store->open())!=NULL) {
		char buffer[1024];
		
		sprintf(buffer, "select Rules.id, md5(concat(Rules.id, id_type, Type_Traffic.id_group, Direct, Proto, Net, Mask, Ports, Action, onActive, Price)) from Rules RIGHT JOIN Type_Traffic ON Rules.id_type = Type_Traffic.id where id_user = %u", node->id);
  		if (!store->query(sid, buffer)) {
			st_log->print_log(IP_ERR, "[core_users.c][rules_sync]: error execute query");
			store->close(sid);
			return FALSE;
		}
		
		while ((row = store->fetch_row(sid))!=NULL) {
 			int			t_id;
 			char		t_md5[33];
			NODE_RULE	*node_rule;
			
   			t_id = atoi(row[0]);
			strncpy(t_md5, row[1], 32);
				
			if ((node_rule = rules_find(node, t_id))!=NULL) {
    			if (memcmp(node_rule->md5, t_md5, 32)!=0) flag_update = TRUE;
				else node_rule->time_live = time(NULL);	
   			}else flag_update = TRUE;
   		}
		
		if (flag_update==FALSE){
			GList				*t_tek, *t_tmp;
			NODE_RULE			*tn;
			
			t_tmp = node->id_rules;
			while ((t_tek = t_tmp) != NULL) {
				t_tmp = g_list_next(t_tmp);
				
				tn = (NODE_RULE*) t_tek->data;
				if ((time(NULL) - tn->time_live)>90) flag_update = TRUE;
			}
		}
  		store->close(sid); 
	}

	if (debug) st_log->print_log(IP_DEBUG, "[core_users.c][rules_sync]: sync rules flag=%d", flag_update);

 	return flag_update;
}

//-----------------------------------------------------------------------------
//                                rules_sync_traffic
//-----------------------------------------------------------------------------
static gboolean
rules_sync_traffic(NODE_USER *id, STORE_API *store)
{
 	NODE_RULE 				*node;
 	SQL_SESSION 			*sid;
 	GList					*t_tmp, *t_tek;
 	gboolean				err;
 
 	IP_size 				sum_traffic = 0, t_traffic = 0;
                                                                                                                                                                  
	if ((sid = store->open())!=NULL) {
  		
		t_tmp = id->id_rules;
		while ((t_tek = t_tmp) !=NULL) {
			t_tmp = g_list_next(t_tmp);
			node = (NODE_RULE*) t_tek->data;
			if (node->traffic) {
   				sum_traffic 	+= node->traffic;
   				t_traffic		= node->traffic;
   				node->traffic 	-= t_traffic;
  				if ((err = rules_mysql_export(sid, node->id, t_traffic, node->action))==0) node->traffic+= t_traffic;
			}
		}
  		store->close(sid);
 	}else return FALSE; 

	if (debug) st_log->print_log(IP_DEBUG, "[rules.c][rules_sync_traffic][%d]: rules_sync_traffic - OK\n", (int)getpid());
 	return TRUE; 
}


//-----------------------------------------------------------------------------
//                           		rules_init     
//-----------------------------------------------------------------------------
static gboolean
rules_init(NODE_USER *id)
{
	if (id->id_rules) return FALSE;
		
	id->id_rules = NULL;
	
	if (debug) st_log->print_log(IP_DEBUG, "User %d - struct list rules create", id->id);
	return TRUE;
}

//-----------------------------------------------------------------------------
//                                 rules_insert
//-----------------------------------------------------------------------------
static gboolean
rules_insert(NODE_USER *id, NODE_RULE *node)	// key - идентификатор правила
{
	NODE_RULE	*t_node;
  
	if ((t_node = (NODE_RULE*) g_malloc0(sizeof(NODE_RULE)))==NULL) {
		st_log->print_log(IP_ALERT, "[core_users.c][rules_init]: error allocate memory for NODE_RULE");
		return FALSE;
	}
	memcpy(t_node, node, sizeof(NODE_RULE));
	id->id_rules = g_list_append(id->id_rules, t_node);
	
	if (debug) st_log->print_log(IP_DEBUG, "User %d - rules %d insert", id->id, node->id);
	return TRUE;
}

//-----------------------------------------------------------------------------
//                                  rules_clean
//-----------------------------------------------------------------------------
static gboolean
rules_clean(NODE_USER *id)
{
	GList		*ptr, *t_tek, *t_tmp;
	
	ptr = t_tmp = id->id_rules;
	id->id_rules = NULL;
	
	while ((t_tek = t_tmp) != NULL) {
		t_tmp = g_list_next(t_tmp);
		if (t_tek->data) g_free(t_tek->data);
	}
	g_list_free(ptr);
	
	if (debug) st_log->print_log(IP_DEBUG, "User %d - rules clean", id->id);

	return TRUE;
}

//-----------------------------------------------------------------------------
//                                  rules_free
//-----------------------------------------------------------------------------
static gboolean
rules_free(NODE_USER *id)
{

	if (debug) st_log->print_log(IP_DEBUG, "User %d - rules free", id->id);
	return rules_clean(id);
}

//-----------------------------------------------------------------------------
//                                rules_find
//-----------------------------------------------------------------------------
static NODE_RULE
*rules_find(NODE_USER *id, const IP_id key)
{
 	GList				*node; 
	NODE_RULE			t_node;
 
	t_node.id = key;
	node = g_list_find_custom(id->id_rules, &t_node, users_compare_rules_id);

	if (node)	return node->data;
	else		return NULL;
}

//-----------------------------------------------------------------------------
//							users_compare_rules_id
//-----------------------------------------------------------------------------
static gint
users_compare_rules_id(gconstpointer a, gconstpointer b)
{
	register NODE_RULE *node1 = (NODE_RULE*) a;
	register NODE_RULE *node2 = (NODE_RULE*) b;
                                                                                
 	return memcmp(&node1->id, &node2->id, sizeof(IP_id));	
}


//-----------------------------------------------------------------------------
//                                rules_mysql_load
//-----------------------------------------------------------------------------
static gboolean
rules_mysql_load(NODE_USER *id, STORE_API *store)
{

	SQL_ROW			row;
 	
	SQL_SESSION 	*sid;
	IP_addr			t_ip;
		
	if (rules_init(id)!=TRUE) {
		st_log->print_log(IP_ERR, "[core_users.c][rules_mysql_load]: error create rules struct");
		return FALSE;
	}

	if ((sid = store->open())!=NULL) {
		char 	buffer[1024];
		
		sprintf(buffer, "select Rules.id, Direct, Proto, Net, Mask, Ports, Action, onActive, Price, id_type, Type_Traffic.id_group, md5(concat(Rules.id, id_type, Type_Traffic.id_group, Direct, Proto, Net, Mask, Ports, Action, onActive, Price)) from Rules RIGHT JOIN Type_Traffic ON Rules.id_type = Type_Traffic.id where id_user = %u order by  Proto, Mask desc, Ports desc, Direct", id->id);
  		if (!store->query(sid, buffer)) {
			st_log->print_log(IP_ERR, "[core_users.c][rules_mysql_load]: error execute query");
			if (id->id_rules) rules_free(id);
			store->close(sid);
			return FALSE;
		}

  		while ((row = store->fetch_row(sid))!=NULL) {
			NODE_RULE		node;
				
			bzero(&node, sizeof(NODE_RULE));

			hostbyip(row[3], &t_ip);
			
			node.id 		= atoi(row[0]);
   			node.direct 	= atoi(row[1]);
   		    node.proto		= atoi(row[2]);
			node.net		= t_ip;
			node.mask		= atoi(row[4]);
			node.ports		= atoi(row[5]);
			node.action		= atoi(row[6]);
			node.status		= atoi(row[7]);
			node.price		= (float) atof(row[8]);
			node.traffic	= 0;
			node.id_type	= atoi(row[9]);
			node.id_group	= atoi(row[10]);
			if (node.proto == 254)	node.service = SERVICE_SQUID;
			else					node.service = SERVICE_TRAFFIC;
				
			strncpy(node.md5, row[11], 32);
   			if (rules_insert(id, &node)!=TRUE) st_log->print_log(IP_WARNING, "[core_users.c][rules_mysql_load]: Node Rules %d not correct", node.id);
  		}  
		if (row) free(row);
  		store->close(sid); 
 	} 
 	if (debug) st_log->print_log(IP_DEBUG, "[core_users.c][rules_mysql_load]: insert rule to users id = %d", id->id);
	return TRUE;
}

//-----------------------------------------------------------------------------
//                                 rules_mysql_unload
//-----------------------------------------------------------------------------
static gboolean
rules_mysql_unload(NODE_USER *id, STORE_API *store)
{
	if (rules_sync_traffic(id, store)) {
		rules_clean(id);
		return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//                                rules_mysql_export
//-----------------------------------------------------------------------------
static gboolean
rules_mysql_export(SQL_SESSION *sid, IP_id id_sql, IP_size traffic, IP_flag action)
{
	char            buffer[1024];
	SQL_ROW         row;   
	float			price, tr_kb;
	IP_id			id_user;	
                                                                                                                                                             
	if (!store->exec(sid, "set autocommit=0")) goto error;
	if (!store->exec(sid, "start transaction")) goto error;
                                                                                                                                                             
                                                                                                                                                             
	sprintf(buffer, "update Rules SET Traffic = Traffic + %u, TDay = TDay + %u, TMonth = TMonth + %u where id=%u", traffic, traffic, traffic, id_sql);
	if (!store->exec(sid, buffer)) goto error;
		if (!((action & CONST_SUM)==CONST_SUM)) {			
			sprintf(buffer, "select Rules.id_user, Price from Rules LEFT JOIN Type_Traffic ON Rules.id_type = Type_Traffic.id where Rules.id = %u", id_sql);
			if (!store->query(sid, buffer)) goto error;

			row 	= store->fetch_row(sid);
        	id_user = atol(row[0]);
			price 	= atof(row[1]);
			tr_kb	= traffic/1024;
        	store->free(sid);
		
			sprintf(buffer, "update Users SET Balance = Balance - (%f * %f) where id = %u", tr_kb, price, id_user);
			if (!store->exec(sid, buffer)) goto error;
		}
        if (!store->exec(sid, "commit")) goto error;
                                                                                                                                                             
		if (debug) st_log->print_log(IP_DEBUG, "[system.c][rules_mysql_export]: row %d insert to dbase Rules", id_sql);
        return TRUE;
error:
        store->exec(sid, "rollback");
        return FALSE;
}
