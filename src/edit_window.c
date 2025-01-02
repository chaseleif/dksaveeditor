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
enum { PARTY=0, PLAYER=1, ATTR=2, SKILL=4,
        ITEM=8, SAINT=16, FORMULA=32 };
enum { BASE=0, ADD=1, DESC=2 };

static struct character *player = NULL;
static int saveindex, state, lastindex;

static void setup_player();
static void player_enter();
static void setup_attributes();
static void attributes_enter();
static void setup_skills();
static void skills_enter();
static void setup_addthing();
static void addthing_enter();
static void addthing_confirm();

static void setup_thing();
static void thing_enter();
static void describe();

static void setup_addthing() {
  state |= ADD;
  sprintf(msgstr,"Press \'a\' to add the highlighted %s",
          (state&ITEM)?"item":(state&SAINT)?"saint":"formula");
  if (strcmp(msgstr,titlebar)) {
    strcpy(tmpstr,titlebar);
    strcpy(titlebar,msgstr);
  }
  lastindex = 1;
  if (state&ITEM) {
    for (int i=0;i<num_items;++i) {
      int haveit = 0;
      for (int x=0;x<player->num_items;++x) {
        if (player->items[x].code==i) { haveit=1; break; }
      }
      if (!haveit)
        sprintf(menuoptions[lastindex++], "%d: %s", i, items[i].name);
    }
  }
  else if (state&SAINT) {
    for (int i=0;i<num_saints;++i) {
      const int byte = i>>3;
      const int shmt = 7-(i&7);
      if (!((player->saints_known[byte]>>shmt)&1))
        sprintf(menuoptions[lastindex++], "%d: %s (%s)",
                i, saints[i].name, saints[i].short_name);
    }
  }
  else { //if(state&FORMULA) {
    for (int i=0;i<num_formulas;++i) {
      const int byte = i/3;
      const int shmt = i%3;
      if (!((player->formulas_known[byte]>>shmt)&1))
        sprintf(menuoptions[lastindex++], "%d: %s (%s) %s",
                i, formulas[i].name, formulas[i].short_name,
                (shmt==0)?"q25":(shmt==1)?"q35":"q45");
    }
  }
  sprintf(menuoptions[lastindex++], "Finished adding %s",
          (state&ITEM)?"items":(state&SAINT)?"saints":"formulas");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
}

static void addthing_enter() {
  if (highlight==lastindex) {
    strcpy(titlebar,tmpstr);
    state -= ADD;
    setup_thing();
    topy = 0;
    edit_processinput(KEY_END);
    edit_processinput(KEY_UP);
    return;
  }
  if (state&(SAINT|FORMULA)) {
    saveindex = highlight;
    describe();
  }
}

static void addthing_confirm() {
  saveindex = highlight;
  const int oldtopy = topy;
  char *colon = strchr(menuoptions[highlight],':');
  *colon = '\0';
  highlight = atoi(menuoptions[highlight]);
  *colon = ':';
  if (state&ITEM) {
    player->items[player->num_items].code = highlight;
    player->items[player->num_items].type = items[highlight].type;
    player->items[player->num_items].quality = items[highlight].quality;
    player->items[player->num_items].quantity = 1;
    player->items[player->num_items].weight = items[highlight].weight;
    if (++player->num_items==64) {
      printerror(2,"Players are limited to 64 items",
                    "Unable to add any more items");
      state -= ADD;
      setup_thing();
      edit_processinput(KEY_END);
      edit_processinput(KEY_UP);
      return;
    }
  }
  else if (state&SAINT) {
    const int byte = highlight>>3;
    const int shmt = 7-(highlight&7);
    player->saints_known[byte] |= 1<<shmt;
  }
  else { //if (state&FORMULA) {
    const int byte = highlight/3;
    const int shmt = highlight%3;
    player->formulas_known[byte] |= 1<<shmt;
  }
  setup_addthing();
  topy = oldtopy;
  highlight = saveindex;
  if (highlight==lastindex) edit_processinput(KEY_UP);
}

