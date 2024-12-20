/*
 * system.c	System functions.
 *
 * Version:	$Id: misc.c,v 1.2 2005/12/24 09:54:39 kornet Exp $
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

static const char rcsid[] = "$Id: misc.c,v 1.2 2005/12/24 09:54:39 kornet Exp $";

#include "../include/misc.h"
#include "../include/log.h"
#include "../include/store.h"
#include "../include/plugin.h"
#include "../include/log.h"
#include "../include/tasks.h"

#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdarg.h>

static char * ip_strtok_r (char *s, const char *delim, char **save_ptr);

//-----------------------------------------------------------------------------
//								save_pid
//-----------------------------------------------------------------------------
gboolean
save_pid(const char *file, const pid_t pid)
{
	FILE 	*fd;
	char	str_pid[10];
	
	sprintf(str_pid, "%d", (int) pid);
 	
	if ((fd = fopen(file, "w"))!=NULL){
  		fwrite(str_pid, 1, strlen(str_pid), fd);
 		fclose(fd);
	}else return FALSE;
	return TRUE;
}

//-----------------------------------------------------------------------------
//								restore_pid
//-----------------------------------------------------------------------------
pid_t
restore_pid(const char *file)
{
	FILE	*fd;
	char	str_pid[10];
	
	if ((fd = fopen(file, "r"))!=NULL){
  		fread(&str_pid, 1, 9, fd);
  		fclose(fd);
		return (pid_t) atoi(str_pid);
	}
 	return (pid_t)0;
}

//-----------------------------------------------------------------------------
//								set_env
//-----------------------------------------------------------------------------
void
set_env(void)
{
 	FILE *fd;

 	if ((fd = fopen("/proc/sys/net/core/rmem_max", "w"))!=NULL){
  		fprintf(fd, "1048576");
  		fclose(fd);
 	}
 	if ((fd = fopen("/proc/sys/net/core/rmem_default", "w"))!=NULL){
  		fprintf(fd, "1048576");
  		fclose(fd);
 	}
 	if ((fd = fopen("/proc/sys/net/core/wmem_max", "w"))!=NULL){
  		fprintf(fd, "1048576");
  		fclose(fd);
 	}
 	if ((fd = fopen("/proc/sys/net/core/wmem_default", "w"))!=NULL){
  		fprintf(fd, "1048576");
  		fclose(fd);
 	}
}

//-----------------------------------------------------------------------------
//                                save_timestamp
//-----------------------------------------------------------------------------
int
save_timestamp(SQL_SESSION *sid, STORE_API *store, const char *name)
{
    char	buf[1024];
    time_t	target_time;
                                                                                                                                                             
    target_time = time(NULL);
	
    sprintf(buf, "replace into System_Table (Name, Value) values ('%s', '%u')", name, (int) target_time);
    
    if (!store->exec(sid, buf)) goto error;
    return TRUE;

error:
    return FALSE;
}

int		
check_core_tables(STORE_API *store, LOG_API *log)
{
    SQL_SESSION	*sid;
    char	*buffer;
    gboolean	res = TRUE;
	
    if ((sid = store->open())!=NULL) {
    	if ((buffer = (char*) g_malloc0(2048))==NULL) {
    		store->close(sid);
    		return FALSE;
    	}
    	if (store->list_tables(sid, "Users")) {
    		if (store->fetch_row(sid) == NULL) {
    			sprintf(buffer, "CREATE TABLE Users (id int(10) unsigned NOT NULL auto_increment, id_group int(10) unsigned NOT NULL default '0', Login char(100) NOT NULL, Passwd char(100) NOT NULL, Name char(100) NOT NULL default '', Comment char(255) NOT NULL default '', Addr char(18) NOT NULL default '0.0.0.0', Mac char(18) NOT NULL default '', Type tinyint(3) unsigned NOT NULL default '0', Limitation bigint(20) NOT NULL default '-1', Traffic bigint(20) unsigned NOT NULL default '0', Balance double NOT NULL default '0', YDay bigint(20) unsigned NOT NULL default '0', TDay bigint(20) unsigned NOT NULL default '0', YMonth bigint(20) unsigned NOT NULL default '0', TMonth bigint(20) unsigned NOT NULL default '0', onActive enum('0','1') NOT NULL default '1', onBlocked enum('0','1') NOT NULL default '0', Date datetime NOT NULL default '0000-00-00 00:00:00', PRIMARY KEY  (id), KEY key_limit_1 (Type,Limitation,Traffic,onActive,onBlocked), KEY key_limit_2 (Type,Balance), KEY key_limit_3 (onBlocked), KEY key_limit_4 (Login, Passwd)) TYPE=InnoDB");
    			res = store->exec(sid, buffer);
    			log->print_log(IP_INFO, "create table 'Users'");
    		}else{
    		    if (!store->query(sid, "select Login from Users")){
    			if (!store->exec(sid, "ALTER TABLE Users ADD Login char(100) NOT NULL default '' AFTER id_group")) {
    			    log->print_log(IP_ERR, "error ALTER TABLE Users: add fild Login");
    			    exit(1);
    			}
    			log->print_log(IP_INFO, "Add field 'Login' in DB shema");
    			
    			if (!store->exec(sid, "ALTER TABLE Users ADD Passwd char(100) NOT NULL default '' AFTER Login")) {
    			    log->print_log(IP_ERR, "error ALTER TABLE Users: add fild Passwd");
    			    exit(1);
    			}
    			log->print_log(IP_INFO, "Add field 'Passwd' in DB shema");
    			
    			if (!store->exec(sid, "ALTER TABLE Users ADD INDEX key_limit_4 (Login, Passwd)")) {
    			    log->print_log(IP_ERR, "error ALTER TABLE Users: add index key_limit_4");
    			    exit(1);
    			}
    			log->print_log(IP_INFO, "Add index 'key_limit_4' in DB shema");
    		    }    
    		}
    	}
	if (store->list_tables(sid, "Rules")) {
		if (store->fetch_row(sid) == NULL) {
			sprintf(buffer, "CREATE TABLE Rules (id int(10) unsigned NOT NULL auto_increment,    id_user int(10) unsigned NOT NULL default '0', id_type int(10) unsigned NOT NULL default '0', Type tinyint(3) unsigned NOT NULL default '0', Limitation bigint(20) NOT NULL default '-1', Traffic bigint(20) unsigned NOT NULL default '0', YDay bigint(20) unsigned NOT NULL default '0', TDay bigint(20) unsigned NOT NULL default '0', YMonth bigint(20) unsigned NOT NULL default '0', TMonth bigint(20) unsigned NOT NULL default '0', Action tinyint(3) unsigned NOT NULL default '0', onActive enum('0','1') NOT NULL default '1', onBlocked enum('0','1') NOT NULL default '0', PRIMARY KEY  (id), KEY key1 (Type,Limitation,Traffic,onActive,onBlocked), KEY key_limit_1 (Limitation,Traffic), KEY key_limit_2 (onBlocked), KEY key_limit_3 (Type), KEY key_limit_4 (id_user)) TYPE=InnoDB");
			res = store->exec(sid, buffer);
			log->print_log(IP_INFO, "create table 'Rules'");
		}
	}
	if (store->list_tables(sid, "Group_Traffic")) {
		if (store->fetch_row(sid) == NULL) {
			sprintf(buffer, "CREATE TABLE Group_Traffic (id int(10) unsigned NOT NULL auto_increment, Name char(100) NOT NULL default '', PRIMARY KEY  (id)) TYPE=InnoDB");
			res = store->exec(sid, buffer);
			sprintf(buffer, "INSERT INTO Group_Traffic VALUES (1,'Default type')");
			res = store->exec(sid, buffer);
			log->print_log(IP_INFO, "create table 'Group_Traffic'");
		}
	}
	if (store->list_tables(sid, "Group_Users")) {
		if (store->fetch_row(sid) == NULL) {
			sprintf(buffer, "CREATE TABLE Group_Users (id int(10) unsigned NOT NULL auto_increment, Name char(100) NOT NULL default '', PRIMARY KEY (id)) TYPE=InnoDB");
			res = store->exec(sid, buffer);
			sprintf(buffer, "INSERT INTO Group_Users VALUES (1,'Default group users')");
			res = store->exec(sid, buffer);
			log->print_log(IP_INFO, "create table 'Group_Users'");
		}
	}
	if (store->list_tables(sid, "System_Table")) {
		if (store->fetch_row(sid) == NULL) {
			sprintf(buffer, "CREATE TABLE System_Table (Name varchar(50) NOT NULL default '', Value varchar(100) NOT NULL default '', PRIMARY KEY (Name)) TYPE=InnoDB");
			res = store->exec(sid, buffer);
			log->print_log(IP_INFO, "create table 'System_Table'");
		}
	}
	if (store->list_tables(sid, "Type_Traffic")) {
		if (store->fetch_row(sid) == NULL) {
			sprintf(buffer, "CREATE TABLE Type_Traffic (id int(10) unsigned NOT NULL auto_increment, id_group int(10) unsigned NOT NULL default '0', Name char(100) NOT NULL default '', Direct tinyint(3) unsigned NOT NULL default '3', Proto tinyint(3) unsigned NOT NULL default '255', Net char(15) NOT NULL default '0.0.0.0', Mask tinyint(3) unsigned NOT NULL default '0', Ports int(10) unsigned NOT NULL default '0', Start_time time default NULL, End_time time default NULL, Price double NOT NULL default '0', PRIMARY KEY  (id), KEY key_order (Direct,Mask,Proto,Ports)) TYPE=InnoDB");
			res = store->exec(sid, buffer);
			sprintf(buffer, "INSERT INTO Type_Traffic VALUES (1,1,'Input and Output traffic',3,255,'0.0.0.0',0,0,NULL,NULL,0)");
			res = store->exec(sid, buffer);
			sprintf(buffer, "INSERT INTO Type_Traffic VALUES (2,1,'Input traffic',1,255,'0.0.0.0',0,0,NULL,NULL,0)");
			res = store->exec(sid, buffer);
			sprintf(buffer, "INSERT INTO Type_Traffic VALUES (3,1,'Output traffic',2,255,'0.0.0.0',0,0,NULL,NULL,0)");
			res = store->exec(sid, buffer);
			sprintf(buffer, "INSERT INTO Type_Traffic VALUES (4,1,'Default SQUID traffic',1,254,'0.0.0.0',0,0,NULL,NULL,0)");
			res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Type_Traffic'");
		}
	}

		if (store->list_tables(sid, "System_Report")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE System_Report (Date date NOT NULL, id_user int(10) unsigned NOT NULL default '0',id_type int(10) unsigned NOT NULL default '0', Traffic bigint(20) unsigned NOT NULL default '0', Action tinyint unsigned NOT NULL default '0', KEY key1 (Action, id_user, Date, id_type))");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'System_Report'");
			}
		}
		if (store->list_tables(sid, "log_interface")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE log_interface (id int(11) NOT NULL auto_increment, date datetime NOT NULL, ip varchar(16) NOT NULL default '', action varchar(255) NOT NULL default '', PRIMARY KEY  (id)) TYPE=MyISAM");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'System_Report'");
			}
		}
		
		g_free(buffer);
		store->close(sid);
	}
	return TRUE;
}

int		
check_netfilter_tables(STORE_API *store, LOG_API *log)
{
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	res = TRUE;
	
	if ((sid = store->open())!=NULL) {
		if ((buffer = (char*) g_malloc0(2048))==NULL) {
			store->close(sid);
			return FALSE;
		}
		if (store->list_tables(sid, "Log_IP")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Log_IP (id_user int(11) NOT NULL default '0', Proto tinyint(3) unsigned NOT NULL default '255', Direct tinyint(4) NOT NULL default '0', Addr int(10) unsigned NOT NULL default '0', Port int(10) unsigned NOT NULL default '0', Traffic int(10) unsigned NOT NULL default '0', Date date NOT NULL default '0000-00-00', KEY key1 (id_user,Proto,Direct,Addr,Port,Date)) TYPE=MyISAM");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Log_IP'");
			}
		}
		if (store->list_tables(sid, "Services")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Services (id int(11) NOT NULL auto_increment, Name varchar(100) NOT NULL default '', Proto varchar(10) NOT NULL default '', Port int(11) NOT NULL default '0', PRIMARY KEY  (id)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Services'");
			}
		}

		if (store->list_tables(sid, "Log_Temp")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Log_Temp (id int(11) NOT NULL default '0', Traffic int(10) unsigned NOT NULL default '0', KEY key1 (id), KEY key_group (Traffic)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Log_Temp'");
			}
		}
		if (store->list_tables(sid, "Log_Time")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Log_Time (Time datetime NOT NULL default '0000-00-00 00:00:00', id_user int(11) NOT NULL default '0', Traffic int(10) unsigned NOT NULL default '0', KEY key1 (id_user,Time)) TYPE=InnoDB");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Log_Time'");
			}
		}
		
		g_free(buffer);
		store->close(sid);
	}
	return TRUE;
}

int
check_squid_tables(STORE_API *store, LOG_API *log)
{
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	res = TRUE;
	
	if ((sid = store->open())!=NULL) {
		if ((buffer = (char*) g_malloc0(2048))==NULL) {
			store->close(sid);
			return FALSE;
		}
		if (store->list_tables(sid, "Log_Squid")) {
			if (store->fetch_row(sid) == NULL) {
				sprintf(buffer, "CREATE TABLE Log_Squid (Time datetime NOT NULL default '0000-00-00 00:00:00', id_user int(11) NOT NULL default '0', URL varchar(200) NOT NULL default '', Traffic int(10) unsigned NOT NULL default '0', KEY key1 (id_user,Time,URL), KEY key2 (Time,URL)) TYPE=MyISAM");
				res = store->exec(sid, buffer);
				log->print_log(IP_INFO, "create table 'Log_Squid'");
			}
		}
		
		g_free(buffer);
		store->close(sid);
	}
	return TRUE;
}

int
check_correct_day(PLUGIN_API *api)
{
	SQL_ROW		row;
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	flag_tmp = FALSE;
	STORE_API 	*store = NULL;
	LOG_API 	*log = NULL;
	
	if ((store = (STORE_API*) api->store)==NULL) return FALSE;
	log = (LOG_API*) api->log;
	
	if ((sid = store->open())!=NULL) {
		if ((buffer = (char*) g_malloc0(1024))==NULL) {
			store->close(sid);
			return FALSE;
		}
		
		sprintf(buffer, "select count(*) from System_Table where Name='date_last_every_day' and (UNIX_TIMESTAMP(NOW()) - Value)>86400");
		if (!store->query(sid, buffer)) goto error;
		row = store->fetch_row(sid);
   		flag_tmp = atol(row[0]);
    	store->free(sid);		

		if (flag_tmp) {
			task_every_day((void*) api);
			log->print_log(IP_ALERT, "correct data day");
		}
error:			
		g_free(buffer);
		store->close(sid);
	}else return FALSE;
	return TRUE;
}

int
check_correct_week(PLUGIN_API *api)
{
	SQL_ROW		row;
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	flag_tmp = FALSE;
	STORE_API 	*store = NULL;
	LOG_API 	*log = NULL;
	
	if ((store = (STORE_API*) api->store)==NULL) return FALSE;
	log = (LOG_API*) api->log;
	
	if ((sid = store->open())!=NULL)
	{
		if ((buffer = (char*) g_malloc0(1024))==NULL) {
			store->close(sid);
			return FALSE;
		}
		
		sprintf(buffer, "select count(*) from System_Table where Name='date_last_every_week' and (UNIX_TIMESTAMP(NOW()) - Value)>604800");
		if (!store->query(sid, buffer)) goto error;
		row = store->fetch_row(sid);
   		flag_tmp = atol(row[0]);
    	store->free(sid);		

		if (flag_tmp) {
			task_every_week((void*) api);
			log->print_log(IP_ALERT, "correct data week");
			
		}
error:			
		g_free(buffer);
		store->close(sid);
	}else return FALSE;
	return TRUE;
}

int
check_correct_month(PLUGIN_API *api)
{
	SQL_ROW		row;
	SQL_SESSION	*sid;
	char		*buffer;
	gboolean	flag_tmp = FALSE;
	STORE_API 	*store = NULL;
	LOG_API 	*log = NULL;
	
	if ((store = (STORE_API*) api->store)==NULL) return FALSE;
	log = (LOG_API*) api->log;
	
	if ((sid = store->open())!=NULL)
	{
		if ((buffer = (char*) g_malloc0(1024))==NULL) {
			store->close(sid);
			return FALSE;
		}
		
		sprintf(buffer, "select count(*) from System_Table where Name='date_last_every_month' and (UNIX_TIMESTAMP(NOW()) - Value)>2678400");
		if (!store->query(sid, buffer)) goto error;
		row = store->fetch_row(sid);
   		flag_tmp = atol(row[0]);
    	store->free(sid);		

		if (flag_tmp) {
			task_every_month((void*) api);
			log->print_log(IP_ALERT, "correct data month");
		}
error:			
		g_free(buffer);
		store->close(sid);
	}else return FALSE;
	return TRUE;
}

int
send_to_socket(int sock, const char *msg, ...)
{
	char *buffer;
	va_list ap;
	int res;
	
	if ((buffer = g_malloc0(8192)) == NULL) return FALSE;
		
	va_start(ap, msg);
#ifdef HAVE_VSNPRINTF
	vsnprintf(buffer, 8191, msg, ap);
#else
	vsprintf(buffer, msg, ap);
	if (strlen(buffer) >= 8191) _exit(42);
#endif
	res = send(sock, buffer, strlen(buffer), 0);

	va_end(ap);
	
	if (res == strlen(buffer)) {
		g_free(buffer);
		return TRUE;
	}
	
	g_free(buffer);
	return FALSE;
}

int
daemon(const int nochdir, const int noclose)
{
    int fd;

    switch (fork()) {
        case -1:
                    return (-1);
        case 0:
                    break;
        default:
		    _exit(0);
    }
    if (setsid() == -1)
        return (-1);
    if (!nochdir)
        (void)chdir("/");
    if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        (void)dup2(fd, STDIN_FILENO);
        (void)dup2(fd, STDOUT_FILENO);
        (void)dup2(fd, STDERR_FILENO);
        if (fd > 2)
            (void)close(fd);
    }
    return (0);
}

int 
run_prog(const char *name, char *var[], int stream_in, int stream_out)
{
	pid_t pid = fork();
	
	if (pid == (pid_t) -1) {
                fprintf(stderr, "error fork (%s)\n", strerror(errno));
	} else if (pid == 0) { 
		signal(SIGTERM, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT,  SIG_DFL);
		signal(SIGALRM, SIG_DFL);

		if(stream_in != STDIN_FILENO) {
			dup2(stream_in,STDIN_FILENO);
			close(stream_in);
		}
		if(stream_out != STDOUT_FILENO) {
			dup2(stream_out,STDOUT_FILENO);
			close(stream_out);
		}
		execvp(name, var);
	} else {
		if(stream_in != STDIN_FILENO)
			close(stream_in);
		if(stream_out != STDOUT_FILENO)
			close(stream_out);
    }
    return pid;
		
}

int 
exec_prog(const char *name, char *var[])
{
	pid_t 	pid = fork();
	int 	status;
	pid_t	w_pid;
	
	if (pid == (pid_t) -1) {
                fprintf(stderr, "error fork (%s)\n", strerror(errno));
		_exit(1);
	} else if (pid == 0) { 
		execvp(name, var);
		fprintf(stderr, "error execvp (%s)\n", strerror(errno));
		_exit(1);
	} else {
		status = 0;
		for(;;) {
			w_pid = waitpid(pid, &status, 0);
			if (w_pid > 0) break;
			if( w_pid == -1 && errno == EINTR )  continue;
			if( pid < 0)  break;
		}
	}
    return status;		
}

void 
ip_parse_arg(char *buffer, int *argc, char *argv[], int max_tok, const char *delim)
{
    int		i	= 0;
    char	*ptr	= NULL;
    char	*buf_ptr= NULL;

    *argc = 0;
    if (!*buffer) return;
    if ((ptr = ip_strtok_r(buffer, delim, &buf_ptr)) == NULL) return;
    
    argv[i++] = strdup(ptr);
                                                                                
    while ((ptr = ip_strtok_r('\0', delim, &buf_ptr))!=NULL) {
	if (i>max_tok) break;
        argv[i++] = strdup(ptr);
    }

    *argc = i;
    return;
}

static char *
ip_strtok_r (char *s, const char *delim, char **save_ptr)
{
    char *token;

    if (s == NULL) s = *save_ptr;

/* Scan leading delimiters.  */
    s += strspn (s, delim);
    if (*s == '\0') {
       *save_ptr = s;
       return NULL;
    }

/* Find the end of the token.  */
    token = s;
    s = strpbrk(token, delim);
    if (s == NULL)
/* This token finishes the string.  */
       *save_ptr = strchr(token, '\0');
    else {
/* Terminate the token and make *SAVE_PTR point past it.  */
       *s = '\0';
       *save_ptr = s + 1;
    }
    return token;
}
