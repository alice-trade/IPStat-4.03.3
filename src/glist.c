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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "../include/glist.h"

//#define DEBUG

static GList*	g_list_sort_merge (GList *l1, GList *l2, GFunc compare_func, gpointer user_data);
static GList*	g_list_sort_real(GList *list, GFunc compare_func, gpointer user_data);

void    
g_list_free(GList *list)
{
    GList	*tek;

    if (list){
	while ((tek = list->next)){
	    list = list->next;
	    free(tek);
	}
    }
}

GList*
g_list_append(GList *list, gpointer data)
{
    GList *ptr;
    GList *last;
      
    if ((ptr = (GList*) malloc(sizeof(struct GList))) == NULL) return NULL;
#ifdef DEBUG
    fprintf(stderr, "[g_list_append]: list=%x, ptr=%x\n", list, ptr);
#endif
    
    ptr->data = data;
    ptr->next = NULL;
            
    if (list){
	last = g_list_last(list);
        last->next = ptr;
                            
	return list;
    }else return ptr;
}

GList*
g_list_prepend(GList *list, gpointer data)
{
    GList *ptr;

    if ((ptr = (GList*) malloc(sizeof(struct GList))) == NULL) return NULL;
#ifdef DEBUG
    fprintf(stderr, "[g_list_prepend]: ptr=%x\n", ptr);
#endif
    ptr->data = data;
    ptr->next = list;

    return ptr;
}

GList*
g_list_insert(GList *list, gpointer data, gint position)
{
    GList *prev_list;
    GList *tmp_list;
    GList *ptr;

    if (position < 0)		return g_list_append (list, data);
    else if (position == 0)	return g_list_prepend (list, data);

    if ((ptr = (GList*) malloc(sizeof(struct GList))) == NULL) return NULL;

    ptr->data = data;

    if (!list) {
	ptr->next = NULL;
	return ptr;
    }

    prev_list = NULL;
    tmp_list = list;

    while ((position-- > 0) && tmp_list){
	prev_list = tmp_list;
	tmp_list = tmp_list->next;
    }

    if (prev_list){
	ptr->next = prev_list->next;
	prev_list->next = ptr;
    }else{
	ptr->next = list;
	list = ptr;
    }
    return list;

}

GList*
g_list_concat(GList *list1, GList *list2)
{
    if (list2){
	if (list1)  g_list_last (list1)->next = list2;
	else  list1 = list2;
    }
    return list1;
}

GList*
g_list_remove(GList *list, gpointer data)
{
    GList *tmp, *prev = NULL;

    tmp = list;
    while (tmp){
	if (tmp->data == data){
	    if (prev)	prev->next = tmp->next;
	    else	list = tmp->next;
	    free(tmp);
            break;
        }
	prev = tmp;
        tmp = prev->next;
    }
    return list;
}

GList*
g_list_reverse(GList *list)
{
    GList *prev = NULL;
  
    while (list){
	GList *next = list->next;

	list->next = prev;

	prev = list;
	list = next;
    }                    
    return prev;
}

GList*
g_list_copy(GList *list)
{
    GList *ptr = NULL;
    GList *last;

    if (list) {
	if ((ptr = (GList*) malloc(sizeof(struct GList))) == NULL) return NULL;
	ptr->data = list->data;
	last = ptr;
	list = list->next;
	while (list){
	    if ((last->next = (GList*) malloc(sizeof(struct GList))) == NULL) return NULL;
	    last = last->next;
	    last->data = list->data;
	    list = list->next;
	}
	last->next = NULL;
    }
    return ptr;
}

GList*
g_list_copy2(GList *list, size_t size_object)
{
    GList *ptr = NULL;
    GList *last;

    if (list) {
	if ((ptr = (GList*) malloc(sizeof(struct GList))) == NULL) return NULL;
	if ((ptr->data = (gpointer) malloc(size_object)) != NULL) {
	
	    memcpy(ptr->data, list->data, size_object);
	    last = ptr;
	    list = list->next;
	    while (list){
		if ((last->next = (GList*) malloc(sizeof(struct GList))) == NULL) return NULL;
		last = last->next;
		if ((last->data = (gpointer) malloc(size_object)) == NULL) return NULL;
		memcpy(last->data, list->data, size_object);
		list = list->next;
	    }
	    last->next = NULL;
	}
    }
    return ptr;
}

GList*
g_list_find(GList *list, gpointer data)
{
    while (list){
	if (list->data == data)	break;
	list = list->next;
    }

    return list;
}

GList*
g_list_find_custom(GList *list, gpointer data, GCompareFunc func)
{
    if (func == NULL) return list;
    while (list){
	if (! func (list->data, data))	return list;
	list = list->next;
    }
    return NULL;
}

gint
g_list_position(GList *list, GList *link)
{
    gint i;

    i = 0;
    while (list){
	if (list == link) return i;
	i++;
        list = list->next;
    }
    return -1;
}

gint
g_list_index(GList *list, gpointer data)
{
    gint i;
  
    i = 0;
    while (list){
        if (list->data == data)	return i;
	i++;
	list = list->next;
    }

    return -1;
}

GList*
g_list_last(GList *list)
{
    if (list){
	while (list->next)
	    list = list->next;
    }    
    return list;
}

guint   
g_list_length(GList *list)
{
    guint length;
  
    length = 0;
    while (list){
	length++;
        list = list->next;
    }                  
    return length;
}

void    
g_list_foreach(GList *list, GFunc func, gpointer user_data)
{
    while (list){
	GList *next = list->next;
	(*func) (list->data, user_data);
        list = next;
    }
}

GList*
g_list_sort(GList *list, GCompareFunc compare_func, gpointer user_data)
{
    return g_list_sort_real(list, (GFunc) compare_func, user_data);
}

GList*
g_list_sort_real(GList *list, GFunc compare_func, gpointer user_data)
{
    GList *l1, *l2;

    if (!list)  return NULL;
    if (!list->next) return list;
    
    l1 = list;
    l2 = list->next;
    while ((l2 = l2->next) != NULL){
	if ((l2 = l2->next) == NULL) break;
	l1=l1->next;
    }
    l2 = l1->next;
    l1->next = NULL;
    
    return  g_list_sort_merge (g_list_sort_real(list, compare_func, user_data),
	    g_list_sort_real(l2, compare_func, user_data),
	    compare_func,
	    user_data);
}


static GList*
g_list_sort_merge (GList *l1, GList *l2, GFunc compare_func, gpointer user_data)
{
    GList list, *l;
    gint cmp;

    l=&list;

    while (l1 && l2){
	cmp = ((GCompareDataFunc) compare_func) (l1->data, l2->data, user_data);
	if (cmp <= 0){
	    l=l->next=l1;
            l1=l1->next;
        }else{
	    l=l->next=l2;
    	    l2=l2->next;
	}
    }
    l->next= l1 ? l1 : l2;
    return list.next;
}
