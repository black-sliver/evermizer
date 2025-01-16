#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


// NOTE: this file defines all used non-patch data
//       and defines some helper functions at the end


#if (!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)) && !defined(_Static_assert)
#   error "Please define _Static_assert if not builtin before including this"
#endif
#if !defined(ARRAY_SIZE)
#   error "Please define ARRAY_SIZE before including this"
#endif


// Data
#define LOC_END (size_t)-1
// yes, c++ would make this way nicer...
const static size_t acidrain_locations[] = {
    0x13af2a +2, 0x13af5a +2, 0x13ae83 +1, LOC_END };
const static size_t atlas_locations[] = {
    0x15e97f +2, 0x15e749 +14, 0x15e773 +2, 0x15e7fe +2, 0x15e805 +1, LOC_END };
const static size_t barrier_locations[] = {
    0x15d3d7 +1, 0x15d3cd +2, LOC_END };
const static size_t callup_locations[] = {
    0x1bcec8 +1, LOC_END };
const static size_t corrosion_locations[] = {
    0x18b2f7 +2, 0x18b301 +1, LOC_END };
const static size_t crush_locations[] = {
    0x15ba42 +1, LOC_END };
const static size_t cure_locations[] = {
    0x1683b2 +2, 0x16843d +1, 0x14e756 +1, 0x14c726 +2, 0x14c81c +2, 0x168401 +2, LOC_END };
const static size_t defend_locations[] = {
    0x14e506 +1, LOC_END };
const static size_t doubledrain_locations[] = {
    0x14a682 +6, 0x14a68e +1, LOC_END };
const static size_t drain_locations[] = {
    0x14a692 +2, 0x14a6c3 +2, 0x14a6ca +1, LOC_END };
const static size_t energize_locations[] = {
    0x1bcf34 +6, 0x1bcf40 +1, LOC_END };
const static size_t escape_locations[] = {
    0x16db70 +11, 0x16db86 +1, /*with escape_after_des*/0x16db5b +6, LOC_END };
const static size_t explosion_locations[] = {
    0x198793 +2, 0x1987af +2, 0x1987c8 +2, 0x1987cf +1, 0x198d2d +2, LOC_END };
const static size_t fireball_locations[] = {
    0x17a2d0 +2, 0x17a027 +6, 0x17a03a +1, 0x17a012 +2, 0x17a002 +2, 0x16dbc6 +2, LOC_END };
const static size_t firepower_locations[] = {
    0x17e9ed +2, 0x17ea2a +1, LOC_END };
const static size_t flash_locations[] = {
    0x14da42 +1, 0x14d49b +2, 0x14d4ab +2, 0x14d4c0 +2, LOC_END };
const static size_t forcefield_locations[] = {
    0x19da23 +1, LOC_END };
const static size_t hardball_locations[] = {
    0x14a9a1 +9, 0x14a3eb +1, 0x14a425 +2, 0x14a89c +2, LOC_END };
const static size_t heal_locations[] = {
    0x13dacd +1, LOC_END };
const static size_t lance_locations[] = {
    0x18e164 +2, 0x18dd9a +9, 0x18d26e +1, LOC_END };
const static size_t levitate_locations[] = {
    0x13ed93 +2, 0x148099 +1, LOC_END };
const static size_t lightningstorm_locations[] = {
    0x1996f0 +1, LOC_END };
const static size_t miraclecure_locations[] = {
    0x14e70a +2, 0x14e711 +1, 0x14c70e +2, LOC_END };
const static size_t nitro_locations[] = {
    0x198793 +11, 0x1987f1 +1, 0x198d22 +2, LOC_END };
const static size_t oneup_locations[] = {
    0x19b750 +2, 0x19b75b +1, LOC_END };
const static size_t reflect_locations[] = {
    0x1be026 +2, 0x1be030 +1, LOC_END };
const static size_t regrowth_locations[] = {
    0x18de1b +2, 0x18de25 +1, LOC_END };
const static size_t revealer_locations[] = {
    0x16db3b +2, 0x16db41 +1, 0x17a027 +2, LOC_END };
const static size_t revive_locations[] = {
    0x15b216 +2, 0x15b238 +1, LOC_END };
const static size_t slowburn_locations[] = {
    0x19881d +2, 0x198843 +1, LOC_END };
const static size_t speed_locations[] = {
    0x149c9c +2, 0x149cc1 +1, LOC_END };
const static size_t sting_locations[] = {
    0x16e141 +2, 0x16e155 +1, LOC_END };
const static size_t stop_locations[] = {
    0x19da2d +1, LOC_END };
const static size_t superheal_locations[] = {
    0x1abb91 +2, 0x1abc18 +1, LOC_END };

struct alchemy_data {
    const char* name;
    uint16_t id;
    const size_t* locations;
    const size_t preselects[2];
};

