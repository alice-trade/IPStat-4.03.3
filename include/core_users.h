
#ifndef _CORE_USERS_H
#define _CORE_USERS_H

#include "types.h"
#include "conf.h"
#include "log.h"
#include "gfunc.h"

#include <time.h>
#include <unistd.h>
#include <stdio.h>

typedef struct NODE_RULE
{
        IP_id               id;
		IP_id				id_type;
		IP_id				id_group;
		IP_id				service;
        IP_flag             direct;
        IP_proto            proto;
        IP_addr             net;
        IP_mask             mask;
        IP_ports            ports;
        IP_flag             action;
        IP_flag             status;
        float               price;
        IP_size             traffic;
        char                md5[33];
        time_t              time_live;
} NODE_RULE;

typedef struct NODE_USER
{
        IP_id               id;                     // Идентификатор пользователя
		IP_id				id_group;				// Идентификатор группы
        IP_addr				addr;           		// Адрес пользователя
        char				mac[18];
        IP_size				traffic;        		// Объём скаченного трафика
        GSList				*id_rules;      		// Указатель на список правил
        char				md5[33];        		// Хеш пользователя
        IP_flag				status;
        time_t				time_live;      		// Время жизни
} NODE_USER;

typedef struct st_users
{
 	GSList					*id_users;
} USERS;

gboolean		users_init(CONF_API*, LOG_API*);
gboolean		users_free(void);

gboolean		users_auth(IP_id service, void *data, gboolean flag_extern_auth, void *ret);			// аутентификация
gboolean		users_autz(IP_id service, void *data, gboolean flag_extern_autz, void *ret);			// авторизация
gboolean			users_acct(IP_id service, void *data, gboolean flag_extern_acct, void *ret);			// акаунтинг


gboolean		users_sync(void);																		// загрузка информации из базы


gboolean		core_set_store(void*);
gboolean		core_set_server(void*);
gboolean		core_set_sched(void*);

gboolean		register_auth(gboolean (*func_auth)(IP_id service, void *data, void *ret));
gboolean		register_autz(gboolean (*func_autz)(IP_id service, void *data, void *ret));
gboolean		register_acct(gboolean (*func_acct)(IP_id service, void *data, void *ret));

gboolean		unregister_auth(gboolean (*func_auth)(IP_id service, void *data, void *ret));
gboolean		unregister_autz(gboolean (*func_autz)(IP_id service, void *data, void *ret));
gboolean		unregister_acct(gboolean (*func_acct)(IP_id service, void *data, void *ret));

#endif /* include/core_users.h */
