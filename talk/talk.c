/* 
 * simple talk protocol
 *
 *   parameter: hostname to connect to
 *   if no parameter then work as server
 */


#define PORT 17543   /* default TCP port number */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include "readline.h"


void letclient(const char *hostname);
void letserver(void);
void docommunicate(int sock );
void help();

int main(int argc, char * argv[] )
{

   /* parameter processing */
   switch( argc ){
       case 1: /* no param */
       	      letserver();
              break;
       case 2: /* one param */
              letclient(argv[1]);
              break;
       default: help();
   }
   return 0;
}



/***************************************************************************/

void help(void)
{
    puts(" Usage as server:");
    puts("  talk");
    puts(" Usage as client:");
    puts("  talk remotename");
}

/***************************************************************************/

void letclient(const char *hostname)
{
   
    int sock;
    struct hostent hostentstruct;    /* Storage for hostent data.  */
    struct hostent *hostentptr;      /* Pointer to hostent data.   */
    struct sockaddr_in sock_name;
    int port;
    char *b,*p;

   if( (sock=socket(AF_INET,SOCK_STREAM,0))== -1) {
       perror("Error in letclient(): socket() returns: ");
       return ; 
   }

   b=strdup(hostname);
   if(NULL == b){
       perror(" Error in letclient(): strdup() returns: ");
       return;
       
   }
   p=strchr(b, (int) ':');
   if( NULL != p ){
   	port = atoi(p+1);
	if(0==port) port = PORT;
	*p=0;
   }else 
   	port = PORT;
	
   if( (hostentptr = gethostbyname(b) ) == NULL){
       perror("Error in letclient(): gethostbyname() returns: ");
       return;
   }
   hostentstruct= *hostentptr;  /* save data to safe storage */
   free(b);

   
   sock_name.sin_family= hostentstruct.h_addrtype;
   sock_name.sin_port  = htons(port);
   sock_name.sin_addr  = *((struct in_addr *)hostentstruct.h_addr);
   printf("Connecting: %s:%d\n",
              inet_ntoa(sock_name.sin_addr),
	      ntohs(sock_name.sin_port) );

   if( connect(sock, (struct sockaddr *) &sock_name, sizeof (sock_name)) ){
      perror("Error in letclient(): connect() returns: ");
      close( sock);
      return;
   }
   docommunicate(sock);
   close(sock);
}



void letserver()
{
   int servsock,sock, sockopt;
   struct sockaddr_in srvportdef;
   struct sockaddr_in kliensdef;
   int kliensdeflen= sizeof(struct sockaddr_in);

   printf("Server mode. Waiting for the party.\n",PORT);

   if( (servsock=socket(AF_INET,SOCK_STREAM,0))== -1) {
       perror("Error in letserver(): socket() returns: ");
       return ;
   }
   sockopt=1;  /* enable this option */
   if( setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) ) {
       perror("Error in letserver(): setsockopt() returns: ");
       return;
   }

   /* bind port */
   srvportdef.sin_family=AF_INET;
   srvportdef.sin_port=htons(PORT) ;
   srvportdef.sin_addr.s_addr=htons(INADDR_ANY);

   if( -1 == bind(servsock, (struct sockaddr *) &srvportdef,sizeof(srvportdef)) ){
       perror("Error in letserver():  bind() returns: ");
       return;
   }
   if( -1 == listen(servsock,1) ){
       perror("Error in letserver(): listen() returns: ");
       return;
   }
   
   sock=accept(servsock,(struct sockaddr *) &kliensdef, &kliensdeflen );
   if( -1 == sock ){
       perror("Error in letserver(): accept() returns: ");
       return;
   }
   printf(" Client connecting from: %s:%d \n", 
              inet_ntoa(kliensdef.sin_addr),
	      ntohs(kliensdef.sin_port) );

   close(servsock);
   docommunicate(sock);
   close(sock);

}



/***************************************************************************/

int open_term();
void close_term();
void write_term(char * buff);
int read_term(char * buff);

