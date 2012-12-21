/*
 * filename: readline.h
 *
 *  an curses-based readline functionality
 *
 */

#ifndef READLINE__H
#define READLINE__H

#include <curses.h>

/*
**   readline structures and functions
**   it reads a line from the terminal using curses
**      cureses must be initialized
**      it scrolls if the line does not fit in window
**      backpsace and left/right key are interpreted
**      can use prompt (may null);
**      
*/
#define READLINE_SUCCESS 1
#define READLINE_BADMAGIC 2
#define READLINE_PENDING 4  /* non-blocking feature: the input is not ready yet*/
#define READLINE_NULLP 6 /* one parameter is NULL but it fordibben */
#define READLINE_NULLRP 8 /* struct READLINE pointer is null but this is fordibben */
#define READLINE_NOMEM 10 /* cannot allocate internal storage */
#define READLINE_NOSPACE 12 /* there is no space to display window */


struct READLINE {
    int magic;
    char * buff;
    int bufflen;
    int size; /* string current size (in the buff) */
    int beg; /* the window's first charatcer (position of teh buff ) */
    int pos; /* the virtual cursor position from the begining of the buff */
    int width; /* window width */
    WINDOW * w; /* the input (sub) window */
}; /* end struct READLINE */

int init_readline(struct READLINE *rl, WINDOW *w, const char * prompt, int bufflen);
int readline_nonblock(struct READLINE *rl, char * outbuff);
int readline(struct READLINE *rl, char * outbuff);
int destroy_readline(struct READLINE *rl);

#endif /* READLINE__H */
