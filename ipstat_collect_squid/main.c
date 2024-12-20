/*
 * main.c	Collector module for export squid traffic 
 *
 * Version:	$Id: main.c,v 1.2 2005/12/24 09:58:14 kornet Exp $
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

static const char rcsid[] = "$Id: main.c,v 1.2 2005/12/24 09:58:14 kornet Exp $";

#include "../include/sched.h"
#include "../include/collectors.h"
#include "../include/types.h"
#include "../include/plugin.h"
#include "../include/conf.h"
#include "../include/log.h"
#include "../include/core.h"
#include "../include/inet.h"
#include "../include/store.h"
#include "../include/server.h"
#include "../include/func_squid.h"
#include "../include/misc.h"
#include "../include/threads.h"

#include <string.h>
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

#define FIFO_CONNECT            TRUE
#define FIFO_NOT_CONNECT        FALSE

typedef struct accepted_client
{
        struct sockaddr_un      		*client_addr;
        int                             client_socket;
        FILE                            *client_fd;
		PLUGIN_API						*api;
} accepted_client;

static PLUGIN_INFO	module_info;

static pthread_t	id_auth_squid;
static pthread_t	id_acct_squid;

IP_size				total_auth_query = 0;
IP_size				success_auth_query = 0;
IP_size				failed_auth_query = 0;

static pthread_mutex_t 	squid_var_mutex = PTHREAD_MUTEX_INITIALIZER;

#define SQUID_VAR_LOCK		pthread_mutex_lock(&squid_var_mutex)
#define SQUID_VAR_UNLOCK	pthread_mutex_unlock(&squid_var_mutex)

gboolean			status_fifo = FIFO_NOT_CONNECT;
gboolean			status_socket = FALSE;

static char			squid_socket[FILENAME_MAX];
static char			squid_copy_log[FILENAME_MAX];
static char			redirect_socket[FILENAME_MAX];
static char			deny_url[201];
static int			max_servers;



static sig_atomic_t	do_exit = 0;
static PLUGIN_API	*api;

static INIT_POOL 	init_pool;				// структура начальной инициализации
static THREAD_POOL	*thread_pool;

int debug = 0;
static CONF_PARSER module_config[] = {
        { "collect_squid_socket", CF_TYPE_STRING, FILENAME_MAX, squid_socket, "/opt/IPStat/tmp/ipstat_squid.socket" },
        { "collect_squid_copy", CF_TYPE_STRING, FILENAME_MAX, squid_copy_log, "" },
        { "collect_squid_max_servers", CF_TYPE_INT, 0, &max_servers, "10" },
        { "collect_squid_redirect_socket", CF_TYPE_STRING, FILENAME_MAX, redirect_socket, "/opt/IPStat/tmp/ipstat_redirect.socket" },
        { "collect_squid_deny_url", CF_TYPE_STRING, 200, deny_url, "http://localhost/warning/" },
        { NULL, -1, 0, NULL, NULL }
};



static void 	thr_auth_squid(void *data);
static void 	thr_acct_squid(void *data);
static gboolean	thr_squid_clientredirector(void *data);
static gboolean	save_copy(char *str);
static void		parse_arg(char *buffer, int *argc, char *argv[], int max_tok);


static char	*id 		= "401nti";
static char	*name 		= "squid";
static char	*version 	= "0.1";
static char	*description	= "Squid collector module for IPStat";
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
		
    fprintf(stderr, "Start Squid Collector...\n");
	
	if (check_squid_tables(store, log) != TRUE) {
		log->print_log(IP_ERR, "[squid.c]: failed 'check_squid_tables'");
		return FALSE;
	}

	load_url_policy(data);
	
    if (pthread_attr_init(&tattr)!=0) {
		log->print_log(IP_ERR, "[squid]: error init attr pthread (%s)", strerror(errno));
		return FALSE;
    }

    if (pthread_attr_setstacksize(&tattr, PTHREAD_STACK_MIN + 0x8000)!=0) {
		log->print_log(IP_ERR, "[squid]: error init attr pthread (%s)", strerror(errno));
		pthread_attr_destroy(&tattr);
		return FALSE;
    }
    do_exit = 0;									    
    if (pthread_create (&id_auth_squid, &tattr, (void*)&thr_auth_squid, data)!=0) {
        log->print_log(IP_ALERT, "[squid]: creation thread [thr_auth_squid] failed (%s)", strerror(errno));
        return FALSE;
    }
    if (pthread_create (&id_acct_squid, &tattr, (void*)&thr_acct_squid, data)!=0) {
        log->print_log(IP_ALERT, "[squid]: creation thread [thr_squid] failed (%s)", strerror(errno));
        return FALSE;
    }

    pthread_attr_destroy(&tattr);
	
	if (squid_init(api) != TRUE) {
        log->print_log(IP_ALERT, "[squid]: init squid functions failed");
        return FALSE;
	}
	
    return TRUE;
}

gboolean
unload(void *data)
{
	LOG_API		*log = NULL;
	
	log		= (LOG_API*) api->log;
	
	do_exit = 1;
//    pthread_kill(id_auth_squid, SIGUSR2);
//    pthread_kill(id_acct_squid, SIGUSR2);
    pthread_cancel(id_auth_squid);
    pthread_cancel(id_acct_squid);
    pthread_join(id_auth_squid, NULL);
    pthread_join(id_acct_squid, NULL);
	
	unload_url_policy(data);
	squid_free();
	
    log->print_log(IP_INFO, "[squid]: module unload");
    return TRUE;
}

static void 
thr_auth_squid(void *data)
{
	LOG_API				*log  = (LOG_API*) api->log;
	struct sockaddr_un	server_addr, client_addr;
	int					addrlen;
	int					server_socket, client_socket, res;
	accepted_client		*cli;	
	
	
	log->print_log(IP_INFO, "Thread Squid (auth) (%x) started\n", (int)pthread_self());

	// --- инициализируем пул потоков
	init_pool.start_threads = 2;
	init_pool.max_threads = max_servers;
    init_pool.min_spare_threads = 3;
	init_pool.max_spare_threads = 5;
	init_pool.max_requests_per_thread = 0;
	init_pool.cleanup_delay = 30;
	
	if ((thread_pool = IPStat_pool_init(&init_pool, thr_squid_clientredirector))==NULL) {
		log->print_log(IP_ERR, "failed init threads pool");
		goto l_end;
	}

	res = unlink(redirect_socket);
	
	if (res == -1) {
		log->print_log(IP_ERR, "[squid]: unlink failed (%s)", strerror(errno));
	}
	if((server_socket = socket(PF_UNIX, SOCK_STREAM, 0))==-1 ) {
		log->print_log(IP_ERR, "[squid]: creation of server 'socket' failed (%s)", strerror(errno));
		goto l_end;
	}

	bzero((u_char*)(&server_addr), sizeof(server_addr));

	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, redirect_socket, UNIX_PATH_MAX);

	if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		log->print_log(IP_ERR, "[squid]: bind of server socket failed (%s)", strerror(errno));
		goto l_end;
	}
	
	if(chmod(redirect_socket, S_IROTH+S_IWOTH+S_IXOTH)==-1) {
		log->print_log(IP_ERR, "[squid]: chmod failed (%s)", strerror(errno));
		goto l_end;
	}

	if(listen(server_socket, 100) == -1) {
		log->print_log(IP_ERR, "[squid]: listen of server socket failed (%s)", strerror(errno));
		goto l_end;
	}

	addrlen = sizeof(client_addr);
	status_socket = TRUE;
	
	while(do_exit == 0) {
		bzero((u_char*)(&client_addr), addrlen);
		client_socket = accept(server_socket, (struct sockaddr*)(&client_addr), &addrlen);

		if (client_socket == -1) {
			if (errno == EINTR) continue;
			log->print_log(IP_ERR, "[squid]: accept of client failed (%s)", strerror(errno));
			continue;
		}
		cli = (accepted_client*) malloc(sizeof(accepted_client));
		memset((void*)cli, '\0', sizeof(accepted_client));
		if (cli == NULL) {
			log->print_log(IP_ERR, "[squid.c]: client structure creation failed (%s)", strerror(errno));
			goto l_end;
		}

		cli->client_socket = client_socket;
		cli->api = api;
		if (IPStat_pool_addrequest(thread_pool, cli)!=TRUE) {
			log->print_log(IP_ERR, "[squid.c]: failed addrequest to pool");
		}
	}

l_end:	
	status_socket = FALSE;
	log->print_log(IP_INFO, "Thread %d exit\n", (int) pthread_self());
    pthread_exit(NULL);
}

static void 
thr_acct_squid(void *data)
{
    CORE_API		*core	= (CORE_API*) api->core;
    LOG_API		*log	= (LOG_API*) api->log;
    unsigned char 	*buffer = NULL;
    char		*tmp_buf= NULL;
    char                *argv[20];
    int                 argc 	= 0, i;
    
    FILE		*fd 	= NULL;
    SQUID_INFO		squid_info;
    SQUID_DATA_INFO	squid_data;
	
    log->print_log(IP_INFO, "Thread Squid (acct) (%u) started\n", (int)pthread_self());

    if (squid_socket==NULL){
	log->print_log(IP_ERR, "[squid_collect][thr_acct_squid]: error squid_socket=NULL");
	goto l_end2;
    }

    if ((buffer = (unsigned char*)malloc(2048)) == NULL) {
	log->print_log(IP_ERR, "[squid_collect][thr_acct_squid]: error malloc memory");
	goto l_end1;
    }
    memset((void*)buffer, '\0', 2048);
    
start:
    status_fifo = FIFO_NOT_CONNECT;

    log->print_log(IP_INFO, "[squid]: Use fifo-file = '%s'", squid_socket);
    if (*squid_copy_log) log->print_log(IP_INFO, "[squid]: Copy log to file = '%s'", squid_copy_log);
			
    if ((fd = fifo_open(api, squid_socket))!=NULL) {
	status_fifo = FIFO_CONNECT;
	log->print_log(IP_INFO, "[squid]: connect to fifo-file");
				
	while(!feof(fd)) {
	    int 	res;
					
	    res = fifo_read(fd, buffer, 2023, 10000);
	    if (do_exit!=0) {
		goto l_end;
	    }
	    if (res == -1) continue;
	    if (res == 0) break;
	    if (res == -2) {
		log->print_log(IP_ERR, "error fifo-read (%s)", strerror(errno));
		break;
	    }
	    if (*squid_copy_log) save_copy(buffer);
	    if ((tmp_buf = strdup(buffer)) == NULL) {
		log->print_log(IP_ERR, "[squid]: failed in 'strdup' (%s)", strerror(errno));
                continue;
            }
            argc = 0;
            ip_parse_arg(tmp_buf, &argc, argv, 20, " ");
            free(tmp_buf);
                
	    if (argc < 7){
            	log->print_log(IP_ERR, "[squid]: not correct count result with fifo-file (%s)", buffer);
            	for(i=0;i<argc;i++) if (argv[i]) free(argv[i]);
		continue;
	    }	
	    if ((strstr(argv[3], "MISS"))!=NULL) {
		hostbyip(argv[2], &squid_info.ip);
		squid_data.len	= atol(argv[4]);
		if (core->auth(SERVICE_SQUID, &squid_info, TRUE, &squid_data) == TRUE) {
		    if (core->autz(SERVICE_SQUID, &squid_info, TRUE, &squid_data) == TRUE){
			if (autz_url_policy(squid_data.id_user, argv[6]) == TRUE) {
			    core->acct(SERVICE_SQUID, &squid_info, TRUE, &squid_data);
			    squid_acct(argv[6], &squid_data);
			}
		    }
		}
	    }
	    for(i=0;i<argc;i++) if (argv[i]) free(argv[i]);
	}
    } else {
	log->print_log(IP_ERR, "[squid]: failed open fifo-file");
    }	
    sleep(10);
    goto start;

l_end:
    fifo_close(api, fd);   
l_end1:	
    free(buffer);
l_end2:
    status_fifo = FIFO_NOT_CONNECT;	
    log->print_log(IP_INFO, "Thread %d exit\n", (int) pthread_self());
    pthread_exit(NULL);
}

static gboolean
save_copy(char *str)
{	
	FILE *fd;
	
	if ((fd = fopen(squid_copy_log, "a"))!=NULL) {
		fwrite(str, strlen(str), 1, fd);
		fclose(fd);
	}else return FALSE;
	return TRUE;
}

static gboolean
thr_squid_clientredirector(void *data)
{
	char    		*buffer = NULL;
	accepted_client	*cli = NULL;
	LOG_API			*log = NULL;
	CORE_API		*core = NULL;
	char			*argv[6], *cp, *tmp_buf;
	int				argc = 0, j, res;
	SQUID_INFO     	squid_info;
	SQUID_DATA_INFO	squid_data;
    
	if ((buffer = (char*) malloc(2048))==NULL) goto end_label3;
	
	memset((void*)buffer, '\0', 2048);
	
	if (api)  log = (LOG_API*) api->log;
	if (api)  core = (CORE_API*) api->core;
	if (data) cli = (accepted_client*) data;
		
l_next:	
	res = recv(cli->client_socket, buffer, 2048, 0);

	if (res == -1) {
		if (errno == EINTR) goto l_next;
		log->print_log(IP_ERR, "failed in 'recv' (%s)", strerror(errno));	
		goto end_label2;
	}
	
	res = shutdown(cli->client_socket, SHUT_RD);
	
	if (res == -1) {
		log->print_log(IP_ERR, "failed in 'shutdown' (%s)", strerror(errno));	
		goto end_label2;
	}
			
	if ((tmp_buf = strdup(buffer)) == NULL) return FALSE;
				
	argc = 0;
	parse_arg(tmp_buf, &argc, argv, 5);
	free(tmp_buf);
			
	if (argc<2) {
		log->print_log(IP_ERR, "[squid_clientredirector]: error count result with redirect_query\n");
		goto end_label;
	}
	if ((cp = (char*) strchr(argv[1], '/'))!=NULL) *cp = '\0';

//	bzero(&packet, sizeof(PACKET));
	hostbyip(argv[1], &squid_info.ip);
			
	if (core->auth(SERVICE_SQUID, &squid_info, TRUE, &squid_data) == TRUE) {
		if (core->autz(SERVICE_SQUID, &squid_info, TRUE, &squid_data) == TRUE){
			if (autz_url_policy(squid_data.id_user, argv[0]) == TRUE) {
				res = send(cli->client_socket, buffer, strlen(buffer), 0);
			
				if (res == -1) {
					log->print_log(IP_ERR, "failed in 'send' (%s)", strerror(errno));	
					goto end_label2;
				}
				SQUID_VAR_LOCK;
					total_auth_query++;
					success_auth_query++;
				SQUID_VAR_UNLOCK;
				goto end_label;
			}
		}
	}
	SQUID_VAR_LOCK;
		total_auth_query++;
		failed_auth_query++;
	SQUID_VAR_UNLOCK;
	
	sprintf(buffer,"%s\n", deny_url);
	res = send(cli->client_socket, buffer, strlen(buffer), 0);
	if (res == -1) {
		log->print_log(IP_ERR, "failed in 'send' (%s)", strerror(errno));	
		goto end_label2;
	}
	

end_label:
	for(j=0;j<argc;j++) if (argv[j]) free(argv[j]);
		
end_label2:	
	free(buffer);
end_label3:
    close(cli->client_socket);
    free(data);
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
