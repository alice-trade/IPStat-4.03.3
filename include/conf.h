/*
  (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _CONF_H
#define _CONF_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "types.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "gfunc.h"

#define	CF_TYPE_STRING	1
#define CF_TYPE_INT		2
#define CF_TYPE_LONG	3

typedef struct CONF_PARSER {
	const char 	*name;				/* name var */
	int 		type;               /* type variable */
	size_t		size;				/* size value */
	void 		*vol;               /* variable */
	const char 	*defvol;            /* default volume */
} CONF_PARSER;

typedef struct CONF_API
{
	gboolean	(*parse) (CONF_PARSER *variables);
	char		*cf_file;
} CONF_API;

CONF_API	*conf_init(char*);
gboolean	conf_free(CONF_API*);


#endif /* include/conf.h */