// NOTE: ingredient data is at 0x4601F + id*4 with ingredient ID1, 2, amount 1, 2
const static struct alchemy_data alchemy_locations[] = {
    {"Acid Rain",       0x00, acidrain_locations,       {0xb08000 +4 -0x800000,LOC_END}},
    {"Atlas",           0x01, atlas_locations,          {0xb08006 +4 -0x800000,LOC_END}},
    {"Barrier",         0x02, barrier_locations,        {0xb0800c +4 -0x800000,LOC_END}},
    {"Call Up",         0x03, callup_locations,         {0xb08012 +4 -0x800000,LOC_END}},
    {"Corrosion",       0x04, corrosion_locations,      {0xb08018 +4 -0x800000,LOC_END}},
    {"Crush",           0x05, crush_locations,          {0xb0801e +4 -0x800000,LOC_END}},
    {"Cure",            0x06, cure_locations,           {0xb08024 +4 -0x800000,LOC_END}},
    {"Defend",          0x07, defend_locations,         {0xb0802a +4 -0x800000,LOC_END}},
    {"Double Drain",    0x08, doubledrain_locations,    {0xb08030 +4 -0x800000,LOC_END}},
    {"Drain",           0x09, drain_locations,          {0xb08036 +4 -0x800000,LOC_END}},
    {"Energize",        0x0a, energize_locations,       {0xb0803c +4 -0x800000,LOC_END}},
    {"Escape",          0x0b, escape_locations,         {0xb08042 +4 -0x800000,LOC_END}},
    {"Explosion",       0x0c, explosion_locations,      {0xb08048 +4 -0x800000,LOC_END}},
    {"Fireball",        0x0d, fireball_locations,       {0xb0804e +4 -0x800000,LOC_END}},
    {"Fire Power",      0x0e, firepower_locations,      {0xb08054 +4 -0x800000,LOC_END}},
    {"Flash",           0x0f, flash_locations,          {0xb0805a +4 -0x800000,LOC_END}},
    {"Force Field",     0x10, forcefield_locations,     {0x99da1e +4 -0x800000,LOC_END}},
    {"Hard Ball",       0x11, hardball_locations,       {0x94a3ef +4 -0x800000,LOC_END}},
    {"Heal",            0x12, heal_locations,           {0x93dbec +4 -0x800000,0x93dc8d +4 -0x800000}},
    {"Lance",           0x13, lance_locations,          {0x98d354 +4 -0x800000,LOC_END}},
    // no laser
    {"Levitate",        0x15, levitate_locations,       {0x94809d +4 -0x800000,LOC_END}},
    {"Lightning Storm", 0x16, lightningstorm_locations, {0x9996f4 +4 -0x800000,LOC_END}},
    {"Miracle Cure",    0x17, miraclecure_locations,    {0x94e71b +4 -0x800000,LOC_END}},
    {"Nitro",           0x18, nitro_locations,          {0x9987f5 +4 -0x800000,LOC_END}},
    {"One Up",          0x19, oneup_locations,          {0x99b75f +4 -0x800000,LOC_END}},
    {"Reflect",         0x1a, reflect_locations,        {0x9be034 +4 -0x800000,LOC_END}},
    {"Regrowth",        0x1b, regrowth_locations,       {0x98de29 +4 -0x800000,LOC_END}},
    {"Revealer",        0x1c, revealer_locations,       {0x96db4b +4 -0x800000,LOC_END}},
    {"Revive",          0x1d, revive_locations,         {0x95b242 +4 -0x800000,LOC_END}},
    {"Slow Burn",       0x1e, slowburn_locations,       {0x998847 +4 -0x800000,LOC_END}},
    {"Speed",           0x1f, speed_locations,          {0x949cc5 +4 -0x800000,LOC_END}},
    {"Sting",           0x20, sting_locations,          {0xb08060 +8 -0x800000,LOC_END}},
    {"Stop",            0x21, stop_locations,           {0xb0806a +4 -0x800000,LOC_END}},
    {"Super Heal",      0x22, superheal_locations,      {0x9abc1c +4 -0x800000,LOC_END}}
};

enum alchemy_indices { // has to match alchemy_locations
    ACID_RAIN_IDX, ATLAS_IDX, BARRIER_IDX, CALL_UP_IDX,
    CORROSION_IDX, CRUSH_IDX, CURE_IDX, DEFEND_IDX, DOUBLE_DRAIN_IDX,
    DRAIN_IDX, ENERGIZE_IDX, ESCAPE_IDX, EXPLOSION_IDX,
    FIREBALL_IDX, FIRE_POWER_IDX, FLASH_IDX, FORCE_FIELD_IDX,
    HARD_BALL_IDX, HEAL_IDX, LANCE_SPELL_IDX, /* no LASER_IDX, */
    LEVITATE_IDX, LIGHTNING_STORM_IDX, MIRACLE_CURE_IDX, NITRO_IDX,
    ONE_UP_IDX, REFLECT_IDX, REGROWTH_IDX, REVEALER_IDX, REVIVE_IDX,
    SLOW_BURN_IDX, SPEED_IDX, STING_IDX, STOP_IDX, SUPER_HEAL_IDX
};
#define ALCHEMY_COUNT ARRAY_SIZE(alchemy_locations)
_Static_assert(ACID_RAIN_IDX==0 && SUPER_HEAL_IDX==ALCHEMY_COUNT-1, "Bad alchemy indices");

#pragma pack(push)
#pragma pack(1)
struct formula {
    uint8_t type1;
    uint8_t type2;
    uint8_t amount1;
    uint8_t amount2;
};
#pragma pack(pop)

enum ingredient_numbers { // has to match ingame-numbers
    WAX, WATER, VINEGAR, ROOT, OIL, MUSHROOM, MUD_PEPPER, METEORITE,
    LIMESTONE, IRON, GUNPOWDER, GREASE, FEATHER, ETHANOL, DRY_ICE,
    CRYSTAL, CLAY, BRIMSTONE, BONE, ATLAS_AMULET, ASH, ACORN
};

const char* ingredient_names[] = { // has to match ingame-numbers/order
    "Wax", "Water", "Vinegar", "Root", "Oil", "Mushroom", "Mud Pepper", "Meteorite",
    "Limestone", "Iron", "Gunpowder", "Grease", "Feather", "Ethanol", "Dry Ice",
    "Crystal", "Clay", "Brimstone", "Bone", "Atlas Medallion", "Ash", "Acorn"
};

#define INGREDIENT_TYPES { /* vanilla formula ingredients, incl. Laser */\
    0x14,0x01, 0x14,0x13, 0x08,0x12, 0x07,0x0e, 0x05,0x01,\
    0x08,0x00, 0x03,0x04, 0x10,0x14, 0x0d,0x02, 0x0d,0x03,\
    0x0f,0x09, 0x00,0x02, 0x0d,0x14, 0x11,0x14, 0x0c,0x11,\
    0x00,0x04, 0x0b,0x09, 0x0f,0x10, 0x03,0x01, 0x09,0x15,\
    0x0f,0x11, 0x01,0x06, 0x09,0x14, 0x03,0x02, 0x0a,0x0b,\
    0x0c,0x03, 0x0b,0x09, 0x15,0x01, 0x14,0x00, 0x03,0x12,\
    0x09,0x11, 0x00,0x01, 0x01,0x02, 0x00,0x0f, 0x0d,0x15\
};

#define INGREDIENT_AMOUNTS { /* vanilla formula ingredient count, incl. Laser */\
    1,3, 1,1, 1,2, 1,1, 1,3,\
    1,1, 2,1, 1,1, 2,2, 1,2,\
    1,1, 1,1, 2,1, 1,2, 1,1,\
    1,2, 1,1, 1,1, 1,1, 1,1,\
    1,1, 1,1, 1,2, 2,1, 1,2,\
    1,1, 2,1, 1,2, 2,1, 3,1,\
    1,1, 1,2, 2,1, 2,1, 2,1\
};

_Static_assert(WAX==0x00 && ACORN==0x15, "Bad ingredient numbering");
_Static_assert(ARRAY_SIZE(ingredient_names)==ACORN+1, "Bad ingredient names");

