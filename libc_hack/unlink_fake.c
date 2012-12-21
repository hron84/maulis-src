/*
 * ez csak egy proba egy libc rutin kicserelesere
 * 2006. 07. 05, Maulis Adam
 * GNU GPL v2 or newer
 * 
 *  replaces unlink() for dummy one, prints parameters to stdout.
 * to build:
 *    gcc --share -ldl -o unlink_fake.so unlink_fake.c
 * usage:
 * LD_PRELOAD=./unlink_fake.so someprogram
 *    
 */

#include <sys/types.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#ifndef NULL
# define NULL (void *)0
#endif


typedef int (* functptr)(const char *pathname);
int unlink(const char *pathname);
static functptr libc_unlink;
static void * libchandle;
void  fakeinit() __attribute__ ((constructor));
void  fakedestroy() __attribute__ ((destructor));

void fakeinit()
{
	

	libchandle = dlopen("libc.so.6", RTLD_LAZY);
	libc_unlink= (functptr) dlsym(libchandle, "unlink");

}

void fakedestroy() 
{
	dlclose(libchandle);
}


int unlink(const char *pathname)
{
	
	fprintf(stderr,"unlink(%s)\n",pathname);
	return 0;
}
