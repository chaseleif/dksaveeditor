#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "shared.h"

extern struct character *players;
extern struct item_definition *items;
extern struct saint *saints;
extern struct formula *formulas;
extern char **menuoptions;
extern char *strdst, *dksavefile, *dkdir;
extern int menuwidth, highlight;

static int need_data;
static int lastindex;

void setup_prep() {
  lastindex=0;
  if (!items || !saints || !formulas) {
    need_data=1;
    strcpy(menuoptions[lastindex++],"Set path to Darklands game");
  }
  else need_data=0;
  strcpy(menuoptions[lastindex],"Save file = ");
  if (!dksavefile) strcat(menuoptions[lastindex],"(not set)");
  else strcat(menuoptions[lastindex],dksavefile);
  strcpy(menuoptions[++lastindex],"Open/edit save file");
  strcpy(menuoptions[++lastindex],"Return to main menu");
  menuoptions[lastindex+1][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
}

int prep_processinput(const int ch) {
  switch(ch) {
    case KEY_RESIZE: clear(); break;
    case KEY_HOME: highlight=0; break;
    case KEY_END: highlight=lastindex; break;
    case KEY_UP:
      if (highlight==0) highlight=lastindex;
      else highlight -= 1;
      break;
    case KEY_DOWN:
      if (highlight==lastindex) highlight=0;
      else highlight += 1;
      break;
    case KEY_ENTER: case '\n':
      switch(highlight) {
        case 0:
          strdst = (need_data)?dkdir:dksavefile;
          return FILEMENU;
        case 1:
          if (need_data) {
            strdst = dksavefile;
            return FILEMENU;
          }
          if (!players) {
            printerror(1,"First select a save file to open");
            break;
          }
          return EDITMENU;
        case 2:
          if (need_data && !players) {
            printerror(1,"First select a save file to open");
            break;
          }
          return (need_data)?EDITMENU:MAINMENU;
        case 3:
          return MAINMENU;
      }
      break;
    case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
    case 4: case 'q': case 'Q': case 'c': case 'C':
      return MAINMENU;
    default:
      break;
  }
  return PREPMENU;
}
