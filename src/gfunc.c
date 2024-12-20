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

#include "../include/gfunc.h"

#include <string.h>
#include <stdlib.h>

void*
g_malloc0(size_t size)
{
    void *ptr = NULL;
    
    if (size == 0) return NULL;
    if ((ptr = malloc(size))==NULL) return NULL;
    memset(ptr, '\0', size);
    
    return ptr;
}

void
g_free(void *ptr)
{
    if (ptr) free(ptr);
    ptr = NULL;
}

