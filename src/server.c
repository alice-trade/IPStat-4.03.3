/*
 * server.c	Interface for administrative.
 *
 * Version:	$Id: server.c,v 1.2 2005/12/19 09:36:13 kornet Exp $
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

static const char rcsid[] = "$Id: server.c,v 1.2 2005/12/19 09:36:13 kornet Exp $";

#include "../include/server.h"
#include "../include/log.h"
#include "../include/conf.h"
#include "../include/misc.h"
#include "../include/threads.h"
#include "../include/glist.h"

#ifndef VERSION
#define VERSION "1.1"
#endif

#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <errno.h>

typedef struct socket_client
{
 	struct sockaddr_in 	*client_addr;
 	int 				client_socket;
} socket_client;

extern int debug;

static 	SERVER		*st_server 	= NULL;
static	CONF_API	*st_conf	= NULL;
static	LOG_API		*st_log		= NULL;

static INIT_POOL 	init_pool;				// структура начальной инициализации
static THREAD_POOL	*thread_pool;

static	int			max_clients;
static	int			debug_console;
static	char		term_port[10];

static CONF_PARSER module_config[] = {
        { "term_port", CF_TYPE_STRING, 10, term_port, "556" },
        { "max_console_clients", CF_TYPE_INT, 0, &max_clients, "5" },
        { "debug_console",    CF_TYPE_INT, 0, &debug_console, "0" },
        { NULL, -1, 0, NULL, NULL }
};

static pthread_mutex_t 		server_mutex = PTHREAD_MUTEX_INITIALIZER;

#define CHECK_CORRECT_SERVER	if (!st_server) return FALSE

#define SERVER_LOCK 	pthread_mutex_lock(&server_mutex)
#define SERVER_UNLOCK 	pthread_mutex_unlock(&server_mutex)

static void				thr_server(SERVER *);
static void 			thr_clientproc(struct socket_client *cli);

static void				parse_arg(char *buffer, int *argc, char *argv[], int max_tok);
static void				get_date(time_t s_time, char *date);

gboolean				unregister_server(char*);
gboolean				register_server(char*, gboolean (*func)(int, int, char**, void*), void*);
static gint				compare(gconstpointer a, gconstpointer b);
//-----------------------------------------------------------------------------
//			    server_init
//-----------------------------------------------------------------------------
SERVER_API
*server_init(CONF_API *conf, LOG_API *log)
{
 	SERVER_API		*server;
	SERVER 			*id;
	pthread_attr_t 	tattr;

	if ((st_conf = conf) == NULL) return NULL;
	if ((st_log  = log) == NULL) return NULL;
	
	if (conf) conf->parse(module_config);
	
	if ((server = (SERVER_API*) g_malloc0(sizeof(SERVER_API)))==NULL) {
		st_log->print_log(IP_ALERT, "[server.c][server_init]: error allocate memory for SERVER_API struct");
		return NULL;
	}

	if (debug) st_log->print_log(IP_DEBUG, "[server.c][server_init]: create SERVER_API struct [%x]", server);
	
 	if ((st_server = id = (SERVER*) g_malloc0(sizeof(SERVER)))==NULL) {
  		st_log->print_log(IP_ALERT, "[server.c][server_init]: error allocate memory for SERVER struct");
		g_free(server);
		return NULL;
	}
	if (debug) st_log->print_log(IP_DEBUG, "[server.c][server_init]: create SERVER struct [%x]", id);
	
	server->registred	= register_server;
	server->unregistred	= unregister_server;
	
	st_server->jobs = NULL;
	
	pthread_attr_init(&tattr);
	pthread_attr_setstacksize(&tattr, PTHREAD_STACK_MIN + 0x4000);
  	if (pthread_create(&id->id_thread, &tattr, (void*)&thr_server, id)!=0) {
   		st_log->print_log(IP_ERR, "[server.c][server_init]: error creation thread [server] failed");
		g_free(id);
		g_free(server);
		return NULL;
	}
	pthread_attr_destroy(&tattr);
	
	if (debug) st_log->print_log(IP_DEBUG, "[server.c][server_init]: create server thread");
 	return server;
}

//-----------------------------------------------------------------------------
//			    server_free
//-----------------------------------------------------------------------------
IP_result               
server_free(SERVER_API *server) 
{
 	NODE_SERVER_JOBS	*node_job;
	GList				*t_tek, *t_tmp;
	
	CHECK_CORRECT_SERVER;
	
	st_server->do_exit = 1;
	
	pthread_kill(st_server->id_thread, SIGUSR2);
	if (debug) st_log->print_log(IP_DEBUG, "send server signal SIGUSR2"); 

	SERVER_LOCK;
  		t_tmp = st_server->jobs;
  		while ((t_tek = t_tmp)!=NULL){
			t_tmp = g_list_next(t_tmp);
			
			node_job = t_tek->data;
  			if (node_job) g_free(node_job);
  		}
		g_list_free(st_server->jobs);
		st_server->jobs = NULL;
  	SERVER_UNLOCK;

  	free(st_server);
	st_server = NULL;
 	return TRUE;
}

//-----------------------------------------------------------------------------
//			    register_server
//-----------------------------------------------------------------------------
gboolean
register_server(char *name, gboolean (*func)(int fd, int argc, char **argv, void *data), void *data)
{
 	NODE_SERVER_JOBS 	*t_row;
	
	CHECK_CORRECT_SERVER;
	
 	if ((t_row = (NODE_SERVER_JOBS*) g_malloc0(sizeof(NODE_SERVER_JOBS)))==NULL) {
  		st_log->print_log(IP_ALERT, "[server.c][register_server]: error allocate memory for NODE_SERVER_JOBS");
		return FALSE;	
	}
	
	strncpy(t_row->name, name, 50);
	t_row->func		= func;
	t_row->data		= data;
	
 	SERVER_LOCK;
		st_server->jobs = g_list_prepend(st_server->jobs, t_row);
 	SERVER_UNLOCK;

	if (debug) st_log->print_log(IP_DEBUG, "[server.c]: add node addr=%d, func=%x\n", (int) t_row, func);

	return TRUE;
}

//-----------------------------------------------------------------------------
//			    unregister_server
//-----------------------------------------------------------------------------
gboolean
unregister_server(char *name)
{
 	NODE_SERVER_JOBS 	node;
	GList				*ptr;
	
	CHECK_CORRECT_SERVER;
	
	strncpy(node.name, name, 50);

	SERVER_LOCK;
		ptr = g_list_find_custom(st_server->jobs, &node, compare);
		if (ptr) {
			st_server->jobs = g_list_remove(st_server->jobs, ptr->data);
		}
	SERVER_UNLOCK;
	
	return TRUE;
}

//-----------------------------------------------------------------------------
//			    parse_arg
//-----------------------------------------------------------------------------
static void 
parse_arg(char *buffer, int *argc, char *argv[], int max_tok)
{
    int         i = 0;
    char        *ptr;
                 
	if (!*buffer) return;
    ptr = strtok(buffer, " ");                                  // Разбираем на
    argv[i++] = strdup(ptr);
                                                                                
    while ((ptr = strtok('\0', " "))!=NULL) {
		if (i>max_tok) break;
        argv[i++] = strdup(ptr);
    }
                                                                                
    *argc = i;
	return;
}

//-----------------------------------------------------------------------------
//			    get_date
//-----------------------------------------------------------------------------
static void
get_date(time_t s_time, char *date)
{
	struct tm	td;
	
	if (s_time!=0) {	
		localtime_r(&s_time, &td);
		sprintf(date, "%02d.%02d.%02d %02d:%02d:%02d", td.tm_mday, td.tm_mon+1, td.tm_year-100, td.tm_hour, td.tm_min, td.tm_sec);
	}else{
		sprintf(date, "-----------------");		
	}
}
//-----------------------------------------------------------------------------
//			    thr_server
//-----------------------------------------------------------------------------
static void 
thr_server(SERVER *id)
{
 	int 				server_socket, client_socket;
 	int 				status;
 	int 				addrlen;
 	struct sockaddr_in 	server_addr;
 	struct sockaddr_in 	client_addr;
 	int 				reuseaddr_on=1;
 	socket_client 		*cli;
 	int 				server_port;

 	st_log->print_log(IP_INFO, "Start IPStat console server");

// --- инициализируем пул потоков
	init_pool.start_threads = 1;
	init_pool.max_threads = max_clients;
    init_pool.min_spare_threads = 3;
	init_pool.max_spare_threads = 5;
	init_pool.max_requests_per_thread = 0;
	init_pool.cleanup_delay = 30;
		
	if ((thread_pool = IPStat_pool_init(&init_pool, (void*)&thr_clientproc))==NULL) return;
		
	if (debug) st_log->print_log(IP_DEBUG, "port = %s", term_port);
// --- находим нужный порт и садимся на него ---
 	server_port = atoi(term_port);
 	server_port = htons((unsigned short)(server_port));

// --- создаём сокет ---
 	if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){ 
  		st_log->print_log(IP_ALERT, "[server.c][thr_server]: creation of server socket() failed");
		goto l_end;
	}

//--- заполняем структуру server_addr ---
 	bzero((u_char*)(&server_addr), sizeof(server_addr));
 	server_addr.sin_family		= AF_INET;
 	server_addr.sin_addr.s_addr	= INADDR_ANY;
 	server_addr.sin_port		= server_port;

//--- установка параметров сокетов ---
 	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseaddr_on, sizeof(reuseaddr_on));

//--- Биндим сокет ---
 	if ((status = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1) {
  		st_log->print_log(IP_ALERT, "[server.c][thr_server]: bind of server socket failed [%s]", strerror(errno));
		exit(1);
	}

//--- Начинаем слушать сокет ---
 	if ((status = listen(server_socket, -1)) == -1) {
  		st_log->print_log(IP_ALERT, "[server.c][thr_server]: listen of server socket failed");
		exit(1);
	}

 	if (debug) st_log->print_log(IP_DEBUG, "Server is listening at server_socket=%d with status=%d\n", server_socket, status);

 	addrlen = sizeof(client_addr);

 	while(1){
  		bzero((u_char*)(&client_addr), addrlen);

// Ожидаем подключения клиентов
  		if ((client_socket = accept(server_socket, (struct sockaddr*)(&client_addr), &addrlen)) == -1) {
   			st_log->print_log(IP_ALERT,  "[server.c][thr_server]: accept of client failed");
			exit(1);
		}
   		
		if ((cli = (socket_client*) g_malloc0(sizeof(socket_client))) == NULL) {
				st_log->print_log(IP_ALERT,  "[server.c][thr_server]: client structure creation failed\n");
				exit(1);
		}
		cli->client_addr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
   		if (cli->client_addr==NULL) {
    		st_log->print_log(IP_ALERT,  "[server.c][thr_server]: cli->sockaddr_in structure creation failed\n");
			exit(1);
		}

   		bcopy(&client_addr, cli->client_addr, sizeof(client_addr));
   		cli->client_socket = client_socket;

		IPStat_pool_addrequest(thread_pool, cli);
  	}
l_end:
	IPStat_pool_free(thread_pool);
	st_log->print_log(IP_INFO, "Close IPStat console server");
  	pthread_exit(NULL);
}
//-------------------------------------------------------------------------------------------

static void 
thr_clientproc(struct socket_client *cli)
{
 	char 				*buffer;
	char				*argv[20];
	int					argc = 0, j, ret;
	GList				*t_tmp, *t_tek;
	NODE_SERVER_JOBS	*job;
	
 	if ((buffer = (char*) malloc(4*1024+1)) == NULL) {
		st_log->print_log(IP_ALERT,  "[server.c][thr_clientproc]: error allocate memory for buffer\n");
		exit(1);
	}
	
	bzero(buffer, 4*1024);
	
 	if (debug) st_log->print_log(IP_DEBUG, "Client connected from %s\n", inet_ntoa(cli->client_addr->sin_addr));

 	ret = send_to_socket(cli->client_socket, "\nWELCOME to console IPStat %s, build %s %s\n\r", VERSION, (__DATE__), (__TIME__));
	if (ret == FALSE) {
		st_log->print_log(IP_ERR, "error send to socket");
		goto l_end_loop_1;
	}

 	while (st_server->do_exit==0){
  		ret = send_to_socket(cli->client_socket, "IPStat>");
		if (ret == FALSE) {
            	    st_log->print_log(IP_ERR, "error send to socket");
		    goto l_end_loop_1;
		}		
		ret = recv(cli->client_socket, buffer, 4*1024, 0);
    	
		if (ret == -1){ 
   			if (errno==EINTR) continue; 
			st_log->print_log(IP_ERR, "[server.c][thr_clientproc]: failed receive message (%s)", strerror(errno));
			goto l_end_loop_1;
  		}
		
  		buffer[strlen(buffer)-2]=0;
		argc = 0;
		parse_arg(buffer, &argc, argv, 19);
  		
		if (argc==0) goto l_loop;

		if (strncmp(argv[0],"help", 4) == 0) {
   			send_to_socket(cli->client_socket, "\nCommands:\n\r");
   			send_to_socket(cli->client_socket, "***********************************************\n\r");
   			send_to_socket(cli->client_socket, "*************** System Commands ***************\n\r");
   			send_to_socket(cli->client_socket, "***********************************************\n\r");
   			send_to_socket(cli->client_socket, "*  - help          - This is help             *\n\r");
   			send_to_socket(cli->client_socket, "*  - date          - Print current date       *\n\r");
   			send_to_socket(cli->client_socket, "*  - exit or quit  - Exit from console        *\n\r");
   			send_to_socket(cli->client_socket, "***********************************************\n\r");
		}else if (strncmp(argv[0], "date", 4) == 0) {		// command "show"
			char 	buffer[18];
			
			get_date(time(NULL), buffer);
			send_to_socket(cli->client_socket, "Current Date: %s\n\r\n\r", buffer);
			goto l_loop;
  		}else if (strncmp(argv[0], "exit", 4) == 0 || strncmp(argv[0], "quit", 4) == 0) {
   			send_to_socket(cli->client_socket, "Bye Bye!!!\n\n\r");
   			goto l_end_loop;
		}
		
		SERVER_LOCK;
			t_tmp = st_server->jobs;
			while ((t_tek = t_tmp) != NULL) {
				t_tmp = g_list_next(t_tmp);
				
				job = (NODE_SERVER_JOBS*) t_tek->data;
				if (job->func(cli->client_socket, argc, (char**)argv, job->data) == TRUE) {
					SERVER_UNLOCK;
					goto l_loop;
				}
			}
		SERVER_UNLOCK;
		if (strncmp(argv[0],"help", 4) == 0) send_to_socket(cli->client_socket, "***********************************************\n\r\n");
		else send_to_socket(cli->client_socket, "Error: unknow command\n\r");

			
l_loop:		
		for(j=0;j<argc;j++) if (argv[j]) free(argv[j]);
				
  		bzero(buffer, 4*1024);
 	}
	
l_end_loop:
	for(j=0;j<argc;j++) if (argv[j]) free(argv[j]);
	
l_end_loop_1:
 	close(cli->client_socket);

 	free(cli);
	
}


static gint 
compare(gconstpointer a, gconstpointer b)
{
	NODE_SERVER_JOBS	*node1 = (NODE_SERVER_JOBS*) a;
	NODE_SERVER_JOBS	*node2 = (NODE_SERVER_JOBS*) b;
	
	return (gint) strncmp(node1->name, node2->name, 50);
}
