#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

struct money {
  uint16_t florin;
  uint16_t groschen;
  uint16_t pfennig;
};

struct item {
  uint16_t code;
  uint8_t type;
  uint8_t quality;
  uint8_t quantity;
  uint8_t weight;
};

struct attribute_set {
  uint8_t endurance;
  uint8_t strength;
  uint8_t agility;
  uint8_t perception;
  uint8_t intelligence;
  uint8_t charisma;
  uint8_t divinefavor;
};

struct skill_set {
  uint8_t wEdge;
  uint8_t wImp;
  uint8_t wFll;
  uint8_t wPol;
  uint8_t wThr;
  uint8_t wBow;
  uint8_t wMsl;
  uint8_t alch;
  uint8_t relg;
  uint8_t virt;
  uint8_t spkC;
  uint8_t spkL;
  uint8_t r_w;
  uint8_t heal;
  uint8_t artf;
  uint8_t stlh;
  uint8_t strw;
  uint8_t ride;
  uint8_t wdws;
};

struct character {
  uint8_t unkn1[17];
  uint16_t age;
  uint8_t unkn2;
  char shield;
  uint8_t unkn3[12];
  uint16_t equip_missile_type;
  // fire_resistance starts at zero
  // take firewall ~ q25: +50, q35: +70, q45: +90
  // st. polycarp  ~ +90
  uint8_t fire_resistance;
  char name[25];
  char short_name[10];
  // weapon_buff starts at zero
  // strongedge  ~ q25: +3, q35: +4, q45: +5
  // greatpower  ~ q25: +14, q35: +22, q45: +30
  // deadlyblade ~ q25: +48, q34: +64, q45: +80
  uint8_t weapon_buff;
  uint8_t equipped_weight;
  // (unkn4 = 0xff since beginning)
  uint8_t unkn4;
  uint8_t equip_vital_type;
  uint8_t equip_leg_type;
  uint8_t unkn5[2];
  uint8_t equip_vital_q;
  uint8_t equip_leg_q;
  uint8_t equip_weapon_type;
  uint8_t unkn6[3];
  // armor_buff starts at zero
  // take hardarmor ~ q25: +1, q35: +2, q45: +3
  // take firewall  ~ q25/35/45: +16
  // st. polycarp   ~ +176
  uint8_t armor_buff;
  uint8_t unkn7[2];
  uint8_t equip_weapon_q;
  uint8_t unkn8;
  uint8_t equip_missile_q;
  uint8_t equip_shield_q;
  uint8_t equip_shield_type;
  struct attribute_set curr_attrs;
  struct attribute_set max_attrs;
  struct skill_set skills;
  uint16_t num_items;
  uint8_t saints_known[20];
  uint8_t formulas_known[22];
  struct item items[64];
};

// wallace.net notes indicate colors range from 0x00 to 0x3f
// this is 6-bit VGA
// https://moddingwiki.shikadi.net/wiki/VGA_Palette
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

// Beginning of save file
struct savefileheader {
  char curr_location_name[12];
  uint8_t unkn1[9];
  char save_game_label[23];
  uint16_t unkn2[28];
  uint16_t city_contents_seed;
  uint16_t unkn3;
  uint16_t year;
  uint16_t month;
  uint16_t day;
  uint16_t hour;
  struct money party_money;
  uint16_t unkn4[2];
  uint16_t reputation;
  uint16_t curr_location;
  uint16_t x_coord;
  uint16_t y_coord;
  uint16_t curr_menu;
  uint16_t unkn5[3];
  uint16_t prev_menu;
  uint16_t bank_notes;
  uint16_t unkn6[2];
  uint16_t philosopher_stone;
  uint8_t unkn7[7];
  uint8_t party_order_indices[5];
  uint8_t unkn8;
  uint8_t party_leader_index;
  uint8_t unkn9;
  uint16_t unkn10[38];
};

struct partyheader {
  uint16_t num_curr_characters;
  uint16_t num_characters;
  uint16_t party_char_indices[5];
  char party_images[5][4];
  struct person_colors party_colors[5];
};

struct equipment {
  uint8_t description[20];
  int quality;
  int quantity;
  int weight;
  int number;
  int byte;
};

struct item_definition {
  char name[20];
  char short_name[10];
  uint16_t type;
  uint8_t bitmask[5];
  /*
  index 0
    1(edged) 2(impact) 3(polearm) 4(flail)
    5(thrown) 6(bow) 7(metal_armor) 8(shield)
  index 1
    1 and 2 (pawn items?) 3(alch component) 4(potion)
    5(relic) 6(horse) 7(quest items?) 8(zero)
  index 2
    1(lockpick) 2(giveslight) 3(arrow) 4(zero)
    5(quarrel) 6(ball) 7(zero) 8(outdoor quest items?)
  index 3
    1(throwpotion) 2(zero) 3(nonmetal armor) 4(missile weapon)
    5(zero) 6(chest items from mines?) 7(instrument) 8(zero)
  index 4
    1 and 2 (set when 8 not set except cloth armor, superb horse, fast horse)
    3-7 zero
    8(cloth armor, quest items, relics, creature parts ...)
    8 always set if any of these set: 1/5 (is_relic), 1/7, 2/8
  */
  uint8_t weight;
  uint8_t quality;
  // non-zero only for relics
  // from St. Edward's Ring 0x6 to St. Gabriel's Horn 0x50
  uint16_t relicvalue;
  // non-zero for relics and unused residency permit
  // residency permit ranges from 0x5 to 0x27
  uint16_t relicorpermitvalue;
  uint16_t value;
};

struct ingredient {
  uint16_t quantity_needed;
  uint16_t item_code; // references offsets of item definitions in .LST file
};

struct formula_definition {
  char description[80];
  uint16_t mystic_number;
  uint16_t risk_factor;
  struct ingredient ingredients[5];
};

struct formula {
  char name[26];
  char short_name[12];
  char description[80];
  uint16_t mystic_number;
  uint16_t risk_factor;
  struct ingredient ingredients[5];
};

struct saint {
  char name[24];
  char short_name[13];
  char description[349];
};
#endif //STRUCTS_H
