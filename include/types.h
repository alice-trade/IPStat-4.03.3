/*
  (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _TYPES_H
#define _TYPES_H 1

#ifndef FALSE
#define FALSE 0
#undef TRUE
#define TRUE !FALSE
#endif

#include <arpa/inet.h>

typedef enum
{
	IPSTAT_PLUGIN_UNKNOWN  = -1,  	/* Unknown type.    */
	IPSTAT_PLUGIN_COLLECTOR = 0,   	/* Collectors plugin. */
	IPSTAT_PLUGIN_STORE,         	/* Store plugin.   */
	IPSTAT_PLUGIN_SERVICE        	/* Service plugin. */
} IPStatPluginType;

typedef int IPStatPluginPriority;

#define IPSTAT_PRIORITY_DEFAULT     0
#define IPSTAT_PRIORITY_HIGHEST  9999
#define IPSTAT_PRIORITY_LOWEST  -9999

typedef uint32_t 	IP_addr;
typedef uint8_t		IP_mask;
typedef uint32_t	IP_ports;
typedef uint16_t	IP_port;
typedef uint8_t		IP_result;
typedef uint8_t		IP_proto;
typedef uint32_t	IP_size;
typedef	uint64_t	IP_bigsize;
typedef uint8_t		IP_flag;
typedef uint32_t	IP_id;

#define	FS_NONE		-1
#define FS_DROP		0
#define FS_ACCEPT	1

#define PROTO_ALL	255

#define CONST_NEXT	1
#define CONST_SUM	2

#define UNIX_PATH_MAX 50

#define SERVICE_TRAFFIC	1
#define SERVICE_SQUID	2
#define SERVICE_MAIL	3
#define SERVICE_DNS		4
#define SERVICE_LOGIN	5

#define	TRAFFIC_IN		1
#define	TRAFFIC_OUT		2
#define	TRAFFIC_ALL		3


#define _MAX_LENGTH_BUFFER	2048 
#define _MAX_LENGTH_LOGIN	50 
#define _MAX_LENGTH_PASSWD	50 
#define _MAX_LENGTH_MAC		18 
#define _MAX_LENGTH_IP		16 
#define _MAX_LENGTH_SID		33 
#define _MAX_LENGTH_MD5		33

//--------------------------------------
#define IPSTAT_PREFIX		"/opt/IPStat"

//#define CONFIG_FILE_NAME 	"IPSTAT_PREFIX/IPStat.cfg"
//#define PID_FILE_NAME 		"/var/run/IPStat.pid"
//#define LOCK_FILE_NAME 		"/var/run/IPStat.lock"
//--------------------------------------

typedef struct st_dbt
{
	void			*data;
	size_t			size;
} DBT;

typedef struct st_packet_info
{
 	unsigned char 	mac[18];
	IP_proto 		proto; 
 	IP_addr 		src; 
 	IP_addr 		dest; 
 	IP_port 		src_port; 
 	IP_port 		dest_port; 
} PACKET_INFO;

typedef struct st_packet_data_info
{
	IP_id			service;
	
 	IP_flag			direct;
	IP_id			id_user;
	IP_id			id_group_user;
	
	IP_id			id_traffic_group;
	IP_id			id_traffic_type;
	
	IP_flag			not_sum;

 	IP_size			len;
	
	void			*data;
} PACKET_DATA_INFO;


typedef struct st_squid_info
{
 	unsigned char 	mac[18];
 	IP_addr 		ip; 
} SQUID_INFO;

typedef struct st_squid_data_info
{
	IP_id			service;
	
	IP_id			id_user;
	IP_id			id_group_user;
	IP_id			id_traffic_group;
	IP_id			id_traffic_type;
	IP_flag			not_sum;
 	IP_size			len;
} SQUID_DATA_INFO;


#endif /* include/types.h */