// boss_drop_jumps is where we put the relocation of drops.
// This jumps to a fixed address in v040+
const static size_t boss_drop_jumps[] = { // has to match PATCH 81,83-105
    0x93d0b8 + 1 - 0x800000, // Thraxx
    0x93d1ed + 1 - 0x800000, // Coleoptera
    0x93dadd + 1 - 0x800000, // Mammoth Viper
    0x94a775 + 1 - 0x800000, // Cave Raptors
    0x94ddd0 + 1 - 0x800000, // Salabog
    0x99e533 + 1 - 0x800000, // Vigor
    0x97a434 + 1 - 0x800000, // Megataur
    0x95ab5d + 1 - 0x800000, // Rimsala
    0x97e516 + 1 - 0x800000, // Aquagoth
    0x99c87d + 1 - 0x800000, // Bad Boys
    0x99ce9f + 1 - 0x800000, // Timberdrake
    0x9a807c + 1 - 0x800000, // Verminator
    0x95a3d3 + 1 - 0x800000, // Tiny
    0x9a927b + 1 - 0x800000, // Mungola
    0x9589d7 + 1 - 0x800000, // Sons of Sth.
    0x949321 + 1 - 0x800000, // Volcano Viper
    0x96c113 + 1 - 0x800000, // Magmar
    0x96c11c + 1 - 0x800000, // Mad Monk
    0x96c125 + 1 - 0x800000, // Footknight
};

// boss_drop_setup_jumps is where we have to set the address of the drop setup
// (in the relocated code mentioned above)
// setup jump target we put there is calculated in get_drop_setup_target
const static uint32_t boss_drop_setup_jumps[] = { // has to match boss_rando_drops.txt
    0x96C1AA +  0 * 9 + 1 - 0x800000, // Thraxx
    0x96C1AA +  1 * 9 + 1 - 0x800000, // Coleoptera
    0x96C1AA +  2 * 9 + 1 - 0x800000, // Mammoth Viper
    0x96C1AA +  3 * 9 + 1 - 0x800000, // Cave Raptors
    0x96C1AA +  4 * 9 + 1 - 0x800000, // Salabog
    0x96C1AA +  5 * 9 + 1 - 0x800000, // Vigor
    0x96C1AA +  6 * 9 + 1 - 0x800000, // Megataur
    0x96C1AA +  7 * 9 + 1 - 0x800000, // Rimsala
    0x96C1AA +  8 * 9 + 1 - 0x800000, // Aquagoth
    0x96C1AA +  9 * 9 + 1 - 0x800000, // Bad Boys
    0x96C1AA + 10 * 9 + 1 - 0x800000, // Timberdrake
    0x96C1AA + 11 * 9 + 1 - 0x800000, // Verminator
    0x96C1AA + 12 * 9 + 1 - 0x800000, // Tiny
    0x96C1AA + 13 * 9 + 1 - 0x800000, // Mungola
    0x96C1AA + 14 * 9 + 1 - 0x800000, // Sons of Sth.
    0x96C1AA + 15 * 9 + 1 - 0x800000, // Volcano Viper
    0x96C1AA + 16 * 9 + 1 - 0x800000, // Magmar
    0x96C1AA + 17 * 9 + 1 - 0x800000, // Mad Monk
    0x96C1AA + 18 * 9 + 1 - 0x800000, // Footknight
};

enum boss_indices { // has to match boss_drop_jumps
    THRAXX_IDX, COLEOPTERA_IDX, MAMMOTH_VIPER_IDX, CAVE_RAPTORS_IDX,
    SALABOG_IDX, VIGOR_IDX, MEGATAUR_IDX, RIMSALA_IDX, AQUAGOTH_IDX,
    BAD_BOYS_IDX, TIMBERDRAKE_IDX, VERMINATOR_IDX, TINY_IDX, MUNGOLA_IDX,
    SONS_IDX, VOLCANO_VIPER_IDX, MAGMAR_IDX, MONK_IDX, FOOTKNIGHT_IDX
};

const static char* boss_names[] = {
    "Thraxx", "Coleoptera", "Mammoth Viper", "Cave Raptors",
    "Salabog", "Vigor", "Megataur", "Rimsala", "Aquagoth",
    "Bad Boys", "Timberdrake", "Verminator", "Tiny", "Mungola",
    "Sons of Sth.", "Volcano Viper", "Magmar", "Mad Monk", "Footknight",
};

_Static_assert(THRAXX_IDX==0 && FOOTKNIGHT_IDX==ARRAY_SIZE(boss_drop_jumps)-1, "Bad boss indices");
_Static_assert(ARRAY_SIZE(boss_names)==ARRAY_SIZE(boss_drop_jumps), "Bad boss name list");


enum boss_drop_indices { // has to match boss_rando_drops.txt
    NOTHING_IDX, WHEEL_IDX, GLADIATOR_SWORD_IDX, CRUSADER_SWORD_IDX,
    SPIDER_CLAW_IDX, BRONZE_AXE_IDX, HORN_SPEAR_IDX, BRONZE_SPEAR_IDX,
    LANCE_WEAPON_IDX, HONEY_DROP_IDX, DINO_DROP_IDX, BAZOOKA_DROP_IDX,
    GOLD_10K_IDX, MUD_PEPPER_DROP_IDX, DIAMOND_EYE_DROP_IDX
};

const static char* boss_drop_names[] = {
    "Nothing", "Wheel", "Gladiator Sword", "Crusader Sword",
    "Spider Claw", "Bronze Axe", "Horn Spear", "Bronze Spear",
    "Lance (Weapon)", "Honey", "Progressive vest", "Bazooka",
    "10,000 Gold Coins", "Mud Pepper", "Diamond Eye"
};

#define BOSS_DROPS { /* vanilla drops in same boss order as boss_drop_jumps */\
    SPIDER_CLAW_IDX,      WHEEL_IDX,            HORN_SPEAR_IDX,\
    DINO_DROP_IDX,        MUD_PEPPER_DROP_IDX,  GLADIATOR_SWORD_IDX,\
    DIAMOND_EYE_DROP_IDX, DIAMOND_EYE_DROP_IDX, HONEY_DROP_IDX,\
    CRUSADER_SWORD_IDX,   LANCE_WEAPON_IDX,     BAZOOKA_DROP_IDX,\
    DIAMOND_EYE_DROP_IDX, GOLD_10K_IDX,         BRONZE_AXE_IDX,\
    MUD_PEPPER_DROP_IDX,  NOTHING_IDX,          BRONZE_SPEAR_IDX,\
    NOTHING_IDX,\
};

_Static_assert(NOTHING_IDX==0/* && DIAMOND_EYE_DROP_IDX==ARRAY_SIZE(boss_drop_setup_jumps)-1*/, "Bad boss drop indices");
_Static_assert(ARRAY_SIZE(boss_drop_setup_jumps)==ARRAY_SIZE(boss_drop_jumps), "Bad boss jump list");
_Static_assert(ARRAY_SIZE(boss_drop_names)==DIAMOND_EYE_DROP_IDX+1, "Bad boss drop name list");

