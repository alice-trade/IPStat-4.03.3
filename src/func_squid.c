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

#include "../include/func_squid.h"
#include "../include/log.h"
#include "../include/sched.h"
#include "../include/server.h"
#include "../include/store.h"
#include "../include/misc.h"
#include "../include/glist.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>

static GSList		*ag_squid = NULL;
static PLUGIN_API	*t_api;
static gboolean		flag_export	= FALSE;
static IP_size		total_acct_query = 0;

extern IP_size		total_auth_query;
extern IP_size		success_auth_query;
extern IP_size		failed_auth_query;

extern gboolean		status_fifo;
extern gboolean		status_socket;

static pthread_mutex_t 	squid_mutex = PTHREAD_MUTEX_INITIALIZER;

#define SQUID_LOCK 	pthread_mutex_lock(&squid_mutex)
#define SQUID_UNLOCK 	pthread_mutex_unlock(&squid_mutex)


static int	squid_aggregation_timeout = 1;

static CONF_PARSER module_config[] = {
        { "squid_aggregation_timeout", CF_TYPE_INT, 0, &squid_aggregation_timeout, "1" },
        { NULL, -1, 0, NULL, NULL }
};

static gboolean		squid_sync(void *data);
static gboolean		squid_clean(GSList *table);
static gboolean		squid_print(int fd);
static gboolean		squid_job (int fd, int argc, char **argv, void *data);


static void			squid_traverse_print (gpointer data, gpointer user_data);
static void			squid_traverse_free (gpointer data, gpointer user_data);
static void			squid_traverse_sync (gpointer data, gpointer user_data);
//========================================================================================
//							Function FIFO - channal
//========================================================================================
gboolean
fifo_create(PLUGIN_API *api, char *file_name)
{
	LOG_API	*log = (LOG_API*) api->log;
	
	umask(0);                                                                                                                                                      
 	if (mkfifo(file_name, S_IRUSR+S_IWOTH+S_IROTH) == -1) {
  		log->print_log(IP_ERR, "[squid]: failed create fifo '%s' (%s)\n", file_name, strerror(errno));
  		return FALSE;
 	}
 	return TRUE;
}

FILE
*fifo_open(PLUGIN_API *api, char *file_name) 
{
 	FILE			*fd;
	LOG_API			*log = (LOG_API*) api->log;
 	struct stat    	t_stat;
    
	if (stat(file_name, &t_stat)==-1) {
   		if (errno == ENOENT) {
			if (!fifo_create(api, file_name)) return NULL;
		}else{
			log->print_log(IP_ERR, "[squid]: failed get stat fifo-file '%s' (%s)\n", file_name, strerror(errno));
			return NULL;
		}
 	}else {
  		if (!S_ISFIFO(t_stat.st_mode)) {
			if (unlink(file_name)==-1) log->print_log(IP_ERR, "[squid]: failed remove 'squid_socket' file (%s)", file_name);
   			if (!fifo_create(api, file_name)) return NULL;
  		}
 	}
 
 	if ((fd = fopen(file_name, "r"))==NULL) {
  		log->print_log(IP_ERR, "[squid]: failed open fifo '%s' (%s)\n", file_name, strerror(errno));
  		return NULL;
 	}
	log->print_log(IP_INFO, "Open fifo-file");
	return fd;
}

gboolean
fifo_close(PLUGIN_API *api, FILE *fd) 
{
	LOG_API	*log = (LOG_API*) api->log;
	
	if (fclose(fd)==-1) {
  		log->print_log(IP_ERR, "[squid]: failed close fifo-file (%s)\n", strerror(errno));
  		return FALSE;
 	}
 	return TRUE;
}

int
fifo_read(FILE *fd, void *buf, size_t count, int timeout)
{
	char	*res;

	if (timeout != 0) {
		int ret;
		struct timeval tv;
		fd_set read_fds;
		int fd_inode;
		
		if ((fd_inode = fileno(fd)) == -1) return -2;
		
		if (timeout < 0) {
			tv.tv_sec = 0;
			tv.tv_usec = 0;
		} else {
			tv.tv_sec = timeout / 1000000;
			tv.tv_usec = timeout % 1000000;
		}
		FD_ZERO(&read_fds);
		FD_SET(fd_inode, &read_fds);
		ret = select(fd_inode+1, &read_fds, NULL, NULL, &tv);
		if (ret == -1) return -2;
		if (ret == 0) return -1;
		if (!FD_ISSET(fd_inode, &read_fds)) {
			return -1;
		}
	}
	
	res = fgets(buf, count, fd);
	
	if (res == NULL) return 0;

	return 1;
}

gboolean
squid_init(PLUGIN_API *api)
{
	SERVER_API		*server = NULL;
	SCHEDULER_API	*sched 	= NULL;
	LOG_API			*log 	= NULL;
	CONF_API		*conf 	= NULL;
	char			str[20];
	
	t_api = api;
	log = (LOG_API*) t_api->log;
	conf = (CONF_API*) t_api->conf;
	
	if (conf) conf->parse(module_config);
		
	server = api->server;
	sched = api->sched;
	
	ag_squid = NULL;

	sprintf(str, "*/%d * * * *", squid_aggregation_timeout);
	
	if (server) server->registred("squid_aggregation", (void*)&squid_job, NULL);
	if (sched) sched->registred("squid_aggregation", str, (void*)&squid_sync, NULL);
		
	return TRUE;
}

gboolean
squid_free(void)
{
	SERVER_API	*server = NULL;
	SCHEDULER_API	*sched = NULL;
	
	if (!ag_squid) return FALSE;
		
	server = t_api->server;
	sched = t_api->sched;
	
	if (server) server->unregistred("squid_aggregation");
	if (sched) sched->unregistred("squid_aggregation");
	
	squid_sync(NULL);	
	SQUID_LOCK;
		squid_clean(ag_squid);
		g_list_free(ag_squid);
		ag_squid = NULL;
	SQUID_UNLOCK;
	return TRUE;
}

