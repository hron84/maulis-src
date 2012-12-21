/*
 * paramalloc.h
 *    helps debugging malloc/free bugs.
 *
 *    use
 *       #include "paramalloc.h"
 *       aftar all other includes.
 *    restrictions:
 *       memory allocated with strdup cannot be freed with this modified free.
 */


#ifndef PARAMALLOC_H_LOADED
#define PARAMALLOC_H_LOADED

#include <stdlib.h>

#define malloc(i)    paramalloc(i,    __FILE__, __LINE__)
#define calloc(i,j)  paracalloc(i,j,  __FILE__, __LINE__)
#define realloc(i,j) pararealloc(i,j, __FILE__, __LINE__)
#define free(i)      parafree(i,      __FILE__, __LINE__)

void * paramalloc(size_t size, const char * module, int line);
void * paracalloc(size_t nmemb, size_t size, const char * module, int line);
void * pararealloc(void *ptr, size_t size, const char * module, int line);
void parafree(void *ptr, const char * module, int line);

#endif /* PARAMALLOC_H_LOADED */
