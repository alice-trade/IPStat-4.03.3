/*
 * main.c	PCAP Collector
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

#ifdef HAVE_CONFIG_H 
#include <config.h> 
#endif


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


#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <netinet/ip.h>
#include <netinet/in.h>

#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>

#include <pcap.h>


static PLUGIN_INFO	module_info;
static pthread_t	pcap_collect;
static int			max_size_buffer;
static char			name_dev[20];

static sig_atomic_t	do_exit = 0;
static PLUGIN_API	*api;

struct sniff_ethernet {
    u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
    u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
    u_short ether_type; /* IP? ARP? RARP? etc */
};

int debug = 0;
static CONF_PARSER module_config[] = {
        { "collect_pcap_size_buffer", CF_TYPE_INT, 0, &max_size_buffer, "2024" },
        { "collect_pcap_lookup_dev", CF_TYPE_STRING, 19, name_dev, "eth0" },
        { NULL, -1, 0, NULL, NULL }
};



static void thr_pcap(void *data);

static char	*id 		= "401pti";
static char	*name 		= "pcap_time_ip";
static char	*version 	= "0.1";
static char	*description	= "Pcap collector module for IPStat with IP+Time agregate";
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
		
    fprintf(stderr, "Start PCAP Collector...\nSize buffer = %d\n", max_size_buffer);    
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
    if (pthread_create (&pcap_collect, &tattr, (void*)&thr_pcap, data)!=0) {
        log->print_log(IP_ALERT, "[netfilter_time_ip]: error creation thread [thr_pcap] failed");
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
	ip_free();
	time_free();
    do_exit = 1;
    pthread_kill(pcap_collect, SIGUSR2);
    pthread_join(pcap_collect, NULL);
    log->print_log(IP_INFO, "[netfilter_time_ip]: module unload");
    return TRUE;
}

static void 
thr_pcap(void *data)
{
    CORE_API			*core = api->core;
	LOG_API				*log = api->log;
    
	const u_char 		*buffer = NULL;
    PACKET_INFO			packet_info;
	PACKET_DATA_INFO 	packet_data;
	int					err;
    
	pcap_t 				*handle;                    /* Session handle */
	char 				errbuf[PCAP_ERRBUF_SIZE]; 	/* Error string */
	struct bpf_program 	filter;            			/* The compiled filter */
	char 				filter_app[] = "";       	/* The filter expression */
	bpf_u_int32 		mask;                     	/* Our netmask */
	bpf_u_int32 		net;                        /* Our IP */
	struct pcap_pkthdr 	header;          			/* The header that pcap gives us */

	
    log->print_log(IP_INFO, "Thread PCAP (%x) started\n", (int)pthread_self());
    
    err = pcap_lookupnet(name_dev, &net, &mask, errbuf);
	log->print_log(IP_INFO, "Net=%x Mask=%x", net, mask);	

    if (err == -1) {
		log->print_log(IP_ALERT, "[pcap_time_ip]: pcap_lookupnet (%s)\n", errbuf);
		goto l_end_2;
    }
    
	handle = pcap_open_live(name_dev, max_size_buffer, 1, 0, errbuf);
    
    if (!handle) {
        log->print_log(IP_ALERT, "[pcap_time_ip]: pcap_open_live (%s)\n", errbuf);
        goto l_end_1;
    }
	
 	err = pcap_compile(handle, &filter, filter_app, 0, net);
    if (err == -1) {
		log->print_log(IP_ALERT, "[pcap_time_ip]: pcap_compile (%s)\n", pcap_geterr(handle));
		goto l_end_1;
    }
	
	err = pcap_setfilter(handle, &filter);
     if (err == -1) {
		log->print_log(IP_ALERT, "[pcap_time_ip]: pcap_setfilter (%s)\n", pcap_geterr(handle));
		goto l_end_1;
    }   
	
	
    while (do_exit == 0) {
        IP_size			e_len;
 		struct ip 		*ip;
 		struct tcphdr 	*tcp;
 		struct udphdr 	*udp;
 		IP_port 		src_port, dest_port;
		char			src[18], dest[18];
		
		buffer = pcap_next(handle, &header);
		
		if (!buffer) {
            log->print_log(IP_ERR, "[pcap_time_ip]: pcap_next (%s)\n", pcap_geterr(handle));
            continue;
        }

		ip = (struct ip*)(buffer + sizeof(struct ether_header));

		if (ntohs(ip->ip_off) & (IP_MF | IP_OFFMASK)) {
		    src_port = dest_port = 0;
		}else
 		if (ip->ip_p == IPPROTO_TCP) {
  			tcp 			= (struct tcphdr*) (buffer + sizeof(struct ether_header) + sizeof(struct ip));
  			src_port  		= ntohs(tcp->th_sport);
  			dest_port 		= ntohs(tcp->th_dport);
 		}else
 		if (ip->ip_p == IPPROTO_UDP) {
	  		udp 			= (struct udphdr*) (buffer + sizeof(struct ether_header) + sizeof(struct ip));
  			src_port  		= ntohs(udp->uh_sport);
  			dest_port 		= ntohs(udp->uh_dport);
 		}else {
	  		src_port  		= 0;
  			dest_port 		= 0;
 		}

// 		sprintf(packet->mac, "%02x:%02x:%02x:%02x:%02x:%02x", hp->hw_addr[0], hp->hw_addr[1], hp->hw_addr[2], hp->hw_addr[3], hp->hw_addr[4], hp->hw_addr[5]);
	
	 	packet_info.proto      	= ip->ip_p;
	 	packet_info.src       	= ip->ip_src.s_addr;
	 	packet_info.dest      	= ip->ip_dst.s_addr;
	 	packet_info.src_port  	= src_port;
	 	packet_info.dest_port 	= dest_port;
	 	e_len       			= header.len - sizeof(struct ether_header);  
		
		ipbyhost(packet_info.src, src);
		ipbyhost(packet_info.dest, dest);
		
//		log->print_log(IP_DEBUG, "[pcap_time_ip]: packet size=%d proto=%d src=%s dest=%s src_p=%d, dest_p=%d\n", packet.len, packet.proto, src, dest, packet.src_port, packet.dest_port);
    
		
		if (core->auth(SERVICE_TRAFFIC, &packet_info, TRUE, &packet_data) == TRUE) {
			packet_data.len = e_len;
			if (core->autz(SERVICE_TRAFFIC, &packet_info, TRUE, &packet_data) == TRUE) {
				core->acct(SERVICE_TRAFFIC, &packet_info, TRUE, &packet_data);
				time_acct(&packet_data);
				ip_acct(&packet_info, &packet_data);
		    }
		}
	}
l_end_1:
    pcap_close(handle);
l_end_2:
    log->print_log(IP_INFO, "Thread %d exit \n", (int) pthread_self());
    pthread_exit(NULL);
}
