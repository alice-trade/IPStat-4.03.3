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

#include "../include/log.h"
#include "../include/server.h"
#include "../include/sched.h"
#include "../include/misc.h"
#include "../include/glist.h"

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define SCHEDULER_UNKNOWN	0
#define	SCHEDULER_SLEEP		1
#define SCHEDULER_RUN		2

typedef struct st_scheduler_node
{
	char						name[50];
 	char 						crontab[50];
 	struct tm					vol;
 	struct tm					vol_div;

 	void          				*data;
  
	time_t						last_runs;				// time last runs
	time_t						delay_time;				// time runs
	
 	pthread_t					id_thread;
 	int							status;
	gboolean					last_status;

 	gboolean					(*func)(const void*);
} NODE_SCHEDULER;


typedef struct st_scheduler
{
	GList						*sched_list;
  	sig_atomic_t				do_exit;
 	pthread_t					id_thread;
} SCHEDULER;	

static SCHEDULER 	*st_scheduler 	= NULL;
static SERVER_API	*st_server 	= NULL;
static CONF_API		*st_conf	= NULL;
static LOG_API		*st_log		= NULL;

extern int debug;

static pthread_mutex_t	scheduler_mutex = PTHREAD_MUTEX_INITIALIZER;

static void traverse(gpointer, gpointer);
static int	time_cmp(NODE_SCHEDULER*);
static void	parse_vol(char*, int*, int*);
static gint compare(gconstpointer, gconstpointer);

static void thr_scheduler(void *data);
static void thr_scheduler_child(NODE_SCHEDULER *task);

static gboolean	schedulers_init(void);
static gboolean	schedulers_free(void);
static gboolean	schedulers_print(int fd);
static gboolean	schedulers_register(char *name, char *crontab, gboolean (*func)(const void*), void *data);
static gboolean	schedulers_unregister(char *name);
static gboolean	server_job(int fd, int argc, char **argv, void *data);
static gboolean	schedulers_set_server(void *data);

#define CHECK_CORRECT_SCHED	if (!st_scheduler) return FALSE

#define SCHEDULER_LOCK 		pthread_mutex_lock(&scheduler_mutex)
#define SCHEDULER_UNLOCK 	pthread_mutex_unlock(&scheduler_mutex)

SCHEDULER_API		
*scheduler_init(CONF_API *conf, LOG_API *log)
{
	SCHEDULER_API	*api;
	
	st_conf = conf;
	st_log	= log;
	
	if ((api = g_malloc0(sizeof(SCHEDULER_API)))==NULL) {
		st_log->print_log(IP_ALERT, "[scheduler.c][sched_init]: error allocate memory for SCHEDULER_API");
		return NULL;
	}
	if (schedulers_init()==FALSE) {
		g_free(api);
		return NULL;
	}
	
	api->registred		= schedulers_register;
	api->unregistred	= schedulers_unregister;
	api->print		= schedulers_print;
	api->set_server		= schedulers_set_server;
	
	return api;
}

gboolean		
scheduler_free(SCHEDULER_API *api)
{
	schedulers_free();
	
	if (api) g_free(api);
	if (debug) st_log->print_log(IP_DEBUG, "Free SCHED struct");
	api = NULL;

	return TRUE;
}

gboolean	
schedulers_set_server(void *data)
{
	st_server = (SERVER_API*) data;

	if (st_server) st_server->registred("scheduler", (void*)&server_job, NULL);
	return TRUE;
}

gboolean
schedulers_init(void)
{
 	SCHEDULER 		*id;
	pthread_attr_t 	tattr;

 	if ((id = (SCHEDULER*) g_malloc0(sizeof(SCHEDULER)))==NULL) {
  		st_log->print_log(IP_ALERT, "[sched.c][scheduler_init]: error allocate memory for SCHEDULER struct");
		return FALSE;
	}

	id->sched_list	= NULL;
	id->do_exit		= 0;
	if (debug) st_log->print_log(IP_DEBUG, "create struct SCHEDULER [%x]", id);

	pthread_attr_init(&tattr);
	pthread_attr_setstacksize(&tattr, PTHREAD_STACK_MIN + 0x4000);
  	if (pthread_create (&id->id_thread, &tattr, (void*)&thr_scheduler, id)!=0) {
		st_log->print_log(IP_ALERT, "[sched.c][scheduler_init]: error creation thread [scheduler] failed");
		g_free(id);
		return FALSE;
	}
	pthread_attr_destroy(&tattr);
	
	if (debug) st_log->print_log(IP_DEBUG, "create scheduler thread [%d]", (int) id->id_thread);

	st_scheduler = id;
 	return TRUE;
}

