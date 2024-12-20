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
#include "../include/inet.h"
#include "../include/misc.h"
#include "../include/gtree.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

typedef struct st_key
{
    IP_id               id_user;
    IP_proto    	proto;
    IP_flag             direct;
    IP_addr             addr;
    IP_port             port;
} st_key;
                                                                                                                                                             
typedef struct st_value
{
    time_t              stime;
    IP_size             pkt;
    IP_size             bytes;
} st_value;

static gint			ip_traverse_print (gpointer key, gpointer value, gpointer data);
static gint			ip_traverse_free (gpointer key, gpointer value, gpointer data);
static gint			ip_traverse_sync (gpointer key, gpointer value, gpointer data);
static gboolean		ip_export(SQL_SESSION *sid, st_key *key, IP_size size);
static gboolean     ip_print(int fd);
static gint			ip_compare (gconstpointer  a, gconstpointer  b);
static gboolean		ip_clean(ITree *table);
static gboolean		ip_job (int fd, int argc, char **argv, void *data);
static gboolean		ip_sync(void *data);

static pthread_mutex_t	ip_mutex = PTHREAD_MUTEX_INITIALIZER;
static ITree			*ag_ip = NULL;
static PLUGIN_API		*t_api;
static gboolean			flag_export = FALSE;

#define IP_LOCK 	pthread_mutex_lock(&ip_mutex)
#define IP_UNLOCK 	pthread_mutex_unlock(&ip_mutex)

static int	ip_aggregation_timeout = 1;

static CONF_PARSER module_config[] = {
        { "ip_aggregation_timeout", CF_TYPE_INT, 0, &ip_aggregation_timeout, "1" },
        { NULL, -1, 0, NULL, NULL }
};

gboolean
ip_init(PLUGIN_API *api)
{
	SERVER_API		*server = NULL;
	SCHEDULER_API	*sched 	= NULL;
	LOG_API			*log 	= NULL;
	CONF_API		*conf	= NULL;
	char			str[20];
	
	t_api = api;
	log = (LOG_API*) t_api->log;
	
	server 	= (SERVER_API*)	api->server;
	sched 	= (SCHEDULER_API*) api->sched;
	conf 	= (CONF_API*) api->conf;
	
	if (conf) conf->parse(module_config);
		
	if ((ag_ip = i_tree_new((void*)&ip_compare))==NULL) {
		log->print_log(IP_ERR, "Error create g_hash");
		return FALSE;
	}
	
	sprintf(str, "*/%d * * * *", ip_aggregation_timeout);
	
	if (server) server->registred("ip_aggregation", (void*)&ip_job, NULL);
	if (sched) sched->registred("ip_aggregation", str, (void*)&ip_sync, NULL);
		
	return TRUE;
}

gboolean
ip_free(void)
{
	SERVER_API	*server = NULL;
	SCHEDULER_API	*sched = NULL;
	
	if (!ag_ip) return FALSE;
		
	server = t_api->server;
	sched = t_api->sched;
	
	if (server) server->unregistred("ip_aggregation");
	if (sched) sched->unregistred("ip_aggregation");
	
	ip_sync(NULL);	
	IP_LOCK;
		ip_clean(ag_ip);
		i_tree_destroy(ag_ip);
		ag_ip = NULL;
	IP_UNLOCK;
	return TRUE;
}

gboolean
ip_acct(PACKET_INFO *packet_info, PACKET_DATA_INFO *packet_data)
{
	gpointer			ptr;
	st_key				*key, key_ptr;
	st_value			*value;
	LOG_API				*log = NULL;
	
	log = (LOG_API*) t_api->log;
	
	if (!ag_ip) return FALSE;
	
	key_ptr.id_user = packet_data->id_user;
	key_ptr.proto	= packet_info->proto;
	key_ptr.direct	= packet_data->direct;
	
	if (packet_data->direct == 1) {
		key_ptr.addr = packet_info->src;
		key_ptr.port = packet_info->src_port;
	}else {
		key_ptr.addr = packet_info->dest;
		key_ptr.port = packet_info->dest_port;
	}	
	IP_LOCK;
		ptr = i_tree_lookup(ag_ip, (gpointer) &key_ptr);
	IP_UNLOCK;
	if (ptr!=NULL) {
		IP_LOCK;
			value = (st_value*) ptr;
			value->pkt++;
			value->bytes+=packet_data->len;
		IP_UNLOCK;
	} else {
		if ((key = g_malloc0(sizeof(st_key)))==NULL) {
			log->print_log(IP_ERR, "error allocate memofy for key");
			return FALSE;
		}
		if ((value = g_malloc0(sizeof(st_value)))==NULL) {
			log->print_log(IP_ERR, "error allocate memofy for value");
			g_free(key);
			return FALSE;
		}
		
		memcpy(key, &key_ptr, sizeof(st_key));
		
		value->pkt 		= 1;
		value->bytes 	= packet_data->len;

		IP_LOCK;
			i_tree_insert(ag_ip, key, value);
		IP_UNLOCK;
	}
	return TRUE;
}

