#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "structs.h"
#include "shared.h"

#define ERROR(file, ptr, msg)         \
  do {                                \
    if (file)                         \
      fclose(file);                   \
    if (ptr) {                        \
      free(ptr);                      \
      ptr = NULL;                     \
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

extern char *msgstr;
extern item_definition *items;

static void *null = NULL;

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
  FILE *infile = fopen(filename, "rb");
  if (!infile)
    ERROR(infile, null, "Unable to open save file");
  if (fread(saveinfo, sizeof(savefileheader), 1, infile) != 1)
    ERROR(infile, null, "Failed to read save header of save file");
  if (fseek(infile, 239, SEEK_SET))
    ERROR(infile, null, "Failed to seek in save file");
  if (fread(partyinfo, sizeof(partyheader), 1, infile) != 1)
    ERROR(infile, null, "Failed to read party header in save file");
  *players = malloc(sizeof(character)*partyinfo->num_curr_characters);
  for (int chari=0;chari<partyinfo->num_curr_characters;++chari) {
    if (fseek(infile, 393+partyinfo->party_char_indices[chari]*554, SEEK_SET))
      ERROR(infile, *players, "Failed to seek in save file");
    if (fread(&(*players)[chari], sizeof(character), 1, infile) != 1)
      ERROR(infile, *players, "Failed to read character in save file");
  }
  fclose(infile);
  return;
}

void savesave(char *filename, character *players,
              savefileheader *saveinfo, partyheader *partyinfo) {
  FILE *outfile = fopen(filename, "rb+");
  if (!outfile)
    ERROR(outfile, null, "Unable to open save file");
  if (fwrite(saveinfo, sizeof(savefileheader), 1, outfile) != 1)
    ERROR(outfile, null, "Failed to write in save file");
  for (int chari=0;chari<partyinfo->num_curr_characters;++chari) {
    if (fseek(outfile, 393+partyinfo->party_char_indices[chari]*554, SEEK_SET))
      ERROR(outfile, null, "Failed to seek in save file");
    if (fwrite(&players[chari], sizeof(character), 1, outfile) != 1)
      ERROR(outfile, null, "Failed to write in save file");
  }
  fclose(outfile);
  return;
}

void load_lst(char *filename) {
  FILE *infile = fopen(filename, "rb");
  if (!infile)
    ERROR(infile, null, "Unable to open LST file");
  uint8_t num_items=0, num_item_slots, num_saints, num_formulas;
  if (fread(&num_item_slots, sizeof(uint8_t), 1, infile) != 1)
    ERROR(infile, null, "Failed to read item slots in LST file");
  if (fread(&num_saints, sizeof(uint8_t), 1, infile) != 1)
    ERROR(infile, null, "Failed to read num saints in LST file");
  if (fread(&num_formulas, sizeof(uint8_t), 1, infile) != 1)
    ERROR(infile, null, "Failed to read num formulas in LST file");
  item_definition item;
  for (int i=0;i<num_item_slots;++i) {
    if (fread(&item, sizeof(item), 1, infile) != 1)
      ERROR(infile, null, "Failed to read item in LST file");
    if (item.name[0] == '\0') {
      num_items = i;
      if (fseek(infile, 3, SEEK_SET))
        ERROR(infile, null, "Failed to reset file position in LST file");
      break;
    }
  }
  if (num_items++ < 172)
    ERROR(infile, null, "Expected at least 172 items in LST file");
  items = malloc(sizeof(item_definition)*(num_items));
  for (int i=0;i<num_items;++i) {
    if (fread(&items[i],sizeof(item_definition),1,infile)!=1)
      ERROR(infile, items, "Failed to read items in LST file");
  }
  fclose(infile);
}
