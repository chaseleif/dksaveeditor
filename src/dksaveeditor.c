#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <stdint.h>
#include <ncurses.h>
#include <signal.h>
#include <stdarg.h>
#include "structs.h"
#include "shared.h"

struct character *players = NULL;
struct item_definition *items = NULL;
struct saint *saints=NULL;
struct formula *formulas=NULL;
struct savefileheader saveinfo;
struct partyheader partyinfo;

char *strdst, *tmpstr, *titlebar, *dksavefile, *dkdir, *msgstr, *field;
char **menuoptions;
int topy, nrows, menuwidth, highlight;
uint8_t num_items, num_saints, num_formulas;

void printerror(const int n, ...) {
  va_list args;
  va_start(args, n);
  clear();
  int maxy, maxx;
  getmaxyx(stdscr,maxy,maxx);
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
  notimeout(stdscr,FALSE);
  ESCDELAY = 0;
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
  dkdir = malloc(sizeof(char)*MAXSTRLEN);
  titlebar = malloc(sizeof(char)*MAXSTRLEN);
  msgstr = malloc(sizeof(char)*MAXSTRLEN);
  field = malloc(sizeof(char)*MAXSTRLEN);
  tmpstr = malloc(sizeof(char)*MAXSTRLEN);
  load_static_darklands_data();
  
  int lastmenu=EXIT, menulevel=MAINMENU;
  int (*processinput)(const int);
  do {
    if (lastmenu != menulevel) {
      int sethighlight = (lastmenu==ABOUTMENU)?1:
                          (lastmenu==EDITMENU&&menulevel==PREPMENU)?2:0;
      if (lastmenu == FILEMENU && strcmp(tmpstr,strdst)) {
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
          strcpy(tmpstr,strdst);
          if (strdst == dksavefile)
            strcpy(titlebar,"Select the DKSAVEx.SAV file");
          else
            strcpy(titlebar,"Select the Darklands game folder");
          strdst[0]='.'; strdst[1]='\0';
          setup_file();
          processinput = &file_processinput;
          break;
        case EDITMENU:
          switch(saveinfo.month) {
            case 0: strcpy(msgstr,"Januar"); break;
            case 1: strcpy(msgstr,"Februar"); break;
            case 2: strcpy(msgstr,"Maerz"); break;
            case 3: strcpy(msgstr,"April"); break;
            case 4: strcpy(msgstr,"Mai"); break;
            case 5: strcpy(msgstr,"Juni"); break;
            case 6: strcpy(msgstr,"Juli"); break;
            case 7: strcpy(msgstr,"August"); break;
            case 8: strcpy(msgstr,"September"); break;
            case 9: strcpy(msgstr,"Oktober"); break;
            case 10: strcpy(msgstr,"November"); break;
            case 11: strcpy(msgstr,"Dezember"); break;
            default: strcpy(msgstr,"ERROR"); break;
          }
          sprintf(titlebar,"Save \"%s\", der %d. %s %d um %u Uhr bei %s",
                  saveinfo.save_game_label,
                  saveinfo.day, msgstr, saveinfo.year, saveinfo.hour,
                  saveinfo.curr_location_name);
          setup_edit();
          processinput = &edit_processinput;
          break;
        default:
          processinput = &main_processinput;
          menulevel = EXIT;
        break;
      }
      lastmenu = menulevel;
      if (sethighlight) highlight=sethighlight;
    }
    int maxy, maxx;
    getmaxyx(stdscr,maxy,maxx);
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
  free(dkdir);
  free(msgstr);
  free(field);
  free(tmpstr);
  if (items) free(items);
  if (players) free(players);
  if (saints) free(saints);
  if (formulas) free(formulas);
  nocbreak();
  endwin();
  exit(0);
}
