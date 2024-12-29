#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "shared.h"

extern char **menuoptions, **item_names;
extern char *msgstr, *dksavefile, *field;
extern int topy, nrows, menuwidth, highlight;

extern struct character *players;
extern struct savefileheader saveinfo;
extern struct partyheader partyinfo;

static const char const attributename[14][8] = {
"End", "Str", "Agl", "Per", "Int", "Chr", " DF",
"Max End", "Max Str", "Max Agl", "Max Per", "Max Int", "Max Chr", "Max DR " };
static const char const skillname[19][5] = {
"Edge", "Imp", "Fll", "Pol", "Thr", "Bow", "Msl", "Alch", "Relg", "Virt",
"SpkC", "SpkL", "R&W", "Heal", "Artf", "Stlh", "StrW", "Ride", "WdWs" };
enum { PARTY, PLAYER, ATTR, SKILL, ITEM };

static struct character *player = NULL;
static int state = PARTY, lastindex = 0;

static void setup_player();
static void player_enter();
static void setup_attributes();
static void attributes_enter();
static void setup_skills();
static void skill_enter();
static void setup_items();
static void items_enter();

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
  highlight = 1;
  state = PARTY;
}

static int party_enter() {
  // the final options after player(s)
  if (highlight==lastindex) return PREPMENU;
  else if (highlight==lastindex-1)
    savesave(dksavefile, players, &saveinfo, &partyinfo);
  else if (highlight==lastindex-2) {
    loadsave(dksavefile, &players, &saveinfo, &partyinfo);
    setup_edit();
    highlight = lastindex-2;
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
    const int position = highlight;
    setup_edit();
    highlight = position;
    clear();
  }
  return EDITMENU;
}

static void setup_player() {
  sprintf(menuoptions[0],"%s (%s)",player->name,player->short_name);
  sprintf(menuoptions[1],"Age: %u",player->age);
  strcpy(menuoptions[2],"Player Attributes");
  strcpy(menuoptions[3],"Player Skills");
  strcpy(menuoptions[4],"Player Items");
  strcpy(menuoptions[5],"Return to party");
  menuoptions[6][0] = '\0';
  clear();
  state = PLAYER;
  highlight = 1;
  lastindex = 5;
}

static void player_enter() {
  switch(highlight) {
    case 1:
      strcpy(msgstr,"Age");
      sprintf(field,"%u",player->age);
      getinput();
      player->age = (uint16_t)atoi(field);
      setup_player();
      highlight = 1;
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
      setup_edit();
      for (int i=0;i<partyinfo.num_curr_characters;++i) {
        if (player == &players[i]) {
          highlight = 7+i;
          break;
        }
      }
      clear();
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
  highlight = position;
}

static void setup_items() {
  lastindex = 1;
  for (int i=0;i<player->num_items;++i) {
    sprintf(menuoptions[lastindex++], "%s quality(%u) quantity(%u)",
            item_names[player->items[i].code],
            player->items[i].quality, player->items[i].quantity);
  }
  strcpy(menuoptions[lastindex++], "Finished with items");
  menuoptions[lastindex--][0] = '\0';
  menuwidth=0;
  for (int i=0;i<=lastindex;++i) {
    if (menuwidth<strlen(menuoptions[i])) menuwidth=strlen(menuoptions[i]);
  }
  clear();
  highlight = 1;
  state = ITEM;
}

static void items_enter() {
  if (highlight==lastindex) {
    setup_player();
    highlight=4;
    topy=0;
    return;
  }
  strcpy(msgstr,item_names[player->items[highlight-1].code]);
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
  highlight = position;
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
  highlight = position;
}

int edit_processinput(const int ch) {
  switch(ch) {
    case KEY_RESIZE: clear(); break;
    case KEY_HOME:
      highlight=1;
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
      if (highlight==1) {
        highlight=lastindex;
        if (lastindex > LINES-MENUFIRSTLINE-2) {
          topy = lastindex-LINES+MENUFIRSTLINE+2;
          clear();
        }
      }
      else if (--highlight < topy) { --topy; clear(); }
      break;
    case KEY_DOWN:
      if (highlight==lastindex) { highlight=1; topy=0; clear(); }
      else if (++highlight-topy > LINES-MENUFIRSTLINE-2) {
        topy = highlight-LINES+MENUFIRSTLINE+2;
        clear();
      }
      break;
    case KEY_RESET: case KEY_BREAK: case KEY_CANCEL: case KEY_EXIT: case 27:
    case 4: case 'q': case 'Q': case 'c': case 'C':
      int position = 1;
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
      }
      setup_player();
      highlight = position;
      break;
    case KEY_ENTER: case '\n':
      switch(state) {
        case PARTY:
          return party_enter();
        case PLAYER:
          player_enter(); break;
        case ATTR:
          attributes_enter(); break;
        case SKILL:
          skills_enter(); break;
        case ITEM:
          items_enter(); break;
      }
      break;
    default: break;
  }
  return EDITMENU;
}