gboolean
schedulers_free(void)
{
 	GList	*t_tek, *t_tmp = (GList*) st_scheduler->sched_list;
 
	if (st_server) st_server->unregistred("scheduler");
		
	CHECK_CORRECT_SCHED;
	
	st_scheduler->do_exit = 1;
	pthread_kill(st_scheduler->id_thread, SIGUSR2);
	if (debug) st_log->print_log(IP_DEBUG, "send scheduler signal SIGUSR2");
		
 	SCHEDULER_LOCK;
		while ((t_tek = t_tmp) != NULL) {
  			t_tmp = g_list_next(t_tmp);
			if (t_tek->data) g_free(t_tek->data);
		}
		g_list_free(st_scheduler->sched_list);
		st_scheduler->sched_list = NULL;
  	SCHEDULER_UNLOCK;
	if (debug) st_log->print_log(IP_DEBUG, "remove list jobs in schedulers");
	
	pthread_join(st_scheduler->id_thread, NULL);			// ожидаем окончание нити и выходим
	if (debug) st_log->print_log (IP_DEBUG, "destroy scheduler thread");
  	if (st_scheduler) g_free(st_scheduler);
	st_scheduler = NULL;
 	return TRUE;
}

gboolean
schedulers_print(int fd)
{
	CHECK_CORRECT_SCHED;
	
    send_to_socket(fd, "-------------------------------------------------------------------------\n\r");
    send_to_socket(fd, "| %20s | %18s | %7s | %7s | %5s |\n\r", "Name", "Last Runs" , "Delay", "Status", "rcode");
    send_to_socket(fd, "-------------------------------------------------------------------------\n\r");
    
	SCHEDULER_LOCK;
		g_list_foreach(st_scheduler->sched_list, traverse, &fd);
    SCHEDULER_UNLOCK;
    send_to_socket(fd, "-------------------------------------------------------------------------\n\r");
    return TRUE;
}

gboolean					
schedulers_register(char *name, char *crontab, gboolean (*func)(const void*), void *data)
{
 	char 			*buffer, *ptr;
 	NODE_SCHEDULER 	*node;
	
	CHECK_CORRECT_SCHED;
	
	if ((node = (NODE_SCHEDULER*) g_malloc0(sizeof(NODE_SCHEDULER)))==NULL) {
  		st_log->print_log(IP_ALERT, "[sched.c][scheduler_register]: error allocate memory for NODE_SCHEDULER struct");
		return FALSE;
	}
	
	strncpy(node->name, name, 50);
	strncpy(node->crontab, crontab, 50);
	node->last_runs	= 0;
	node->delay_time = 0;
	
	node->func 		= func;
	node->data 		= data;
	node->status	= SCHEDULER_SLEEP;
	node->last_status = TRUE;
	
	if ((buffer = strdup(node->crontab))==NULL) return FALSE;
  		ptr = strtok(buffer, " ");			// min
  		parse_vol(ptr, &node->vol.tm_min, &node->vol_div.tm_min);
  		ptr = strtok('\0', " ");			// hour
  		parse_vol(ptr, &node->vol.tm_hour, &node->vol_div.tm_hour);
  		ptr = strtok('\0', " ");			// mday
  		parse_vol(ptr, &node->vol.tm_mday, &node->vol_div.tm_mday);
  		ptr = strtok('\0', " ");			// mon
  		parse_vol(ptr, &node->vol.tm_mon, &node->vol_div.tm_mon);
  		ptr = strtok('\0', " ");			// wday
  		parse_vol(ptr, &node->vol.tm_wday, &node->vol_div.tm_wday);
 	g_free(buffer);
	SCHEDULER_LOCK;	
		st_scheduler->sched_list = g_list_prepend(st_scheduler->sched_list, node);
	SCHEDULER_UNLOCK;

 	return TRUE;
}

