#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <dirent.h>
#include "shared.h"
#include "structs.h"

extern char **menuoptions;
extern char *msgstr, *strdst, *tmpstr, *dksavefile, *dkdir;
extern int topy, nrows, menuwidth, highlight;

extern struct item_definition *items;
extern struct saint *saints;
extern struct formula *formulas;

static int cmp(const void *a, const void *b) {
  return strcmp(*(const char**)a, *(const char**)b);
}
static void sort(char **list, const int n) {
  qsort(list, n, sizeof(char*), cmp);
}

static int lastindex = 0;

void setup_file() {
  struct dirent *de;
  DIR *dr = opendir(strdst);
  if (!dr) {
    printerror(2, "Could not open path", strdst);
    strdst[0]='.'; strdst[1]='\0';
    return;
  }
  nrows=-1;
  const int need_data = (strdst==dkdir)?1:0;
  int havelst=0, havealc=0, havesnt=0;
  while ((de=readdir(dr))) {
    if (!strcmp(de->d_name,".")) continue;
    if (!strcmp(de->d_name,"DARKLAND.LST")) {
      havelst=1;
      continue;
    }
    if (!strcmp(de->d_name,"DARKLAND.ALC")) {
      havealc=1;
      continue;
    }
    if (!strcmp(de->d_name,"DARKLAND.SNT")) {
      havesnt=1;
      continue;
    }
    if (++nrows==MAXMENUITEMS) {
      --nrows;
      printerror(1, "Have to break, nrows already at MAXMENUITEMS");
      break;
    }
    if (strchr(strdst,'/') && !strcmp(de->d_name,"..")) {
      char *check = strchr(strdst,'/')+1;
      while (strchr(check,'/')) { check=strchr(check,'/')+1; }
      if (strcmp(check,"..")) {
        strcpy(menuoptions[nrows], strdst);
        int i=strlen(menuoptions[nrows])-1;
        while (menuoptions[nrows][--i]!='/') { }
        menuoptions[nrows][i] = '\0';
      }
      else
        snprintf(menuoptions[nrows], MAXSTRLEN-1, "%s/%s",
                strdst, de->d_name);
    }
    else
      snprintf(menuoptions[nrows], MAXSTRLEN-1, "%s/%s",
              strdst, de->d_name);
    if (need_data) {
      if (!isdirectory(menuoptions[nrows])) {
        --nrows;
        continue;
      }
    }
    else if (!isdirectory(menuoptions[nrows])) {
      if (strncmp(de->d_name,"DKSAVE",6)) {
        --nrows;
        continue;
      }
      char *ext = strchr(de->d_name,'.');
      if (!ext) { --nrows; continue; }
      if (strcmp(++ext,"SAV")) { --nrows; continue; }
    }
  }
  if (need_data && havelst && havealc && havesnt) {
    load_darklands_data();
    if (items && saints && formulas) {
      menuoptions[0][0] = '\0';
      return;
    }
  }
  lastindex = nrows;
  closedir(dr);
  menuoptions[nrows+1][0] = '\0';
  sort(menuoptions, nrows);
  menuwidth=0;
  for (int i=0;i<nrows;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  if (menuwidth < 50) menuwidth=50;
}

int file_processinput(const int ch) {
  int maxy, maxx;
  getmaxyx(stdscr,maxy,maxx);
  switch(ch) {
    case KEY_RESIZE: clear(); break;
    case KEY_HOME:
      highlight=0;
      if (topy) { topy=0; clear(); }
      break;
    case KEY_END:
      highlight=lastindex;
      if (lastindex > maxy - MENUFIRSTLINE - 2) {
        topy = lastindex-maxy+MENUFIRSTLINE+2;
        clear();
      }
      break;
    case KEY_UP:
      if (highlight==0) {
        highlight=lastindex;
        if (lastindex > maxy - MENUFIRSTLINE - 2) {
          topy = lastindex-maxy+MENUFIRSTLINE+2;
          clear();
        }
      }
      else if (--highlight < topy) { --topy; clear(); }
      break;
    case KEY_DOWN:
      if (highlight==lastindex) { highlight=0; topy=0; clear(); }
      else if (++highlight-topy > maxy - MENUFIRSTLINE - 2) {
        topy = highlight-maxy+MENUFIRSTLINE+2;
        clear();
      }
      break;
    case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
    case 4: case 'q': case 'Q': case 'c': case 'C':
      strcpy(strdst, tmpstr);
      return PREPMENU;
    case 'o': case 'O': case KEY_ENTER: case '\n':
      if (isdirectory(menuoptions[highlight])) {
        strcpy(strdst, menuoptions[highlight]);
        highlight = topy = 0;
        clear();
        setup_file();
        if (menuoptions[0][0]=='\0')
          return PREPMENU;
        break;
      }
      strcpy(strdst, menuoptions[highlight]);
      return PREPMENU;
    default: break;
  }
  return FILEMENU;
}
