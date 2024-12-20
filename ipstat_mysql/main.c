/*
 * mysql.c	Function for works with MySql databases.
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

#include "../include/mysql.h"
#include "../include/store.h"
#include "../include/types.h"
#include "../include/plugin.h"

#include <string.h>
#include <unistd.h>

static PLUGIN_INFO	module_info;

char		*id 		= "I401ml";
char		*name 		= "mysql library";
char		*version 	= "0.1";
char		*description	= "Mysql module for IPStat";
char		*author		= "Kukuev Dmitry";
char		*homepage	= "http://ipstat.code-art.ru/";

//----------------------------------------------------------------------------
//				sql_info
//----------------------------------------------------------------------------
PLUGIN_INFO
*info(void)
{
    module_info.api_version 	= 1;
    module_info.type		= IPSTAT_PLUGIN_STORE;
    module_info.flags		= 0;
    module_info.priority	= IPSTAT_PRIORITY_DEFAULT;
    module_info.id 		= id;
    module_info.name		= name;
    module_info.version	= version;
    module_info.summary	= NULL;
    module_info.description	= description;
    module_info.author		= author;
    module_info.homepage	= homepage;
    return &module_info;
}
//----------------------------------------------------------------------------
//				sql_open
//----------------------------------------------------------------------------
SQL_SESSION 
*sql_open(const char *host, const char *user, const char *pass, const char *db_name) 
{
 	SQL_SESSION	*sid = NULL;

 	if ((sid = (SQL_SESSION*) malloc(sizeof(SQL_SESSION)))==NULL){
  		fprintf(stderr, "[mysql.c][%d]: Error allocate memory\n", (int)getpid());
  		return NULL;
 	}
 	bzero(sid, sizeof(SQL_SESSION));
 	
	mysql_init(&sid->id_db);
// 	mysql_options(&sid->id_db, MYSQL_OPT_COMPRESS, 0);
// 	mysql_options(&sid->id_db, MYSQL_READ_DEFAULT_GROUP, "odbc");
	
	if (mysql_real_connect(&sid->id_db, host, user, pass, db_name, 0, NULL, 0)==NULL){
  		fprintf(stderr, "[mysql.c][%d]: error connect to MySQL (host='%s', user='%s', pass='%s', db_name='%s', error: %s)\n", (int)getpid(), host, user, pass, db_name, mysql_error(&sid->id_db));
  		return NULL;
 	}
 	sid->sres = NULL;
 	return sid;
}

//----------------------------------------------------------------------------
//								   sql_close
//----------------------------------------------------------------------------
gboolean
sql_close(SQL_SESSION *sid) 
{
 	if (sid->sres!=NULL) sql_free(sid);
	mysql_close(&sid->id_db);
 	free(sid);
	sid = NULL;
	return TRUE;
}

//----------------------------------------------------------------------------
//								sql_check_error
//----------------------------------------------------------------------------
gboolean
sql_check_error(SQL_SESSION *sid)
{
	switch(mysql_errno(&sid->id_db)) {
        case CR_SERVER_GONE_ERROR:
        case CR_SERVER_LOST:
        case -1:
                fprintf(stderr, "sql_mysql: MYSQL check_error: (%s), returning SQL_DOWN\n", mysql_error(&sid->id_db));
                return FALSE;
                break;
        case 0:
                return TRUE;
                break;
        case CR_OUT_OF_MEMORY:
        case CR_COMMANDS_OUT_OF_SYNC:
        case CR_UNKNOWN_ERROR:
        default:
                fprintf(stderr, "sql_mysql: MYSQL check_error: (%s) received\n", mysql_error(&sid->id_db));
                return FALSE;
                break;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
//								  sql_exec
//----------------------------------------------------------------------------
gboolean
sql_exec(SQL_SESSION *sid, const char *query) 
{
 	if (mysql_query(&sid->id_db, query)!=0) {
		fprintf(stderr, "Error query='%s'\n", query);
		return sql_check_error(sid);
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//								  sql_query
//----------------------------------------------------------------------------
gboolean
sql_query(SQL_SESSION *sid, const char *query) 
{
 	if (mysql_query(&sid->id_db, query)!=0) {
		fprintf(stderr, "Error query='%s'\n", query);
		return sql_check_error(sid);
	}
	if (sid->sres) sql_free(sid);
	if ((sid->sres = mysql_use_result(&sid->id_db))==NULL) return sql_check_error(sid);
	return TRUE;
}

//----------------------------------------------------------------------------
//								  sql_free
//----------------------------------------------------------------------------
gboolean
sql_free(SQL_SESSION *sid) 
{
 	mysql_free_result(sid->sres);
	sid->sres = NULL;
 	
	return TRUE;
}

//----------------------------------------------------------------------------
//								sql_fetch_row
//----------------------------------------------------------------------------
SQL_ROW 
sql_fetch_row(const SQL_SESSION *sid) 
{
 	return mysql_fetch_row(sid->sres);
}

//----------------------------------------------------------------------------
//								sql_num_fields
//----------------------------------------------------------------------------
gboolean
sql_num_fields(const SQL_SESSION *sid) 
{
 	return mysql_num_fields(sid->sres);
}

//----------------------------------------------------------------------------
//								sql_list_tables
//----------------------------------------------------------------------------
gboolean
sql_list_tables(SQL_SESSION *sid, const char *wild)
{
	if (sid->sres) sql_free(sid);
	
	if ((sid->sres = mysql_list_tables(&sid->id_db, wild)) == NULL) {
		return sql_check_error(sid);
	}
	
	return TRUE;
}
