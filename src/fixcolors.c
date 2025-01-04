#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "shared.h"

extern char *dksavefile;
extern struct partyheader partyinfo;

static int runfixcolors() {
  FILE *savefile = fopen(dksavefile, "rb+");
  if (!savefile) {
    printerror(2,"Unable to open save file", dksavefile);
    return -1;
  }
  // 275 is 2 bytes/16 bits past where the colors are supposed to be
  fseek(savefile, 275, SEEK_SET);
  struct person_colors party_colors[5];
  fread(party_colors, sizeof(struct person_colors), 5, savefile);
  // this is where colors should be
  fseek(savefile, 273, SEEK_SET);
  fwrite(party_colors, sizeof(struct person_colors), 5, savefile);
  memcpy(partyinfo.party_colors, party_colors, sizeof(struct person_colors)*5);
  fclose(savefile);
  return 0;
}

void fixcolors() {
  clear();
  const int menuwidth = 40;
  int maxy,maxx;
  getmaxyx(stdscr,maxy,maxx);
  printwithattr(MENUFIRSTLINE,MENUMARGIN,A_STANDOUT,"%s",
                "This fix will fix color-errors caused by");
  printwithattr(MENUFIRSTLINE+1,MENUMARGIN,A_STANDOUT,"%s",
                "the color bytes being written incorrectly.");
  printwithattr(MENUFIRSTLINE+3,MENUMARGIN,A_STANDOUT,"%s",
                "This should not be done unless necessary.");
  printwithattr(MENUFIRSTLINE+4,MENUMARGIN,A_STANDOUT,"%s",
                "Do this fix once and colors should be corrected.");
  printwithattr(MENUFIRSTLINE+5,MENUMARGIN,A_STANDOUT,"%s",
                "Don\'t repeatedly apply this fix if issues persist");
  printwithattr(MENUFIRSTLINE+7,MENUMARGIN,A_STANDOUT,"%s",
                "Press \'y\' to apply this fix.");
  printwithattr(MENUFIRSTLINE+8,MENUMARGIN,A_STANDOUT,"%s",
                "Press any other key to cancel.");
  refreshwithborder(DKBLKGRN);
  flushinp();
  int ch = getch();
  if ((ch|0x20) =='y' && !runfixcolors())
    printerror(1,"Color-fix patch was applied, do not run again.");
  else
    printerror(1,"Color-fix patch was not applied.");
}
