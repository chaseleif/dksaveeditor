#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include "shared.h"

extern int menuwidth;

uint8_t atouint8(char *a) {
  uint16_t ret = 0;
  for (int i=strlen(a)-1,mult=1;i>=0;--i,mult*=10) {
    if (a[i]!='0') {
      ret += (a[i]-'0')*mult;
      if (ret >= UINT8_MAX) return UINT8_MAX;
    }
  }
  return ret;
}

uint16_t atouint16(char *a) {
  uint32_t ret = 0;
  for (int i=strlen(a)-1,mult=1;i>=0;--i,mult*=10) {
    if (a[i]!='0') {
      ret += (a[i]-'0')*mult;
      if (ret >= UINT16_MAX) return UINT16_MAX;
    }
  }
  return ret;
}

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
