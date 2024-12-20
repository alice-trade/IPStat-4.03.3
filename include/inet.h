/*
 (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _INET_H
#define _INET_H	1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>  
#include <netinet/in_systm.h>  
#include <netinet/in.h>  
#include <netinet/ip.h> 

#ifndef __FAVOR_BSD  
 #define __FAVOR_BSD  
 #endif  
 #include <netinet/tcp.h>  
 #include <netinet/udp.h>
 
#include <libipq.h>
#include <linux/netfilter.h>

#include "types.h"

//----------------------------------------------------------------
//						Compare functions
//----------------------------------------------------------------
IP_result			compare_ports(register IP_port, register IP_port);
IP_result			compare_with_mask(register IP_addr*, register IP_addr*, register IP_mask);

//----------------------------------------------------------------
//						 Host functions
//----------------------------------------------------------------
IP_result 			hostbyip(char*, IP_addr*);						// host -> addr
IP_result			hostbyip_with_mask(char*, IP_addr*, IP_mask*);	// host -> addr + mask
IP_result			ipbyhost(IP_addr, char*);						// addr -> host

//----------------------------------------------------------------
//						 Port functions
//----------------------------------------------------------------
IP_result			recbyports(register IP_ports, IP_port*, IP_port*);
IP_ports			portsbyrec(register IP_port, register IP_port);
IP_port				agregate_port(IP_port);
IP_result			get_ports(IP_ports, char*);
//----------------------------------------------------------------
//						 Proto functions
//----------------------------------------------------------------
IP_result			get_proto(IP_proto, char*);

//----------------------------------------------------------------
//					    Packets functions
//----------------------------------------------------------------
IP_size				ipq_grep_packet(ipq_packet_msg_t*, PACKET_INFO*);
//----------------------------------------------------------------


#endif /* include/inet.h */
