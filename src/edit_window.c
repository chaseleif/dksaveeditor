#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "shared.h"

#define scrollto(index)                           \
  do {                                            \
    int maxy, midy;                               \
    getmaxyx(stdscr,maxy,midy);                   \
    midy = maxy/2-4;                              \
    if (midy < 0) midy=0;                         \
    while (highlight<index+midy) {                \
      if (highlight==lastindex) break;            \
      edit_processinput(KEY_DOWN);                \
    }                                             \
    while (highlight>index) {                     \
      edit_processinput(KEY_UP);                  \
    }                                             \
  }while(0)

extern char **menuoptions;
extern char *tmpstr, *titlebar, *msgstr, *dksavefile, *field;
extern int topy, nrows, menuwidth, highlight;
extern uint8_t num_items, num_saints, num_formulas;

extern struct character *players;
extern struct item_definition *items;
extern struct saint *saints;
extern struct formula *formulas;
extern struct savefileheader saveinfo;
extern struct partyheader partyinfo;

static const char attributename[14][8] = {
"End", "Str", "Agl", "Per", "Int", "Chr", " DF",
"Max End", "Max Str", "Max Agl", "Max Per", "Max Int", "Max Chr", "Max DR " };
static const char skillname[19][5] = {
"Edge", "Imp", "Fll", "Pol", "Thr", "Bow", "Msl", "Alch", "Relg", "Virt",
"SpkC", "SpkL", "R&W", "Heal", "Artf", "Stlh", "StrW", "Ride", "WdWs" };
enum { PARTY, PLAYER, ATTR, SKILL,
        ITEM, ADDITEM,
        SAINT, DESCSAINT, ADDSAINT,
        FORMULA, DESCFORMULA, ADDFORMULA };

static struct character *player = NULL;
static int saveindex, laststate, state = PARTY, lastindex = 0;

static void setup_player();
static void player_enter();
static void setup_attributes();
static void attributes_enter();
static void setup_skills();
static void skills_enter();
static void setup_items();
static void items_enter();
static void setup_additem();
static void additem_enter();
static void setup_saints();
static void saint_describe();
static void saints_enter();
static void setup_addsaint();
static void addsaint_enter();
static void addsaint_a();
static void setup_formulas();
static void formula_describe();
static void formulas_enter();
static void setup_addformula();
static void addformula_enter();
static void addformula_a();

void setup_edit() {
  strcpy(menuoptions[0],"Party");
  sprintf(menuoptions[1],"Florin: %u",saveinfo.party_money.florin);
  sprintf(menuoptions[2],"Groschen: %u",saveinfo.party_money.groschen);
  sprintf(menuoptions[3],"Pfennig: %u",saveinfo.party_money.pfennig);
  sprintf(menuoptions[4],"Bank notes: %u",saveinfo.bank_notes);
  sprintf(menuoptions[5],"Reputation: %u",saveinfo.reputation);
  sprintf(menuoptions[6],"Philosopher Stone: %u",saveinfo.philosopher_stone);
  lastindex=7;
  for (int i=0;i<partyinfo.num_curr_characters;++i) {
    sprintf(menuoptions[lastindex++],"%s (%s) age %u",
            players[i].name, players[i].short_name, players[i].age);
  }
  strcpy(menuoptions[lastindex++], "Revert all changes");
  strcpy(menuoptions[lastindex++], "Save all changes");
  strcpy(menuoptions[lastindex++], "Exit saved game");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;menuoptions[i][0]!='\0';++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  topy = 0;
  highlight = 1;
  state = PARTY;
}

