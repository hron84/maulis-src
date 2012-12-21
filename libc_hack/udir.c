/* egyszeru teszt program a getpwnam_fake.c -hez
 * entry point
 * 2005.09.14, Maulis Adam
 * GNU GPL v2
 */

#include <sys/types.h>
       #include <pwd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char * argv[] )
{
	struct passwd * res;
	if( argc !=2 ){
		printf(" usage: %s username\n", argv[0]);
		return 1;
	}
	
	res = getpwnam(argv[1]);
	if( NULL == res) {
		puts("Err: nulll pointer returned");
		return 2;
	}
	printf("homedir of user %s is %s\n", argv[1], res->pw_dir ); 
	return 0;
}

