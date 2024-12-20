/*
 (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _SERVER_H
#define _SERVER_H	1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include "gfunc.h"

#include "types.h"

#include "conf.h"
#include "log.h"


typedef struct st_server_node
{
	char						name[50];	// Name subscribe
 	gboolean 					(*func)(int, int, char**, void*);
	void						*data;
} NODE_SERVER_JOBS;

typedef struct st_server
{
 	pthread_t			id_thread;
	GSList				*jobs;
	sig_atomic_t		do_exit;
} SERVER;

typedef struct SERVER_API
{
	gboolean	(*registred)(char*, gboolean (*func)(int, int, char**, void*), void*);
	gboolean	(*unregistred)(char*);
	gboolean	(*print)(FILE *fd);	
} SERVER_API;

SERVER_API	*server_init(CONF_API*, LOG_API*);
IP_result	server_free(SERVER_API *);

#endif /* include/server.h */