/*
 *  docommunication
 *     concept:
 *      use select() for read and write simultanously
 *   used subfunctions: 
 *      write_term();
 *      read_term();
 */

#define BUFFMAX 3000

void docommunicate(int sock)
{
    int letexit = 0 ; 
    int termi;
    int rlen;
    int blen;
    int maxfd;
    int status;
    fd_set rset,errset;
    char buff[BUFFMAX+1];

    termi=open_term();


    maxfd=termi;
    maxfd= maxfd>sock ? maxfd:sock ;
    maxfd++;


     while( !letexit ){
         FD_ZERO(&rset);
	 FD_ZERO(&errset);
	 FD_SET(termi,&rset);
	 FD_SET(sock,&rset);
	 FD_SET(sock,&errset);

	 if( -1 == select(maxfd, &rset, NULL, &errset, NULL ) ){
	     perror("Error in docommunicate() select() returns: ");
	     return ;
	 }
	 if( FD_ISSET(termi, &rset) ){
	     status = read_term(buff);
	     if( READLINE_SUCCESS == status ){ 
	     
	        if(0 == strncasecmp(buff,"/exit",5) ){
	           letexit = 1;
	        } else {
	           blen = strlen(buff);
	           buff[blen] = '\n';
		   buff[blen+1]=0;
	           write(sock, buff,blen+1 );
                }
	     }/* end if status */
	 }
	 if( FD_ISSET(sock, &rset) ){
	     rlen = read(sock, buff, BUFFMAX);
	     if( 0 == rlen ){
	         letexit = 1;
	     }else{
	         if( '\n' == buff[rlen-1] ) buff[rlen-1]=0;
	         buff[rlen]=0;
	         write_term(buff);
	     }
	 }
	 if( FD_ISSET(sock, &errset) ){
	     letexit = 1;
	 }
     }
	


    close_term();

}

/*
 *  curses dependent function 
 */


static int curses_initialized = 0;
static WINDOW * mainwin;
static WINDOW * statwin;
static WINDOW * downwin;
static struct READLINE rl;
static int lineicnt, lineocnt;

int open_term()
{
    int status ;
    if(!curses_initialized){
        curses_initialized=1;
	initscr();
	mainwin=newwin(LINES-2,COLS, 0,0);
	scrollok( mainwin, 1);
	statwin=newwin(1,COLS, LINES-2, 0);
	waddstr(statwin,"Welcome to talk");
	downwin=newwin(1,COLS,LINES-1,0);
	status=init_readline(&rl, downwin, ">", BUFFMAX);
	lineicnt = lineocnt = 0;
        if( READLINE_SUCCESS != status ){
	      fprintf(stderr, "Error: init_readline() == %d\n", status);
	      fflush(stderr);
	      curses_initialized = 0;
	      endwin();
	      return;
        }

    }
    return 0;
}

void close_term(int fd)
{
   if(curses_initialized){
       curses_initialized=0;
       destroy_readline(&rl);
       delwin(statwin);
       delwin(mainwin);
       delwin(downwin);
       endwin();
   }
}

void write_term(char * buff)
{
   char tmpstr[200];
   
   lineocnt ++;
   waddstr(mainwin,"\n-- "); waddstr(mainwin, buff);
   sprintf(tmpstr, "Your lines: %d Party lines: %d  type /exit to quit", lineicnt, lineocnt);
   werase(statwin);
   waddstr(statwin,tmpstr);
   wnoutrefresh(statwin);
   wnoutrefresh(mainwin);
   doupdate();
}

int read_term(char * buff)
{
   char tmpstr[200];
   int status;

   status = readline_nonblock(&rl, buff);
   if( READLINE_SUCCESS == status ){
      lineicnt ++;
      waddstr(mainwin,"\n*  "); waddstr(mainwin, buff);
      sprintf(tmpstr, "Your lines: %d Party lines: %d  type /exit to quit", lineicnt, lineocnt);
      werase(statwin);
      waddstr(statwin,tmpstr);
      wnoutrefresh(statwin);
      wnoutrefresh(mainwin);
      doupdate();

   }
   return status;
}

