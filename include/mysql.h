/*
  (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _MYSQL_H
#define _MYSQL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#endif

#ifdef HAVE_MYSQL_MYSQL_H
#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#else
#ifdef HAVE_MYSQL_H
#include <errmsg.h>
#include <mysql.h>
#endif
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "gfunc.h"

#include "types.h"


typedef MYSQL_ROW  SQL_ROW;

typedef struct SQL_SESSION
{
 	MYSQL           id_db;
 	MYSQL_RES      *sres;
} SQL_SESSION;

SQL_SESSION     *sql_open  (const char*, const char*, const char*, const char*);
gboolean		sql_close (SQL_SESSION*);
gboolean  		sql_exec  (SQL_SESSION*, const char*);
gboolean		sql_query (SQL_SESSION*, const char*);
gboolean		sql_list_tables(SQL_SESSION*, const char*);
gboolean		sql_free  (SQL_SESSION*);
SQL_ROW			sql_fetch_row (const SQL_SESSION*);
gboolean		sql_num_fields(const SQL_SESSION*);
gboolean		sql_check_error(SQL_SESSION*);

#endif /* include/mysql.h */