static int edit_enter() {
  // the final options after player(s)
  if (highlight==lastindex) return PREPMENU;
  else if (highlight==lastindex-1) {
    savesave(dksavefile, players, &saveinfo, &partyinfo);
    printerror(2,"Saved",dksavefile);
  }
  else if (highlight==lastindex-2) {
    loadsave(dksavefile, &players, &saveinfo, &partyinfo);
    setup_edit();
    scrollto(lastindex-2);
    clear();
  }
  // a player, playerindex == highlight-7
  else if (highlight>=7) {
    player = &players[highlight-7];
    setup_player();
    clear();
  }
  else {
    uint16_t *dst;
    switch(highlight) {
      case 1:
        strcpy(msgstr,"Party florin");
        dst = &saveinfo.party_money.florin;
        break;
      case 2:
        strcpy(msgstr,"Party groschen");
        dst = &saveinfo.party_money.groschen;
        break;
      case 3:
        strcpy(msgstr,"Party pfennig");
        dst = &saveinfo.party_money.pfennig;
        break;
      case 4:
        strcpy(msgstr,"Party bank notes");
        dst = &saveinfo.bank_notes;
        break;
      case 5:
        strcpy(msgstr,"Party reputation");
        dst = &saveinfo.reputation;
        break;
      case 6:
        strcpy(msgstr,"Party philosopher stone");
        dst = &saveinfo.philosopher_stone;
        break;
    }
    sprintf(field,"%u",*dst);
    getinput();
    *dst = (uint16_t)atoi(field);
    if (dst == &saveinfo.party_money.pfennig) {
      uint16_t pfennig = saveinfo.party_money.pfennig%12;
      uint16_t groschen = saveinfo.party_money.pfennig/12;
      uint16_t florin = groschen/20;
      groschen %= 20;
      saveinfo.party_money.pfennig = pfennig;
      saveinfo.party_money.groschen += groschen;
      saveinfo.party_money.florin += florin;
    }
    else if (dst == &saveinfo.party_money.groschen) {
      const uint16_t groschen = saveinfo.party_money.groschen % 20;
      const uint16_t florin = groschen/20;
      saveinfo.party_money.groschen = groschen;
      saveinfo.party_money.florin += florin;
    }
    const int position = highlight;
    setup_edit();
    scrollto(position);
    clear();
  }
  return EDITMENU;
}

static void setup_player() {
  sprintf(menuoptions[0],"%s (%s)",player->name,player->short_name);
  sprintf(menuoptions[1],"Age: %u",player->age);
  strcpy(menuoptions[2],"Player Attributes");
  strcpy(menuoptions[3],"Player Skills");
  lastindex = 3;
  if (items) {
    strcpy(menuoptions[++lastindex],"Player Items");
    strcpy(menuoptions[++lastindex],"Player Saints");
    strcpy(menuoptions[++lastindex],"Player Formulas");
  }
  strcpy(menuoptions[++lastindex],"Return to party");
  menuoptions[lastindex+1][0] = '\0';
  clear();
  state = PLAYER;
  topy = 0;
  highlight = 1;
}

static void player_enter() {
  if (highlight == lastindex) {
    setup_edit();
    for (int i=0;i<partyinfo.num_curr_characters;++i) {
      if (player == &players[i]) {
        highlight = 7+i;
        break;
      }
    }
    clear();
    return;
  }
  switch(highlight) {
    case 1:
      strcpy(msgstr,"Age");
      sprintf(field,"%u",player->age);
      getinput();
      player->age = (uint16_t)atoi(field);
      setup_player();
      break;
    case 2:
      setup_attributes();
      break;
    case 3:
      setup_skills();
      break;
    case 4:
      setup_items();
      break;
    case 5:
      setup_saints();
      break;
    case 6:
      setup_formulas();
      break;
  }
}

static void setup_attributes() {
  uint8_t *curr=(uint8_t*)&player->curr_attrs;
  uint8_t *max=(uint8_t*)&player->max_attrs;
  for (int i=0;i<7;++i) {
    sprintf(menuoptions[i+1],"%s: %u/%u",attributename[i],*curr,*max);
    ++curr;
    ++max;
  }
  strcpy(menuoptions[8], "Finished with attributes");
  menuoptions[9][0] = '\0';
  lastindex = 8;
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
  state = ATTR;
}

static void attributes_enter() {
  if (highlight == lastindex) {
    setup_player();
    highlight = 2;
    return;
  }
  strcpy(msgstr,attributename[highlight-1]);
  uint8_t *curr=(uint8_t*)&player->curr_attrs;
  uint8_t *max=(uint8_t*)&player->max_attrs;
  curr += highlight-1;
  max += highlight-1;
  sprintf(field,"Current:%u/Max:%u", *curr, *max);
  getinput();
  char *value = field;
  char *end = strchr(value,'.');
  *end='\0';
  *curr = (uint8_t)atoi(value);
  value = end+1;
  *max = (uint8_t)atoi(value);
  const int position = highlight;
  setup_attributes();
  scrollto(position);
}

