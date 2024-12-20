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

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/libxml.h"
#include "../include/types.h"
#include "../include/misc.h"
#include "../include/glist.h"

#ifndef _GNU_SOURCE
 #define _GNU_SOURCE
#endif
#include <string.h>

static void	traverse(gpointer data, gpointer user_data);
static gint	compare(gconstpointer a, gconstpointer b);

typedef struct st_xml_node
{
    char	key[XML_LENGTH_KEY];
    char	value[XML_LENGTH_VALUE];
} XML_NODE;

const char *
m_strtok_r (char *s, const char *delim, char **save_ptr)
{
    char *token;

    if (s == NULL) s = *save_ptr;

/* Scan leading delimiters.  */
    s += strspn (s, delim);
    if (*s == '\0') {
       *save_ptr = s;
       return NULL;
    }

/* Find the end of the token.  */
    token = s;
    s = strpbrk(token, delim);
    if (s == NULL)
/* This token finishes the string.  */
       *save_ptr = strchr(token, '\0');
    else {
/* Terminate the token and make *SAVE_PTR point past it.  */
       *s = '\0';
       *save_ptr = s + 1;
    }
    return token;
}

void
parse_arg(char *buffer, int *argc, char *argv[], int max_tok, const char *delim)
{
    int		i	= 0;
    char	*ptr	= NULL;
    char	*buf_ptr= NULL;

    *argc = 0;
    if (!*buffer) return;
    if ((ptr = m_strtok_r(buffer, delim, &buf_ptr)) == NULL) return;
    
    argv[i++] = strdup(ptr);
                                                                                
    while ((ptr = m_strtok_r('\0', delim, &buf_ptr))!=NULL) {
	if (i>max_tok) break;
        argv[i++] = strdup(ptr);
    }

    *argc = i;
    return;
}


                                                                                        
XML_SESSION
*xpath_init(const char *buffer) 
{
    XML_SESSION *sid = NULL;
    char	*ptr;
    char	*argv[50];
    int		argc, i;
    
    if ((sid = (XML_SESSION *) malloc(sizeof(XML_SESSION))) == NULL) return NULL;
    
    sid->list	= NULL;
    sid->ptr	= NULL;
    
    if ((ptr = (char*) malloc(strlen(buffer)+1)) == NULL) {
	free(sid);
	return NULL;
    }
	strncpy(ptr, buffer, strlen(buffer));
    
	parse_arg(ptr, &argc, argv, 49, "\n");
	    for(i=0;i<argc;i++){
		char		tmp[XML_LENGTH], *t;
		XML_NODE	*node;
		
		strncpy(tmp, argv[i], XML_LENGTH-1);
		if ((t = strchr(tmp, '=')) != NULL) {
		    *t++ = '\0';
		    if ((node = malloc(sizeof(XML_NODE))) != NULL){
    			strncpy(node->key, tmp, XML_LENGTH_KEY-1);
			strncpy(node->value, t, XML_LENGTH_VALUE-1);
			sid->list = g_list_append(sid->list, node);
		    }
		}
	    }
	for(i=0;i<argc;i++) free(argv[i]);
    free(ptr);
    
    return sid;
}

gboolean 
xpath_free(XML_SESSION *sid) 
{
    GList  *t_tek, *t_tmp = (GList*) sid->list;
    
    while ((t_tek = t_tmp) != NULL) {
	t_tmp = g_list_next(t_tmp);
        if (t_tek->data) free(t_tek->data);
    }
    g_list_free(sid->list);
    sid->list = NULL;
    
    free(sid);
    sid = NULL;
	
    return TRUE;
}

gboolean
xpath_get_param(XML_SESSION *sid, const char *key, char *value, size_t size_value)
{
    GList	*ptr;
    XML_NODE	node;
    
    strncpy(node.key, key, XML_LENGTH_KEY-1);
    bzero(value, size_value);
    
    ptr = g_list_find_custom(sid->list, &node, compare);
    if (ptr) {
	XML_NODE *t_node = (XML_NODE*) ptr->data;
	strncpy(value, t_node->value, size_value-1);
	return TRUE;
    }
    
    return FALSE;
}

XML_SESSION
*tree_init(void) 
{
	XML_SESSION *sid = NULL;

	if ((sid = (XML_SESSION *) malloc(sizeof(XML_SESSION))) == NULL) return NULL;
	sid->list	= NULL;
	sid->ptr	= NULL;	
	
	return sid;
}

gboolean 
tree_free(XML_SESSION *sid) 
{
    GList  *t_tek, *t_tmp = (GList*) sid->list;
    
    while ((t_tek = t_tmp) != NULL) {
	t_tmp = g_list_next(t_tmp);
        if (t_tek->data) free(t_tek->data);
    }
    g_list_free(sid->list);
    sid->list = NULL;
    if (sid) free(sid);
    sid = NULL;
	
    return TRUE;
}

gboolean		
tree_addnode(XML_SESSION *sid, char *name, char *value)
{
    XML_NODE	*node;
        
    if ((node = malloc(sizeof(XML_NODE))) == NULL) return FALSE;
    
    strncpy(node->key, name, XML_LENGTH_KEY-1);
    strncpy(node->value, value, XML_LENGTH_VALUE-1);
    sid->list = g_list_append(sid->list, node);

    return TRUE;
}

gboolean		
tree_getxml(XML_SESSION *sid, char *ptr, int size)
{
    bzero(ptr, size);
    g_list_foreach(sid->list, traverse, ptr);
    return TRUE;
}

static void
traverse(gpointer data, gpointer user_data)
{
    XML_NODE 		*node = (XML_NODE*) data;
    unsigned char 	*ptr = (unsigned char*) user_data;
    char		buffer[1024];
    
    sprintf(buffer, "%s=%s\n", node->key, node->value);
    strcat((char*)ptr, buffer);
}

static gint
compare(gconstpointer a, gconstpointer b)
{
    XML_NODE  *node1 = (XML_NODE*) a;
    XML_NODE  *node2 = (XML_NODE*) b;
        
    return (gint) strncmp(node1->key, node2->key, XML_LENGTH_KEY-1);
}
