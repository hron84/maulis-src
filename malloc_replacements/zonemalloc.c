/*
** zonemalloc.c
**
** imlepemntation of zone_* routines
**
*/


#include "zonemalloc.h"
#include <stdio.h>
#include <string.h>


#define ZPMAGIC 0xFE4A670B

#ifdef DEBUG
# define    acntup()  (zp->acnt++)
# include "paramalloc.h"
# include <stdlib.h>
#else
# define    acntup()
#endif


struct MEMHEAD {
    struct MEMHEAD * prev;
    struct MEMHEAD * next;
};

struct ZONE {
    struct MEMHEAD * prev;  /* child of MEMHEAD */
    struct MEMHEAD * next;  /* must be align */
    unsigned int magic;
    unsigned int count; /* unnessessary, debugging, stats only */
#  ifdef DEBUG
    unsigned int acnt;
#  endif
};


/*
**  inset  - for debugging only
*/

#ifdef DEBUG

# define CHKSET(zp, a) {if( !inset(zp, a, __LINE__ ) ){ \
    fprintf(stderr, "Zone::CHKSET error, pointer not in the set (line %d)\n", \
    __LINE__); exit(-1); }}

static int inset(struct ZONE * zp, void * a, int line)
{
    struct MEMHEAD *p;
    unsigned int i;

    i = 0;
    for(p = zp->next; p != (struct MEMHEAD* ) zp; p = p->next){
	if( i >= zp->count ){
	    fprintf(stderr, "Zone::inset inconsfail at line %d\n", line);
	    return 0; /* not in the set */
	}
	if( a == (p+1) ) 
	    return 1;

	i++;
    }
    return 0;  /* not in the set */
}
#else
# define CHKSET(zp, a) 
#endif

/*
** create_zone
*/

struct ZONE * create_zone()
{
    struct ZONE * zp;
    zp = (struct ZONE *) malloc(sizeof(struct ZONE));
    if( NULL == zp ) return zp;
    zp->magic = ZPMAGIC;
    zp->prev = (struct MEMHEAD *) zp;
    zp->next = (struct MEMHEAD *) zp;
    zp->count = 0;  /* unnessessary, debugging, stats only */
#  ifdef DEBUG
    zp->acnt = 0;   /* access count */
#  endif
    return zp;
}

/*
**  destroy_zone
*/

void destroy_zone(struct ZONE * zp)
{

    if( zp == NULL) {
	fprintf(stderr, "Zone::destroy_zone called with null pointer\n");
	return;
    }
    if( zp->magic != ZPMAGIC ){
	fprintf(stderr, "Zone::destroy_zone called with invalid zp\n");
	return;
    }

#  ifdef DEBUG
    if( zp -> count > 0)
	fprintf(stderr,"Zone::destroy_zone some (%d) used area found\n", zp->count);
#  endif

#  if 1    
    /* dual implementation, one */
    while( zp->next != (struct MEMHEAD*)zp ){
	zone_free(zp, zp->next+1);  /* +1 == sizeof(struct MEMHEAD) */
    }
#  else
    /* dual implementation, two */
    {
        unsigned int i;
	struct MEMHEAD *p;

        for(i = 0; i< zp->cnt; i++){
	    p = zp->next->next;
	    free(zp->next);
	    zp->next =p;
        }
	zp->cnt = 0;
    }
#  endif

    zp->magic = 0;
    free(zp);
#  ifdef DEBUG
    fprintf(stderr,"Zone::destroy_zone called, access count:%d\n", zp->acnt);
#  endif
}

/*
** zone_malloc
*/

void * zone_malloc(struct ZONE *zp, size_t len)
{
    struct MEMHEAD *p;
    if( zp == NULL) {
	fprintf(stderr, "Zone::zone_malloc called with null pointer\n");
	return NULL;
    }
    if( zp->magic != ZPMAGIC ){
	fprintf(stderr, "Zone::zone_malloc called with invalid zp\n");
	return NULL;
    }
    acntup();

    p = (struct MEMHEAD *) malloc(len + sizeof(struct MEMHEAD));
    if( NULL == p )
	return NULL;

    /* insert into double linked list, after zp */
    p->prev = (struct MEMHEAD *) zp;
    zp->next->prev = p;
    p->next = zp->next;
    zp->next = p;

    zp->count ++;

    CHKSET(zp, p+1);
    return p + 1;
}

/*
** zone_calloc
*/

void * zone_calloc(struct ZONE *zp, size_t nmemb, size_t size)
{
    struct MEMHEAD *p;

    if( zp == NULL) {
	fprintf(stderr, "Zone::zone_calloc called with null pointer\n");
	return NULL;
    }
    if( zp->magic != ZPMAGIC ){
	fprintf(stderr, "Zone::zone_calloc called with invalid zp\n");
	return NULL;
    }

    acntup();

    p = (struct MEMHEAD *) malloc(nmemb*size + sizeof(struct MEMHEAD));
    if( NULL == p )
	return NULL;
    memset (p, 0, nmemb*size + sizeof(struct MEMHEAD));

    /* insert into double linked list, after zp */
    p->prev = (struct MEMHEAD *) zp;
    zp->next->prev = p;
    p->next = zp->next;
    zp->next = p;

    zp->count ++;

    CHKSET(zp, p+1);
    return p + 1;
}

/*
** zone_realloc
*/

void * zone_realloc(struct ZONE * zp, void *ptr, size_t size)
{
    struct MEMHEAD * p;

    if( zp == NULL) {
	fprintf(stderr, "Zone::zone_realloc called with null pointer\n");
	return NULL;
    }
    if( zp->magic != ZPMAGIC ){
	fprintf(stderr, "Zone::zone_realloc called with invalid zp\n");
	return NULL;
    }

    acntup();

    if( ptr == NULL) {
	return zone_malloc(zp, size);
    }
    if( size == 0 ){
	zone_free(zp, ptr);
	return NULL;
    }

    CHKSET(zp, ptr);


    p = (struct MEMHEAD *) realloc((char *)ptr - sizeof(struct MEMHEAD),
	    size + sizeof(struct MEMHEAD));
    if( NULL == p )
	return NULL;

    (p->next)->prev = p; /* realloc preserves data*/
    (p->prev)->next = p;

    CHKSET(zp, p+1);
    return p + 1;

}

/*
** zone_free
*/

void zone_free(struct ZONE * zp, void * ptr)
{
    struct MEMHEAD * oldp;

    if( zp == NULL) {
	fprintf(stderr, "Zone::zone_free called with null pointer\n");
	return;
    }
    if( zp->magic != ZPMAGIC ){
	fprintf(stderr, "Zone::zone_free called with invalid zp\n");
	return;
    }

    CHKSET(zp, ptr);
    acntup();

    if( ptr == NULL) {
	fprintf(stderr, "Zone::zone_free called with null pointer\n");
	return;
    }

    oldp = (struct MEMHEAD *) ((char *)ptr - sizeof(struct MEMHEAD));

    oldp->next->prev = oldp->prev;
    oldp->prev->next = oldp->next;
    free(oldp);
    zp->count--;
}
