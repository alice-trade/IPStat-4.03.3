/*
 * threads.c	Interface for administrative.
 *
 * Version:	$Id: threads.c,v 1.2 2006/02/02 10:24:19 kornet Exp $
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
 *   (C) 2001-2004 Kukuev Dmitry aka Kornet [kornet@code-art.ru]
*/

static const char rcsid[] = "$Id: threads.c,v 1.2 2006/02/02 10:24:19 kornet Exp $";

#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#ifndef _GNU_SOURCE
 #define _GNU_SOURCE
#endif
#include <string.h>

#include "../include/threads.h"

#define PTHREAD_STACK_MIN 16384


THREAD_HANDLE		*IPStat_spawn_thread(THREAD_POOL *, time_t);	// создание потока
void 			IPStat_drop_thread(THREAD_POOL *, THREAD_HANDLE *);
void			IPStat_request_handler(THREAD_HANDLE *);
void			IPStat_request_enqueue(THREAD_POOL *, void *request);
void			IPStat_request_dequeue(THREAD_POOL *, void **request);

// создание пула
THREAD_POOL
*IPStat_pool_init(INIT_POOL *init_pool, int (*request_func)(void *))
{
	int			i, rcode;
	time_t			now;
	THREAD_POOL		*pool;

	now = time(NULL);

	if (request_func == NULL) return NULL;
		
	if ((pool = (THREAD_POOL*) malloc(sizeof(THREAD_POOL))) == NULL){
		fprintf(stderr, "[IPStat_pool_init]: Error allocate memory\n");
		return NULL;
	}
	
	memset(pool, 0, sizeof(THREAD_POOL));
	
	pool->head = NULL;
	pool->tail = NULL;
	pool->total_threads = 0;
	pool->max_thread_num = 1;
	pool->request_count = 0;
	pool->time_last_spawned = 0;
	pool->active_threads = 0;
	pool->request_func = request_func;
	
	if (init_pool!=NULL){
		pool->start_threads = init_pool->start_threads;
		pool->max_threads = init_pool->max_threads;
		pool->min_spare_threads = init_pool->min_spare_threads;
		pool->max_spare_threads = init_pool->max_spare_threads;
		pool->max_requests_per_thread = init_pool->max_requests_per_thread;
		pool->cleanup_delay = init_pool->cleanup_delay;
	}else{
		pool->start_threads = 5;
		pool->max_threads = 12;
		pool->min_spare_threads = 3;
		pool->max_spare_threads = 20;
		pool->max_requests_per_thread = 0;
		pool->cleanup_delay = 5;
	}
	

	rcode = sem_init(&pool->semaphore, 0, SEMAPHORE_LOCKED);
	if (rcode != 0) {
		fprintf(stderr, "[IPStat_pool_init]: Failed to initialize semaphore: %s", strerror(errno));
		return NULL;
	}

	rcode = pthread_mutex_init(&pool->mutex, NULL);
	if (rcode != 0) {
		fprintf(stderr, "[IPStat_pool_init]: Failed to initialize mutex: %s", strerror(errno));
		return NULL;
	}

	pool->queue_size = 256;
	if ((pool->queue = (request_queue_t *) malloc(sizeof(*pool->queue) * pool->queue_size))==NULL){
		free(pool);
		sem_destroy(&pool->semaphore);
		pthread_mutex_destroy(&pool->mutex);
		return NULL;
	}
	memset(pool->queue, 0, (sizeof(*pool->queue) * pool->queue_size));

	for (i = 0; i < pool->start_threads; i++) {
		if (IPStat_spawn_thread(pool, now) == NULL) {
			return NULL;
		}
	}
	
	return pool;
}

// Удаление пула
void 
IPStat_pool_free(THREAD_POOL *pool)
{
	THREAD_HANDLE 	*handle, *next;

	for (handle = pool->head; handle; handle = next) {
		next = handle->next;
		handle->status = THREAD_CANCELLED;
	}
	for (handle = pool->head; handle; handle = next) {
		next = handle->next;
		pthread_join(handle->pthread_id, NULL);
	}

	free(pool);
	pool = NULL;
}

