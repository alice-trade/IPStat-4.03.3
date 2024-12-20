/*
 * Version:	$Id: conf.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $
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
 *   (C) 2001-2004 Dmirty Kukuev aka Kornet [kornet@code-art.ru]
*/

#include "../include/conf.h"
#include "../include/types.h"

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define	MAX_LEN_BUFFER	2048
#define CONFIG_FILE_NAME        "/opt/IPStat/etc/IPStat.cfg"

static char	redirect_socket[FILENAME_MAX];
static char	conf_file[FILENAME_MAX];
sig_atomic_t	do_exit = 0;
FILE		*open_squid_socket(char *path);

static CONF_PARSER module_config[] = {
        { "collect_squid_redirect_socket", CF_TYPE_STRING, FILENAME_MAX-1, redirect_socket, "/opt/IPStat/tmp/ipstat_redirect.socket"},
        { NULL, -1, 0, NULL, NULL }
};


int main(int argc, char **argv)
{
    FILE		*fd;
    char		*buffer;
    CONF_API		*conf;
    int 		op;
    
    sprintf(conf_file, "/opt/IPStat/etc/IPStat.cfg");
    
    while((op = getopt(argc, argv, "C:")) != EOF){
	switch(op){
	    case 'C':
            	    strncpy(conf_file, optarg, FILENAME_MAX-1);
		    break;
	    default:break;
	}
    }
    
    if ((conf = conf_init(conf_file)) == NULL) {
        fprintf(stderr, "[ERROR]: failed init conf module\n");
        exit(1);
    }
    conf->parse(module_config);
    
    if ((buffer = (char*) malloc(MAX_LEN_BUFFER))==NULL) {
	fprintf(stderr, "[redirector]: error allocate memory for buffer\n");
	exit(1);
    }

    while (do_exit!=SIGTERM) {
	if (fgets(buffer,MAX_LEN_BUFFER, stdin)) {

	    fd = open_squid_socket(redirect_socket);
	    if (fd == NULL) {
		fprintf(stderr, "[redirector]: error open squid socket\n");
		free(buffer);
		exit(1);
	    }
	    fprintf(fd, "%s", buffer);
	    fgets(buffer, MAX_LEN_BUFFER-1, fd);
	    fprintf(stdout, "%s", buffer);
	    fflush(stdout);
	}else {
    	    fprintf(stderr, "[redirector]: error gets buffer\n");
    	    exit(1);
	}	
	fclose(fd);    
    }
    free(buffer);
    exit(0);
}

FILE
*open_squid_socket(char *path)
{
    int 		sid, ret;
    FILE		*fd;
    struct sockaddr_un	saddr;
    

    if((sid = socket(AF_UNIX, SOCK_STREAM, 0))==-1 ) {
	fprintf(stderr, "error create socket\n");
	return NULL;
    }

    saddr.sun_family = AF_UNIX;
    strncpy(saddr.sun_path, path, 50);

    ret = connect(sid, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret==-1) {
	fprintf(stderr, "error connect to '%s' socket\n", saddr.sun_path);
	return NULL;
    }
    fd = fdopen(sid, "w+");
    setlinebuf(fd);    

    return fd;
}

void
func_signal(int sig)
{
    do_exit = sig;        
}
