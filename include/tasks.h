/*
  (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@perm.ru]
*/

#ifndef _TASKS_H
#define _TASKS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gfunc.h"

gboolean	task_check_limit(void*);
gboolean	task_every_day(void*);
gboolean	task_every_week(void*);
gboolean	task_every_month(void*);

#endif /* include/types.h */
