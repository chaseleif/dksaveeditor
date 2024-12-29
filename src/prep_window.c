#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "shared.h"

extern struct character *players;
extern char **menuoptions, **item_names;
extern char *strdst, *dksavefile, *dklstfile;
extern int menuwidth, highlight;

static const int lastindex = 3;

void setup_prep() {
  strcpy(menuoptions[0],"Path to darklands.lst = ");
  if (!dklstfile) strcat(menuoptions[0],"(not set)");
  else strcat(menuoptions[0],dklstfile);
  strcpy(menuoptions[1],"Save file = ");
  if (!dksavefile) strcat(menuoptions[1],"(not set)");
  else strcat(menuoptions[1],dksavefile);
  strcpy(menuoptions[2],"Open/edit save file");
  strcpy(menuoptions[3],"Return to main menu");
  menuoptions[4][0] = '\0';
  menuwidth=0;
  for (int i=0;menuoptions[i][0]!='\0';++i) {
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
          strdst = dklstfile;
          return FILEMENU;
        case 1:
          strdst = dksavefile;
          return FILEMENU;
        case 2:
          if (!players || !item_names) {
            printerror(1,"First set the save file and LST file");
            break;
          }
          return EDITMENU;
        case 3:
          return MAINMENU;
        default:
          break;
      }
      break;
    case 'c': case 'C': case 'q': case 'Q': return MAINMENU;
    default:
      break;
  }
  return PREPMENU;
}
