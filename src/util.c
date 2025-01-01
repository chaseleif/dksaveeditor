#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include "shared.h"

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
  for (int i=0;i<n;++i) {
    char *s = va_arg(args,char*);
    const int menuwidth = strlen(s);
    int maxy,maxx;
    getmaxyx(stdscr,maxy,maxx);
    printwithattr(i+MENUFIRSTLINE,MENUMARGIN,A_STANDOUT,"%s",s);
  }
  va_end(args);
  refreshwithborder(DKBLKGRN);
  flushinp();
  getch();
  clear();
  refreshwithborder(DKBLKGRN);
}
