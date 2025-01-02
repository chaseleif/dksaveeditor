#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct rgb {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct person_colors {
  struct rgb first_hi;
  struct rgb first_lo;
  struct rgb second_hi;
  struct rgb second_med;
  struct rgb second_lo;
  struct rgb third_hi;
  struct rgb third_med;
  struct rgb third_lo;
};

int print_usage(char *prog) {
  printf("This utility will fix bad save games that have corrupted colors\n");
  printf("Usage:\n");
  printf("%s DKSAVE1.SAV\n",prog);
  printf("The save file will be corrected in place, create a backup\n");
  return EXIT_FAILURE;
}

int main(int argc,char **argv) {
  if (argc!=2) {
    return print_usage(argv[0]);
  }
  FILE *savefile = fopen(argv[1], "rb+");
  if (!savefile) {
    printf("Unable to open save file \"%s\"\n\n",argv[1]);
    return print_usage(argv[0]);
  }
  // 275 is 2 bytes/16 bits past where the colors are supposed to be
  fseek(savefile, 275, SEEK_SET);
  struct person_colors party_colors[5];
  fread(party_colors, sizeof(struct person_colors), 5, savefile);
  // this is where colors should be
  fseek(savefile, 273, SEEK_SET);
  fwrite(party_colors, sizeof(struct person_colors), 5, savefile);
  fclose(savefile);
  return EXIT_SUCCESS;
}