static gboolean		
ip_sync(void *data)
{
	STORE_API	*store 	= NULL;
	SQL_SESSION	*sid 	= NULL;
	LOG_API		*log	= NULL;
	ITree		*backup = NULL;
	
	
	if (flag_export) return FALSE;
	log = t_api->log;
	
	if (!(store = t_api->store)) {
		fprintf(stderr, "netfilter_time: error find store api\n");
		return FALSE;
	}
	
	
	if ((sid = store->open())!=NULL) {
		flag_export = TRUE;
		IP_LOCK;
			backup = ag_ip;
			ag_ip = NULL;
			if ((ag_ip = i_tree_new((void*)&ip_compare))==NULL) {
				if (log) log->print_log(IP_ERR, "Error create g_hash");
				return FALSE;
			}
		IP_UNLOCK;
		i_tree_traverse(backup, (void*)&ip_traverse_sync, I_IN_ORDER, sid);
		
		ip_clean(backup);
		i_tree_destroy(backup);			
		flag_export = FALSE;
		store->close(sid);
	}else fprintf(stderr, "netfilter_time: error open connection\n");
	return TRUE;	
}

static gboolean              
ip_print(int fd) 
{
	send_to_socket(fd, "IP Aggregations:\n\r");
	send_to_socket(fd, "-----------------------------------------------------------------\n\r");
	send_to_socket(fd, "| %4s | %7s | %15s | %6s | %10s | %4s |\n\r", "Prot", "id_user", "Addr", "S_port", "Size", "Pkt");
	send_to_socket(fd, "-----------------------------------------------------------------\n\r");
	IP_LOCK;
		i_tree_traverse(ag_ip, (void*)&ip_traverse_print, I_IN_ORDER, &fd);
	IP_UNLOCK;
	send_to_socket(fd, "-----------------------------------------------------------------\n\r");
 	return TRUE;
}

static gint 
ip_traverse_print (gpointer key, gpointer value, gpointer data)
{
	int             *fd = (int*) data;
	st_key      	*t_key = (st_key*) key;
	st_value		*t_value = (st_value*) value;
	char            proto[9], addr[19];
	
	get_proto(t_key->proto, proto);
    ipbyhost(t_key->addr, addr);
	send_to_socket(*fd, "| %4s | %7d | %15s | %6d | %10u | %4u |\n\r", proto, t_key->id_user, addr, t_key->port, t_value->bytes, t_value->pkt);
	return FALSE;
}

static gint 
ip_traverse_sync (gpointer key, gpointer value, gpointer data)
{
	SQL_SESSION *sid = (SQL_SESSION*) data;
	
	st_key		*t_key = (st_key*) key;
	st_value	*t_value = (st_value*) value;
	int			n_repeat = 3;
	
l_start:
	if (ip_export(sid, t_key, t_value->bytes)==TRUE) {
		fprintf(stderr, "export - ok\n");
	}else {
		sleep(1);
		if (n_repeat--) goto l_start;  
	}		
	return FALSE;
}

static gint 
ip_traverse_free (gpointer key, gpointer value, gpointer data)
{
	if (key) g_free(key);
	if (value) g_free(value);
		
	return FALSE;
}

static gboolean
ip_export(SQL_SESSION *sid, st_key *key, IP_size size)
{
	SQL_ROW     row;
	char        *buffer;
	STORE_API	*store = NULL;
	int    		flag_ip;
	st_key 		rec;
	LOG_API		*log = NULL;
		
	log = (LOG_API*) t_api->log;
	if (!(store = t_api->store))	return FALSE;
		
    if ((buffer = (char*) g_malloc0(1024))==NULL) {
        log->print_log(IP_ERR, "error allocate memory for buffer");
        return FALSE;
    }
	
    memcpy(&rec, key, sizeof(st_key));
    
	rec.addr  = ntohl(rec.addr);
                                                                                                                                                             
    sprintf(buffer, "select COUNT(*) from Log_IP where id_user=%d and Proto=%d and Direct=%d and Addr=%u and Port=%u and Date=CURDATE()", rec.id_user, rec.proto, rec.direct, rec.addr, rec.port);
	if (!store->query(sid, buffer)) goto error;
                                                                                                                                                             
    row = store->fetch_row(sid);
    flag_ip = atol(row[0]);
    store->free(sid);
                                                                                                                                                             
    if (flag_ip)	sprintf(buffer, "update Log_IP SET Traffic=Traffic+%u where id_user=%d and Proto=%d and Direct=%d and Addr=%u and Port=%u and Date=CURDATE()", size, rec.id_user, rec.proto, rec.direct, rec.addr, rec.port);
	else			sprintf(buffer, "insert into Log_IP (id_user, Proto, Direct, Addr, Port, Traffic, Date) values (%d, %d, %d, %u, %u, %u, CURDATE())", rec.id_user, rec.proto, rec.direct, rec.addr, rec.port, size);

	if (!store->exec(sid, buffer)) goto error;
                     
	g_free(buffer);	
    return TRUE;
error:
	g_free(buffer);
    return FALSE;
	
}

static gint
ip_compare (gconstpointer  a, gconstpointer  b)
{
	st_key	*id_1	= (st_key*) a;
	st_key	*id_2	= (st_key*) b;
	
	return memcmp(id_1, id_2, sizeof(st_key));
}



static gboolean
ip_clean(ITree *table)
{
	i_tree_traverse(table, (void*)&ip_traverse_free, I_IN_ORDER, NULL);
	return TRUE;
}

static gboolean		
ip_job (int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - show ip        - Show ip aggregation     *\n\r");
		return FALSE;
    }else if ((argc == 2) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "ip", 2) == 0)) {
		ip_print(fd);
		return TRUE;
	}
	return FALSE;
}
