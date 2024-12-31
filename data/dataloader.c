#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "structs.h"
#include "shared.h"

typedef struct item_definition item_definition;
typedef struct formula_definition formula_definition;
typedef struct formula formula;
typedef struct saint saint;

uint8_t num_items, num_saints, num_formulas;

char *dkdir, *msgstr;

item_definition *items;
saint *saints;
formula *formulas;

static int error=0;
void printerror(const int n, ...) {
  error = 1;
  va_list args;
  va_start(args, n);
  for (int i=0;i<n;++i) {
    printf("%s\n",va_arg(args,char*));
  }
  va_end(args);
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
  dkdir = malloc(sizeof(char)*MAXSTRLEN);
  msgstr = malloc(sizeof(char)*MAXSTRLEN);
  strcpy(dkdir,argv[1]);
  load_darklands_data();
  free(dkdir);
  free(msgstr);
  if (error) return 1;
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