enum progression {
    P_NONE,
    P_WEAPON,
    P_NON_SWORD,
    P_BRONZE_AXE_PLUS,
    P_REAL_BRONZE_AXE_PLUS,
    P_BRONZE_AXE,
    P_KNIGHT_BASHER,
    P_KNIGHT_BASHER_PLUS,
    P_BRONZE_SPEAR_PLUS,
    P_ENERGY_CORE,
    P_CORE_FRAGMENT,
    P_FINAL_BOSS,
    P_DE,
    P_GAUGE,
    P_WHEEL,
    P_ROCKET,
    P_JAGUAR_RING,
    P_QUEENS_KEY,
    P_ORACLE_BONE,
    P_REVEALER,
    P_LEVITATE,
    P_ATLAS,
    P_PYRAMID_OR_RUINS,
    P_AEGIS_DEAD,
    P_PYRAMID_ACCESSIBLE,
    P_ALLOW_OOB,
    P_ALLOW_SEQUENCE_BREAKS,
    P_GAUGE_ACCESS,
    P_QUEENS_KEY_CHESTS_ACCESS,
    P_ENERGY_CORE_ACCESS,
    P_ECRUSTACIA_CHESTS_ACCESS,
    P_OGLIN_CAVE_ACCESS,
    P_VOLCANO_ENTERED,
    P_STERLING_DEAD,
    P_MUNGOLA,
    P_ACT1_WEAPON,
    P_ACT2_WEAPON,
    P_ACT3_WEAPON,
    P_ACT4_WEAPON,
    P_ARMOR,
    P_AMMO,
    P_GLITCHED_AMMO,
    P_BAZOOKA,
    P_CALLBEAD,
    P_WINGS,
    P_OFFENSIVE_FORMULA, // >= Lance
    P_PASSIVE_FORMULA, // used to mark "useful" items
    P_END
};

struct progression_requirement {
    int8_t pieces;
    enum progression progress;
};

struct progression_provider {
    int8_t pieces;
    enum progression progress;
};

#define NO_REQ {0,P_NONE}
#define NOTHING_REQUIRED { NO_REQ,NO_REQ,NO_REQ,NO_REQ }
#define NO_PVD {0,P_NONE}
#define NOTHING_PROVIDED { NO_PVD,NO_PVD,NO_PVD,NO_PVD,NO_PVD,NO_PVD }
#define REQ1(p) { {1,p},NO_REQ,NO_REQ,NO_REQ }
#define REQ2(p,q) { {1,p},{1,q},NO_REQ,NO_REQ }
#define REQ3(p,q,r) { {1,p},{1,q},{1,r},NO_REQ }
#define REQ4(p,q,r,s) { {1,p},{1,q},{1,r},{1,s} }
#define REQ1N(n,p) { {n,p},NO_REQ,NO_REQ,NO_REQ }
#define REQ2N(n,p,m,q) { {n,p},{m,q},NO_REQ,NO_REQ }
#define REQ3N(j,p,k,q,l,r) { {j,p},{k,q},{l,r},NO_REQ }
#define REQ4N(j,p,k,q,l,r,m,s) { {j,p},{k,q},{l,r},{m,s} }
#define PVD1(p) { {1,p},NO_PVD,NO_PVD,NO_PVD,NO_PVD,NO_PVD }
#define PVD2(p,q) { {1,p},{1,q},NO_PVD,NO_PVD,NO_PVD,NO_PVD }
#define PVD3(p,q,r) { {1,p},{1,q},{1,r},NO_PVD,NO_PVD,NO_PVD }
#define PVD4(p,q,r,s) { {1,p},{1,q},{1,r},{1,s},NO_PVD,NO_PVD }
#define PVD5(p,q,r,s,t) { {1,p},{1,q},{1,r},{1,s},{1,t},NO_PVD }
#define PVD6(p,q,r,s,t,u) { {1,p},{1,q},{1,r},{1,s},{1,t},{1,u} }
#define PVD1N(n,p) { {n,p},NO_PVD,NO_PVD,NO_PVD,NO_PVD,NO_PVD }
#define PVD2N(n,p,m,q) { {n,p},{m,q},NO_PVD,NO_PVD,NO_PVD,NO_PVD }
#define PVD3N(j,p,k,q,l,r) { {j,p},{k,q},{l,r},NO_PVD,NO_PVD,NO_PVD }
#define PVD4N(j,p,k,q,l,r,m,s) { {j,p},{k,q},{l,r},{m,s},NO_PVD,NO_PVD }
#define PVD5N(j,p,k,q,l,r,m,s,n,t) { {j,p},{k,q},{l,r},{m,s},{n,t},NO_PVD }
#define PVD6N(j,p,k,q,l,r,m,s,n,t,o,u) { {j,p},{k,q},{l,r},{m,s},{n,t},{o,u} }

// extra items and traps are custom items and have a script that sets up receiving them (this is currently required)
typedef struct extra_item {
    const char* name;
    uint16_t item_id;
    uint32_t setup;
    struct progression_provider provides[6];
} extra_item;

const static extra_item extra_data[] = {
    {"Energy Core Fragment", 0x012a, 0xb08280 + 6 * 6 - 0x800000, PVD1(P_CORE_FRAGMENT)},
};

const static extra_item trap_data[] = {
    {"Quake Trap",    0x0151, 0xb08280 + 0 * 6 - 0x800000, NOTHING_PROVIDED},
    {"Poison Trap",   0x0152, 0xb08280 + 1 * 6 - 0x800000, NOTHING_PROVIDED},
    {"Confound Trap", 0x0153, 0xb08280 + 2 * 6 - 0x800000, NOTHING_PROVIDED},
    {"HUD Trap",      0x0154, 0xb08280 + 3 * 6 - 0x800000, NOTHING_PROVIDED},
    {"OHKO Trap",     0x0155, 0xb08280 + 4 * 6 - 0x800000, NOTHING_PROVIDED},
};

enum check_tree_item_type {
    CHECK_NONE,
    CHECK_ALCHEMY,
    CHECK_BOSS,
    CHECK_GOURD,
    CHECK_SNIFF,
    CHECK_EXTRA,
    CHECK_TRAP,
    CHECK_NPC,
    CHECK_RULE,
};

typedef struct check_tree_item {
    bool reached;
    enum check_tree_item_type type; // spell, boss or gourd
    uint16_t index; // which spell, boss or gourd
    bool missable;
    int8_t difficulty;
    struct progression_requirement requires[4];
    struct progression_provider provides[6];
} check_tree_item;

typedef struct drop_tree_item {
    enum check_tree_item_type type; // spell, boss or gourd
    uint16_t index; // which spell, boss or gourd
    struct progression_provider provides[6];
} drop_tree_item;