void setup_edit() {
  strcpy(menuoptions[0],"Party");
  sprintf(menuoptions[1],"Florin: %u",saveinfo.party_money.florin);
  sprintf(menuoptions[2],"Groschen: %u",saveinfo.party_money.groschen);
  sprintf(menuoptions[3],"Pfennig: %u",saveinfo.party_money.pfennig);
  sprintf(menuoptions[4],"Bank notes: %u",saveinfo.bank_notes);
  sprintf(menuoptions[5],"Reputation: %u",saveinfo.reputation);
  sprintf(menuoptions[6],"Philosopher Stone: %u",saveinfo.philosopher_stone);
  sprintf(menuoptions[7],"City contents seed: %u",saveinfo.city_contents_seed);
  lastindex=8;
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
    edit_processinput(KEY_END);
    edit_processinput(KEY_UP);
    edit_processinput(KEY_UP);
    clear();
  }
  // a player, playerindex == highlight-8
  else if (highlight>=8) {
    player = &players[highlight-8];
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
      case 7:
        strcpy(msgstr,"City contents random seed");
        dst = &saveinfo.city_contents_seed;
        break;
    }
    sprintf(field,"%u",*dst);
    getinput();
    *dst = atouint16(field);
    if (dst == &saveinfo.party_money.pfennig) {
      const uint16_t pfennig = saveinfo.party_money.pfennig%12;
      const uint16_t groschen = saveinfo.party_money.pfennig/12;
      const uint16_t florin = groschen/20;
      saveinfo.party_money.pfennig = pfennig;
      saveinfo.party_money.groschen += groschen%20;
      saveinfo.party_money.florin += florin;
      saveinfo.party_money.florin += saveinfo.party_money.groschen/20;
      saveinfo.party_money.groschen %= 20;
    }
    else if (dst == &saveinfo.party_money.groschen) {
      const uint16_t groschen = saveinfo.party_money.groschen % 20;
      const uint16_t florin = saveinfo.party_money.groschen/20;
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
        highlight = 8+i;
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
      player->age = atouint16(field);
      setup_player();
      break;
    case 2:
      setup_attributes();
      break;
    case 3:
      setup_skills();
      break;
    case 4:
      state = ITEM;
      setup_thing();
      break;
    case 5:
      state = SAINT;
      setup_thing();
      break;
    case 6:
      state = FORMULA;
      setup_thing();
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
  *curr = atouint8(value);
  value = end+1;
  *max = atouint8(value);
  const int position = highlight;
  setup_attributes();
  scrollto(position);
}

static void setup_thing() {
  lastindex = 1;
  if (state&ITEM) {
    for (int i=0;i<player->num_items;++i) {
      sprintf(menuoptions[lastindex++], "%s quality(%u) quantity(%u)",
              items[player->items[i].code].name,
              player->items[i].quality, player->items[i].quantity);
    }
  }
  else if (state&SAINT) {
    for (int i=0;i<num_saints;++i) {
      const int byte = i>>3;
      const int shmt = 7-(i&7);
      if ((player->saints_known[byte]>>shmt)&1)
        sprintf(menuoptions[lastindex++], "%d: %s (%s)",
                i, saints[i].name, saints[i].short_name);
    }
  }
  else /*if (state&FORMULA)*/ {
    for (int i=0;i<num_formulas;++i) {
      const int byte = i/3;
      const int shmt = i%3;
      if ((player->formulas_known[byte]>>shmt)&1)
        sprintf(menuoptions[lastindex++], "%d: %s (%s) %s",
                i, formulas[i].name, formulas[i].short_name,
                (shmt==0)?"q25":(shmt==1)?"q35":"q45");
    }
  }
  char *type = (state&ITEM)?"items":(state&SAINT)?"saints":"formulas";
  sprintf(menuoptions[lastindex++], "Add %s", type);
  sprintf(menuoptions[lastindex++], "Finished with %s", type);
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  topy = 0;
  highlight = 1;
}