gboolean 
squid_acct(char *url, SQUID_DATA_INFO *squid_data)
{
	ACCT_SQUID_NODE	*ptr = NULL;

	if ((ptr = g_malloc0(sizeof(ACCT_SQUID_NODE)))==NULL) return FALSE;
	
	ptr->id_user = squid_data->id_user;
	strncpy(ptr->url, url, 1023);
	ptr->traffic = squid_data->len;
	
	SQUID_LOCK;
		ag_squid = g_list_prepend(ag_squid, ptr);
	SQUID_UNLOCK;
	total_acct_query++;
	return TRUE;
}

static gboolean		
squid_sync(void *data)
{
	STORE_API	*store 	= NULL;
	SQL_SESSION	*sid 	= NULL;
	LOG_API		*log	= NULL;
	GSList		*backup = NULL;
	
	
	if (flag_export) return FALSE;
	log = t_api->log;
	
	if (!(store = t_api->store)) {
		log->print_log(IP_ERR, "[netfilter_time_ip]: error find store api");
		return FALSE;
	}
	
	
	if ((sid = store->open())!=NULL) {
		flag_export = TRUE;
		SQUID_LOCK;
			backup = ag_squid;
			ag_squid = NULL;
		SQUID_UNLOCK;
		g_list_foreach(backup, (void*)&squid_traverse_sync, sid);
		
		squid_clean(backup);
		g_list_free(backup);			
		flag_export = FALSE;
		store->close(sid);
	}else log->print_log(IP_ERR, "[netfilter_time_ip]: failed open connection");
	return TRUE;		
}

static gboolean
squid_clean(GSList *table)
{
	g_list_foreach(table, (void*)&squid_traverse_free, NULL);
	return TRUE;
}

static gboolean              
squid_print(int fd) 
{
	char	buf[12];
	
	send_to_socket(fd, "Squid Aggregations:\n\r");
	
	send_to_socket(fd, "---------------------------------------------------------------------------\n\r");
    send_to_socket(fd, "| %7s | %50s | %8s |\n\r", "id_user", "URL", "Traffic");
    send_to_socket(fd, "---------------------------------------------------------------------------\n\r");
	SQUID_LOCK;
		g_list_foreach(ag_squid, (void*)&squid_traverse_print, (gpointer) &fd);
	SQUID_UNLOCK;
    send_to_socket(fd, "---------------------------------------------------------------------------\n\r");
	send_to_socket(fd, "Aggregate nodes - %d\n\r", g_list_length(ag_squid));
	send_to_socket(fd, "Total auth query - %d\n\r", total_auth_query);
	send_to_socket(fd, "Total success auth query - %d\n\r", success_auth_query);
	send_to_socket(fd, "Total failed auth query - %d\n\r", failed_auth_query);
	send_to_socket(fd, "Total acct query - %d\n\r", total_acct_query);
	if (status_socket==TRUE)	sprintf(buf, "CONNECT");
	else 						sprintf(buf, "NOT CONNECT");
	send_to_socket(fd, "Status auth - %s\n\r", buf);

	if (status_fifo==TRUE)	sprintf(buf, "CONNECT");
	else 					sprintf(buf, "NOT CONNECT");
	send_to_socket(fd, "Status acct - %s\n\r", buf);
 	return TRUE;
}

static gboolean
squid_export(SQL_SESSION *sid, ACCT_SQUID_NODE *node)
{
	char *buffer;
	STORE_API 	*store	= (STORE_API*)	t_api->store;
	LOG_API		*log	= (LOG_API*)	t_api->log;
	
	if ((buffer = (char*) g_malloc0(2048))==NULL) {
		log->print_log(IP_ERR, "[squid][squid_export]: failed malloc momory for buffer");
		return FALSE;
	}

	sprintf(buffer, "insert into Log_Squid (Time, id_user, URL, Traffic) values (NOW(), %d, '%s', %u)", node->id_user, node->url, node->traffic);
	if (!store->exec(sid, buffer)) goto error;
                                                                                                                                                             
        free(buffer);
        return TRUE;
                                                                                                                                                             
error:
        free(buffer);
        return FALSE;
}

static gboolean		
squid_job (int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
		send_to_socket(fd, "*  - show squid     - Show squid aggregation  *\n\r");
		return FALSE;
    }else if ((argc == 2) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "squid", 4) == 0)) {
		squid_print(fd);
		return TRUE;
	}
	return FALSE;
}

static void
squid_traverse_print (gpointer data, gpointer user_data)
{
	int					*fd = (int*) user_data;
	ACCT_SQUID_NODE		*node = (ACCT_SQUID_NODE*) data;

	send_to_socket(*fd, "| %7d | %50s | %8d |\n\r", node->id_user, node->url, node->traffic);
}

static void
squid_traverse_free (gpointer data, gpointer user_data)
{
	if (data) g_free(data);
}

static void
squid_traverse_sync (gpointer data, gpointer user_data)
{
	SQL_SESSION 		*sid = (SQL_SESSION*) user_data;
	ACCT_SQUID_NODE		*node = (ACCT_SQUID_NODE*) data;
	LOG_API				*log = (LOG_API*) t_api->log;
	int					n_repeat = 3;
	
l_start:
	if (squid_export(sid, node)==TRUE) {
		fprintf(stderr, "export - ok\n");
	}else {
		sleep(1);
		if (n_repeat--) goto l_start;  
		log->print_log(IP_ERR, "node not export!!!");
	}		
}
