#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <stdint.h>
#include <ncurses.h>
#include <signal.h>
#include "structs.h"
#include "shared.h"

struct character *players = NULL;
struct savefileheader saveinfo;
struct partyheader partyinfo;

char *strdst, *dstbak, *titlebar, *dksavefile, *dklstfile, *msgstr, *field;
char **menuoptions, **item_names=NULL;
int topy, nrows, menuwidth, highlight;

void printerror(const int n, ...) {
  va_list args;
  va_start(args, n);
  clear();
  const int margin = MENUMARGIN;
  for (int i=0;i<n;++i) {
    printwithattr(i+MENUFIRSTLINE,margin,
                  A_STANDOUT,"%s",va_arg(args,char*));
  }
  va_end(args);
  refreshwithborder(DKBLKGRN);
  getch();
  clear();
  refreshwithborder(DKBLKGRN);
}

static void finish(int sig);

int main(int argc, char **argv) {
  signal(SIGINT, finish);
  setlocale(LC_ALL,"");
  initscr();
  keypad(stdscr,TRUE);
  cbreak();
  noecho();
  notimeout(stdscr,TRUE);
  intrflush(stdscr,TRUE);
  hide_cursor();
  if (has_colors()) {
    start_color();
    init_pair(DKBLKGRN, COLOR_BLACK, COLOR_GREEN);
    init_pair(DKBLKCYN, COLOR_BLACK, COLOR_CYAN);
    init_pair(DKBLKRED, COLOR_BLACK, COLOR_RED);
    init_pair(DKWHTBLK, COLOR_WHITE, COLOR_BLACK);
    init_pair(DKWHTBLU, COLOR_WHITE, COLOR_BLUE);
    init_pair(DKCYNBLU, COLOR_CYAN, COLOR_BLUE);
    init_pair(DKCYNBLK, COLOR_CYAN, COLOR_BLACK);
    init_pair(DKGRNBLK, COLOR_GREEN, COLOR_BLACK);
    init_pair(DKREDBLK, COLOR_RED, COLOR_BLACK);
    init_pair(DKREDCYN, COLOR_RED, COLOR_CYAN);
    init_pair(DKBLUWHT, COLOR_BLUE, COLOR_WHITE);
  }

  menuoptions = malloc(sizeof(char*)*MAXMENUITEMS);
  for (int i=0;i<MAXMENUITEMS;++i) {
    menuoptions[i] = malloc(sizeof(char)*MAXSTRLEN);
  }
  dksavefile = malloc(sizeof(char)*MAXSTRLEN);
  dklstfile = malloc(sizeof(char)*MAXSTRLEN);
  titlebar = malloc(sizeof(char)*MAXSTRLEN);
  msgstr = malloc(sizeof(char)*MAXSTRLEN);
  field = malloc(sizeof(char)*MAXSTRLEN);
  dstbak = malloc(sizeof(char)*MAXSTRLEN);
  
  int lastmenu=EXIT, menulevel=MAINMENU;
  int (*processinput)(const int);
  do {
    if (lastmenu != menulevel) {
      int sethighlight = (lastmenu==ABOUTMENU)?1:
                          (lastmenu==FILEMENU&&strdst==dksavefile)?1:
                          (lastmenu==FILEMENU&&strdst==NULL)?2:
                          (lastmenu==EDITMENU&&menulevel==PREPMENU)?2:0;
      if (lastmenu == FILEMENU && strdst && strcmp(dstbak,strdst)) {
        if (strdst == dksavefile) {
          if (isfile(dksavefile)) {
            if (players) {
              free(players);
              players = NULL;
            }
            loadsave(dksavefile, &players, &saveinfo, &partyinfo);
            if (!players) {
              printerror(2,"Error loading save file:", dksavefile);
              strcpy(dksavefile, ".");
            }
          }
        }
        else if (!item_names && isfile(dklstfile)) {
          item_names = get_item_names(dklstfile);
          if (!item_names) {
            printerror(2,"Error loading LST file:", dklstfile);
            strcpy(dklstfile, ".");
          }
        }
        strdst = NULL;
      }
      clear();
      topy = nrows = highlight = 0;
      switch(menulevel) {
        case MAINMENU:
          strcpy(titlebar,"Darklands game-save editor and viewer");
          setup_main();
          processinput = &main_processinput;
          break;
        case ABOUTMENU:
          titlebar[0] = '\0';
          setup_about();
          processinput = &about_processinput;
          break;
        case PREPMENU:
          strcpy(titlebar,"Setup paths and open save file");
          setup_prep();
          processinput = &prep_processinput;
          break;
        case FILEMENU:
          strcpy(dstbak,strdst);
          char *samplename = (strdst==dksavefile)?"DKSAVEx.SAV":
                                                  "DARKLAND.LST";
          sprintf(titlebar,"Browse to the %s file", samplename);
          printerror(4, titlebar,
                        "Press \'o\' to open a directory",
                        "Press \'q\' or \'c\' to cancel and return",
                        "Press enter to confirm choice");
          sprintf(titlebar,"Select the %s file", samplename);
          strdst[0]='.'; strdst[1]='\0';
          setup_file();
          processinput = &file_processinput;
          break;
        case EDITMENU:
          sprintf(titlebar,"Save \"%s\" am %d.%d.%d um %u Uhr bei %s",
                  saveinfo.save_game_label,
                  saveinfo.day, saveinfo.month, saveinfo.year, saveinfo.hour,
                  saveinfo.curr_location_name);
          setup_edit();
          processinput = &edit_processinput;
          break;
        default:
          printerror(1, "Oh no, menulevel not caught");
          menulevel = EXIT;
        break;
      }
      lastmenu = menulevel;
      if (sethighlight) highlight=sethighlight;
    }
    if (titlebar[0]!='\0')
      printcolor(TITLELINENUM,TITLEMARGIN,DKGRNBLK,"%s",titlebar);
    const int leftmargin = MENUMARGIN;
    for (int i=topy;menuoptions[i][0]!='\0';++i) {
      if (i==highlight)
        printwithattr(i-topy+MENUFIRSTLINE,leftmargin,
                      A_STANDOUT,"%s",menuoptions[i]);
      else
        mvprintw(i-topy+MENUFIRSTLINE,leftmargin,"%s",menuoptions[i]);
    }
    refreshwithborder(DKBLKGRN);
    menulevel = (*processinput)(getch());
  }while (menulevel != EXIT);
  finish(0);
  return 0;
}

static void finish(int sig) {
  for (int i=0;i<MAXMENUITEMS;++i) {
    free(menuoptions[i]);
  }
  free(menuoptions);
  free(titlebar);
  free(dksavefile);
  free(dklstfile);
  free(msgstr);
  free(field);
  free(dstbak);
  if (item_names) {
    for (int i=0;item_names[i];++i) { free(item_names[i]); }
    free(item_names);
  }
  if (players) free(players);
  nocbreak();
  endwin();
  exit(0);
}
