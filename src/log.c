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

#include "../include/log.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

typedef struct IPSTAT_NAME_NUMBER {
        const char      *name;
        int             number;
} IPSTAT_NAME_NUMBER;

static char			log_file[FILENAME_MAX];

#define CHECK_CORRECT_LOG if (!st_log_api) return FALSE

static CONF_PARSER module_config[] = {
        { "log_file", CF_TYPE_STRING, FILENAME_MAX, log_file, "/opt/IPStat/log/IPStat.log" },
        { NULL, -1, 0, NULL, NULL }
};

static IPSTAT_NAME_NUMBER levels[] = {
        { ": Debug: ",          IP_DEBUG   },
        { ": Info: ",           IP_INFO  },
        { ": Notice: ",         IP_NOTICE },
        { ": Warning: ",		IP_WARNING  },
        { ": Error: ",          IP_ERR   },
        { ": Critical: ",		IP_CRIT   },
        { ": Alert: ",          IP_ALERT   },
        { NULL, 0 }
};

static gboolean 		vlog(int lvl, const char *fmt, va_list ap);
static const char 		*IPStat_int2str(const IPSTAT_NAME_NUMBER *table, int number, const char *def);
static gboolean			print_log(int lvl, const char *msg, ...);
static pthread_mutex_t 	log_mutex = PTHREAD_MUTEX_INITIALIZER;

#define LOG_LOCK 	pthread_mutex_lock(&log_mutex)
#define LOG_UNLOCK 	pthread_mutex_unlock(&log_mutex)

LOG_API
*log_init(CONF_API *conf)
{
	LOG_API	*log;
	
	if ((log = (LOG_API*) g_malloc0(sizeof(LOG_API))) == NULL) {
		fprintf(stderr, "[log.c][log_init]: error allocate memory for log_api\n");
		return NULL;
	}

	if (conf) conf->parse(module_config);
	log->print_log 		= print_log;
	return log;	
}

gboolean	
log_free(LOG_API *log)
{
	if (log) g_free(log);
	log = NULL;

	return TRUE;
}

gboolean
print_log(int lvl, const char *msg, ...)
{
	va_list ap;
	int r;

	va_start(ap, msg);
	r = vlog(lvl, msg, ap);
	va_end(ap);

	return r;
}

static gboolean
vlog(int lvl, const char *fmt, va_list ap)
{
	FILE *msgfd = NULL;
	unsigned char *p;
	char buffer[8192];
	int len;
	const char *s;
	time_t timeval;
	
	
	if (log_file == NULL) {
		fprintf(stderr, "IPStat: not correct log_file\n");
		return FALSE;
	}
	
	LOG_LOCK;
		if ((msgfd = fopen(log_file, "a")) == NULL) {
			fprintf(stderr, "IPStat: Couldn't open %s for logging: %s\n", log_file, strerror(errno));
			fprintf(stderr, "  (");
			vfprintf(stderr, fmt, ap);  /* the message that caused the log */
			fprintf(stderr, ")\n");
			return FALSE;
		}
		timeval = time(NULL);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d.%H:%M:%S", localtime(&timeval));
		s = IPStat_int2str(levels, lvl, ": ");
		strcat(buffer, s);
		len = strlen(buffer);
#ifdef HAVE_VSNPRINTF
		vsnprintf(buffer + len, sizeof(buffer) - len -1, fmt, ap);
#else
		vsprintf(buffer + len, fmt, ap);
		if (strlen(buffer) >= sizeof(buffer) - 1) _exit(42);
#endif
		for (p = (unsigned char *)buffer; *p != '\0'; p++) {
			if (*p == '\r' || *p == '\n') *p = ' ';
			else if (*p < 32 || (*p >= 128 && *p <= 160)) *p = '?';
		}
		strcat(buffer, "\n");
	
		fputs(buffer, msgfd);
		fclose(msgfd);
	LOG_UNLOCK;
	return TRUE;
}

static const char 
*IPStat_int2str(const IPSTAT_NAME_NUMBER *table, int number, const char *def)
{
	const IPSTAT_NAME_NUMBER *this;

	for (this = table; this->name != NULL; this++) {
		if (this->number == number) {
			return this->name;
		}
	}
	return def;
}
