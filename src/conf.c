/*
 * config.c	Read the IPStat.conf file.
 *
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
 *   (C) 2001-2004 Dmitry Kukuev aka Kornet [kornet@code-art.ru]
*/

static const char rcsid[] = "$Id: conf.c,v 1.1.1.1 2005/12/19 08:53:20 kornet Exp $";

#include "../include/conf.h"
#include "../include/gfunc.h"

#include <stdlib.h>
#include <string.h>

static CONF_API		*st_conf_api = NULL;
#define CHECK_CORRECT_CONF if (!st_conf_api) return FALSE

static gboolean			conf_parse(CONF_PARSER *variables);

CONF_API 	
*conf_init(char *conf_file)
{
	CONF_API	*conf;
	
	if (!conf_file) return NULL;
		
	if ((st_conf_api = conf = (CONF_API*) g_malloc0(sizeof(CONF_API))) == NULL) {
		fprintf(stderr, "[conf.c][conf_init]: error allocate memory for conf_api\n");
		return NULL;
	}

	conf->parse 		= conf_parse;
	conf->cf_file		= conf_file;

	return conf;
}

gboolean	
conf_free(CONF_API *conf)
{
	if (conf) g_free(conf);
	conf = NULL;

	return TRUE;
}

static gboolean
conf_parse(CONF_PARSER *variables)
{
 	char 		buffer[1024], *str;
	CONF_PARSER *this;
 	FILE 		*fd;
	int			t_int;
	long		t_long;

	CHECK_CORRECT_CONF;
	for (this = variables; this->name != NULL; this++) {
		switch (this->type) {
			case CF_TYPE_STRING:
						strncpy(this->vol, this->defvol, this->size);
						break;
			case CF_TYPE_INT:
						t_int = atoi(this->defvol);
						memcpy(this->vol, &t_int, sizeof(int));
						break;
			case CF_TYPE_LONG:
						t_long = atol(this->defvol);
						memcpy(this->vol, &t_long, sizeof(long));
						break;
				
		}
	}
	
	fd = fopen(st_conf_api->cf_file, "r");

 	if (!fd) return FALSE;
 	
	
	while(fgets(buffer,sizeof(buffer),fd))
 	{
  		if(*buffer && buffer[0]!='#' && buffer[0]!='\n'){
   			for(str=buffer;*str;str++) if (*str=='\n') *str='\0';
   			if ((str = strchr(buffer, '='))!=NULL){
				*str='\0'; str++;
				for (this = variables; this->name != NULL; this++) {
                	if (strncasecmp(this->name, buffer, strlen(this->name))==0) {
                        switch (this->type) {
							case CF_TYPE_STRING:
										strncpy(this->vol, str, this->size);
										break;
							case CF_TYPE_INT:
										t_int = atoi(str);
										memcpy(this->vol, &t_int, sizeof(int));
										break;							
							case CF_TYPE_LONG:
										t_long = atoi(str);
										memcpy(this->vol, &t_long, sizeof(long));
										break;							
							
						}
                	}
        		}
			}
		}
	}
	fclose(fd);
	return TRUE;
}
