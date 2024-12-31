#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "structs.h"

typedef struct item_definition item_definition;
typedef struct formula_definition formula_definition;
typedef struct formula formula;
typedef struct saint saint;

FILE *lstfile, *sntfile, *alcfile;
uint8_t num_items, num_saints, num_formulas;

item_definition *items;
saint *saints;
formula *formulas;
void parse_items() {
  uint8_t num_item_slots;
  fread(&num_item_slots, sizeof(uint8_t), 1, lstfile);
  fread(&num_saints, sizeof(uint8_t), 1, lstfile);
  fread(&num_formulas, sizeof(uint8_t), 1, lstfile);
  for (int i=0;i<num_item_slots;++i) {
    item_definition item;
    fread(&item, sizeof(item_definition), 1, lstfile);
    if (item.name[0] == '\0') {
      num_items = i;
      break;
    }
  }
  fseek(lstfile, 3, SEEK_SET);
  items = calloc(num_items, sizeof(item_definition));
  for (int i=0;i<num_items;++i) {
    fread(&items[i], sizeof(item_definition), 1, lstfile);
  }
  fseek(lstfile,sizeof(item_definition)*(num_item_slots-num_items),SEEK_CUR);
}

void parse_saints() {
  uint8_t checknum;
  fread(&checknum,sizeof(uint8_t),1,sntfile);
  if (checknum != num_saints) {
    printf("saint check!=num: %u != %u\n",checknum,num_saints);
    return;
  }
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
  fread(&checknum,sizeof(uint8_t),1,alcfile);
  if (checknum != num_formulas) {
    printf("formula check!=num: %u != %u\n",checknum,num_formulas);
    return;
  }
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

int print_usage(char *prog) {
    fprintf(stderr,"This utility is used to create darklands.data\n");
    fprintf(stderr,"Usage:\n");
    fprintf(stderr,"%s /path/to/darklands\n", prog);
    fprintf(stderr,"Expect files: DARKLAND.LST DARKLAND.SNT DARKLAND.ALC\n");
  return EXIT_FAILURE;
}

int main(int argc,char **argv) {
  if (argc != 2)
    return print_usage(argv[0]);
  char filename[256];
  snprintf(filename,256,"%s/DARKLAND.LST",argv[1]);
  lstfile = fopen(filename, "rb");
  if (!lstfile) return print_usage(argv[0]);
  snprintf(filename,256,"%s/DARKLAND.SNT",argv[1]);
  sntfile = fopen(filename, "rb");
  if (!sntfile) return print_usage(argv[0]);
  snprintf(filename,256,"%s/DARKLAND.ALC",argv[1]);
  alcfile = fopen(filename, "rb");
  if (!alcfile) return print_usage(argv[0]);
  parse_items();
  parse_saints();
  parse_formulas();
  fclose(lstfile);
  fclose(sntfile);
  fclose(alcfile);
  FILE *outfile = fopen("darklands.data","wb");
  fwrite(&num_items,sizeof(uint8_t),1,outfile);
  fwrite(items,sizeof(item_definition),num_items,outfile);
  fwrite(&num_saints,sizeof(uint8_t),1,outfile);
  fwrite(saints,sizeof(saint),num_saints,outfile);
  fwrite(&num_formulas,sizeof(uint8_t),1,outfile);
  fwrite(formulas,sizeof(formula),num_formulas,outfile);
  fclose(outfile);
  free(items);
  free(saints);
  free(formulas);
  return 0;
}
