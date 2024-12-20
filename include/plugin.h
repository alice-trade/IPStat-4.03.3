
#ifndef _PLUGIN_H
#define _PLUGIN_H

#include "types.h"

typedef struct PLUGIN_INFO
{
	unsigned int			api_version;
	IPStatPluginType		type;
	unsigned long			flags;
	IPStatPluginPriority    priority;
                                                                                                 
	char					*id;
	char					*name;
	char					*version;
	char					*summary;
	char					*description;
	char					*author;
	char					*homepage;
} PLUGIN_INFO;

typedef struct PLUGIN_API
{
	void					*license;
	void					*store;
	void					*server;
	void					*sched;
	void					*core;
	void					*conf;
	void					*log;
} PLUGIN_API;

#endif /* include/plugin.h */