static void setup_additem() {
  if (player->num_items==64) {
    printerror(2,"Players are limited to 64 items",
                  "Unable to add another item");
    return;
  }
  strcpy(tmpstr, titlebar);
  strcpy(titlebar,"Add item");
  lastindex = 1;
  for (int i=0;i<num_items;++i) {
    strcpy(menuoptions[lastindex++],items[i].name);
  }
  strcpy(menuoptions[lastindex++], "Finished adding items");
  menuoptions[lastindex--][0] = '\0';
  clear();
  topy = 0;
  highlight = 1;
  state = ADDITEM;
}
static void additem_enter() {
  if (highlight < lastindex && player->num_items==64) {
    printerror(2,"Players are limited to 64 items",
                  "Unable to add another item");
    highlight = lastindex;
  }
  if (highlight==lastindex) {
    strcpy(titlebar,tmpstr);
    setup_items();
    topy = 0;
    edit_processinput(KEY_END);
    edit_processinput(KEY_UP);
    return;
  }
  player->items[player->num_items].code = highlight-1;
  player->items[player->num_items].type = items[highlight-1].type;
  player->items[player->num_items].quality = items[highlight-1].quality;
  player->items[player->num_items].quantity = 1;
  player->items[player->num_items].weight = items[highlight-1].weight;
  ++player->num_items;
  sprintf(msgstr, "Added %s\n", items[highlight-1].name);
  printerror(1, msgstr);
}

static void setup_items() {
  lastindex = 1;
  for (int i=0;i<player->num_items;++i) {
    sprintf(menuoptions[lastindex++], "%s quality(%u) quantity(%u)",
            items[player->items[i].code].name,
            player->items[i].quality, player->items[i].quantity);
  }
  strcpy(menuoptions[lastindex++], "Add items");
  strcpy(menuoptions[lastindex++], "Finished with items");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
  state = ITEM;
}

static void items_enter() {
  if (highlight==lastindex) {
    setup_player();
    highlight=4;
    return;
  }
  if (highlight==lastindex-1) {
    setup_additem();
    return;
  }
  strcpy(msgstr,items[player->items[highlight-1].code].name);
  sprintf(field,"Quality:%u/Quantity:%u",
          player->items[highlight-1].quality,
          player->items[highlight-1].quantity);
  getinput();
  char *value = field;
  char *end = strchr(value,'.');
  *end='\0';
  player->items[highlight-1].quality = (uint8_t)atoi(value);
  value = end+1;
  player->items[highlight-1].quantity = (uint8_t)atoi(value);
  const int position = highlight;
  setup_items();
  scrollto(position);
}

static void setup_saints() {
  lastindex = 1;
  for (int i=0;i<num_saints;++i) {
    const int byte = i>>3;
    const int shmt = 7-(i&7);
    if ((player->saints_known[byte]>>shmt)&1)
      sprintf(menuoptions[lastindex++], "%d: %s (%s)",
              i, saints[i].name, saints[i].short_name);
  }
  strcpy(menuoptions[lastindex++], "Add saints");
  strcpy(menuoptions[lastindex++], "Finished with saints");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
  state = SAINT;
}

static void saint_describe() {
  char *start = menuoptions[highlight];
  char *end = strchr(start,':');
  *end = '\0';
  start = saints[atoi(start)].description;
  *end = ':';
  strcpy(tmpstr,menuoptions[0]);
  strcpy(menuoptions[0],menuoptions[highlight]);
  end = strchr(start, ' ');
  lastindex = 0;
  while (end) {
    if (end-start > 50) {
      *end='\0';
      strcpy(menuoptions[++lastindex], start);
      *end=' ';
      start = end+1;
      end = strchr(start, ' ');
    }
    else end = strchr(end+1, ' ');
  }
  strcpy(menuoptions[++lastindex], start);
  menuwidth = 0;
  for (int i=0;i<=lastindex;++i) {
    if (strlen(menuoptions[i])>menuwidth) menuwidth = strlen(menuoptions[i]);
  }
  menuoptions[lastindex+1][0] = '\0';
  topy = 0;
  highlight = 0;
  state = DESCSAINT;
  clear();
}

static void saints_enter() {
  if (highlight==lastindex) {
    setup_player();
    highlight=5;
    return;
  }
  if (highlight==lastindex-1) {
    setup_addsaint();
    return;
  }
  laststate = state;
  saveindex = highlight;
  saint_describe();
}

