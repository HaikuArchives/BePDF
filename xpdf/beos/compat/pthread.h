//========================================================================
//
// Compatibility pthread functions.
//
// Copyright 2004-2005 Michael W. Pfeiffer
//
//========================================================================

#ifndef _P_THREAD_H
#define _P_THREAD_H

#include <OS.h>
		
typedef sem_id pthread_mutex_t;
		
void pthread_mutex_init(pthread_mutex_t* mutex, void* flags);
void pthread_mutex_destroy(pthread_mutex_t* mutex);
void pthread_mutex_lock(pthread_mutex_t* mutex);
void pthread_mutex_unlock(pthread_mutex_t* mutex);

#endif
