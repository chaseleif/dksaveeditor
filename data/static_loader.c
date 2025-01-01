#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

unsigned char darklands_data[];

extern uint8_t num_items, num_saints, num_formulas;
extern struct item_definition *items;
extern struct saint *saints;
extern struct formula *formulas;

void load_static_darklands_data() {
  int readi = 0;
  num_items = darklands_data[readi++];
  items = malloc(sizeof(struct item_definition)*num_items);
  memcpy(items,darklands_data+readi,sizeof(struct item_definition)*num_items);
  readi += sizeof(struct item_definition)*num_items;
  num_saints = darklands_data[readi++];
  saints = malloc(sizeof(struct saint)*num_saints);
  memcpy(saints,darklands_data+readi,sizeof(struct saint)*num_saints);
  readi += sizeof(struct saint)*num_saints;
  num_formulas = darklands_data[readi++];
  formulas = malloc(sizeof(struct formula)*num_formulas);
  memcpy(formulas,darklands_data+readi,sizeof(struct formula)*num_formulas);
}