static void setup_addsaint() {
  strcpy(tmpstr,"Press \'a\' to add the highlighted saint");
  if (strcmp(tmpstr,titlebar)) {
    strcpy(msgstr,titlebar);
    strcpy(titlebar,tmpstr);
  }
  lastindex = 1;
  for (int i=0;i<num_saints;++i) {
    const int byte = i>>3;
    const int shmt = 7-(i&7);
    if (!((player->saints_known[byte]>>shmt)&1))
      sprintf(menuoptions[lastindex++], "%d: %s (%s)",
              i, saints[i].name, saints[i].short_name);
  }
  strcpy(menuoptions[lastindex++], "Finished adding saints");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
  state = ADDSAINT;
}

static void addsaint_enter() {
  if (highlight==lastindex) {
    strcpy(titlebar,msgstr);
    setup_saints();
    edit_processinput(KEY_END);
    edit_processinput(KEY_UP);
    return;
  }
  laststate = state;
  saveindex = highlight;
  saint_describe();
}

static void addsaint_a() {
  saveindex = highlight;
  char *colon = strchr(menuoptions[highlight],':');
  *colon = '\0';
  highlight = atoi(menuoptions[highlight]);
  *colon = ':';
  const int byte = highlight>>3;
  const int shmt = 7-(highlight&7);
  player->saints_known[byte] |= 1<<shmt;
  const int oldtopy = topy;
  setup_addsaint();
  topy = oldtopy;
  highlight = saveindex;
  if (highlight==lastindex) edit_processinput(KEY_UP);
}

static void setup_formulas() {
  lastindex = 1;
  for (int i=0;i<num_formulas;++i) {
    const int byte = i/3;
    const int shmt = i%3;
    if ((player->formulas_known[byte]>>shmt)&1)
      sprintf(menuoptions[lastindex++], "%d: %s (%s) %s",
              i, formulas[i].name, formulas[i].short_name,
              (shmt==0)?"q25":(shmt==1)?"q35":"q45");
  }
  strcpy(menuoptions[lastindex++], "Add formulas");
  strcpy(menuoptions[lastindex++], "Finished with formulas");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
  state = FORMULA;
}

static void formula_describe() {
  strcpy(tmpstr,menuoptions[0]);
  strcpy(menuoptions[0],menuoptions[highlight]);
  char *colon = strchr(menuoptions[highlight],':');
  *colon = '\0';
  strcpy(menuoptions[1],formulas[atoi(menuoptions[highlight])].description);
  *colon = ':';
  menuoptions[2][0] = '\0';
  menuwidth = 0;
  for (int i=0;i<3;++i) {
    if (strlen(menuoptions[i])>menuwidth) menuwidth = strlen(menuoptions[i]);
  }
  topy = 0;
  highlight = 0;
  state = DESCFORMULA;
  clear();
}

static void formulas_enter() {
  if (highlight==lastindex) {
    setup_player();
    highlight=6;
    return;
  }
  if (highlight==lastindex-1) {
    setup_addformula();
    return;
  }
  laststate = state;
  saveindex = highlight;
  formula_describe();
}

static void setup_addformula() {
  strcpy(tmpstr,"Press \'a\' to add the highlighted formula");
  if (strcmp(tmpstr,titlebar)) {
    strcpy(msgstr,titlebar);
    strcpy(titlebar,tmpstr);
  }
  lastindex = 1;
  for (int i=0;i<num_formulas;++i) {
    const int byte = i/3;
    const int shmt = i%3;
    if (!((player->formulas_known[byte]>>shmt)&1))
      sprintf(menuoptions[lastindex++], "%d: %s (%s) %s",
              i, formulas[i].name, formulas[i].short_name,
              (shmt==0)?"q25":(shmt==1)?"q35":"q45");
  }
  strcpy(menuoptions[lastindex++], "Finished adding formulas");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
  state = ADDFORMULA;
}

static void addformula_enter() {
  if (highlight==lastindex) {
    strcpy(titlebar,msgstr);
    setup_formulas();
    edit_processinput(KEY_END);
    edit_processinput(KEY_UP);
    return;
  }
  laststate = state;
  saveindex = highlight;
  formula_describe();
}

static void addformula_a() {
  saveindex = highlight;
  char *colon = strchr(menuoptions[highlight],':');
  *colon = '\0';
  highlight = atoi(menuoptions[highlight]);
  *colon = ':';
  const int byte = highlight/3;
  const int shmt = highlight%3;
  player->formulas_known[byte] |= 1<<shmt;
  const int oldtopy = topy;
  setup_addformula();
  topy = oldtopy;
  highlight = saveindex;
  if (highlight==lastindex) edit_processinput(KEY_UP);
}

