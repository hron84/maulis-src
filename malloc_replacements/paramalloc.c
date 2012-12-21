/* file: paramalloc.c
 * author: Adam Maulis <maulis@ludens.elte.hu>
 * cre.date: 21-sep-2006
 * Copyright: GNU GPL v2 or newer 
 * description:
 *     malloc/calloc/free/realloc replacement for debug.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef VMS
/* a vms-es C fordito balhezik a void* aritmetikaert */
#pragma message disable BADPTRARITH
#endif


/*
 * major define section (compile-time options etc)
 */


#define VERSIONSTR "paramalloc 0.1"
#ifndef NULL
#define NULL ((void*)0)
#endif


#define magic_malloc1 (0x9A4D7CB9)
#define magic_malloc2 (0xD1DB227B)
#define magic_malloc3 (0xDEADBEEF)


/*
 * include section
 */


/*
 * types
 */

struct 	_paramstruct { 
		int magic; 
		size_t size; 
	};
typedef struct _paramstruct paramstruct;

/*
 * functions
 */


void * paramalloc(size_t size, const char * module, int line)
{
	void * ptr;
	if( 0==size ){
		fprintf(stderr,"WARN: malloc(0) called at %s:%d\n",module,line);
	}
	ptr = (void *) malloc(size+ sizeof(paramstruct) + sizeof(int));
	if( NULL == ptr ){ 
		fprintf(stderr,"ERR: NULL=malloc(%d) at %s:%d\n",
				size,module,line);
		return ptr;
	}
	((paramstruct*)ptr)->magic = magic_malloc1;  /* magic key in header */
	((paramstruct*)ptr)->size  = size; /* store length */
	*(int*)(ptr + sizeof(paramstruct) + size ) = magic_malloc2;
	return ptr + sizeof(paramstruct);
	
}/* end of paramalloc */

void * paracalloc(size_t nmemb, size_t size, const char * module, int line)
{
	size_t b;
	void * t;

	b = nmemb * size;
	if( nmemb != b/size || size != b/nmemb ){
		fprintf(stderr,"CRIT: calloc integer owerflow at  %s:%d\n",
				module,line);
		exit(-1);
	}
	t = paramalloc(b,module,line);
	if( NULL == t) 
		return t;
	return memset(t, 0, b);
}



void parafree(void* ptr, const char * module, int line)
{
	paramstruct * origptr;
	if(NULL == ptr){
		fprintf(stderr,"CRIT: free(NULL) called at %s:%d\n",module,line);
		exit( -1 );
	}
        origptr = (paramstruct *) (ptr - sizeof(paramstruct));
	
	if( 0 == origptr->size && magic_malloc3 == origptr->magic ){
		fprintf(stderr,"CRIT: double free() at %s:%d\n",
				module,line);
		exit( -1 );
	}
	if( magic_malloc1 != origptr->magic ){
		fprintf(stderr,"CRIT: free(invalid) called at %s:%d\n",module,line);
		exit( -1 );
	}
	if( magic_malloc2 != *(int*)(ptr + origptr->size) ){
		fprintf(stderr,"CRIT: free(endcorrupted) called at %s:%d\n",
				module,line);
		exit( -1 );
	}
	origptr->magic = magic_malloc3;
	origptr->size = 0;
	free(origptr);
}/* end of parafree */

void * pararealloc(void *ptr, size_t size, const char * module, int line)
{
	paramstruct * origptr;
	void * newptr;
	
	/* at first, if size 0, eq free() */
	if( 0 == size){
		parafree(ptr, module, line);
		return NULL;
	}
	
	/* at second, if ptr are null, eq malloc() */
	if( NULL == ptr){
		return paramalloc(size, module, line);
	}

	origptr = (paramstruct *) (ptr - sizeof(paramstruct));

	if(  magic_malloc1 != origptr->magic ){
		fprintf(stderr, "CRIT: realloc(invalid,%d) called at %s:%d\n",
				size,module,line);
		exit(-1);
	}
	if( magic_malloc2 != *(int*)(ptr + origptr->size) ){
		fprintf(stderr,"CRIT: realloc(endcorrupted) called at %s:%d\n",
				module,line);
		exit( -1 );
	}

	*(int*)(ptr + origptr->size) = 0; /* clear trailing */

	origptr = (paramstruct *) realloc(origptr, size + sizeof(paramstruct) + sizeof(int));
	origptr->size = size;
	newptr = origptr +1;
	*(int*)(newptr + origptr->size) = magic_malloc2;
	return newptr;
	
}