static const check_tree_item blank_check_tree[] = {
    // Alchemy checks               missable  d. requires                            provided by the check itself
    {0, CHECK_ALCHEMY,ACID_RAIN_IDX,       0, 0, NOTHING_REQUIRED,                              NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,ATLAS_IDX,           0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,BARRIER_IDX,         0, 1, REQ2N(1,P_WEAPON,2,P_DE),                      NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,CALL_UP_IDX,         0, 0, REQ1(P_ROCKET),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,CORROSION_IDX,       0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,CRUSH_IDX,           0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,CURE_IDX,            0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,DEFEND_IDX,          0, 0, NOTHING_REQUIRED,                              NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,DOUBLE_DRAIN_IDX,    0, 0, REQ2N(1,P_REAL_BRONZE_AXE_PLUS,2,P_DE),        NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,DRAIN_IDX,           0, 0, REQ1(P_REAL_BRONZE_AXE_PLUS),                  NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,ENERGIZE_IDX,        0, 0, REQ2(P_ROCKET,P_ENERGY_CORE),                  NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,ESCAPE_IDX,          0, 0, REQ2(P_WEAPON,P_PYRAMID_OR_RUINS),             NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,EXPLOSION_IDX,       0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,FIREBALL_IDX,        0, 1, REQ3(P_WEAPON,P_REVEALER,P_JAGUAR_RING),       NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,FIRE_POWER_IDX,      0, 0, REQ2(P_WEAPON,P_QUEENS_KEY),                   NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,FLASH_IDX,           0, 0, NOTHING_REQUIRED,                              NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,FORCE_FIELD_IDX,     0, 0, REQ1(P_ROCKET),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,HARD_BALL_IDX,       0, 0, NOTHING_REQUIRED,                              NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,HEAL_IDX,            0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,LANCE_SPELL_IDX,     0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,LEVITATE_IDX,        0, 0, REQ1(P_NON_SWORD),                             NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,LIGHTNING_STORM_IDX, 0, 0, REQ1(P_KNIGHT_BASHER),                         NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,MIRACLE_CURE_IDX,    0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,NITRO_IDX,           0, 0, REQ1(P_ROCKET),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,ONE_UP_IDX,          0, 1, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,REFLECT_IDX,         0, 0, REQ1(P_ROCKET),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,REGROWTH_IDX,        0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,REVEALER_IDX,        0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,REVIVE_IDX,          0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,SLOW_BURN_IDX,       0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,SPEED_IDX,           0, 0, REQ1(P_VOLCANO_ENTERED),                       NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,STING_IDX,           0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,STOP_IDX,            0, 0, REQ2(P_ROCKET,P_ORACLE_BONE),                  NOTHING_PROVIDED},
    {0, CHECK_ALCHEMY,SUPER_HEAL_IDX,      0, 0, REQ1(P_WEAPON),                                NOTHING_PROVIDED},
    // Boss checks
    {0, CHECK_BOSS,THRAXX_IDX,        0, 0, NOTHING_REQUIRED,                        NOTHING_PROVIDED},
    {0, CHECK_BOSS,COLEOPTERA_IDX,    0, 1, REQ1(P_WEAPON),                          NOTHING_PROVIDED}, // NOTE: we added REQ(P_WEAPON) since the boss is too hard to get the first weapon
    {0, CHECK_BOSS,MAMMOTH_VIPER_IDX, 0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_BOSS,CAVE_RAPTORS_IDX,  0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_BOSS,SALABOG_IDX,       0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_BOSS,VOLCANO_VIPER_IDX, 0, 0, REQ1(P_VOLCANO_ENTERED),                 NOTHING_PROVIDED},
    {0, CHECK_BOSS,MAGMAR_IDX,        0, 0, REQ1(P_VOLCANO_ENTERED),                 PVD1(P_GAUGE_ACCESS)},
    {0, CHECK_BOSS,VIGOR_IDX,         0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_BOSS,MONK_IDX,          0, 0, REQ3(P_WEAPON,P_REVEALER,P_JAGUAR_RING), NOTHING_PROVIDED},
    {0, CHECK_BOSS,MEGATAUR_IDX,      0, 0, REQ2(P_BRONZE_SPEAR_PLUS,P_REVEALER),    PVD1(P_PYRAMID_OR_RUINS)},
    {0, CHECK_BOSS,SONS_IDX,          0, 0, REQ2(P_WEAPON,P_PYRAMID_ACCESSIBLE),     NOTHING_PROVIDED},
    {0, CHECK_BOSS,RIMSALA_IDX,       0, 0, REQ2(P_BRONZE_AXE,P_PYRAMID_ACCESSIBLE), PVD1(P_PYRAMID_OR_RUINS)},
    {0, CHECK_RULE,P_AEGIS_DEAD,      0, 0, REQ2N(1,P_WEAPON,2,P_DE),                PVD3(P_AEGIS_DEAD,P_PYRAMID_ACCESSIBLE,P_OGLIN_CAVE_ACCESS)},
    {0, CHECK_BOSS,AQUAGOTH_IDX,      0, 0, REQ2(P_WEAPON,P_OGLIN_CAVE_ACCESS),      NOTHING_PROVIDED},
    {0, CHECK_BOSS,FOOTKNIGHT_IDX,    0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_BOSS,BAD_BOYS_IDX,      0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_BOSS,TIMBERDRAKE_IDX,   0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_BOSS,VERMINATOR_IDX,    0, 0, REQ1(P_WEAPON),                          NOTHING_PROVIDED},
    {0, CHECK_RULE,P_STERLING_DEAD,   0, 1, REQ1(P_KNIGHT_BASHER),                   PVD1(P_STERLING_DEAD)},
    {0, CHECK_BOSS,MUNGOLA_IDX,       0, 0, REQ1(P_STERLING_DEAD),                   PVD1(P_MUNGOLA)},
    {0, CHECK_BOSS,TINY_IDX,          0, 0, REQ3(P_WEAPON,P_AEGIS_DEAD,P_LEVITATE),  NOTHING_PROVIDED},
    // Required checks that are not randomized (yet)
    {0, CHECK_RULE,P_JAGUAR_RING,     0, 0, NOTHING_REQUIRED,           PVD1(P_JAGUAR_RING)},
    {0, CHECK_RULE,P_QUEENS_KEY,      0, 0, REQ1(P_WEAPON),             PVD2(P_QUEENS_KEY, P_QUEENS_KEY_CHESTS_ACCESS)},
    {0, CHECK_RULE,P_ORACLE_BONE,     0, 0, REQ1(P_WEAPON),             PVD1(P_ORACLE_BONE)},
    {0, CHECK_RULE,P_ROCKET,          0, 0, REQ4N(1,P_GAUGE,1,P_WHEEL,2,P_DE,1,P_WEAPON), PVD3N(1,P_ROCKET,-2,P_DE,1,P_ENERGY_CORE_ACCESS)},
    {0, CHECK_RULE,P_FINAL_BOSS,      0, 0, REQ2(P_ROCKET,P_ENERGY_CORE), PVD1(P_FINAL_BOSS)},
    {0, CHECK_RULE,P_VOLCANO_ENTERED, 0, 0, REQ2(P_WEAPON,P_LEVITATE),  PVD1(P_VOLCANO_ENTERED)},
    {0, CHECK_RULE,P_ALLOW_SEQUENCE_BREAKS, 0, 0, REQ2(P_WEAPON,P_ALLOW_SEQUENCE_BREAKS), PVD3(P_REVEALER,P_PYRAMID_ACCESSIBLE,P_VOLCANO_ENTERED)},
    {0, CHECK_RULE,P_ALLOW_SEQUENCE_BREAKS, 0, 0, REQ2(P_ROCKET,P_ALLOW_SEQUENCE_BREAKS), PVD1(P_FINAL_BOSS)},
    {0, CHECK_RULE,P_ALLOW_OOB,       0, 0, REQ2(P_WEAPON,P_ALLOW_OOB),
            PVD6(P_NON_SWORD,P_BRONZE_AXE_PLUS,P_KNIGHT_BASHER,P_KNIGHT_BASHER_PLUS,P_REVEALER,P_PYRAMID_ACCESSIBLE)},
    {0, CHECK_RULE,P_ALLOW_OOB,       0, 0, REQ2(P_WEAPON,P_ALLOW_OOB),
            PVD5(P_QUEENS_KEY_CHESTS_ACCESS,P_ENERGY_CORE_ACCESS,P_ECRUSTACIA_CHESTS_ACCESS,P_OGLIN_CAVE_ACCESS,P_VOLCANO_ENTERED)},
    // Stuff to work around OoB giving fake progression
    {0, CHECK_RULE,P_BRONZE_AXE_PLUS, 0, 0, REQ1(P_REAL_BRONZE_AXE_PLUS), PVD1(P_BRONZE_AXE_PLUS)},
    // Energy core fragments conversion; will be updated in main
    {0, CHECK_RULE,P_ENERGY_CORE,     0, 0, REQ1N(-1, P_CORE_FRAGMENT), NOTHING_PROVIDED},
    #define CHECK_TREE
    // Gourd checks included from generated gourds.h
    #include "gourds.h"
    // Sniff checks included from generated sniff.h
    #include "sniff.h"
    #undef CHECK_TREE
};

