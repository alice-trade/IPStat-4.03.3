#ifndef _GLIST_M_H 
#define _GLIST_M_H 


#include "gfunc.h"

void   		g_list_free(GList *list);
GList 		*g_list_append(GList *list, gpointer data);
GList 		*g_list_prepend(GList *list, gpointer data);
GList 		*g_list_insert(GList *list, gpointer data, gint position);
GList 		*g_list_insert_sorted(GList *list, gpointer data, GCompareFunc func);
GList 		*g_list_concat(GList *list1, GList *list2);
GList 		*g_list_remove(GList *list, gpointer data);
GList 		*g_list_reverse(GList *list);
GList 		*g_list_copy(GList *list);
GList		*g_list_copy2(GList *list, size_t size_object);					// копирование с содержимым (требуется отдельное удаление)
GList 		*g_list_find(GList *list, gpointer data);
GList 		*g_list_find_custom(GList *list, gpointer data, GCompareFunc func);
gint   		g_list_position(GList *list, GList *link);
gint   		g_list_index(GList *list, gpointer data);
GList 		*g_list_last(GList *list);
guint  		g_list_length(GList *list);
void   		g_list_foreach(GList *list, GFunc func, gpointer user_data);
GList 		*g_list_sort(GList *list, GCompareFunc compare_func, gpointer user_data);


#endif /* include/glist_m.h */
