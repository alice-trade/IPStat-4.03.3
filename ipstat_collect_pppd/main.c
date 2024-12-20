/*
 * main.c	Collector module for export pppd traffic 
 *
 * Version:	$Id: main.c,v 1.5 2006/02/07 05:50:55 kornet Exp $
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

static const char rcsid[] = "$Id: main.c,v 1.5 2006/02/07 05:50:55 kornet Exp $";

#include "../include/sched.h"
#include "../include/collectors.h"
#include "../include/types.h"
#include "../include/plugin.h"
#include "../include/conf.h"
#include "../include/log.h"
#include "../include/core.h"
#include "../include/store.h"
#include "../include/inet.h"
#include "../include/store.h"
#include "../include/server.h"
#include "../include/func_squid.h"
#include "../include/misc.h"
#include "../include/threads.h"
#include "../include/libxml.h"

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/uio.h>

#ifndef _GNU_SOURCE
 #define _GNU_SOURCE
#endif

#include <string.h>

typedef struct accepted_client
{
        struct sockaddr_un      		*client_addr;
        int                             client_socket;
        FILE                            *client_fd;
		PLUGIN_API						*api;
} accepted_client;

static PLUGIN_INFO	module_info;

static pthread_t	id_pppd;

IP_size				total_query = 0;

//static pthread_mutex_t 	pppd_var_mutex = PTHREAD_MUTEX_INITIALIZER;

#define PPPD_VAR_LOCK		pthread_mutex_lock(&pppd_var_mutex)
#define PPPD_VAR_UNLOCK		pthread_mutex_unlock(&pppd_var_mutex)

static char			pppd_connect[FILENAME_MAX];
static int			max_servers;

int				send_msg(int sock, char *msg);
int				recive_msg(int sock, char *msg, size_t size);

static sig_atomic_t	do_exit = 0;
static PLUGIN_API	*api;

static INIT_POOL 	init_pool;				// структура начальной инициализации
static THREAD_POOL	*thread_pool;

int debug = 0;
static CONF_PARSER module_config[] = {
        { "collect_pppd_socket", CF_TYPE_STRING, FILENAME_MAX-1, pppd_connect, "/opt/IPStat/tmp/ipstat_pppd.socket" },
        { "collect_pppd_max_servers", CF_TYPE_INT, 0, &max_servers, "10" },
        { NULL, -1, 0, NULL, NULL }
};

static void 	thr_pppd(void *data);
static gboolean	thr_pppd_client(void *data);
static void		parse_socket(char*, char*, char*, int*, int);


static char	*id 		= "100isi";
static char	*name 		= "pppd";
static char	*version 	= "0.1";
static char	*description	= "PPPD collector module for IPStat";
static char	*author		= "Kukuev Dmitry";
static char	*homepage	= "http://ipstat.code-art.ru/";

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
    module_info.type		= IPSTAT_PLUGIN_COLLECTOR;
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
    pthread_attr_t 	tattr;
	SERVER_API		*server = NULL;
	SCHEDULER_API	*sched  = NULL;
	CONF_API		*conf = NULL;
	LOG_API			*log = NULL;
 	STORE_API		*store = NULL;   
    
	api = (PLUGIN_API*) data;
	server 	= (SERVER_API*) api->server;
	sched  	= (SCHEDULER_API*) api->sched;
	conf	= (CONF_API*) api->conf;
	log		= (LOG_API*) api->log;
	store	= (STORE_API*) api->store;
	
	if (conf) conf->parse(module_config);
		
    fprintf(stderr, "Start PPPD Collector...\n");
	
    if (pthread_attr_init(&tattr)!=0) {
		log->print_log(IP_ERR, "[pppd]: error init attr pthread (%s)", strerror(errno));
		return FALSE;
    }

    if (pthread_attr_setstacksize(&tattr, PTHREAD_STACK_MIN + 0x8000)!=0) {
		log->print_log(IP_ERR, "[pppd]: error init attr pthread (%s)", strerror(errno));
		pthread_attr_destroy(&tattr);
		return FALSE;
    }
    do_exit = 0;									    
    if (pthread_create (&id_pppd, &tattr, (void*)&thr_pppd, data)!=0) {
        log->print_log(IP_ALERT, "[pppd]: creation thread [thr_pppd] failed (%s)", strerror(errno));
        return FALSE;
    }

    pthread_attr_destroy(&tattr);
	
    return TRUE;
}

gboolean
unload(void *data)
{
	LOG_API		*log = NULL;
	
	log		= (LOG_API*) api->log;

    do_exit = 1;
    pthread_cancel(id_pppd);
    pthread_join(id_pppd, NULL);
    log->print_log(IP_INFO, "[pppd]: module unload");
    return TRUE;
}

static void 
thr_pppd(void *data)
{
	LOG_API				*log  = (LOG_API*) api->log;
	int					addrlen;
	int					server_socket, client_socket, res;
	char				*pppd_tmp, pppd_host[100], pppd_socket[100];
	int					pppd_port;	
	accepted_client		*cli;
	
	log->print_log(IP_INFO, "Thread PPPD (%x) started\n", (int)pthread_self());

	init_pool.start_threads = 2;
	init_pool.max_threads = max_servers;
	init_pool.min_spare_threads = 3;
	init_pool.max_spare_threads = 5;
	init_pool.max_requests_per_thread = 0;
	init_pool.cleanup_delay = 30;
	
	if ((thread_pool = IPStat_pool_init(&init_pool, thr_pppd_client))==NULL) {
		log->print_log(IP_ERR, "failed init threads pool");
		goto l_end;
	}
	
	pppd_tmp = strdup(pppd_connect);
		parse_socket(pppd_tmp, pppd_socket, pppd_host, &pppd_port, 99);
	free(pppd_tmp);
	
	if (*pppd_socket) {
		struct sockaddr_un	server_addr, client_addr;
			
		res = unlink(pppd_socket);
	
	
		if((server_socket = socket(PF_UNIX, SOCK_STREAM, 0))==-1 ) {
			log->print_log(IP_ERR, "[pppd]: creation of server 'socket' failed (%s)", strerror(errno));
			goto l_end;
		}

		bzero((u_char*)(&server_addr), sizeof(server_addr));

		server_addr.sun_family = AF_UNIX;
		strncpy(server_addr.sun_path, pppd_socket, UNIX_PATH_MAX-1);

		if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
			log->print_log(IP_ERR, "[pppd]: bind of server socket failed (%s)", strerror(errno));
			goto l_end;
		}
	
		if(chmod(pppd_socket, S_IROTH+S_IWOTH+S_IXOTH)==-1) {
			log->print_log(IP_ERR, "[pppd]: chmod failed (%s)", strerror(errno));
			goto l_end;
		}
	
		if(listen(server_socket, 100) == -1) {
			log->print_log(IP_ERR, "[pppd]: listen of server socket failed (%s)", strerror(errno));
			goto l_end;
		}

		addrlen = sizeof(client_addr);
	
		while(do_exit == 0) {
			bzero((u_char*)(&client_addr), addrlen);
l_loop:
			client_socket = accept(server_socket, (struct sockaddr*)(&client_addr), (socklen_t*)&addrlen);

			if (client_socket == -1) {
				if (errno == EINTR) continue;
				log->print_log(IP_ERR, "[pppd]: accept of client failed (%s)", strerror(errno));
				goto l_loop;
			}
			cli = (accepted_client*) malloc(sizeof(accepted_client));
			bzero(cli, sizeof(accepted_client));
			if (cli == NULL) {
				log->print_log(IP_ERR, "[pppd]: client structure creation failed (%s)", strerror(errno));
				goto l_end;
			}

			cli->client_socket = client_socket;
			cli->api = api;
			if (IPStat_pool_addrequest(thread_pool, cli)!=TRUE) {
				log->print_log(IP_ERR, "[pppd]: failed addrequest to pool");
			}
		}
	} else {
		int 				server_port;
		struct sockaddr_in 	server_addr, client_addr;
		int 				reuseaddr_on=1;
		
 		server_port = htons((unsigned short)(pppd_port));

 		if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){ 
  			log->print_log(IP_ERR, "[pppd]: creation of server socket() failed");
			goto l_end;
		}

 		bzero((u_char*)(&server_addr), sizeof(server_addr));
 		server_addr.sin_family		= AF_INET;
 		server_addr.sin_addr.s_addr	= INADDR_ANY;
 		server_addr.sin_port		= server_port;

 		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseaddr_on, sizeof(reuseaddr_on));

 		if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
  			log->print_log(IP_ALERT, "[server.c][thr_server]: bind of server socket failed [%s]", strerror(errno));
			exit(1);
		}
		if(listen(server_socket, 100) == -1) {
			log->print_log(IP_ERR, "[pppd]: listen of server socket failed (%s)", strerror(errno));
			goto l_end;
		}

		addrlen = sizeof(client_addr);
	
		while(do_exit == 0) {
			bzero((u_char*)(&client_addr), addrlen);
l_loop1:
			client_socket = accept(server_socket, (struct sockaddr*)(&client_addr), (socklen_t*)&addrlen);

			if (client_socket == -1) {
				if (errno == EINTR) continue;
				log->print_log(IP_ERR, "[pppd]: accept of client failed (%s)", strerror(errno));
				goto l_loop1;
			}
			cli = (accepted_client*) malloc(sizeof(accepted_client));
			bzero(cli, sizeof(accepted_client));
			if (cli == NULL) {
				log->print_log(IP_ERR, "[pppd]: client structure creation failed (%s)", strerror(errno));
				goto l_end;
			}

			cli->client_socket = client_socket;
			cli->api = api;
			if (IPStat_pool_addrequest(thread_pool, cli)!=TRUE) {
				log->print_log(IP_ERR, "[pppd]: failed addrequest to pool");
			}
		}
	}

l_end:	
	log->print_log(IP_INFO, "Thread %d exit\n", (int) pthread_self());
    pthread_exit(NULL);
}


static gboolean
thr_pppd_client(void *data)
{
    char    		*buffer = NULL;
    accepted_client	*cli = NULL;
    LOG_API		*log = NULL;
    CORE_API		*core = NULL;
    STORE_API		*store = NULL;
    
    XML_SESSION		*sid;

    char 		cmd[50];
    char		login[_MAX_LENGTH_LOGIN];
    char		mac[_MAX_LENGTH_MAC];
    char		remote_number[_MAX_LENGTH_IP];
    SQL_SESSION		*ssid;
    SQL_ROW		row;		
    
    if ((buffer = (char*) malloc(_MAX_LENGTH_BUFFER)) == NULL) return FALSE;
    bzero(buffer, _MAX_LENGTH_BUFFER);
    if (api)  log = (LOG_API*) api->log;
    if (api)  core = (CORE_API*) api->core;
    if (api)  store = (STORE_API*) api->store;
    if (data) cli = (accepted_client*) data;
    

    if (recive_msg(cli->client_socket, buffer, _MAX_LENGTH_BUFFER-1) == 0) {
	free(buffer);
	return FALSE;
    }
	
    if (shutdown(cli->client_socket, SHUT_RD) == -1) {
	log->print_log(IP_ERR, "failed in 'shutdown' (%s)", strerror(errno));	
	goto end_label2;
    }

    if ((sid = xpath_init(buffer)) == NULL) {
	log->print_log(IP_ERR, "failed in 'xpath_init' (%s)", strerror(errno));	
	goto end_label2;
    }

    if (xpath_get_param(sid, "cmd", cmd, 50) == FALSE) {
	log->print_log(IP_ERR, "failed in 'xpath_get_param (cmd)' (%s)", strerror(errno));	
	goto end_label2;
    }

    xpath_get_param(sid, "login", login, _MAX_LENGTH_LOGIN);
    xpath_get_param(sid, "remote_number", remote_number, _MAX_LENGTH_IP);
    xpath_get_param(sid, "mac", mac, _MAX_LENGTH_MAC);
	
    xpath_free(sid);
	
	
    if (strncmp(cmd, "auth", 4) == 0) {
	char 	passwd[_MAX_LENGTH_PASSWD];

        bzero(passwd, _MAX_LENGTH_PASSWD);
	if ((sid = tree_init()) == NULL) {
	    log->print_log(IP_ERR, "failed in 'tree_init(auth)' (%s)", strerror(errno));	
	    goto end_label2;
	}
	
	if ((ssid = store->open())!=NULL) {
	    char buffer1[2048];
	    
            sprintf(buffer1, "select Passwd from Users where Login = '%s'", login);
#ifdef _DEBUG
            if (debug) st_log->print_log(IP_DEBUG, "%s", buffer);
#endif
            if (!store->query(ssid, buffer1)) {
                log->print_log(IP_ERR, "[pppd][check_user]: error execute query");
    	        store->close(ssid);
    		goto l_go;
            }
            if ((row = store->fetch_row(ssid)) == NULL) {
        	store->close(ssid);
                goto l_go;
            }else{
                strncpy(passwd, row[0], _MAX_LENGTH_PASSWD-1);
            }
	    store->close(ssid);
	}
l_go:	
	if (!*passwd) {
	    tree_addnode(sid, "error_code", "ERR");
	    tree_addnode(sid, "message", "error login");
	}else{
	    tree_addnode(sid, "error_code", "OK");
	    tree_addnode(sid, "password", passwd);
	    tree_addnode(sid, "message", "Permission accept");		
	}
	tree_getxml(sid, buffer, _MAX_LENGTH_BUFFER);
	tree_free(sid);

	if (send_msg(cli->client_socket, buffer) == -1) {
	    log->print_log(IP_ERR, "failed in 'send_mes(autz)' (%s)", strerror(errno));	
	    goto end_label2;
	}			
    } else if (strncmp(cmd, "autz", 4) == 0) {
	IP_addr	ip = 0;
	
	if ((sid = tree_init()) == NULL) {
	    log->print_log(IP_ERR, "failed in 'tree_init(auth)' (%s)", strerror(errno));	
	    goto end_label2;
	}
	
	if ((ssid = store->open())!=NULL) {
	    char buffer1[2048];
	    
            sprintf(buffer1, "select Addr from Users where Login = '%s'", login);
#ifdef _DEBUG
            if (debug) st_log->print_log(IP_DEBUG, "%s", buffer);
#endif
            if (!store->query(ssid, buffer1)) {
                log->print_log(IP_ERR, "[pppd][check_user]: error execute query");
    	        store->close(ssid);
    		goto l_go1;
            }
            if ((row = store->fetch_row(ssid)) == NULL) {
        	store->close(ssid);
                goto l_go1;
            }else{
        	hostbyip(row[0], &ip);
            }
	    store->close(ssid);
	}
l_go1:	
	tree_addnode(sid, "error_code", "OK");
	tree_addnode(sid, "session_id", "1111");
	tree_addnode(sid, "message", "Permission accept");

	if (ip != 0) {
		char tmp[10];
		
	    sprintf(tmp, "%u", ip);
	    tree_addnode(sid, "framed_address", tmp);
	}

	tree_getxml(sid, buffer, _MAX_LENGTH_BUFFER);
	tree_free(sid);

	if (send_msg(cli->client_socket, buffer) == -1) {
	    log->print_log(IP_ERR, "failed in 'send_mes(autz)' (%s)", strerror(errno));	
	    goto end_label2;
	}			
    } else if (strncmp(cmd, "start", 5) == 0) {
	if ((sid = tree_init()) == NULL) {
	    log->print_log(IP_ERR, "failed in 'tree_init(auth)' (%s)", strerror(errno));	
	    goto end_label2;
	}
	tree_addnode(sid, "error_code", "OK");
	tree_addnode(sid, "session_id", "111111111");
	tree_addnode(sid, "message", "Permission accept");
	tree_getxml(sid, buffer, _MAX_LENGTH_BUFFER);
	tree_free(sid);

	if (send_msg(cli->client_socket, buffer) == -1) {
	    log->print_log(IP_ERR, "failed in 'send_mes(start)' (%s)", strerror(errno));	
	    goto end_label2;
	}			
    } else if (strncmp(cmd, "stop", 4) == 0) {
	if ((sid = tree_init()) == NULL) {
	    log->print_log(IP_ERR, "failed in 'tree_init(auth)' (%s)", strerror(errno));	
	    goto end_label2;
	}
	tree_addnode(sid, "error_code", "OK");
	tree_addnode(sid, "session_id", "111111111");
	tree_addnode(sid, "message", "Permission accept");
	tree_getxml(sid, buffer, _MAX_LENGTH_BUFFER);
	tree_free(sid);

	if (send_msg(cli->client_socket, buffer) == -1) {
	    log->print_log(IP_ERR, "failed in 'send_mes(stop)' (%s)", strerror(errno));	
	    goto end_label2;
	}			
    } else if (strncmp(cmd, "interim", 7)==0) {
	if ((sid = tree_init()) == NULL) {
	    log->print_log(IP_ERR, "failed in 'tree_init(auth)' (%s)", strerror(errno));	
	    goto end_label2;
	}
	tree_addnode(sid, "error_code", "OK");
	tree_addnode(sid, "session_id", "111111111");
	tree_addnode(sid, "message", "Permission accept");
	tree_getxml(sid, buffer, _MAX_LENGTH_BUFFER);
	tree_free(sid);

	if (send_msg(cli->client_socket, buffer) == -1) {
	    log->print_log(IP_ERR, "failed in 'send_mes(interim)' (%s)", strerror(errno));	
	    goto end_label2;
	}			
    }
			
end_label2:	
    free(buffer);

    close(cli->client_socket);
    free(data);
    return TRUE;
}

static void
parse_socket(char *buffer, char *pppd_socket, char *pppd_host, int *pppd_port, int strlen)
{
	char *ptr, *cp;
	
	ptr = buffer;
	
	if ((cp = (char*) strchr(buffer, ':'))!=NULL) {
		*cp = '\0';
		strcpy(pppd_host, ptr);
		bzero(pppd_socket, strlen);
		*pppd_port = atoi(cp+1);
	} else {
		bzero(pppd_host, strlen);
		*pppd_port = 0;
		strcpy(pppd_socket, buffer);
	}
}

int
send_msg(int sock, char *msg)
{
        int ret;
                                                                                                                                                             
loop:
        ret = send(sock, msg, strlen(msg), 0);
                                                                                                                                                             
        if (ret == -1) {
                if (errno == EINTR) goto loop;
                return -1;
        }
                                                                                                                                                             
        if (ret != strlen(msg)) return -1;
        return ret;
}

int
recive_msg(int sock, char *msg, size_t size)
{
    int res;

label_1:
    res = recv(sock, msg, size, 0);

    if (res == -1) {
        if (errno == EINTR) goto label_1;
        return 0;
    }
    if (res == strlen(msg)) {
    	return 1;
    }
    return 0;
}