// Функция создаёт новый поток
THREAD_HANDLE 
*IPStat_spawn_thread(THREAD_POOL *pool, time_t now)
{
	int 			rcode;
	THREAD_HANDLE 	*handle;
	pthread_attr_t 	attr;

	if (pool->total_threads >= pool->max_threads) {
		return NULL;
	}

	handle = (THREAD_HANDLE *) malloc(sizeof(THREAD_HANDLE));
	memset(handle, 0, sizeof(THREAD_HANDLE));
	
	handle->prev 			= NULL;
	handle->next 			= NULL;
	handle->pthread_id 		= 0;
	handle->thread_num 		= pool->max_thread_num++;
	handle->request_count 		= 0;
	handle->request_func 		= pool->request_func;
	handle->pool			= (void*) pool;
	handle->status 			= THREAD_RUNNING;
	handle->timestamp 		= time(NULL);
	handle->request			= STATUS_SLEEP;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 0x4000);

	rcode = pthread_create(&handle->pthread_id, &attr, (void*)&IPStat_request_handler, handle);
	if (rcode != 0) {
		fprintf(stderr, "FATAL: Thread create failed: %s\n", strerror(rcode));
		return NULL;
	}
	pthread_attr_destroy(&attr);

	pool->total_threads++;

	if (pool->tail) {
		pool->tail->next 	= handle;
		handle->prev 		= pool->tail;
		pool->tail 		= handle;
	} else {
		pool->head 		= pool->tail = handle;
	}

	pool->time_last_spawned = now;
	fprintf(stderr, "[IPStat_spawn_thread]: Thread %d created\n", handle->thread_num);
	return handle;
}

// Удаляет поток
void 
IPStat_drop_thread(THREAD_POOL *pool, THREAD_HANDLE *handle)
{
	THREAD_HANDLE *prev;
	THREAD_HANDLE *next;

	prev = handle->prev;
	next = handle->next;

	pool->total_threads--;

	if (prev == NULL) {
		pool->head = next;
	} else {
		prev->next = next;
	}

	if (next == NULL) {
		pool->tail = prev;
	} else {
		next->prev = prev;
	}

	free(handle);
	handle = NULL;
}

// Выполнение потока
void 
IPStat_request_handler(THREAD_HANDLE *handle)
{
	THREAD_HANDLE	  		*self = (THREAD_HANDLE *) handle;
	THREAD_POOL			*pool = (THREAD_POOL *) handle->pool;
	
	sigset_t 			set;
	void				*request;
	struct timespec			timeout;
	
	sigemptyset(&set);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	
	do {
re_wait:
		timeout.tv_sec  = time(NULL)+1;
		timeout.tv_nsec = 0;

		if (sem_timedwait(&pool->semaphore, &timeout) != 0) {
		    if (self->status == THREAD_CANCELLED) break;
		    
		    if (errno == EINTR || errno == ETIMEDOUT) {
				goto re_wait;
			}
			
			fprintf(stderr, "Thread %d failed waiting for semaphore: %s: Exiting\n", self->thread_num, strerror(errno));
			break;
		}

		IPStat_request_dequeue(pool, &request);
		if (!request) continue;

		self->request_count++;
		self->request = STATUS_REQUEST;
		    pool->request_func(request);
		self->request = STATUS_SLEEP;
		request = NULL;

		pthread_mutex_lock(&pool->mutex);
			pool->active_threads--;
		pthread_mutex_unlock(&pool->mutex);
	} while (self->status != THREAD_CANCELLED);
	fprintf(stderr, "Thread #%d cancelled\n", (int) pthread_self());
	self->status = THREAD_EXITED;
	return;
}

// Взять запрос из очереди
void 
IPStat_request_dequeue(THREAD_POOL *pool, void **request)
{
    pthread_mutex_lock(&pool->mutex);
	
	if (pool->queue_head == pool->queue_tail) {
	    pthread_mutex_unlock(&pool->mutex);
	    *request = NULL;
	    return;
	}
	*request = pool->queue[pool->queue_head].request;

	pool->queue_head++;
	pool->queue_head &= (pool->queue_size - 1);

	pool->active_threads++;

    pthread_mutex_unlock(&pool->mutex);

    return;
}

