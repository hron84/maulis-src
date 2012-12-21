/*
** zonemalloc.h
**
**  allocation from an zone and frees whole zone at one step
**
*/


#ifndef ZONEMALLOC_H__LOADED
#define ZONEMALLOC_H__LOADED

#include <stdlib.h>

struct ZONE;

/*
** create_zone
*/

struct ZONE * create_zone();

/*
** destroy_zone
*/

void destroy_zone( struct ZONE *zp);

/*
** zone_malloc
*/

void * zone_malloc(struct ZONE *zp, size_t len);

/*
** zone_calloc
*/

void * zone_calloc(struct ZONE *zp, size_t nmemb, size_t size);

/*
** zone_realloc
*/

void * zone_realloc(struct ZONE * zp, void *ptr, size_t size);

/*
** zone_free
*/

void zone_free(struct ZONE * zp, void * ptr);

#endif
