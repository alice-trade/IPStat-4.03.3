/*
 * mysql.c	Function for works with MySql databases.
 *
 * Version:	$Id: main.c,v 1.2 2005/12/24 09:57:22 kornet Exp $
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

static const char rcsid[] = "$Id: main.c,v 1.2 2005/12/24 09:57:22 kornet Exp $";

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
#include "../include/func_time.h"
#include "../include/func_ip.h"
#include "../include/misc.h"
#include "../include/gfunc.h"

#include <bits/local_lim.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static PLUGIN_INFO	module_info;
static pthread_t	ipq_collect;
static int			max_size_buffer = 3000;

static sig_atomic_t	do_exit = 0;
static PLUGIN_API	*api;

int debug = 0;
static CONF_PARSER module_config[] = {
        { "collect_netfilter_size_buffer", CF_TYPE_INT, 0, &max_size_buffer, "2024" },
        { NULL, -1, 0, NULL, NULL }
};



static void thr_ipq(void *data);

static char	*id 		= "401nti";
static char	*name 		= "netfilter_time_ip";
static char	*version 	= "0.1";
static char	*description	= "IPQ + Time collector module for IPStat";
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
    int			stacksize;
	SERVER_API	*server = NULL;
	SCHEDULER_API	*sched  = NULL;
	CONF_API	*conf = NULL;
	LOG_API		*log = NULL;
	STORE_API		*store = NULL;
    
    api = (PLUGIN_API*) data;

	server 	= (SERVER_API*) api->server;
	sched  	= (SCHEDULER_API*) api->sched;
	conf	= (CONF_API*) api->conf;
	log		= (LOG_API*) api->log;
	store		= (STORE_API*) api->store;
	
	if (conf) conf->parse(module_config);
		
    fprintf(stderr, "Start Netfilter Collector...\nSize buffer = %d\n", max_size_buffer);    
    check_netfilter_tables(store, log);
    
    if (pthread_attr_init(&tattr)!=0) {
		log->print_log(IP_ERR, "[netfilter_time_ip]: error init attr pthread");
		return FALSE;
    }
					
    stacksize = (PTHREAD_STACK_MIN + 0x4000);
    if (pthread_attr_setstacksize(&tattr, stacksize)!=0) {
		log->print_log(IP_ERR, "[netfilter_time_ip]: error init attr pthread");
		pthread_attr_destroy(&tattr);
		return FALSE;
    }
    do_exit = 0;									    
    if (pthread_create (&ipq_collect, &tattr, (void*)&thr_ipq, data)!=0) {
        log->print_log(IP_ALERT, "[netfilter_time_ip]: error creation thread [thr_ipq] failed");
        return FALSE;
    }
    pthread_attr_destroy(&tattr);

	time_init(api);
	ip_init(api);
    return TRUE;
}

gboolean
unload(void *data)
{
	LOG_API		*log = NULL;
	
	log		= (LOG_API*) api->log;
//	ip_free();
//	time_free();
    do_exit = 1;
    pthread_kill(ipq_collect, SIGUSR2);
    pthread_join(ipq_collect, NULL);
    log->print_log(IP_INFO, "[netfilter_time_ip]: module unload");
    return TRUE;
}

static void 
thr_ipq(void *data)
{
    CORE_API			*core = api->core;
	LOG_API				*log = api->log;
    
    unsigned char 		*buffer = NULL;
    struct ipq_handle 	*handle_ipq = NULL;
    ipq_packet_msg_t 	*message_ipq = NULL;
    PACKET_INFO			packet_info;
	PACKET_DATA_INFO	packet_data;
    int					status = 0;
    
    log->print_log(IP_INFO, "Thread Netfilter (%x) started\n", (int)pthread_self());
    
    handle_ipq = ipq_create_handle(0, PF_INET);
    if (!handle_ipq) {
		log->print_log(IP_ALERT, "[netfilter_time_ip]: ipq_create_handle (%s)\n", ipq_errstr());
		goto l_end_2;
    }
    
    status = ipq_set_mode(handle_ipq, IPQ_COPY_PACKET, max_size_buffer-1);
    if (status < 0) {
		const char *var[3] = { "modprobe", "ip_queue", NULL };
		int  ret;
		
    		log->print_log(IP_ALERT, "[netfilter_time_ip]: ipq_set_mode (%s)\n", ipq_errstr());
		log->print_log(IP_INFO, "Probe load module ip_queue");
		ret = exec_prog("modprobe", var);
		if (ret == 0) {
			status = ipq_set_mode(handle_ipq, IPQ_COPY_PACKET, max_size_buffer-1);
    		if (status < 0) {
				log->print_log(IP_INFO, "Failed load module ip_queue");
				goto l_end_1;
			}
			log->print_log(IP_INFO, "Load module ip_queue - success");
		}else goto l_end_1;
    }


    buffer = (unsigned char*) malloc(max_size_buffer);
    
    while (do_exit == 0) {
        IP_size	e_len;
	    
    	status = ipq_read(handle_ipq, buffer, max_size_buffer-1, 1000);
		if ((status==0) || ((status==-1) && (errno==EINTR))) continue;

        if (status < 0) {
            log->print_log(IP_ERR, "[netfilter_time_ip]: ipq_read (%s)\n", ipq_errstr());
            continue;
        }
    
		switch (ipq_message_type(buffer)) {
	    	case IPQM_PACKET:
				message_ipq = ipq_get_packet(buffer);
				e_len = ipq_grep_packet(message_ipq, &packet_info);
				if (core->auth(SERVICE_TRAFFIC, &packet_info, TRUE, &packet_data) == TRUE) {
					packet_data.len = e_len;
			    	    if (core->autz(SERVICE_TRAFFIC, &packet_info, TRUE, &packet_data) == TRUE) {
					core->acct(SERVICE_TRAFFIC, &packet_info, TRUE, &packet_data);
					time_acct(&packet_data);
					ip_acct(&packet_info, &packet_data);
					status = ipq_set_verdict(handle_ipq, message_ipq->packet_id, NF_ACCEPT, 0, NULL);
			    	    }else status = ipq_set_verdict(handle_ipq, message_ipq->packet_id, NF_DROP, 0, NULL);
				}else status = ipq_set_verdict(handle_ipq, message_ipq->packet_id, NF_DROP, 0, NULL);

				status = ipq_set_verdict(handle_ipq, message_ipq->packet_id, NF_ACCEPT, 0, NULL);
				if (status < 0) {
		    		log->print_log(IP_ERR, "[netfilter_time_ip]: ipq_set_verdict (%s)", ipq_errstr());
					goto l_end_1;
				}
				break;
		}
    }    
l_end_1:
    ipq_destroy_handle(handle_ipq);
l_end_2:
    free(buffer);
    log->print_log(IP_INFO, "Thread %d exit \n", (int) pthread_self());
    pthread_exit(NULL);
}
