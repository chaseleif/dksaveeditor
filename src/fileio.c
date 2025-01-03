#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "structs.h"
#include "shared.h"

#define ERROR(ptr, msg)               \
  do {                                \
    if (savefile) {                   \
      fclose(savefile);               \
      savefile=NULL;                  \
    }                                 \
    if (lstfile) {                    \
      fclose(lstfile);                \
      lstfile=NULL;                   \
    }                                 \
    if (sntfile) {                    \
      fclose(sntfile);                \
      sntfile=NULL;                   \
    }                                 \
    if (alcfile) {                    \
      fclose(alcfile);                \
      alcfile=NULL;                   \
    }                                 \
    if (ptr) {                        \
      free(ptr);                      \
      ptr = NULL;                     \
    }                                 \
    if (items) {                      \
      free(items);                    \
      items = NULL;                   \
    }                                 \
    if (saints) {                     \
      free(saints);                   \
      saints = NULL;                  \
    }                                 \
    if (formulas) {                   \
      free(formulas);                 \
      formulas = NULL;                \
    }                                 \
    sprintf(msgstr,"ERROR %s:%d %s",  \
            __FILE__, __LINE__, msg); \
    printerror(1, msgstr);            \
    return;                           \
  }while(0)

typedef struct savefileheader savefileheader;
typedef struct partyheader partyheader;
typedef struct character character;
typedef struct item_definition item_definition;
typedef struct formula_definition formula_definition;
typedef struct formula formula;
typedef struct saint saint;

extern item_definition *items;
extern saint *saints;
extern formula *formulas;

extern char *msgstr, *dkdir;
extern uint8_t num_items, num_saints, num_formulas;

static FILE *lstfile=NULL, *sntfile=NULL, *alcfile=NULL, *savefile=NULL;

static void *nullptr = NULL;

int isdirectory(char *name) {
  struct stat path;
  if (stat(name, &path)<0) return 0;
  return S_ISDIR(path.st_mode);
}

int isfile(char *name) {
  struct stat path;
  if (stat(name, &path)<0) return 0;
  return S_ISREG(path.st_mode);
}

void loadsave(char *filename, character **players,
              savefileheader *saveinfo, partyheader *partyinfo) {
  if (*players) { free(*players); *players=NULL; }
  FILE *savefile = fopen(filename, "rb");
  if (!savefile)
    ERROR(nullptr, "Unable to open save file");
  if (fread(saveinfo, sizeof(savefileheader), 1, savefile) != 1)
    ERROR(nullptr, "Failed to read save header of save file");
  if (fseek(savefile, 239, SEEK_SET))
    ERROR(nullptr, "Failed to seek in save file");
  if (fread(partyinfo, sizeof(partyheader), 1, savefile) != 1)
    ERROR(nullptr, "Failed to read party header in save file");
  *players = malloc(sizeof(character)*partyinfo->num_curr_characters);
  for (int chari=0;chari<partyinfo->num_curr_characters;++chari) {
    if (fseek(savefile, 393+partyinfo->party_char_indices[chari]*554, SEEK_SET))
      ERROR(*players, "Failed to seek in save file");
    if (fread(&(*players)[chari], sizeof(character), 1, savefile) != 1)
      ERROR(*players, "Failed to read character in save file");
  }
  fclose(savefile);
  return;
}

void savesave(char *filename, character *players,
              savefileheader *saveinfo, partyheader *partyinfo) {
  FILE *savefile = fopen(filename, "rb+");
  if (!savefile)
    ERROR(nullptr, "Unable to open save file");
  if (fwrite(saveinfo, sizeof(savefileheader), 1, savefile) != 1)
    ERROR(nullptr, "Failed to write in save file");
  for (int chari=0;chari<partyinfo->num_curr_characters;++chari) {
    if (fseek(savefile, 393+partyinfo->party_char_indices[chari]*554, SEEK_SET))
      ERROR(nullptr, "Failed to seek in save file");
    if (fwrite(&players[chari], sizeof(character), 1, savefile) != 1)
      ERROR(nullptr, "Failed to write in save file");
  }
  fclose(savefile);
  return;
}

