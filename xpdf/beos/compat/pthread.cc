//========================================================================
//
// Compatibility pthread functions.
//
// Copyright 2004-2005 Michael W. Pfeiffer
//
//========================================================================

#include "pthread.h"

void pthread_mutex_init(pthread_mutex_t* mutex, void* flags)
{
	*mutex = create_sem(1, "pthread_mutex");
}

void pthread_mutex_destroy(pthread_mutex_t* mutex){
	delete_sem(*mutex);
	*mutex = 0;
}


void pthread_mutex_lock(pthread_mutex_t* mutex){
	while (acquire_sem(*mutex) == B_INTERRUPTED);
}


void pthread_mutex_unlock(pthread_mutex_t* mutex){
	release_sem(*mutex);
}

