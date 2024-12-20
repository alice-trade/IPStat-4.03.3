#ifndef _GFUNC_H 
#define _GFUNC_H 

#include <pthread.h>
#include <arpa/inet.h>

#define g_list_next(slist)	((slist) ? (((GList *)(slist))->next) : NULL)

typedef void*		gpointer;
typedef int		gint;
typedef unsigned int	guint;
typedef const void*	gconstpointer;
typedef int		gboolean;
typedef uint8_t		guint8;

typedef gint (*GCompareFunc)(gconstpointer  a, gconstpointer  b);
typedef void (*GFunc)(gpointer data, gpointer user_data);
typedef gint (*GCompareDataFunc)(gconstpointer a, gconstpointer b, gpointer user_data);
                                                                                                  
typedef struct GList
{
    gpointer		data;
    struct GList	*next;
} GList;

typedef struct GSList
{
    pthread_mutex_t	mutex;
    GList		*list;
} GSList;

void		*g_malloc0(size_t size);
void		g_free(void *ptr);

#endif /* include/gfunc.h */
