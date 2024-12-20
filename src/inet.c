
/*
 * inet.c	Network functions
 *
 * Version:	$Id: inet.c,v 1.2 2005/12/26 04:19:17 kornet Exp $
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

static const char rcsid[] = "$Id: inet.c,v 1.2 2005/12/26 04:19:17 kornet Exp $";

#include "../include/inet.h"



#include <assert.h>
#include <string.h>
#include <strings.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define __FAVOR_BSD
#include <netinet/tcp.h>
#include <netinet/udp.h>
//-----------------------------------------------------------------
//			compare_ports
//-----------------------------------------------------------------
IP_result 
compare_ports(register IP_port port, register IP_port rec_ports) {
 	IP_port 	loword;
 	IP_port 	hiword;

 	recbyports(rec_ports, &loword, &hiword);
 	if (hiword == 0 && loword == 0) {// 0, 0 - all ports
  		return TRUE;
 	}else
 	if (hiword == 0 && loword != 0) {// 10, 0 - one port
 	 	if (port == loword) return TRUE;
 	}else
 	if (hiword != 0 && loword != 0) {// 10,20 - [10-20] ports
  		if (port > loword && port < hiword) return TRUE;
 	}else
 	if (hiword != 0 && loword == 0) {// 0, 20 - [<20] ports
  		if (port < hiword) return TRUE;
 	}
 	return FALSE;
}

//-----------------------------------------------------------------
//			compare_with_mask
//-----------------------------------------------------------------
IP_result
compare_with_mask (register IP_addr *ip, register IP_addr *net, register IP_mask mask) 
{
 	register int 	n, m;
 
 	if (memcmp (ip, net, mask / 8) == 0) {
  		n = mask / 8;
  		m = ((-1) << (8 - (mask % 8)));
  		if (mask % 8 == 0 || (((u_char *)ip)[n] & m) == (((u_char *)net)[n] & m))  return TRUE;
 	}
 	return FALSE;
}

//-----------------------------------------------------------------
//			hostbyip
//-----------------------------------------------------------------
IP_result
hostbyip(char *host, IP_addr *ip) 
{
 	struct in_addr 	t_ip;
 	register char 	*t_host, *cp;  
 	IP_result		t_result = FALSE;
 
 	if ((t_host = strndup(host, 15)) == NULL) return FALSE;

 	if ((cp = (char*) strchr(t_host, '/'))!=NULL) t_host[cp - t_host] = '\0';
 	if (inet_aton(t_host, &t_ip) != 0) {
  		*ip	   = t_ip.s_addr;
  		t_result = TRUE;
 	}else {
		*ip = 0;
		t_result = FALSE;
	}
 	free(t_host);
 	return t_result;
}

//-----------------------------------------------------------------
//			hostbyip_with_mask
//-----------------------------------------------------------------
IP_result		
hostbyip_with_mask(char *host, IP_addr *ip, IP_mask *mask) 
{
 	struct in_addr 	t_ip;
 	IP_mask 		bitlen = 0, maxbitlen = 32;
 	register char	*t_host, *cp;  
 	IP_result		t_result = FALSE;
 
 	if ((t_host = strndup(host, 18)) == NULL) return FALSE;

 	if ((cp = (char*) strchr(t_host, '/'))!=NULL) {
  		bitlen = atoi(cp+1);                                    
  		t_host[cp - t_host] = '\0';                         
  		if (bitlen > maxbitlen) bitlen = maxbitlen;         
 	}else bitlen = maxbitlen;                                

 	if (inet_aton(t_host, &t_ip)!=0) { 
  		*ip      = t_ip.s_addr;
  		*mask    = bitlen;
  		t_result = TRUE;
 	}else {
		*ip = 0;
		*mask = 0;
		t_result = FALSE;
	}

 	free(t_host);
 	return t_result;
}

//-----------------------------------------------------------------
//			ipbyhost
//-----------------------------------------------------------------
IP_result
ipbyhost(IP_addr ip, char *host) 
{
 	struct in_addr tmp_ip;

 	if (host == NULL) return FALSE;
 	tmp_ip.s_addr = ip;
 	sprintf(host, "%s", inet_ntoa(tmp_ip));
 	return TRUE;
}

//-----------------------------------------------------------------
//			portsbyrec
//-----------------------------------------------------------------
IP_ports	
portsbyrec(register IP_port lo_port, register IP_port hi_port) 
{
 	return ((hi_port << 16) + lo_port);
}

//-----------------------------------------------------------------
//			recbyports
//-----------------------------------------------------------------
IP_result
recbyports(register IP_ports rec, IP_port *lo_port, IP_port *hi_port) 
{
 	*lo_port = rec & 65535;
 	*hi_port = (rec & ((-1)<<16))>>16;
 	return TRUE; 
}

//-----------------------------------------------------------------
//			agregate_port
//-----------------------------------------------------------------
IP_port
agregate_port(register IP_port port) 
{
 	IP_port 	tmp = 0;
 
 	if (port<=1024) tmp = port;
 	else if (port>1024 && port<=9999)    tmp = 1025;
 	else if (port>=10000 && port<=19999) tmp = 10000;
 	else if (port>=20000 && port<=29999) tmp = 20000;
 	else if (port>=30000 && port<=39999) tmp = 30000;
 	else if (port>=40000 && port<=49999) tmp = 40000;
 	else if (port>=50000 && port<=59999) tmp = 50000;
 	else if (port>=60000)		      	 tmp = 60000;
 
 	return tmp;
}

//-----------------------------------------------------------------
//			get_proto
//-----------------------------------------------------------------
IP_result		
get_proto(register IP_proto prot, char *str) 
{
 	switch (prot) 
 	{
  	case 255:
		    	sprintf(str, "ALL");
		    	break;
  	case IPPROTO_IP:
		    	sprintf(str, "IP");
		    	break;
  	case IPPROTO_ICMP:
		    	sprintf(str, "ICMP");
		    	break;
  	case IPPROTO_IGMP:
		    	sprintf(str, "IGMP");
		    	break;
  	case IPPROTO_IPIP:
		    	sprintf(str, "IPIP");
		    	break;
  	case IPPROTO_TCP:
		    	sprintf(str, "TCP");
		    	break;
  	case IPPROTO_UDP: 
		    	sprintf(str, "UDP");
		    	break;
  	default:	   
		    	sprintf(str, "?");
 	}
 	return TRUE;
}

//-----------------------------------------------------------------
// get_ports
//-----------------------------------------------------------------
IP_result		
get_ports(IP_ports ports, char *str) 
{
 	IP_port	hiword, loword;
 
 	recbyports(ports, &loword, &hiword);

 	if (ports==0) {
  		sprintf(str, "all");
 	} else
 	if (hiword == 0 && loword != 0) {// 10, 0 - one port
  		sprintf(str, "%d", loword);
 	}else
 	if (hiword != 0 && loword != 0) {// 10,20 - [10-20] ports
  		sprintf(str, "%d - %d", loword, hiword);
 	}else
 	if (hiword != 0 && loword == 0) {// 0, 20 - [<20] ports
  		sprintf(str, "0 - %d", hiword);
 	}
 	return TRUE;
}

//----------------------------------------------------------------
//				ipq_grep_packet
//----------------------------------------------------------------
IP_size
ipq_grep_packet(register ipq_packet_msg_t *hp, register PACKET_INFO *packet) 
{
 	register struct ip 		*ip;
 	struct tcphdr 			*tcp;
 	struct udphdr 			*udp;
 	register IP_port 		src_port, dest_port;

 	ip = (struct ip*) hp->payload;

	if (ntohs(ip->ip_off) & (IP_MF | IP_OFFMASK)) {
	    src_port = dest_port = 0;
	}else
 	if (ip->ip_p == IPPROTO_TCP) {
  		tcp 			= (struct tcphdr*)((unsigned char *)ip + ip->ip_hl*4);
  		src_port  		= ntohs(tcp->th_sport);
  		dest_port 		= ntohs(tcp->th_dport);
 	}else
 	if (ip->ip_p == IPPROTO_UDP) {
  		udp 			= (struct udphdr*)((unsigned char *)ip + ip->ip_hl*4);
  		src_port  		= ntohs(udp->uh_sport);
  		dest_port 		= ntohs(udp->uh_dport);
 	}else {
  		src_port  		= 0;
  		dest_port 		= 0;
 	}
	
 	sprintf(packet->mac, "%02x:%02x:%02x:%02x:%02x:%02x", hp->hw_addr[0], hp->hw_addr[1], hp->hw_addr[2], hp->hw_addr[3], hp->hw_addr[4], hp->hw_addr[5]);

 	packet->proto      	= ip->ip_p;
 	packet->src       	= ip->ip_src.s_addr;
 	packet->dest      	= ip->ip_dst.s_addr;
 	packet->src_port  	= src_port;
 	packet->dest_port 	= dest_port;
 
#ifdef DEBUG_INET
 	fprintf(stderr, "src=%d, dest=%d, mac=%s, len=%u\n", packet->src, packet->dest, packet->mac, packet->len);
#endif
	
 	return hp->data_len;
}
