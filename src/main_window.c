#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "shared.h"

extern char **menuoptions;
extern int menuwidth, highlight;

static const int lastindex = 2;

void setup_main() {
  strcpy(menuoptions[0],"Save game viewer/editor");
  strcpy(menuoptions[1],"About");
  strcpy(menuoptions[2],"Quit");
  menuoptions[3][0] = '\0';
  menuwidth=0;
  for (int i=0;menuoptions[i][0]!='\0';++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
}

int main_processinput(const int ch) {
  switch(ch) {
    case KEY_RESIZE: clear(); break;
    case KEY_HOME: highlight=0; break;
    case KEY_END: highlight=lastindex; break;
    case KEY_UP:
      if (highlight==0) highlight=lastindex;
      else --highlight;
      break;
    case KEY_DOWN:
      if (highlight==lastindex) highlight=0;
      else ++highlight;
      break;
    case KEY_ENTER: case '\n':
      switch(highlight) {
        case 0: return PREPMENU;
        case 1: return ABOUTMENU;
        default: return EXIT;
      }
  }
  return MAINMENU;
}
