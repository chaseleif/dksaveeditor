#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include "shared.h"

#define INPUTSTRSIZE 16

extern char *field, *titlebar, *msgstr;

void getinput() {
  char empty = '\0';
  char *start, *walk = strchr(field,'/');
  const int num_values = (walk)?2:1;
  char **descriptions = malloc(sizeof(char*)*num_values);
  char **values = malloc(sizeof(char*)*num_values);
  char **newvalues = malloc(sizeof(char*)*num_values);
  if (!(walk=strchr(field,':'))) {
    descriptions[0] = &empty;
    walk = field;
  }
  else {
    *walk = '\0';
    descriptions[0] = field;
    ++walk;
  }
  values[0] = start = walk;
  if (num_values==2) {
    walk = strchr(walk,'/');
    *walk = '\0';
    start = ++walk;
    walk = strchr(walk, ':');
    if (!walk)
      descriptions[1] = &empty;
    else {
      *walk = '\0';
      descriptions[1] = start;
      start = walk+1;
    }
    values[1] = start;
  }
  newvalues[0] = malloc(sizeof(char)*INPUTSTRSIZE);
  sprintf(newvalues[0],"%s",values[0]);
  if (num_values==2) {
    newvalues[1] = malloc(sizeof(char)*INPUTSTRSIZE);
    sprintf(newvalues[1],"%s",values[1]);
  }
  clear();
  int ch;
  show_cursor();
  int leftx, writex, writey, writei = 0;
  const int menuwidth=30;
  do {
    int maxy, maxx;
    getmaxyx(stdscr,maxy,maxx);
    const int leftmargin = MENUMARGIN;
    int y = MENUFIRSTLINE;
    printcolor(TITLELINENUM,TITLEMARGIN,DKGRNBLK,"%s",msgstr);
    printwithattr(y, leftmargin, A_UNDERLINE, "%s", "Original");
    for (int i=0;i<num_values;++i) {
      mvprintw(++y, leftmargin+10, "%s = %s", descriptions[i], values[i]);
    }
    printwithattr(++y, leftmargin+5, A_UNDERLINE, "%s", "New");
    for (int i=0;i<num_values;++i) {
      mvprintw(++y, leftmargin+10, "%s = %s", descriptions[i], newvalues[i]);
      if (i==writei) {
        writey = y;
        leftx = leftmargin+10+strlen(descriptions[i])+3;
        writex = strlen(newvalues[i]);
      }
    }
    move(writey,leftx+writex);
    refreshwithborder(DKBLKGRN);
    ch = getch();
    if (isdigit(ch)) {
      if (writex+1==INPUTSTRSIZE) {
        do { flash(); flushinp(); ch = getch(); }while(isdigit(ch));
      }
      else {
        newvalues[writei][writex++] = ch;
        newvalues[writei][writex] = '\0';
        continue;
      }
    }
    switch(ch) {
      case KEY_RESIZE: clear(); break;
      case KEY_BACKSPACE: case 127: case '\b':
        if (!writex) {
          do { flash(); flushinp(); ch=getch(); }while(ch==KEY_BACKSPACE);
          ungetch(ch);
        }
        else {
          newvalues[writei][--writex] = '\0';
          move(writey,leftx+writex);
          clrtoeol();
          refreshwithborder(DKBLKGRN);
        }
        break;
      case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
      case 4: case 'q': case 'Q': case 'c': case 'C':
        ch = -1;
        break;
      case KEY_ENTER: case '\n':
        ch = (*values[0]=='\0')?-1:(num_values==1)?0:(*values[1]=='\0')?-1:0;
        break;
      case KEY_UP:
        if (writei > 0) --writei;
        else if (num_values==2) writei=1;
        break;
      case KEY_DOWN:
        if (writei == 1) writei=0;
        else if (num_values==2) writei=1;
        break;
      default: break;
    }
  }while(ch>0);
  hide_cursor();
  if (ch < 0) {
    if (num_values==1) strcpy(field, values[0]);
    else sprintf(field, "%s.%s", values[0], values[1]);
  }
  else {
    if (num_values==1) strcpy(field, newvalues[0]);
    else sprintf(field, "%s.%s", newvalues[0], newvalues[1]);
  }
  for (int i=0;i<num_values;++i) { free(newvalues[i]); }
  free(descriptions);
  free(values);
  free(newvalues);
}
