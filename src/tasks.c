/*
 * tasks.c	Tasks for scheduler
 *
 * Version:	$Id: tasks.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $
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

static const char rcsid[] = "$Id: tasks.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $";

#include "../include/tasks.h"
#include "../include/store.h"

#include "../include/misc.h"
#include "../include/log.h"

#include <time.h>
#include <unistd.h>


//----------------------------------------------------------------------------
//		  						task_check_limit
//----------------------------------------------------------------------------
gboolean
task_check_limit(void *data)
{
    PLUGIN_API	*plugin = (PLUGIN_API*) data;
    STORE_API	*store = NULL;
    SQL_SESSION *sid;
    
    if ((store = plugin->store) == NULL) return FALSE;
    
    if ((sid = store->open())!=NULL){
	store->exec(sid, "update Users SET onActive='1' where (Type>=0 and Type<=3) and Limitation=-1 and onBlocked='0'");
	store->exec(sid, "update Users SET onActive='0' where (Type>=0 and Type<=3) and Traffic>Limitation and Limitation!=-1 and onBlocked='0'");
	store->exec(sid, "update Users SET onActive='1' where (Type>=0 and Type<=3) and Traffic<=Limitation and onBlocked='0'");
	store->exec(sid, "update Users SET onActive='0' where Type=4 and Balance<0 and onBlocked='0'");
	store->exec(sid, "update Users SET onActive='1' where Type=4 and Balance>=0 and onBlocked='0'");
	store->exec(sid, "update Rules SET onActive='1' where Limitation=-1 and onBlocked='0'");	
	store->exec(sid, "update Rules SET onActive='0' where Traffic>Limitation and Limitation!=-1 and onBlocked='0'");
	store->exec(sid, "update Rules SET onActive='1' where Traffic<=Limitation and onBlocked='0'");
	store->exec(sid, "update Users SET onActive='0' where onBlocked='1'");
	store->exec(sid, "update Rules SET onActive='0' where onBlocked='1'");
	
	save_timestamp(sid, store, "date_last_check_limit");

  	store->close(sid);
    }else return FALSE;
    return TRUE;
}

//----------------------------------------------------------------------------
//		  						task_every_day
//----------------------------------------------------------------------------
gboolean 
task_every_day(void *data)
{
    PLUGIN_API	*plugin = (PLUGIN_API*) data;
    STORE_API	*store	= NULL;
    LOG_API	*log	= NULL;

    SQL_SESSION *sid;
    int		count = 4;
    
    if ((store = plugin->store) == NULL) return FALSE;
    log = plugin->log;
	
    if ((sid = store->open())!=NULL) {
start_label:		
		if (!store->exec(sid, "set autocommit=0")) goto error;
		if (!store->exec(sid, "start transaction")) goto error;
		if (!store->exec(sid, "update Users SET Traffic = 0 where Type=1")) goto error;
  		if (!store->exec(sid, "update Users SET YDay = TDay")) goto error;
		if (!store->exec(sid, "update Users SET TDay = 0")) goto error;
  		if (!store->exec(sid, "update Rules SET Traffic = 0 where Type=1")) goto error;

		if (!store->exec(sid, "insert into System_Report select DATE_ADD(NOW(), INTERVAL -1 DAY), id_user, id_type, TDay, Action from Rules")) goto error;

  		if (!store->exec(sid, "update Rules SET YDay = TDay")) goto error;
  		if (!store->exec(sid, "update Rules SET TDay = 0")) goto error;
		if (save_timestamp(sid, store, "date_last_every_day")!=TRUE) goto error;
		if (!store->exec(sid, "commit")) goto error;
  		goto ok;
error:
  		store->exec(sid, "rollback");
		count--;
		sleep(10);
		if (count>0) goto start_label;
		log->print_log(IP_CRIT, "[task.c][task_every_day]: error execute task_every_day!!!");
		return FALSE;
ok:
		log->print_log(IP_INFO, "task_every_day - success");
		store->close(sid);
    }
    return TRUE;
}

//----------------------------------------------------------------------------
//		  						task_every_week
//----------------------------------------------------------------------------
gboolean 
task_every_week(void *data)
{
    PLUGIN_API	*plugin = (PLUGIN_API*) data;
    STORE_API	*store	= NULL;
    LOG_API	*log	= NULL;

    SQL_SESSION *sid;
    int		count = 4;
    
    if ((store = plugin->store) == NULL) return FALSE;
    log = plugin->log;


    if ((sid = store->open())!=NULL){
start_label:
		if (!store->exec(sid, "set autocommit=0")) goto error;
		if (!store->exec(sid, "start transaction")) goto error;
		if (!store->exec(sid, "update Users SET Traffic = 0 where Type=2")) goto error;
		if (!store->exec(sid, "update Rules SET Traffic = 0 where Type=2")) goto error;
		if (save_timestamp(sid, store, "date_last_every_week")!=TRUE) goto error;
		if (!store->exec(sid, "commit")) goto error;
  		goto ok;
error:
		store->exec(sid, "rollback");
		count--;
		sleep(10);
		if (count>0) goto start_label;
		log->print_log(IP_CRIT, "[task.c][task_every_week]: error execute task_every_week!!!");
		return FALSE;
ok:
		log->print_log(IP_INFO, "task_every_week - success");
		store->close(sid);
    }
    return TRUE;
}

//----------------------------------------------------------------------------
//		  						task_every_month
//----------------------------------------------------------------------------
gboolean 
task_every_month(void *data)
{
    PLUGIN_API	*plugin = (PLUGIN_API*) data;
    STORE_API	*store	= NULL;
    LOG_API	*log	= NULL;

    SQL_SESSION *sid;
    int		count = 4;
    
    if ((store = plugin->store) == NULL) return FALSE;
    log = plugin->log;

    if ((sid = store->open())!=NULL){
start_label:
		if (!store->exec(sid, "set autocommit=0")) goto error;
  		if (!store->exec(sid, "start transaction")) goto error;
	  	
		if (!store->exec(sid, "update Users SET Traffic = 0 where Type=3")) goto error;
	  	if (!store->exec(sid, "update Users SET YMonth = TMonth")) goto error;
	  	if (!store->exec(sid, "update Users SET TMonth = 0")) goto error;
	  	if (!store->exec(sid, "update Rules SET Traffic = 0 where Type=3")) goto error;
	  	if (!store->exec(sid, "update Rules SET YMonth = TMonth")) goto error;
	  	if (!store->exec(sid, "update Rules SET TMonth = 0")) goto error;
		if (save_timestamp(sid, store, "date_last_every_month")!=TRUE) goto error;
		if (!store->exec(sid, "commit")) goto error;
	  	goto ok;
error:
  		store->exec(sid, "rollback");
		count--;
		sleep(10);
		if (count>0) goto start_label;
		log->print_log(IP_CRIT, "[task.c][task_every_month]: error execute task_every_month!!!");
		return FALSE;
ok:
		log->print_log(IP_INFO, "task_every_month - success");
		store->close(sid);
    }
    return TRUE;
}
