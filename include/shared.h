#ifndef SHARED_H
#define SHARED_H

#include "structs.h"

#define hide_cursor() curs_set(0)
#define show_cursor() curs_set(2)

#define setcolor(C) attron(COLOR_PAIR(C))
#define unsetcolor(C) attroff(COLOR_PAIR(C))
// draw the border with color pair C and refresh
#define refreshwithborder(C)  \
  do {                        \
    setcolor(C);              \
    border(0,0,0,0,0,0,0,0);  \
    unsetcolor(C);            \
    refresh();                \
  }while(0)
// print at X,Y using color C with format F and args
#define printcolor(X,Y,C,F,...)   \
  do {                            \
    setcolor(C);                  \
    mvprintw(X,Y,F,__VA_ARGS__);  \
    unsetcolor(C);                \
  }while(0)
// print with attribute A at X,Y with format F and args
#define printwithattr(X,Y,A,F,...)  \
  do {                              \
    attron(A);                      \
    mvprintw(X,Y,F,__VA_ARGS__);    \
    attroff(A);                     \
  }while(0)
// As above, using both attribute A and color C
#define printwithattrandcolor(X,Y,A,C,F,...)  \
  do {                                        \
    setcolor(C);                              \
    attron(A);                                \
    mvprintw(X,Y,F,__VA_ARGS__);              \
    attroff(A);                               \
    unsetcolor(C);                            \
  }while(0)

// menu char buffer size
#define MAXSTRLEN 128
// max rows of any menu (plus one for empty str which signifies menu end)
#define MAXMENUITEMS 256
// row number for the title
#define TITLELINENUM 3
// first line of menus
#define MENUFIRSTLINE 5
// margin specifies left column number
// minimum left margin (if other MARGINs don't work)
#define MINLEFTMARGIN 2
// attempt to center text on the screen
#define TITLEMARGIN \
  ((maxx>>1)-(strlen(titlebar)>>1)+strlen(titlebar)<COLS-MINLEFTMARGIN) ? \
  (maxx>>1)-(strlen(titlebar)>>1) : \
  MINLEFTMARGIN
#define MENUMARGIN \
  ((maxx>>1)-(menuwidth>>1)+menuwidth<COLS-MINLEFTMARGIN) ? \
  (maxx>>1)-(menuwidth>>1) : \
  MINLEFTMARGIN

enum { EXIT, MAINMENU, ABOUTMENU, PREPMENU, FILEMENU, EDITMENU };
enum { DKBLKGRN=1, DKBLKCYN, DKBLKRED, DKWHTBLK, DKWHTBLU,
        DKCYNBLU, DKCYNBLK, DKGRNBLK, DKREDBLK, DKREDCYN, DKBLUWHT };

char *copystr(char *in);
void getinput();
void printerror(const int n, ...);

void setup_main();
int main_processinput(const int ch);
void setup_prep();
int prep_processinput(const int ch);
void setup_about();
int about_processinput(const int ch);
void setup_file();
int file_processinput(const int ch);
void setup_save();
int save_processinput(const int ch);
void setup_edit();
int edit_processinput(const int ch);
int isdirectory(char *name);
int isfile(char *name);
void loadsave(char *filename, struct character **players,
              struct savefileheader *saveinfo, struct partyheader *partyinfo);
void savesave(char *filename, struct character *players,
              struct savefileheader *saveinfo, struct partyheader *partyinfo);
void load_lst(char *filename);
void load_darklands_data();
void load_static_darklands_data();

#endif //SHARED_H
