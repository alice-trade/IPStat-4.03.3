
#ifndef _LIBXML_H
#define _LIBXML_H

#include <unistd.h>

#include "gfunc.h"

#define XML_LENGTH_KEY		50
#define XML_LENGTH_VALUE	255
#define XML_LENGTH		2048

typedef struct st_xml_session
{
	GSList	*list;
	char	*ptr;
} XML_SESSION;

XML_SESSION		*xpath_init(const char *buffer);
gboolean		xpath_free(XML_SESSION *sid);
gboolean		xpath_get_param(XML_SESSION *sid, const char *key, char *value, size_t size_value);

XML_SESSION		*tree_init(void);
gboolean		tree_free(XML_SESSION *sid);

gboolean		tree_setroot(XML_SESSION *sid, char *root);
gboolean		tree_addnode(XML_SESSION *sid, char *name, char *value);
gboolean		tree_getxml(XML_SESSION *sid, char *ptr, int size);

#endif /* include/lib_xml.h */
