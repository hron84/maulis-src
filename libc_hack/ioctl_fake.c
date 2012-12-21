/*
 * ez csak egy proba egy libc rutin kicserelesere
 * 2006. 07. 05, Maulis Adam
 * GNU GPL v2 or newer
 * 
 *  replaces ioctl() for dummy one, prints parameters to stdout.
 */

#include <sys/types.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#ifndef NULL
# define NULL (void *)0
#endif


typedef int (* functptr)(int d, int request, ...);
int ioctl(int d, int request, ... );
static functptr libc_ioctl;
static void * libchandle;
void  fakeinit() __attribute__ ((constructor));
void  fakedestroy() __attribute__ ((destructor));

void fakeinit()
{
	

	libchandle = dlopen("libc.so.6", RTLD_LAZY);
	libc_ioctl = (functptr) dlsym(libchandle, "ioctl");

}

void fakedestroy() 
{
	dlclose(libchandle);
}


int ioctl(int d, int request, ...)
{
	
	fprintf(stderr,"ioctl(%d, 0x%08X ...)\n");
	return 0;
}
