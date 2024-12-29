#include <string.h>
#include <ncurses.h>
#include "shared.h"

extern char **menuoptions;
extern int menuwidth, highlight;

void setup_about() {
  strcpy(menuoptions[0],"        Darklands save-game editor");
  strcpy(menuoptions[1],"      Copyright (C) 2025 ~ Chase Phelps");
  strcpy(menuoptions[2],"This program comes with ABSOLUTELY NO WARRANTY");
  strcpy(menuoptions[3],"This is free software with a GNU GPLv3 license");
  menuoptions[4][0] = '\0';
  menuwidth=0;
  for (int i=0;menuoptions[i][0]!='\0';++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  highlight = -1;
}

int about_processinput(const int ch) {
  if (ch == KEY_RESIZE) { clear(); return ABOUTMENU; }
  return MAINMENU;
}