static const drop_tree_item drops[] = {
    // Extra drops
    {CHECK_EXTRA, 0, PVD1(P_CORE_FRAGMENT)},
    // Alchemy drops with progression
    {CHECK_ALCHEMY,ATLAS_IDX,    PVD1(P_ATLAS)},
    {CHECK_ALCHEMY,REVEALER_IDX, PVD2(P_REVEALER, P_PYRAMID_ACCESSIBLE)},
    {CHECK_ALCHEMY,LEVITATE_IDX, PVD2(P_LEVITATE,P_ECRUSTACIA_CHESTS_ACCESS)},
    // Alchemy drops with difficulty-based progression
    {CHECK_ALCHEMY,CRUSH_IDX,           PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,DOUBLE_DRAIN_IDX,    PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,EXPLOSION_IDX,       PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,FIREBALL_IDX,        PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,FIRE_POWER_IDX,      PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,LANCE_SPELL_IDX,     PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,LIGHTNING_STORM_IDX, PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,NITRO_IDX,           PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,STING_IDX,           PVD1(P_OFFENSIVE_FORMULA)},
    {CHECK_ALCHEMY,BARRIER_IDX,         PVD1(P_PASSIVE_FORMULA)},
    {CHECK_ALCHEMY,CALL_UP_IDX,         PVD1(P_PASSIVE_FORMULA)},
    {CHECK_ALCHEMY,ENERGIZE_IDX,        PVD1(P_PASSIVE_FORMULA)},
    // Boss drops with progression
    {CHECK_BOSS,WHEEL_IDX,            PVD1(P_WHEEL)},
    {CHECK_BOSS,GLADIATOR_SWORD_IDX,  PVD2(P_WEAPON,P_ACT2_WEAPON)},
    {CHECK_BOSS,CRUSADER_SWORD_IDX,   PVD2(P_WEAPON,P_ACT3_WEAPON)},
    {CHECK_BOSS,SPIDER_CLAW_IDX,      PVD3(P_WEAPON,P_NON_SWORD,P_ACT1_WEAPON)},
    {CHECK_BOSS,BRONZE_AXE_IDX,       PVD5(P_WEAPON,P_NON_SWORD,P_BRONZE_AXE,P_REAL_BRONZE_AXE_PLUS,P_ACT2_WEAPON)},
    {CHECK_BOSS,HORN_SPEAR_IDX,       PVD3(P_WEAPON,P_NON_SWORD,P_ACT1_WEAPON)},
    {CHECK_BOSS,BRONZE_SPEAR_IDX,     PVD4(P_WEAPON,P_NON_SWORD,P_BRONZE_SPEAR_PLUS,P_ACT2_WEAPON)},
    {CHECK_BOSS,LANCE_WEAPON_IDX,     PVD4(P_WEAPON,P_NON_SWORD,P_BRONZE_SPEAR_PLUS,P_ACT3_WEAPON)},
    {CHECK_BOSS,DIAMOND_EYE_DROP_IDX, PVD1(P_DE)},
    {CHECK_BOSS,DINO_DROP_IDX,        PVD1(P_ARMOR)},
    {CHECK_BOSS,BAZOOKA_DROP_IDX,     PVD1(P_BAZOOKA)},
    #define DROP_TREE
    // Gourd drops with progression included from generated gourds.h
    #include "gourds.h"
    // Sniff drops with progression included from generated sniff.h
    #include "sniff.h"
    #undef DROP_TREE
};


// Logic/data helper functions
static inline bool check_requires(const check_tree_item* check, enum progression progress)
{
    for (size_t i=0; i<ARRAY_SIZE(check->requires); i++)
        if (check->requires[i].pieces>0 && check->requires[i].progress==progress)
            return true;
    return false;
}

static inline bool check_reached(const check_tree_item* check, const int* progress)
{
    for (size_t i=0; i<ARRAY_SIZE(check->requires); i++)
        if (check->requires[i].pieces>progress[check->requires[i].progress])
            return false;
    return true;
}

static inline void check_progress(check_tree_item* check, int* progress)
{
    check->reached = true;
    if (check->missable) return;
    for (size_t i=0; i<ARRAY_SIZE(check->provides); i++)
        progress[check->provides[i].progress] += check->provides[i].pieces;
}

