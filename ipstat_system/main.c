/*
 * ipstat_system.c	Function for works with system.
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

#include "../include/sched.h"
#include "../include/system.h"
#include "../include/misc.h"
#include "../include/tasks.h"
#include "../include/types.h"
#include "../include/plugin.h"

#include <unistd.h>
#include <stdio.h>

static PLUGIN_INFO	module_info;

static PLUGIN_API	*api;

int debug = 0;

static char	*id 		= "401nti";
static char	*name 		= "system";
static char	*version 	= "0.1";
static char	*description	= "System module for IPStat";
static char	*author		= "Kukuev Dmitry";
static char	*homepage	= "http://ipstat.code-art.ru/";

static int	secondary_daemon = 0;

static CONF_PARSER module_config[] = {
        { "secondary_daemon", CF_TYPE_INT, 0, &secondary_daemon, "0" },
        { NULL, -1, 0, NULL, NULL }
};

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
    module_info.type		= IPSTAT_PLUGIN_SERVICE;
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
    SCHEDULER_API	*sched  = NULL;
    CONF_API		*conf	= NULL;
    
    api = (PLUGIN_API*) data;

    sched  	= (SCHEDULER_API*) api->sched;
	conf	= (CONF_API*) api->conf;
	
	if (conf) conf->parse(module_config);	
    fprintf(stderr, "Start System module...\n");    

	if (secondary_daemon == 0) {    
    	if (sched) sched->registred("check_limit", "*/1 * * * *", (void*)&task_check_limit, api);
    	if (sched) sched->registred("every_day", "0 0 * * *", (void*)&task_every_day, api);
    	if (sched) sched->registred("every_week", "0 0 * * 1", (void*)&task_every_week, api);
    	if (sched) sched->registred("every_month", "0 0 1 * *", (void*)&task_every_month, api);
	}
	check_correct_day(api);
	check_correct_week(api);
	check_correct_month(api);
	
    return TRUE;
}

gboolean
unload(void *data)
{
    SCHEDULER_API	*sched = NULL;
	
    sched = (SCHEDULER_API*) api->sched;

	if (secondary_daemon == 0) {  
    	if (sched) sched->unregistred("every_month");
    	if (sched) sched->unregistred("every_week");
    	if (sched) sched->unregistred("every_day");
    	if (sched) sched->unregistred("check_limit");
	}
    return TRUE;
}
