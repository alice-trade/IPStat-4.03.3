#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>

#define _XOPEN_SOURCE 600
#include <semaphore.h>

#define 	SEMAPHORE_LOCKED	(0)
#define 	SEMAPHORE_UNLOCKED	(1)

#define 	THREAD_RUNNING		(1)
#define 	THREAD_CANCELLED	(2)
#define 	THREAD_EXITED		(3)

#define		STATUS_REQUEST		(0)
#define		STATUS_SLEEP		(1)

typedef struct request_queue_t 		// структура очереди 
{
	void	    	  	*request;
} request_queue_t;

typedef struct THREAD_HANDLE 				// хендл потока
{
	pthread_t            pthread_id;		// идентификатор потока
	int                  thread_num;		// номер потока
	int                  status;			// статус потока
	int					 request;
	int (*request_func)(void *);			// Функция обработчика 
	void				 *pool;				// указатель на пул
	unsigned int         request_count;		// количество запросов
	time_t               timestamp;			// Время
	struct THREAD_HANDLE *prev;				// предыдущий хендл потока
	struct THREAD_HANDLE *next;				// Следующий хендл потока
} THREAD_HANDLE;

typedef struct THREAD_POOL					// Идентификатор пула
{
	THREAD_HANDLE 	*head;
	THREAD_HANDLE 	*tail;
    int (*request_func)(void *);			// Функция обработчика                                                                                                                                                         
	int 			total_threads;			// Количество потоков
	int 			max_thread_num;			// Максимальный номер потока
	int 			start_threads;			// Начальное количество потоков
	int 			max_threads;			// Максимальный поток
	int 			min_spare_threads;		
	int 			max_spare_threads;		
	unsigned int 	max_requests_per_thread;// максимум запросов на поток
	unsigned long 	request_count;			// Количество запросов
	time_t 			time_last_spawned;		// Время последнего создания потока
	int 			cleanup_delay;			// Задержка очистки
	sem_t           semaphore;				// Семафор
	pthread_mutex_t mutex;					// Мутекс
	int             active_threads;			// Активных потоков
	int             queue_head;				// Начало очереди
	int             queue_tail;				// Конец очереди
	int             queue_size;				// Размер очереди
	request_queue_t *queue;					// Указатель на очередь
} THREAD_POOL;

typedef struct INIT_POOL					// Идентификатор пула
{
	int 			start_threads;			// Начальное количество потоков
	int 			max_threads;			// Максимальный поток
	int 			min_spare_threads;		
	int 			max_spare_threads;		
	unsigned int 	max_requests_per_thread;// максимум запросов на поток
	int 			cleanup_delay;			// Задержка очистки
} INIT_POOL;

THREAD_POOL		*IPStat_pool_init(INIT_POOL *, int (*request_func)(void *));	// функция инициализации пула
void 			IPStat_pool_free(THREAD_POOL *);
int			IPStat_pool_clean(THREAD_POOL *, time_t now);
int			IPStat_pool_addrequest(THREAD_POOL *, void *request);

#endif /* include/thread.h */