gboolean					
schedulers_unregister(char *name)
{
 	NODE_SCHEDULER 	node;
	GList			*ptr;
	
	CHECK_CORRECT_SCHED;
	
	strncpy(node.name, name, 50);

	SCHEDULER_LOCK;
		ptr = g_list_find_custom(st_scheduler->sched_list, &node, compare);
		if (ptr) {
			st_scheduler->sched_list = g_list_remove(st_scheduler->sched_list, ptr->data);
		}
	SCHEDULER_UNLOCK;
	
	return TRUE;
}

/*******************************************************************************
 Private functions
*******************************************************************************/
static void 
thr_scheduler(void *data) 
{
	SCHEDULER			*id = (SCHEDULER*) data;
 	time_t				target_time;
 	register struct tm	*tm;
	sigset_t			set, oldset;
	GList				*t_tmp;
 
	if (debug) st_log->print_log(IP_DEBUG, "[sched.c][thr_scheduler]: create thread manager: [%d]", (int)id->id_thread);

	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);
	pthread_sigmask(SIG_UNBLOCK, &set, &oldset);
	
	
	while (id->do_exit==0) {
		target_time = time(NULL);
		tm = localtime(&target_time);
		target_time = (60 - tm->tm_sec);

  		sleep((int)target_time);
  		if (id->do_exit!=0) goto l_exit;
		
		SCHEDULER_LOCK;
			t_tmp = st_scheduler->sched_list;
			while (t_tmp != NULL) {
				NODE_SCHEDULER	*node = (NODE_SCHEDULER*) t_tmp->data;
  				
   				if (time_cmp(node) && node->status == SCHEDULER_SLEEP) {
					pthread_attr_t 	tattr;
					
					pthread_attr_init(&tattr);
					pthread_attr_setstacksize(&tattr, PTHREAD_STACK_MIN + 0x4000);
    				if (pthread_create(&node->id_thread, &tattr, (void*)&thr_scheduler_child, node)!=0) st_log->print_log(IP_ERR, "[sched.c][thr_scheduler]: error creation thread [%s]");
					pthread_attr_destroy(&tattr);
   				}
  				t_tmp = g_list_next(t_tmp);
			}
		SCHEDULER_UNLOCK;
	}
l_exit:
	if (debug) st_log->print_log(IP_DEBUG, "end runing thread scheduler");
	pthread_exit(NULL);
}

static void 
thr_scheduler_child(NODE_SCHEDULER *task) 
{
	time_t		start_time;
	
 	pthread_detach(pthread_self());
	if (debug) st_log->print_log(IP_DEBUG, "Create thread task: name='%s' id=%d\n", task->name, (int)pthread_self());

  	start_time = time(NULL);
	task->last_runs = time(NULL);	
	task->status = SCHEDULER_RUN;
   	task->last_status = task->func(task->data);
  	task->status = SCHEDULER_SLEEP;
 	

	task->delay_time = time(NULL) - start_time;
 	
	pthread_exit(NULL);
}

static void 
traverse(gpointer data, gpointer user_data)
{
	NODE_SCHEDULER	*node = (NODE_SCHEDULER*) data;
	int				*fd = (int*) user_data;
	char			buf[100], status[10], last_status[10];
	struct tm       tm_time;

	if (node->status == SCHEDULER_SLEEP) sprintf(status, "SLEEP");
	else if (node->status == SCHEDULER_RUN) sprintf(status, "RUN");
	else sprintf(status, "UNKNOWN");
		
	if (node->last_status == TRUE) sprintf(last_status, "OK");
	else if (node->last_status == FALSE) sprintf(last_status, "ERR");
	else sprintf(last_status, "???");
	
		
	if (node->last_runs == 0) {
		sprintf(buf, "-----------------");
	}else{
		localtime_r(&node->last_runs, &tm_time);
		strftime(buf, 99, "%d.%m.%y %H:%M:%S", &tm_time);
	}
	send_to_socket(*fd, "| %20s | %18s | %7d | %7s | %5s |\n\r", node->name, buf, (int)node->delay_time, status, last_status);
}

