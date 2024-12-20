/*
 (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _MISC_H
#define _MISC_H 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "types.h"
#include "store.h"

pid_t			restore_pid(const char *file);
int			save_pid(const char *file, const pid_t pid);
void			set_env(void);
int			save_timestamp(SQL_SESSION *sid, STORE_API *store, const char *name);
int			check_core_tables(STORE_API*, LOG_API*);
int			check_netfilter_tables(STORE_API *store, LOG_API *log);
int			check_squid_tables(STORE_API *store, LOG_API *log);
int			check_correct_day(PLUGIN_API*);
int			check_correct_week(PLUGIN_API*);
int			check_correct_month(PLUGIN_API*);
int			send_to_socket(int, const char *msg, ...);

int			daemon(const int nochdir, const int noclose);
int			run_prog(const char *name, char *var[], int stream_in, int stream_out);
int			exec_prog(const char *name, char *var[]);

void 			ip_parse_arg(char *buffer, int *argc, char *argv[], int max_tok, const char *delim);

#endif /* include/misc.h */