void parse_items() {
  uint8_t num_item_slots;
  if (fread(&num_item_slots, sizeof(uint8_t), 1, lstfile) !=1)
    ERROR(nullptr, "Failed to read number of items from LST File");
  if (fread(&num_saints, sizeof(uint8_t), 1, lstfile) != 1)
    ERROR(nullptr, "Failed to read number of saints from LST file");
  if (fread(&num_formulas, sizeof(uint8_t), 1, lstfile) != 1)
    ERROR(nullptr, "Failed to read number of formulas from LST file");
  for (int i=0;i<num_item_slots;++i) {
    item_definition item;
    if (fread(&item, sizeof(item_definition), 1, lstfile) != 1)
      ERROR(nullptr, "Failed to read item definition");
    if (item.name[0] == '\0') {
      num_items = i;
      break;
    }
  }
  if (fseek(lstfile, 3, SEEK_SET))
    ERROR(nullptr, "Failed to seek in LST file");
  items = calloc(num_items, sizeof(item_definition));
  for (int i=0;i<num_items;++i) {
    if (fread(&items[i], sizeof(item_definition), 1, lstfile) != 1)
      ERROR(nullptr, "Failed to read item definition");
  }
  if (fseek(lstfile,sizeof(item_definition)*(num_item_slots-num_items),SEEK_CUR))
    ERROR(nullptr, "Failed to seek in LST file");
}

void parse_saints() {
  uint8_t checknum;
  if (fread(&checknum,sizeof(uint8_t),1,sntfile) != 1)
    ERROR(nullptr, "Failed to read number of saints from SNT file");
  if (checknum != num_saints)
    ERROR(nullptr, "Number of saints in SNT file doesn\'t match LST file");
  saints = calloc(num_saints, sizeof(saint));
  for (int i=0;i<num_saints;++i) {
    for (int x=0; ;++x) {
      if ((saints[i].name[x] = fgetc(lstfile))=='\0') break;
    }
    for (int x=0; ;++x) {
      if ((saints[i].description[x] = fgetc(sntfile))=='\0') {
        if (!x) --x;
        else break;
      }
    }
  }
  for (int i=0;i<num_saints;++i) {
    for (int x=0; ;++x) {
      if ((saints[i].short_name[x] = fgetc(lstfile))=='\0') break;
    }
  }
}

void parse_formulas() {
  uint8_t checknum;
  if (fread(&checknum,sizeof(uint8_t),1,alcfile) != 1)
    ERROR(nullptr, "Failed to read number of formulas from ALC file");
  if (checknum != num_formulas)
    ERROR(nullptr, "Number of formulas in ALC file doesn\'t match LST file");
  formulas = calloc(num_formulas, sizeof(formula));
  formula_definition definition;
  for (int i=0;i<num_formulas;++i) {
    fread(&definition, sizeof(formula_definition), 1, alcfile);
    strcpy(formulas[i].description, definition.description);
    formulas[i].mystic_number = definition.mystic_number;
    formulas[i].risk_factor = definition.risk_factor;
    for (int x=0; ;++x) {
      if ((formulas[i].name[x] = fgetc(lstfile))=='\0') break;
    }
    for (int x=0;x<5;++x) {
      formulas[i].ingredients[x].quantity_needed = \
              definition.ingredients[x].quantity_needed;
      formulas[i].ingredients[x].item_code = \
              definition.ingredients[x].item_code;
    }
  }
  for (int i=0;i<num_formulas;++i) {
    for (int x=0; ;++x) {
      if ((formulas[i].short_name[x] = fgetc(lstfile))=='\0') break;
    }
  }
}

void load_darklands_data() {
  struct dirent *de;
  DIR *dr = opendir(dkdir);
  while (!lstfile || !sntfile || !alcfile) {
    if (!(de=readdir(dr))) break;
    char filename[MAXSTRLEN];
    if (!lstfile && !istrcmp(de->d_name,"DARKLAND.LST")) {
      snprintf(filename,MAXSTRLEN,"%s/%s", dkdir, de->d_name);
      lstfile = fopen(filename, "rb");
    }
    else if (!sntfile && !istrcmp(de->d_name,"DARKLAND.SNT")) {
      snprintf(filename,MAXSTRLEN,"%s/%s", dkdir, de->d_name);
      sntfile = fopen(filename, "rb");
    }
    else if (!alcfile && !istrcmp(de->d_name,"DARKLAND.ALC")) {
      snprintf(filename,MAXSTRLEN,"%s/%s", dkdir, de->d_name);
      alcfile = fopen(filename, "rb");
    }
  }
  if (!lstfile || !sntfile || !alcfile)
    ERROR(nullptr, "Unable to open Darkland data files");
  parse_items();
  parse_saints();
  parse_formulas();
  fclose(lstfile);
  lstfile = NULL;
  fclose(sntfile);
  sntfile = NULL;
  fclose(alcfile);
  alcfile = NULL;
}
