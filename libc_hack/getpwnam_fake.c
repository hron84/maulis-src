/*
 * ez csak egy proba egy libc rutin kicserelesere
 * 2005. 09. 14, Maulis Adam
 * GNU GPL v2 or newer
 * 
 */
/*
 * conffile format: one line:
 * <from><space><to>
 * <from>= amit cserelni szeretnel
 * <to>= amire.
 * A rutin szimplan a masik user rekordjat adja vissza.
 */

#include <sys/types.h>
#include <pwd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#ifndef NULL
# define NULL (void *)0
#endif

static char configfile[]=".getpwnam_fake";

static char from[40], to[40];
static int fromlen;
typedef struct passwd * (* functptr)(const char *);
static functptr libc_getpwnam;
static void * libchandle;
void  fakeinit() __attribute__ ((constructor));
void  fakedestroy() __attribute__ ((destructor));

void fakeinit()
{
	FILE *f;
	fromlen = 0;
	
	f=fopen(configfile,"ro");
	if( NULL != f){
		fscanf(f, "%39s %39s", from, to);
		fromlen = strlen(from);
		fclose(f);
	}else{
		fprintf(stderr, "Error opening configfile: %s\n", configfile);
	}
	

	libchandle = dlopen("libc.so.6", RTLD_LAZY);
	libc_getpwnam = (functptr) dlsym(libchandle, "getpwnam");

}

void fakedestroy() 
{
	dlclose(libchandle);
}


struct passwd *getpwnam(const char *name)
{
	struct passwd * res; 

	if( 0 == strncmp(name, from, fromlen+1) && 0 != fromlen ){
		return libc_getpwnam(to);
	}else{
		return libc_getpwnam(name);
	}
}
