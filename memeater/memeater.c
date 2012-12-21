/* memeater.c
** author: Adam Maulis
**
** eats many memory
**
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long long atoint64(const char *ca)
{
      long long ig=0;
      long long sign=1;
      /* test for prefixing white space */
      while (*ca == ' ' || *ca == '\t' ) 
          ca++;
      /* Check sign entered or no */
      if ( *ca == '-' )       
          sign = -1;
      /* convert string to int */
      while (*ca != '\0')
          if (*ca >= '0' && *ca <= '9')
              ig = ig * 10LL + *ca++ - '0';
          else
              ca++;
      return (ig*sign);
}


int main (int argc, char * argv[])
{
	long long eat, pages;
	long long perc,i,j, oldperc;
	int pagesize;

	void * p; 

	if(argc!=2){
		fprintf(stderr,"Must use first parameter: KiBytes to eat.\n");
		return 1;
	}
	puts("Architecture info:");
	printf("\tsizeof(size_t) = %lu \n", sizeof(size_t));
	printf("\tsizeof(void *) = %lu \n", sizeof(void *));
	printf("\tpage size      = %d\n", getpagesize());
	

	eat = atoint64(argv[1]) * 1024LL;
	p=valloc(eat); /* page-aligned */

	pagesize=getpagesize();
	pages=eat/(long long)pagesize;

	puts("Allocation info:");
	printf("\tstart pointer  = %p\n", p);
	printf("\tsize(Bytes)    = %llu\n", eat);
	printf("\tsize(MiBytes)  = %llu\n", eat/1048576LL);
	printf("\tsize(Pages)    = %llu\n", pages);

	printf("And now sleeping 10 sec\n");fflush(stdout);sleep(10);

	if( NULL == p ) {
		printf("There was insufficient memory.");
		return 1;
	} 
#if 0
	printf("quad (%lu bytes) fill:", sizeof(long long));
	eat = eat /(long long)sizeof(long long);
	perc=eat/50; /* double percent */
	for(i=0; i < eat; i++){
		((long long *) p)[i] = 0xCAFEBABEDEADBEEFLL;
		if( (i % perc) == 0 ){ putchar('.'); fflush(stdout); }
	}
	putchar('\n');
	eat=eat * (long long)sizeof(long long);
#endif 
	for( j = 0 ; j< 4 ; j++ ){
	   printf("tainting pages:");
	   perc=eat/50; /* double percent */
	   for(oldperc=0, i=0; i < eat; i+=pagesize){
		*(long long *)(p+i) = 0xCAFEBABEDEADBEEFLL;;
		if( (i / perc) != oldperc ){ putchar('.'); fflush(stdout); }
		oldperc=i / perc;
	   }
	   putchar('\n');  fflush(stdout);

	printf("And now sleeping 5 sec\n");fflush(stdout);sleep(5);
	}


	return 0;
}