// Положить запрос в очередь
void 
IPStat_request_enqueue(THREAD_POOL *pool, void *request)
{
	int num_entries;

	pthread_mutex_lock(&pool->mutex);

	pool->request_count++;

	if ((pool->queue_head == pool->queue_tail) && (pool->queue_head != 0)) {
		pool->queue_head = pool->queue_tail = 0;
	}

	num_entries = ((pool->queue_tail + pool->queue_size) - pool->queue_head) % pool->queue_size;
	if (num_entries == (pool->queue_size - 1)) {
		request_queue_t *new_queue;

		if (pool->queue_size >= 65536) {
			pthread_mutex_unlock(&pool->mutex);

			fprintf(stderr, "!!! ERROR !!! The server is blocked: discarding new request\n");
			return;
		}

		if ((new_queue = (request_queue_t *) malloc(sizeof(*new_queue) * pool->queue_size * 2))==NULL){
			pthread_mutex_unlock(&pool->mutex);

			fprintf(stderr, "[IPStat_request_enqueue]: Error allocate memory\n");
			return;
		}

		memcpy(new_queue, pool->queue, sizeof(*new_queue) * pool->queue_size);
		memset(new_queue + sizeof(*new_queue) * pool->queue_size, 0, sizeof(*new_queue) * pool->queue_size);

		free(pool->queue);
		pool->queue = new_queue;
		pool->queue_size *= 2;
	}

	pool->queue[pool->queue_tail].request = request;
	pool->queue_tail++;
	pool->queue_tail &= (pool->queue_size - 1);

	pthread_mutex_unlock(&pool->mutex);

	sem_post(&pool->semaphore);

	return;
}

// Зарегестрировать запрос
int
IPStat_pool_addrequest(THREAD_POOL *pool, void *request)
{
    if (pool->active_threads == pool->total_threads) {
	IPStat_spawn_thread(pool, time(NULL));
    }
    IPStat_request_enqueue(pool, request);
    return 1;
}

// Очистка пула
int
IPStat_pool_clean(THREAD_POOL *pool, time_t now)
{
	int 			spare;
	int 			i, total;
	THREAD_HANDLE 	*handle, *next;
	int 			active_threads;
	static time_t 	last_cleaned = 0;

	for (handle = pool->head; handle; handle = next) {
		next = handle->next;

		if (handle->status == THREAD_EXITED) {
			IPStat_drop_thread(pool, handle);
		}
	}

	active_threads	= pool->active_threads;
	spare 			= pool->total_threads - active_threads;

	if (spare < pool->min_spare_threads) {
		total = pool->min_spare_threads - spare;

		for (i = 0; i < total; i++) {
			handle = IPStat_spawn_thread(pool, now);
			if (handle == NULL) {
				return 0;
			}
		}

		return 1;
	}

	if (now == last_cleaned) {
		return 1;
	}
	last_cleaned = now;

	if ((now - pool->time_last_spawned) < pool->cleanup_delay) {
		return 1;
	}

	if (spare > pool->max_spare_threads) {
		spare -= pool->max_spare_threads;
		for (handle = pool->head; (handle != NULL) && (spare > 0) ; handle = next) {
			next = handle->next;
			if ((handle->request == STATUS_SLEEP) &&
				(handle->status == THREAD_RUNNING)) {
				handle->status = THREAD_CANCELLED;
				sem_post(&pool->semaphore);
				spare--;
				break;
			}
		}
	}

	if (pool->max_requests_per_thread > 0) {
		for (handle = pool->head; handle; handle = next) {
			next = handle->next;
			if ((handle->request == STATUS_SLEEP) && 
				(handle->status == THREAD_RUNNING) &&
			    (handle->request_count > pool->max_requests_per_thread)) {
				handle->status = THREAD_CANCELLED;
				sem_post(&pool->semaphore);
			}
		}
	}
	return 1;
}