static void setup_skills() {
  uint8_t *skill=(uint8_t*)&player->skills;
  for (int i=0;i<19;++i) {
    sprintf(menuoptions[i+1],"%s: %u",skillname[i],*skill);
    ++skill;
  }
  strcpy(menuoptions[20], "Finished with skills");
  menuoptions[21][0] = '\0';
  lastindex = 20;
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
  state = SKILL;
}

static void skills_enter() {
  if (highlight == lastindex) {
    setup_player();
    highlight=3;
    return;
  }
  strcpy(msgstr,skillname[highlight-1]);
  uint8_t *skill = (uint8_t*)&player->skills;
  skill += highlight-1;
  sprintf(field,"%u",*skill);
  getinput();
  *skill = (uint8_t)atoi(field);
  const int position = highlight;
  setup_skills();
  scrollto(position);
}

int edit_processinput(const int ch) {
  if (state == DESCSAINT || state == DESCFORMULA) {
    switch(ch) {
      case KEY_RESIZE: clear(); break;
      case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
      case 4: case 'q': case 'Q': case 'c': case 'C':
      case KEY_ENTER: case '\n':
        strcpy(menuoptions[0],tmpstr);
        switch(laststate) {
          case SAINT: setup_saints(); break;
          case ADDSAINT: setup_addsaint(); break;
          case FORMULA: setup_formulas(); break;
          case ADDFORMULA: setup_addformula(); break;
        }
        scrollto(saveindex);
        break;
    }
    return EDITMENU;
  }
  if (highlight != lastindex && (ch=='a' || ch=='A')) {
    if (state == ADDSAINT) {
      addsaint_a();
      return EDITMENU;
    }
    if (state == ADDFORMULA) {
      addformula_a();
      return EDITMENU;
    }
  }
  int maxy, maxx, position=1;
  getmaxyx(stdscr,maxy,maxx);
  switch(ch) {
    case KEY_RESIZE: clear(); break;
    case KEY_HOME:
      highlight=1;
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
      if (highlight==1) {
        highlight=lastindex;
        if (lastindex > maxy-MENUFIRSTLINE-2) {
          topy = lastindex-maxy+MENUFIRSTLINE+2;
          clear();
        }
      }
      else if (--highlight < topy) { --topy; clear(); }
      break;
    case KEY_DOWN:
      if (highlight==lastindex) { highlight=1; topy=0; clear(); }
      else if (++highlight-topy > maxy-MENUFIRSTLINE-2) {
        topy = highlight-maxy+MENUFIRSTLINE+2;
        clear();
      }
      break;
    case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
    case 4: case 'q': case 'Q': case 'c': case 'C':
      switch(state) {
        case PARTY: return PREPMENU;
        case PLAYER:
          setup_edit();
          for (int i=0;i<partyinfo.num_curr_characters;++i) {
            if (player == &players[i]) {
              position = 7+i;
              break;
            }
          }
          clear();
          highlight = position;
          return EDITMENU;
        case ATTR: position = 2; break;
        case SKILL: position = 3; break;
        case ITEM: position = 4; topy = 0; break;
        case SAINT: position = 5; topy = 0; break;
        case FORMULA: position = 6; topy = 0; break;
        case ADDSAINT:
          highlight=lastindex;
          addsaint_enter();
          return EDITMENU;
        case ADDFORMULA:
          highlight = lastindex;
          addformula_enter();
          return EDITMENU;
      }
      setup_player();
      scrollto(position);
      break;
    case KEY_ENTER: case '\n':
      switch(state) {
        case PARTY:
          return edit_enter();
        case PLAYER:
          player_enter(); break;
        case ATTR:
          attributes_enter(); break;
        case SKILL:
          skills_enter(); break;
        case ITEM:
          items_enter(); break;
        case ADDITEM:
          additem_enter(); break;
        case SAINT:
          saints_enter(); break;
        case ADDSAINT:
          addsaint_enter(); break;
        case FORMULA:
          formulas_enter(); break;
        case ADDFORMULA:
          addformula_enter(); break;
        default: break;
      }
      break;
    default: break;
  }
  return EDITMENU;
}