static inline void drop_progress(const drop_tree_item* drop, int* progress)
{
    if (!drop) return; // get_drop may return NULL
    for (size_t i=0; i<ARRAY_SIZE(drop->provides); i++)
        progress[drop->provides[i].progress] += drop->provides[i].pieces;
}

static inline const drop_tree_item* get_drop(enum check_tree_item_type type, uint16_t idx)
{
    // NOTE: since we only have progression in drops, always iterating over everything is fast
    //       this could still be sped up using binary search
    for (size_t i=0; i<ARRAY_SIZE(drops); i++)
        if (drops[i].type == type && drops[i].index == idx) return drops+i;
    return NULL;
}

static inline const drop_tree_item* get_drop_from_packed(uint16_t packed)
{
    enum check_tree_item_type type = (enum check_tree_item_type)(packed>>10);
    uint16_t idx = packed&0x3ff;
    return get_drop(type, idx);
}

static int drop_provides(const drop_tree_item* drop, uint16_t progress)
{
    if (!drop) return 0;
    for (size_t i=0; i<ARRAY_SIZE(drop->provides); i++)
        if (drop->provides[i].progress == progress) return drop->provides[i].pieces;
    return 0;
}

const char* check2str(const check_tree_item* check)
{
    if (check->type == CHECK_BOSS) return boss_names[check->index];
    if (check->type == CHECK_ALCHEMY) return alchemy_locations[check->index].name;
    if (check->type == CHECK_GOURD) return gourd_data[check->index].name;
    if (check->index == P_JAGUAR_RING) return "Jaguar Ring NPC";
    if (check->index == P_KNIGHT_BASHER) return "Knight Baser Chest";
    if (check->index == P_GAUGE) return "Gauge Gourd";
    if (check->index == P_ROCKET) return "Rocket";
    if (check->index == P_ENERGY_CORE) return "Energy Core";
    if (check->index == P_VOLCANO_ENTERED) return "Volcano entered";
    if (check->index == P_ALLOW_SEQUENCE_BREAKS) return "Sequence break";
    if (check->index == P_ALLOW_OOB) return "Out of bounds";
    return "Unknown";
}

const char* drop2str(const drop_tree_item* drop)
{
    if (!drop) return ""; // get_drop may return NULL
    if (drop->type == CHECK_BOSS) return boss_drop_names[drop->index];
    if (drop->type == CHECK_ALCHEMY) return alchemy_locations[drop->index].name;
    if (drop->type == CHECK_GOURD) return gourd_drops_data[drop->index].name;
    if (drop->type == CHECK_EXTRA) return extra_data[drop->index].name;
    if (drop->type == CHECK_TRAP) return trap_data[drop->index].name;
    return "Unknown";
}

bool alchemy_in_act4(uint16_t alchemy_idx)
{
    return (alchemy_idx==CALL_UP_IDX || alchemy_idx==ENERGIZE_IDX ||
            alchemy_idx==FORCE_FIELD_IDX || alchemy_idx==NITRO_IDX ||
            alchemy_idx==REFLECT_IDX || alchemy_idx==STOP_IDX);
}

bool alchemy_is_good(uint16_t alchemy_idx)
{
    switch (alchemy_idx) {
        case CRUSH_IDX:
        case EXPLOSION_IDX:
        case FIRE_POWER_IDX:
        case FIREBALL_IDX:
        case LIGHTNING_STORM_IDX:
        case NITRO_IDX:
        case STING_IDX:
        case CALL_UP_IDX:
            return true;
        default:
            return false;
    }
}

bool alchemy_is_healing(uint16_t alchemy_idx)
{
    switch (alchemy_idx) {
        case BARRIER_IDX:
        case CALL_UP_IDX:
        case HEAL_IDX:
        case MIRACLE_CURE_IDX:
        case ONE_UP_IDX:
        case SUPER_HEAL_IDX:
            return true;
        default:
            return false;
    }
}

bool ingredient_is_cheap(uint8_t i)
{
    return (i==WAX || i==WATER || i==VINEGAR || i==ROOT || i==OIL || i==LIMESTONE || i==ETHANOL || i==CRYSTAL || i==CLAY || i==BONE || i==ASH);
}

bool alchemy_is_cheap(const struct formula* f)
{
    return ingredient_is_cheap(f->type1) && ingredient_is_cheap(f->type2);
}

static bool can_buy_ingredient(uint8_t i)
{
    // technically we CAN buy meteorite, but we don't want to use that
    return (i!=DRY_ICE && i!=METEORITE);
}

static bool can_buy_ingredients(const struct formula* f)
{
    return (can_buy_ingredient(f->type1) && can_buy_ingredient(f->type1));
}

static bool can_buy_ingredient_in_act3(uint8_t i)
{
    return (i!=DRY_ICE && i!=METEORITE && i!=GREASE && i!=GUNPOWDER);
}

static bool can_buy_in_act3(const struct formula* f)
{
    return (can_buy_ingredient_in_act3(f->type1) &&
            can_buy_ingredient_in_act3(f->type2));
}

static const uint8_t pre_thraxx_ingredients[] = {ASH, CRYSTAL, OIL, WATER, WAX};

static bool can_buy_ingredient_pre_thraxx(uint8_t i)
{
    return (i==ASH || i==CRYSTAL || i==OIL || i==WATER || i==WAX);
}

static bool can_buy_pre_thraxx(const struct formula* f)
{
    return (can_buy_ingredient_pre_thraxx(f->type1) &&
            can_buy_ingredient_pre_thraxx(f->type2));
}

#define CHARACTER_TOTAL 142
#define CHARACTER_SKIP  2
#define CHARACTER_DATA_SIZE  74
#define CHARACTER_DATA_COUNT (CHARACTER_TOTAL-CHARACTER_SKIP)
#define CHARACTER_DATA_START (0x0eb678 + CHARACTER_SKIP*CHARACTER_DATA_SIZE)
#define CHARACTER_DATA_END   (CHARACTER_DATA_START + CHARACTER_DATA_COUNT*CHARACTER_DATA_SIZE)
#define CHARACTER_DATA_EXP_OFF   0x23
#define CHARACTER_DATA_MONEY_OFF 0x27

const size_t callbead_menu_action_addrs[] = {
    // points to the action ids of the main ring
    0x0e85b4+6, 0x0e85bc+6, 0x0e85c4+6, 0x0e85cc+6
};

const size_t callbead_spell_item_addrs[] = {
    // points to the item ids that are loaded per ring
    0x0e9ceb+1, 0x0e9cf1+1, 0x0e9cf7+1, 0x0e9cfd+1, // fire eyes
    0x0e9d16+1, 0x0e9d1c+1, 0x0e9d22+1,             // horace (without regenerate and aura)
    0x0e9d64+1, 0x0e9d6a+1, 0x0e9d70+1, 0x0e9d76+1, // camellia
    0x0e9d95+1, 0x0e9d9b+1, 0x0e9d8f+1,             // sidney
};

