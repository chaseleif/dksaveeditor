#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <dirent.h>
#include "shared.h"

extern char **menuoptions;
extern char *msgstr, *strdst, *dstbak, *dksavefile, *dklstfile;
extern int topy, nrows, menuwidth, highlight;

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
  nrows=0;
  while ((de=readdir(dr))) {
    if (!strcmp(de->d_name,".")) continue;
    if (strchr(strdst,'/') && !strcmp(de->d_name,"..")) {
      char *check = strchr(strdst,'/')+1;
      while (strchr(check,'/')) { check=strchr(check,'/')+1; }
      if (strcmp(check,"..")) {
        strcpy(menuoptions[nrows], strdst);
        int i=strlen(menuoptions[nrows])-1;
        while (menuoptions[nrows][--i]!='/') { };
        menuoptions[nrows][i] = '\0';
      }
      else
        snprintf(menuoptions[nrows], MAXSTRLEN-1, "%s/%s",
                strdst, de->d_name);
    }
    else
      snprintf(menuoptions[nrows], MAXSTRLEN-1, "%s/%s",
              strdst, de->d_name);
    if (!isdirectory(menuoptions[nrows])) {
      char *separator = strchr(menuoptions[nrows],'/');
      while (strchr(++separator, '/')) { separator=strchr(separator, '/'); }
      if (!strcmp(separator,"DARKLAND.LST")) { }
      else if (!strncmp(separator,"DKSAVE",6)) { }
      else continue;
    }
    ++nrows;
    if (nrows+1==MAXMENUITEMS) {
      printerror(1, "Have to break, nrows+1==MAXMENUITEMS");
      break;
    }
  }
  lastindex = nrows - 1;
  closedir(dr);
  menuoptions[nrows][0] = '\0';
  sort(menuoptions, nrows);
  menuwidth=0;
  for (int i=0;i<nrows;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  if (menuwidth < 50) menuwidth=50;
}

int file_processinput(const int ch) {
  switch(ch) {
    case KEY_RESIZE: clear(); break;
    case KEY_HOME:
      highlight=0;
      if (topy) { topy=0; clear(); }
      break;
    case KEY_END:
      highlight=lastindex;
      if (lastindex > LINES - MENUFIRSTLINE - 2) {
        topy = lastindex-LINES+MENUFIRSTLINE+2;
        clear();
      }
      break;
    case KEY_UP:
      if (highlight==0) {
        highlight=lastindex;
        if (lastindex > LINES - MENUFIRSTLINE - 2) {
          topy = lastindex-LINES+MENUFIRSTLINE+2;
          clear();
        }
      }
      else if (--highlight < topy) { --topy; clear(); }
      break;
    case KEY_DOWN:
      if (highlight==lastindex) { highlight=0; topy=0; clear(); }
      else if (++highlight-topy > LINES - MENUFIRSTLINE - 2) {
        topy = highlight-LINES+MENUFIRSTLINE+2;
        clear();
      }
      break;
    case 'o': case 'O':
      if (isdirectory(menuoptions[highlight])) {
        strcpy(strdst, menuoptions[highlight]);
        highlight = topy = 0;
        clear();
        setup_file();
      }
      else {
        snprintf(msgstr, MAXSTRLEN-1,
                "%s does not appear to be a directory",
                menuoptions[highlight]);
        printerror(1, msgstr);
      }
      break;
    case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
    case 4: case 'q': case 'Q': case 'c': case 'C':
      strcpy(strdst, dstbak);
      return PREPMENU;
    case KEY_ENTER: case '\n':
      if (isdirectory(menuoptions[highlight])) {
        snprintf(msgstr, MAXSTRLEN-1,
                "%s does not appear to be a file",
                menuoptions[highlight]);
        printerror(1, msgstr);
        break;
      }
      strcpy(strdst, menuoptions[highlight]);
      return PREPMENU;
    default: break;
  }
  return FILEMENU;
}
