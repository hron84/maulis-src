/*
 * filename: readline.c
 *
 * an curses-based readline functionality
 *
 */

 
#include <curses.h>
#include "readline.h"
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>


#define READLINE_NUMBER_MAGIC (0x4F58A20D)

#define SANITYCHK(x) { if(x){ \
                   fprintf(stderr, "Sanity chk error in %s %d\n",\
		   __FILE__, __LINE__);  \
		   exit(8); } }

static void readline_redisplay(struct READLINE *rl)
{
    werase(rl->w);
    mvwaddnstr(rl->w, 0,0, 
         rl->buff + rl->beg,
	 rl->size - rl->beg > rl->width ? rl->width : rl->size - rl->beg );
    wmove(rl->w, 0, rl->pos - rl->beg);

}/* end of readline_redisplay */


int init_readline(struct READLINE *rl, WINDOW *w, const char * prompt, int bufflen)
{
    int tmp,wx,wy;
    int promptlen;
    
    if( NULL == rl ) return READLINE_NULLRP;
    if( READLINE_NUMBER_MAGIC == rl->magic ) return READLINE_BADMAGIC;
    if( NULL == w ) return READLINE_NULLP;
    
    getmaxyx(w, tmp, rl->width);
    if( NULL != prompt){
        promptlen = strlen(prompt);
    }else{
        promptlen = 0;
    }
    
    rl->width -= promptlen;
    if( rl->width < 1 ){
        return READLINE_NOSPACE;
    }
    getbegyx(w, wy, wx);
    
    rl->buff=(char *) malloc(bufflen);
    if( NULL==rl->buff ) return READLINE_NOMEM;
    rl->bufflen = bufflen-1; /* terminating zero */
    
    if( 0!=promptlen ){
        mvwaddstr(w, 0,0, (char *)prompt);
    }
    
    rl->w = newwin(1, rl->width, 0+wy, promptlen+wx);
    
    if( NULL == rl->w ){
        free(rl->buff);
        return READLINE_NOMEM;
    }

    werase(rl->w);
    wnoutrefresh(w);
    wnoutrefresh(rl->w);
    doupdate();
    
    rl->pos = 0;
    rl->size = 0;
    rl->beg = 0;
    
    keypad(rl->w, 1); /* readline depends on the interpreted function keys */
    cbreak(); /* we want read by char */
    noecho();


    rl->magic = READLINE_NUMBER_MAGIC;
    return READLINE_SUCCESS;
}/* end of init_readline */

int destroy_readline(struct READLINE *rl)
{
    if( NULL == rl ) return READLINE_NULLRP;
    if( READLINE_NUMBER_MAGIC != rl->magic ) return READLINE_BADMAGIC;
    
    free(rl->buff);
    
    rl->magic=0;
    rl->buff = NULL;
    rl->bufflen = 0;
    rl->size = 0;
    rl->beg = 0;
    rl->pos = 0;
    
    delwin(rl->w);
    /* nocbreak(); */
    /*  echo();    */
    rl->w = NULL;

    return READLINE_SUCCESS;
} /* end of detroy readline */

int readline_nonblock(struct READLINE *rl, char * outbuff)
{
    int newch;
    int i, tmp, curpos;

    if( NULL == rl ) return READLINE_NULLRP;
    if( READLINE_NUMBER_MAGIC != rl->magic ) return READLINE_BADMAGIC;
    if( NULL == outbuff) return READLINE_NULLP;
    
    getyx(rl->w, tmp, curpos);
    
    wmove(rl->w, tmp, curpos);
    doupdate(); /* we want see the cursosr in this window */
    
    newch = wgetch(rl->w);
    switch( newch ){
       case ERR : /* there is no character yet, do nothing */ 
       		break;
       case KEY_LEFT :
                SANITYCHK( rl->pos < 0 );
       		if(0 == rl->pos){
		    beep();
		    break;
		}
		if(curpos > 0){
		    --rl->pos;
		    wmove(rl->w, 0, curpos-1);
		} else { /* 0 == curpos */
		    --rl->pos;
		    SANITYCHK( rl->beg <=0 );
		    --rl->beg;
		    readline_redisplay(rl);
		}
		wrefresh(rl->w);
		break;
		
       case KEY_RIGHT :
       		SANITYCHK( rl->pos > rl->size);
		if( rl->size == rl->pos ){
		   beep();
		   break;
		}
		if( curpos < rl->width-1 ){
		   wmove(rl->w, 0, curpos+1);
		   ++rl->pos;
		}else{
		   ++rl->pos;
		   ++rl->beg;
		   readline_redisplay(rl);
		}
		wrefresh(rl->w);
		break;
       case KEY_HOME :
       		rl->pos=0;
		rl->beg=0;
		wmove(rl->w, 0, 0 );
		readline_redisplay(rl);
		wrefresh(rl->w);
		break;
       case 8:  /* ctrl-h*/
       case 127: /* ctrl-? */
       case KEY_BACKSPACE :
       case KEY_DC :
       		SANITYCHK( rl->pos < 0 );
		if( rl->pos == 0 ){
		    beep();
		    break;
		}
		for(i = rl->pos; i < rl->size; i ++ )
		   rl->buff[i-1] = rl->buff[i];
		--rl->pos;
		--rl->size;
		if( 0 == curpos){
		   SANITYCHK( rl->beg <= 0 );
		   --rl->beg;
		}else{
		    wmove(rl->w, 0, curpos-1);
		    readline_redisplay(rl);
		}
		wrefresh(rl->w);
		break;
       case KEY_REFRESH :
       		redrawwin(curscr);
		break;
       case '\n' : /* return the result */
		memcpy(outbuff, rl->buff, rl->size );
		outbuff[rl->size] = 0;

       		rl->size = 0;
		rl->pos = 0;
		rl->beg = 0;
		wmove(rl->w, 0, 0);
		werase(rl->w);
       		wrefresh(rl->w);
		return READLINE_SUCCESS;
		break;
       
       default: /* inserting character at current position */
                if( newch > 255 ) break; /* non-handled non printable */
    		SANITYCHK( rl->pos > rl->size );
		if( rl->size == rl->bufflen ){
		   beep();
		   break;
		}
		for(i = rl->size; i > rl->pos; i-- )
		    rl->buff[i] = rl->buff[i-1];
		rl->buff[rl->pos] = (char) newch;
		++rl->pos;
		++rl->size;
		if( curpos == rl->width -1 ){
		   ++rl->beg;
		}else{
		   wmove(rl->w, 0, curpos+1);
		}
		readline_redisplay(rl);
		wrefresh(rl->w);
		break;
    } /* end switch newch */
    return READLINE_PENDING;
    
}/* end of readline_nonblock */

int readline(struct READLINE *rl, char * outbuff)
{
    int status;
    if( NULL == rl ) return READLINE_NULLRP;
    if( READLINE_NUMBER_MAGIC != rl->magic ) return READLINE_BADMAGIC;
    if( NULL == outbuff) return READLINE_NULLP;

    /* chek needed! for non-blocking io: avoid buisy loop */    
    while(  (status = readline_nonblock(rl, outbuff)) == READLINE_PENDING )
         ;
    return status;
}/* end of readline */


