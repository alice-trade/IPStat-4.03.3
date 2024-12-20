/*
 * main.c	Main loop of the billing server.
 *
 * Version:	$Id: main.c,v 1.2 2005/12/24 09:56:36 kornet Exp $
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
 *   (C) 2001-2004 Kornet [kornet@code-art.ru]
*/

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
	      
#include "../include/log.h"
#include "../include/store.h"
#include "../include/sched.h"
#include "../include/server.h"
#include "../include/core.h"
#include "../include/collectors.h"
#include "../include/conf.h"
#include "../include/misc.h"
#include "../include/system.h"

static const char rcsid[] = "$Id: main.c,v 1.2 2005/12/24 09:56:36 kornet Exp $";

static char		conf_file[FILENAME_MAX];
static char		pid_file[FILENAME_MAX];
static void 		action_to_daemon(int);
static void 		sig_fatal(int sig);

static sig_atomic_t	do_exit = 0;
int			debug = 1;

static CONF_PARSER module_config[] = {
        { "debug", CF_TYPE_INT, 0, &debug, "0" },
        { NULL, -1, 0, NULL, NULL }
};

int main(int argc, char **argv)
{
 	int 				op, flag_daemon = 1;
 	int 				lf;

    CONF_API    		*conf   = NULL;
    LOG_API     		*log    = NULL;
    STORE_API   		*store  = NULL;
    SCHEDULER_API   	*sched  = NULL;
    SERVER_API  		*server = NULL;
    CORE_API    		*core   = NULL;
	PLUGIN_API 		plugin;
	
 	if (argc<=1){
  		printf("Usage: ./IPStat -h for help.\n");
  		exit(0);
 	}
 
	sprintf(conf_file, "/opt/IPStat/etc/IPStat.cfg");
	sprintf(pid_file, "/opt/IPStat/run/IPStat.pid");
	
 	while((op = getopt(argc, argv, "C:p:dxtsrh")) != EOF){
  		switch(op){
    		case 't':
            		flag_daemon = 0;
              		break;
    		case 'C':
            		strncpy(conf_file, optarg, FILENAME_MAX-1);
              		break;
    		case 'd':
              		flag_daemon = 1;
              		break;
                case 'p':
		        strncpy(pid_file, optarg, FILENAME_MAX-1);
			break;
    		case 's':
              		action_to_daemon(SIGTERM);
              		exit(0);
              		break;
    		case 'r':
              		action_to_daemon(SIGHUP); 
              		printf("Reload Grant tables IPStat - done.\n");
              		exit(0);
              		break;
    		case 'h':
              		printf("\nIPStat %s - by Kornet [kornet@code-art.ru]\n", VERSION);
              		printf("All local commands begin with a '-' character.\n");
              		printf("\t-d\tStarting a daemon.\n");
              		printf("\t-t\tStarting a terminal.\n");
              		printf("\t-s\tStoping a daemon.\n");
              		printf("\t-r\tReload grant tables.\n");
              		printf("\t-C\tUse other configure file.\n");
			printf("\t-p\tSet pid file.\n");
            		printf("\t-h\tThis help.\n");
              		exit(0);
              		break;
    		default:break;
  		}
 	}


// user not root? exit
 	if (getuid()!=0)
 	{
  		fprintf(stderr, "[ERROR]: IPStat must be run for superuser\n");
  		exit(1);
 	}

//===================
// --- LOCK FILE ---
//===================
 	lf = open(pid_file, O_RDWR|O_CREAT, 0640);
 	if (lf<0) exit(1);
 	if (flock((int)lf, LOCK_EX|LOCK_NB)) {
  		fprintf(stderr, "[ERROR]: run dublicate IPStat. Good bye.\n");
  		exit(1);
 	}
 	fprintf(stderr, "IPStat %s starting (build %s/%s).\n", VERSION, (__DATE__), (__TIME__));
	if (flag_daemon) daemon(0, 0);

	if ((conf = conf_init(conf_file)) == NULL) {
        fprintf(stderr, "[ERROR]: failed init conf module\n");
        exit(1);
    }
	
	conf->parse(module_config);
	if ((log = log_init(conf)) == NULL) {
        fprintf(stderr, "[ERROR]: failed init log module\n");
        exit(1);
    }
	
	log->print_log(IP_INFO, "IPStat %s starting (build %s/%s).\n", VERSION, (__DATE__), (__TIME__));
	
//--- Daemon process started ---

 	setsid();
	set_env();
//======================
// --- INIT SIGNALS ---|
//======================
 	signal(SIGHUP,  &sig_fatal);
	signal(SIGUSR2,  &sig_fatal);
 	signal(SIGINT,  &sig_fatal);
	signal(SIGTERM, &sig_fatal);
 	signal(SIGQUIT, &sig_fatal);

 
//===================================
// --- GET AND WRITE PID IN FILE ---|
//===================================
	save_pid(pid_file, getpid());				// save pid program

	if ((store = store_init(conf, log)) == NULL) {
        log->print_log(IP_ALERT, "failed init store module");
        exit(1);
    }
	
	if ((server = server_init(conf, log)) == NULL) {
        log->print_log(IP_ALERT, "failed init server module");
        exit(1);
    }

    if ((sched = scheduler_init(conf, log)) == NULL) {
        log->print_log(IP_ALERT, "failed init scheduler module");
        exit(1);
    }
    sched->set_server(server);
                                                                                                                                                             
	check_core_tables(store, log);
	
    if ((core = core_init(conf, log)) == NULL) {
        log->print_log(IP_ALERT, "failed init core module");
        exit(1);
    }
                                                                                                                                                             
    core->set_store(store);
    core->set_server(server);
    core->set_sched(sched);
    core->sync();
                                                                                                                                                             
    plugin.store       = store;
    plugin.server      = server;
    plugin.sched       = sched;
    plugin.core        = core;
    plugin.conf        = conf;
    plugin.log         = log;
                    
    if (system_init(&plugin) == FALSE) {
        log->print_log(IP_ALERT, "failed init system modules");
        exit(1);
    }

                                                                                                                                             
    if (collectors_init(&plugin) == FALSE) {
        log->print_log(IP_ALERT, "failed init collectors module");
        exit(1);
    }

    system_load_all();
    collectors_load_all();
    while (do_exit==0){
        sleep(60);
    }
                                                                                                                                                             
    collectors_unload_all();
    system_unload_all();

    log->print_log(IP_INFO, "collectors unload all\n");
                                                                                                                                                             
    collectors_free();
    log->print_log(IP_INFO, "collectors free\n");

    system_free();
    log->print_log(IP_INFO, "system free\n");
                                                                                                                                                             
    core_free(core);
    log->print_log(IP_INFO, "core free\n");
                                                                                                                                                             
    scheduler_free(sched);
    log->print_log(IP_INFO, "scheduler free\n");

    server_free(server);
    log->print_log(IP_INFO, "server free\n");

    store_free(store);
    log->print_log(IP_INFO, "store free\n");
    
    log->print_log(IP_INFO, "End IPStat...\n");
    log_free(log);
    fprintf(stderr, "log free\n");	
	
    conf_free(conf);
    fprintf(stderr, "conf free\n");	

    unlink(pid_file);
    exit(0);
}

//--- action_to_daemon ---
static void 
action_to_daemon(int sig)
{
	pid_t	pid;

	pid = restore_pid(pid_file);	
	kill (pid, sig);
}

//--- sig_fatal ---

static void 
sig_fatal(int sig)
{
    switch(sig) {
        case SIGTERM:
        case SIGQUIT:
        case SIGINT:
		case SIGUSR2:
            do_exit = 1;
            break;
        case SIGHUP:
            do_exit = 2;
            break;
        default:
            do_exit = 0;
            break;
    }
}
