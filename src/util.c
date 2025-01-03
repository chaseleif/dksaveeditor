#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include "shared.h"

int istrcmp(char *a, char*b) {
  if (strlen(a)!=strlen(b)) return 1;
  while (*a) {
    if (*a != *b) {
      char c=*a,d=*b;
      if (c>='A' && c<='Z') c |= 0x20;
      if (d>='A' && d<='Z') d |= 0x20;
      if (c!=d) return 1;
    }
    ++a;
    ++b;
  }
  return 0;
}
int istrncmp(char *a, char *b, int n) {
  if (strlen(a)<n || strlen(b)<n) return 1;
  for (int i=0;i<n;++i) {
    if (*a != *b) {
      char c=*a,d=*b;
      if (c>='A' && c<='Z') c |= 0x20;
      if (d>='A' && d<='Z') d |= 0x20;
      if (c!=d) return 1;
    }
    ++a;
    ++b;
  }
  return 0;
}

uint8_t rgb2vga(const uint8_t rgb) {
  return rgb>>2;
}
uint8_t vga2rgb(const uint8_t vga) {
  return (vga<<2)|(vga>>4);
}

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

#ifndef NO_NCURSES

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
  int maxy,maxx;
  getmaxyx(stdscr,maxy,maxx);
  const int menuwidth = 13;
  printwithattr(n+1+MENUFIRSTLINE,MENUMARGIN,A_STANDOUT,"%s","(Press enter)");
  va_end(args);
  refreshwithborder(DKBLKGRN);
  flushinp();
  int ch = getch();
  while (ch != KEY_ENTER) { ch=getch(); }
  clear();
  refreshwithborder(DKBLKGRN);
}

#endif //NO_NCURSES