static int
time_cmp(NODE_SCHEDULER *node) 
{
 	struct tm	*tm;
 	time_t		ti;
 	int	f_min, f_hour, f_mday, f_mon, f_wday;
	
 	ti = time(NULL);
 	tm = localtime(&ti);

 	f_min = 0;
 	if (node->vol_div.tm_min==-1) {
  		if (node->vol.tm_min!=-1) {
   			if (node->vol.tm_min == tm->tm_min) f_min = 1;
  		}else f_min = 1; 
 	}else {
  		if ((tm->tm_min % node->vol_div.tm_min) == 0) f_min = 1;
 	}

 	f_hour = 0;
 	if (node->vol_div.tm_hour==-1) {
  		if (node->vol.tm_hour!=-1) {
   			if (node->vol.tm_hour == tm->tm_hour) f_hour = 1;
  		}else f_hour = 1;
 	}else {
  		if ((tm->tm_hour % node->vol_div.tm_hour) == 0) f_hour = 1;
 	}

 	f_mday = 0;
 	if (node->vol_div.tm_mday==-1){
  		if (node->vol.tm_mday!=-1){
   			if (node->vol.tm_mday == tm->tm_mday) f_mday = 1;
  		}else f_mday = 1;
 	}else{
  		if ((tm->tm_mday % node->vol_div.tm_mday) == 0) f_mday = 1;
 	}

 	f_mon = 0;
 	if (node->vol_div.tm_mon==-1){
  		if (node->vol.tm_mon!=-1){
   			if (node->vol.tm_mon == tm->tm_mon) f_mon = 1;
  		}else f_mon = 1;
 	}else
 	{
  		if ((tm->tm_mon % node->vol_div.tm_mon) == 0) f_mon = 1;
 	}

 	f_wday = 0;
 	if (node->vol_div.tm_wday==-1){
  		if (node->vol.tm_wday!=-1){
   			if (node->vol.tm_wday == tm->tm_wday) f_wday = 1;
  		}else f_wday = 1;
 	}else
 	{
  		if ((tm->tm_wday % node->vol_div.tm_wday) == 0) f_wday = 1;
 	}
 	if (f_min && f_hour && f_mday && f_mon && f_wday)	return TRUE;
 	else						 						return FALSE;  
}

static void 
parse_vol(char *str, int *vol, int *vol_div) 
{
 	char *pos;
	
 	if ((pos = strchr(str, '/'))!=NULL) {
  		*vol_div = atoi(pos+1);
  		*pos = '\0';
  		if (str[0]=='*') 	*vol = -1;
  		else				*vol = atoi(str);
 	}else {
  		*vol_div	= -1;
  		if (str[0]=='*') 	*vol = -1;
  		else				*vol = atoi(str);
 	}
 	if (debug) st_log->print_log(IP_DEBUG, "[sched.c][parse_vol]: vol=%d vol_div=%d\n", *vol, *vol_div);
}

static gint 
compare(gconstpointer a, gconstpointer b)
{
	NODE_SCHEDULER	*node1 = (NODE_SCHEDULER*) a;
	NODE_SCHEDULER	*node2 = (NODE_SCHEDULER*) b;

	return (gint) strncmp(node1->name, node2->name, 50);
}

gboolean	
server_job(int fd, int argc, char **argv, void *data)
{
	if ((argc == 1) && (strncmp(argv[0], "help", 4)==0)) {
        send_to_socket(fd, "*  - show sched    - Print scheduler jobs     *\n\r");
        return FALSE;
    }else if ((argc == 2) && (strncmp(argv[0], "show", 4) == 0) && (strncmp(argv[1], "sched", 4) == 0)) {
		schedulers_print(fd);
		return TRUE;
	}
	return FALSE;
}