static void describe() {
  state |= DESC;
  strcpy(tmpstr,menuoptions[0]);
  strcpy(menuoptions[0],menuoptions[highlight]);
  char *start = menuoptions[highlight];
  char *end = strchr(start,':');
  *end = '\0';
  if (state&SAINT)
    start = saints[atoi(start)].description;
  else
    start = formulas[atoi(start)].description;
  lastindex = 0;
  *end = ':';
  end = strchr(start, ' ');
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
  menuoptions[lastindex+1][0] = '\0';
  menuwidth = 0;
  for (int i=0;i<=lastindex;++i) {
    if (strlen(menuoptions[i])>menuwidth) menuwidth = strlen(menuoptions[i]);
  }
  topy = 0;
  highlight = 0;
  clear();
}

static void thing_enter() {
  if (highlight==lastindex) {
    const int leavestate = state;
    setup_player();
    if (leavestate&ITEM) highlight = 4;
    else if (leavestate&SAINT) highlight=5;
    else /*if (leavestate&FORMULA)*/ highlight=6;
    return;
  }
  saveindex = highlight;
  if (highlight==lastindex-1) {
    if ((state&ITEM) && player->num_items==64)
      printerror(2,"Players are limited to 64 items",
                    "Unable to add any more items");
    else
      setup_addthing();
    return;
  }
  if (state&(SAINT|FORMULA)) describe();
  else {
    strcpy(msgstr,items[player->items[highlight-1].code].name);
    sprintf(field,"Quality:%u/Quantity:%u",
            player->items[highlight-1].quality,
            player->items[highlight-1].quantity);
    getinput();
    char *value = field;
    char *end = strchr(value,'.');
    *end='\0';
    player->items[highlight-1].quality = atouint8(value);
    value = end+1;
    player->items[highlight-1].quantity = atouint8(value);
    const int position = highlight;
    setup_thing();
    scrollto(position);
  }
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
  *skill = atouint8(field);
  const int position = highlight;
  setup_skills();
  scrollto(position);
}

int edit_processinput(const int ch) {
  if ((state&(SAINT|FORMULA)) && (state&DESC)) {
    switch(ch) {
      case KEY_RESIZE: clear(); break;
      case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
      case 4: case 'q': case 'Q': case 'c': case 'C':
      case KEY_ENTER: case '\n':
        state -= DESC;
        strcpy(menuoptions[0],tmpstr);
        if (state&ADD)
          setup_addthing();
        else
          setup_thing();
        scrollto(saveindex);
        break;
    }
    return EDITMENU;
  }
  if (highlight != lastindex && (ch=='a' || ch=='A')) {
    if ((state&(ITEM|SAINT|FORMULA)) && (state&ADD)) {
      addthing_confirm();
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
      if ((state&(ITEM|SAINT|FORMULA)) && (state&ADD)) {
        highlight=lastindex;
        addthing_enter();
        return EDITMENU;
      }
      switch(state) {
        case PARTY: return PREPMENU;
        case PLAYER:
          setup_edit();
          for (int i=0;i<partyinfo.num_curr_characters;++i) {
            if (player == &players[i]) {
              position = 8+i;
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
        default: break;
      }
      setup_player();
      scrollto(position);
      break;
    case KEY_ENTER: case '\n':
      if (state&(ITEM|SAINT|FORMULA)) {
        if (state&ADD) addthing_enter();
        else thing_enter();
      }
      else switch(state) {
        case PARTY:
          return edit_enter();
        case PLAYER:
          player_enter(); break;
        case ATTR:
          attributes_enter(); break;
        case SKILL:
          skills_enter(); break;
        default: break;
      }
      break;
    default: break;
  }
  return EDITMENU;
}
