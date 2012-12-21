/* meeting.h
**
**	Author: Adam Maulis
**	2012.12.18
**	Copyright: GNU GPL v3 or newer
**
**
**	Description: meeting data type & member functions
**
**	Build notes:
**	gcc -O3 -lpthread -o lagmeter lagmeter.c
*/

#ifndef __MEETING_H
#define __MEETING_H


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

/*
** thread meeting functionality
**
*/

typedef struct {
		pthread_mutex_t m_mutex;
		pthread_cond_t m_cond;
		int count;
		int numthreads;  /* max of count */
	} meeting_t;

static inline int meeting_init( meeting_t * mt, int numthreads )
{
	int status;

	if( NULL == mt ){
		return EFAULT; 
	}
	status = pthread_mutex_init( & mt->m_mutex, NULL);
	if( 0 != status ) return status;
	status = pthread_cond_init( & mt->m_cond, NULL);
	if( 0 != status ) return status;
	mt->numthreads =  numthreads;
	mt->count = 0;
	return 0;
}

static inline int meeting_destroy( meeting_t * mt)
{
	int status;

	if( NULL == mt ){
		return EFAULT; 
	}
	status = pthread_cond_destroy( & mt->m_cond);
	if( 0 != status ) return status;
	status = pthread_mutex_destroy(& mt->m_mutex);
	if( 0 != status ) return status;
	mt->count = 0;
	mt->numthreads = 0;
	return 0;
}

static inline int meeting_wait( meeting_t * mt )
{
	int status;

	if( NULL == mt ){
		return EFAULT; 
	}

	status = pthread_mutex_lock( & mt->m_mutex );
	if( 0 != status ) return status;
	if( mt->count >= mt->numthreads || mt->count <0 ){
		fprintf(stderr, 
		   "Fatal: consistency check failed in line %d of file %s (function %s)\n",\
		   __LINE__, __FILE__, __func__);
		exit(1);
	}
	++mt->count;

	if( mt->count == mt->numthreads ){ /* time to release all */
		status = pthread_cond_broadcast(& mt-> m_cond);
		if( 0 != status ) return status;
	} else {
		status = pthread_cond_wait(& mt-> m_cond, & mt->m_mutex);
		if( 0 != status ) return status;
	}

	--mt->count;

	status = pthread_mutex_unlock( & mt->m_mutex );
	if( 0 != status ) return status;
	
	return 0;

}


#endif /* __MEETING_H */