static uint32_t get_drop_setup_target(enum check_tree_item_type type, uint16_t idx)
{
    if (type == CHECK_NONE) return 0x0fd200; // remote item
    if (type == CHECK_GOURD && idx<ARRAY_SIZE(gourd_drops_data)) return gourd_drops_data[idx].call_addr;
    if (type == CHECK_ALCHEMY && idx<ARRAY_SIZE(alchemy_locations)) {
        uint32_t addr = 0xb0813b + 6 * alchemy_locations[idx].id;
        return (addr & 0x7fff) | ((((addr&0x7fffff)-0x120000) >> 1) & 0xff8000);
    }
    if (type == CHECK_BOSS && idx<ARRAY_SIZE(boss_drop_names)) {
        // TODO: use existing table above instead of doing the calculation again?
        uint32_t addr = 0x96c150 + 6 * idx;
        return (addr & 0x7fff) | ((((addr&0x7fffff)-0x120000) >> 1) & 0xff8000);
    }
    if (type == CHECK_EXTRA) {
        // extra items work the same way as traps; read below
        uint32_t addr = extra_data[idx].setup;
        return (addr & 0x7fff) | ((((addr&0x7fffff)-0x120000) >> 1) & 0xff8000);
    }
    if (type == CHECK_TRAP) {
        // while traps replace regular drops and would not need their own script location
        // currently it's implemented in a way where each item is tied to a script
        uint32_t addr = trap_data[idx].setup;
        return (addr & 0x7fff) | ((((addr&0x7fffff)-0x120000) >> 1) & 0xff8000);
    }
    if (type == CHECK_SNIFF && ((idx >= 0x200 && idx <= 0x215) || idx == 0x1f)) {
        // sniff items are encoded using the in-game item IDs, making the above
        // statement somewhat false
        if (idx == 0x1f) // iron bracer is an odd sniff item, we put it after ingredients
            idx = 0x16;
        else
            idx -= 0x200;
        uint32_t addr = 0x30ff00 + 6 * idx;
        return (addr & 0x7fff) | ((((addr&0x7fffff)-0x120000) >> 1) & 0xff8000);
    }
    assert(0);
    return 0;
}

static uint32_t get_drop_setup_target_from_packed(uint16_t packed)
{
    enum check_tree_item_type type = (enum check_tree_item_type)(packed>>10);
    uint16_t idx = packed&0x3ff;
    return get_drop_setup_target(type, idx);
}

static const char* get_item_name(uint16_t item)
{
    // order of mushroom and mud peppers are swapped in inventory vs item id
    if (item == 0x200 + MUSHROOM)
        return ingredient_names[MUD_PEPPER];
    if (item == 0x200 + MUD_PEPPER)
        return ingredient_names[MUSHROOM];
    // other ingredients are in order starting at 0x200
    if (item >= 0x200 && item < 0x200 + ARRAY_SIZE(ingredient_names))
        return ingredient_names[item - 0x200];
    // special sniff spot item
    if (item == 0x41f)
        return "Iron Bracer";
    assert(0);
    return "";
}

static const char* get_drop_name(enum check_tree_item_type type, uint16_t idx)
{
    if (type == CHECK_NONE) return "Remote";
    if (type == CHECK_GOURD && idx<ARRAY_SIZE(gourd_drops_data)) return gourd_drops_data[idx].name;
    if (type == CHECK_ALCHEMY && idx<ARRAY_SIZE(alchemy_locations)) return alchemy_locations[idx].name;
    if (type == CHECK_BOSS && idx<ARRAY_SIZE(boss_drop_names)) return boss_drop_names[idx];
    if (type == CHECK_EXTRA && idx<ARRAY_SIZE(extra_data)) return extra_data[idx].name;
    if (type == CHECK_TRAP && idx<ARRAY_SIZE(trap_data)) return trap_data[idx].name;
    if (type == CHECK_SNIFF) return get_item_name((idx < 0x100 && idx > 1) ? (0x400 + idx) : idx);
    printf("Couldn't get name for %d 0x%03x\n", type, idx);
    assert(0);
    return "";
}

static const char* get_drop_name_from_packed(uint16_t packed)
{
    enum check_tree_item_type type = (enum check_tree_item_type)(packed>>10);
    uint16_t idx = packed&0x3ff;
    return get_drop_name(type, idx);
}

static bool is_real_progression(const drop_tree_item* drop)
{
    if (drop->type == CHECK_BOSS) {
        // all other defined boss drops are progression
        return !(drop->index == DINO_DROP_IDX || drop->index == BAZOOKA_DROP_IDX);
    }
    if (drop->type == CHECK_ALCHEMY) {
        return (drop->index == REVEALER_IDX || drop->index == LEVITATE_IDX);
    }
    if (drop->type == CHECK_GOURD) {
        return (drop->provides[0].progress == P_GAUGE || drop->provides[0].progress == P_WEAPON ||
                drop->provides[0].progress == P_ENERGY_CORE);
    }
    return false;
}

static bool is_real_progression_from_packed(uint16_t packed)
{
    enum check_tree_item_type type = (enum check_tree_item_type)(packed>>10);
    uint16_t idx = packed&0x3ff;

    if (type == CHECK_SNIFF)
        return false; // manual speed up until we have smarter search

    for (size_t i=0; i<ARRAY_SIZE(drops); i++) {
        const drop_tree_item* drop = drops+i;
        if (drop->type == type && drop->index == idx)
            return is_real_progression(drop);
    }
    return false;
}

static size_t count_real_progression_from_packed(const uint16_t* packed, size_t count)
{
    size_t n = 0;
    for (size_t i=0; i<count; i++) {
        if (is_real_progression_from_packed(packed[i])) n++;
    }
    return n;
}

#ifdef WITH_MULTIWORLD
static bool is_valid_placement_item(enum check_tree_item_type type, uint16_t idx)
{
    switch (type) {
        case CHECK_NONE:
            return true; // "Remote"
        case CHECK_GOURD:
            return idx < ARRAY_SIZE(gourd_drops_data);
        case CHECK_ALCHEMY:
            return idx < ARRAY_SIZE(alchemy_locations);
        case CHECK_BOSS:
            return idx < ARRAY_SIZE(boss_drop_names);
        case CHECK_EXTRA:
            return idx < ARRAY_SIZE(extra_data);
        case CHECK_TRAP:
            return idx < ARRAY_SIZE(trap_data);
        case CHECK_SNIFF:
            return (idx >= 0x200 && idx < 0x200 + ARRAY_SIZE(ingredient_names)) || idx == 0x1f;
        default:
            return false;
    }
}
#endif
