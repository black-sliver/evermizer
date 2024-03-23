// lock barrier dialog behind aegis instead of checking a (now broken) flag and windwalker flag
// NOTE: PATCH6 is useless. See google doc.
DEF(6, 0x95d422 - 0x800000,
    "\x09\x85\x0b\x04");
DEF(7, 0x95d44a - 0x800000,
    "\x08\x85\x0b\x04");
// close palace gate unless aegis is dead (instead of windwalker)
DEF(8, 0x96d641 - 0x800000,
    "\x09\x85\x0b\x04");
DEF(9, 0x95d742 - 0x800000,
    "\x09\x85\x0b\x04");
// merge aegis code path of the two market -> square scripts, add check for vigor
DEF(10, 0x95ea00 - 0x800000,
    //@ 95ea00
    "\xa3\x00"
    "\x08\x05\x07\x04\x29\x05\x06\x04\x28\x29\x05\x0b\x04\x14\x28\x29\x05\x3d\x00\xa8\x28\x00"
    "\xa3\x1d"
    "\x22\x01\x14\x08\x00"
    "\x00"
    //@ 95ea20
    "\xa3\x00"
    "\x08\x05\x07\x04\x29\x05\x06\x04\x28\x29\x05\x0b\x04\x14\x28\x29\x05\x3d\x00\xa8\x08\x00"
    "\xa3\x1d"
    "\x22\x01\x40\x08\x00"
    "\x00"
    //@ 95ea40
    "\x0c\x2b\x00\xb0"
    "\xa3\x2e"
    "\x22\x21\x41\x09\x00"
    "\x00"
    //@ 95ea4d
    "\x00\x00");
// set new address of second market -> square script @ 9283ed
DEF(11, 0x9283ed - 0x800000,
    "\x20\xea\x01");
_Static_assert(sizeof(PATCH10)-1 == 2*0x27, "Bad patch size"); // check if size is still the same
// fixing act3 prison (always change music)
DEF(12, 0x98b0f4 - 0x800000,
    "\x09\x88\x35\x01\x00\x00");
// set side=EAST when entering prison on east side, instead of unknown flag 22eb|=0x40
DEF(13, 0x98e803 - 0x800000,
    "\x0c\x2e\x04\xb1");
// make space for verm check before room change, 2 free bytes, add IF to verm room change
DEF(14, 0x98e7e7 - 0x800000,
    "\xa3\x26" // call script 26
    "\x22\x22\x3b\x0e\x00" // change map
    "\x00" // end
    "\x00\x00" // +2 free bytes
    "\x09\x85\x28\x04\x07\x00"); // IF !verm dead SKIP 7
// make sure we did not miscalculate
GCC_Static_assert((PATCH_LOC14 + 8) == (0x98e7ee - 0x800000 + 1), "Bad patch location");
GCC_Static_assert((PATCH_LOC14 + sizeof(PATCH14)-1) == (0x98e7f3 - 0x800000 + 4), "Bad patch size");
// change (start of) script to IF
DEF(15, 0x9284c8 - 0x800000,
    "\xf1\x67\x03");
// remove (useless) windwalker checks from mungola room, 3 free bytes
DEF(16, 0x9a8c44 - 0x800000,
    "\x04\x1f\x00\x00\x00\x00");
DEF(17, 0x9a8c6d - 0x800000,
    "\x09\x30\x14\x14"); // 14 29 05 29 04 a8 61 06
// putting 2nd drawbridge behind mungola rock (instead of windwalker)
DEF(18, 0x99d99e - 0x800000,
    "\x09\x05\x6e\x04"); // 94 0f 00
// changing 2nd drawbridge to always be east
DEF(19, 0x99d9ab - 0x800000,
    "\x5c\xb4"); // b1
// block off chessboard from naris when drawbridge is not open
// move start of enter-script, removes (useless?) instructions
DEF(20, 0x928066 - 0x800000,
    "\x75\x3c\x04");
// move room-change script to newly freed space
DEF(21, 0x9284e3 - 0x800000,
    "\x62\x3c\x04");
// add IF to those bytes, RJMP into regular code if mungola is dead
DEF(22, 0x9abc62 - 0x800000,
    "\x09\x85\x6e\x04\x02\x00" // IF NOT MUNGOLA DEAD SKIP 2
    "\x05\x10" // RJMP -240
    "\x00" // END
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"); // 10 free
// don't clear out bottom half of act3 towns (always people there)
DEF(23, 0x98c426 - 0x800000,
    "\x04\x08\x00"  // uncond. JMP
    "\x00\x00\x00"); // +3 free bytes
DEF(24, 0x98c435 - 0x800000,
    "\x04\x08\x00"  // uncond. JMP
    "\x00\x00\x00\x00\x00\x00\x00\x00"); // +8 free bytes
DEF(25, 0x98c44f - 0x800000,
    "\x04\x08\x00"  // uncond. JMP
    "\x00\x00\x00\x00\x00\x00\x00\x00"); // +8 free bytes
// change bazooka / ticket lady inventory after doggo race
DEF(26, 0x98c009 - 0x800000,
    "\x09\x85\x20"); // 04 b7 00
// don't clear out top half of act3 town (always people there)
#define PATCH27 PATCH23
DEF_LOC(27, 0x98c9db - 0x800000);
#define PATCH28 PATCH24
DEF_LOC(28, 0x98c9ea - 0x800000);
#define PATCH29 PATCH25
DEF_LOC(29, 0x98ca00 - 0x800000);
DEF(30, 0x98cad3 - 0x800000,
    "\x04\x10\x00"  // uncond. JMP
    "\x00\x00\x00\x00\x00\x00\x00\x00"); // +8 free bytes?
// fix lance
#define PATCH31 PATCH24
DEF_LOC(31, 0x98ddb4 - 0x800000);
// TODO: fix houses; conditional in-house NPCs only in second town now
#if 0 // this has been superseded by 139
// (partially) fix reverse timberdrake
DEF(32, 0x99ce14 - 0x800000,
    "\x04\x03\x00"  // uncond. JMP
    "\x00\x00\x00"); // +3 free bytes
#endif
// keep dog after magmar (ALT: any other 4b INSTR at at 9581df)
DEF(33, 0x9581e2 - 0x800000,
    "\xb0"); // clear bit instead of set bit
// remove checks for WW in act1
DEF(34, 0x97cc4a - 0x800000,
    "\x04\x2a\x00" // uncond. JMP
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // 42 free bytes
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00");
// remove checks for WW in nobilia
DEF(35, 0x97c600 - 0x800000,
    "\x04\x1a\x00" // uncond. JMP
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // 26 free bytes
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00");
// remove checks for WW in crustacia
#define PATCH36 PATCH35
DEF_LOC(36, 0x97c141 - 0x800000);
// remove checks for WW in act3, fix tinker flags by tinker state
DEF(37, 0x9acfe2 - 0x800000,
    "\x0c\x6b\x04\x05\x6b\x04\x29\x07\xfe\x00\x29\x32\x1e\xa7" // $22e5.3 = $22e5.3 | ($2356<2)
    "\x04\x0c\x00" // uncond. JMP
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"); // 12 free bytes
// verify patches above are correct
assert(sizeof(PATCH34)-1-3 == (size_t)PATCH34[1]);
assert(sizeof(PATCH35)-1-3 == (size_t)PATCH35[1]);
assert(sizeof(PATCH36)-1-3 == (size_t)PATCH36[1]);
assert(sizeof(PATCH37)-1-3-14 == (size_t)PATCH37[14+1]);
GCC_Static_assert(PATCH_LOC34 + sizeof(PATCH34)-1 == 0x97cc77 - 0x800000, "Bad patch size");
GCC_Static_assert(PATCH_LOC35 + sizeof(PATCH35)-1 == 0x97c61d - 0x800000, "Bad patch size");
GCC_Static_assert(PATCH_LOC36 + sizeof(PATCH36)-1 == 0x97c15e - 0x800000, "Bad patch size");
GCC_Static_assert(PATCH_LOC37 + sizeof(PATCH37)-1 == 0x9acfff - 0x800000, "Bad patch size");
// convert landing skip to regular landing (location check -> !!!!!FALSE)
DEF(38, 0x9ad10e - 0x800000 + 5,
    "\x30\x14\x14\x14\x14\x14");
// Top-of-volcano in late-game
DEF(39, 0x948476 - 0x800000 + 1, "\x30\x14\x14");
DEF(40, 0x9484a7 - 0x800000 + 1, "\x30\x14\x94");
DEF(41, 0x948529 - 0x800000 + 1, "\x30\x14\x94");
DEF(42, 0x948545 - 0x800000 + 1, "\x30\x14\x14");
DEF(43, 0x948351 - 0x800000 + 1, "\x30\x14\x94");
DEF(44, 0x9483df - 0x800000 + 1, "\x30\x14\x94");
DEF(45, 0x9481f8 - 0x800000, "\x29\x69\x05\x01\x00"); // CALL 0x948569, END
DEF(46, 0x948569 - 0x800000,
    "\x08\x85\x46\x00\x06\x00" // IF $2260 & 0x40 (magmar dead is set) SKIP 6
    "\x22\x4a\x38\x69\x00"     // CHANGE MAP (slide)
    "\x00"                     // END
    "\x22\x5f\x1b\x69\x00"     // CHANGE MAP (Gauge)
    "\x00");                   // END
DEF(47, 0x93ef80 - 0x800000 + 6, "\x46\x00");
// Require Levitation / desolarify
DEF(48, 0x939240 - 0x800000,
    "\x09\x85\x35\x00\x07\x00" //IF !$225e&0x20 (rock levitated) THEN SKIP 7
    "\xa3\x00"                 //CALL script id 0x00
    "\x29\x7b\x05\x01"         //CALL 0x94857b (free space)
    "\x00");                   //END
DEF(49, 0x94857b - 0x800000,
    "\xa3\x26"             // CALL script id 0x26
    "\x14\xf3\x00\xb8"     // $234b = 0x0008
    "\x22\xd5\x95\x3c\x00" // CHANGE MAP 0x3c @ 0x06a8 | 0x04a8
    "\x00");               //END
// Fix fire eyes cutscene order
DEF(50, 0x94e396 - 0x800000 + 2, "\x46\x00"); // magmar dead
DEF(51, 0x94e485 - 0x800000 + 2, "\x46\x00");
DEF(52, 0x94cf69 - 0x800000 + 2, "\x46\x00");
DEF(53, 0x94d04b - 0x800000 + 6, "\x46\x00");
DEF(54, 0x94d5cf - 0x800000 + 2, "\x46\x00");
DEF(55, 0x94d5e2 - 0x800000 + 2, "\x46\x00");
// Fix act2 square dog statue
DEF(56, 0x95e1f3 - 0x800000 + 2, "\x0b\x04"); // aegis dead
// Fix act2 palace guards
DEF(57, 0x95d6a9 - 0x800000 + 2, "\x0b\x04"); // aegis dead
DEF(58, 0x95d63b - 0x800000 + 2, "\x0b\x04");
// Mungola Pt.2 (NOTE: Pt.2 overwrites parts of part1)
DEF(59, 0x9a8c44 - 0x800000,
    "\x09\x85\x29\x04\x04\x00" // IF !<$22dd&0x02> (STERLING DEAD) SKIP 4
    "\x0c\x23\x04\xb0"         // CLEAR <$22dc&0x08> (WINDWALKER FLAG)
    "\x04\x15\x00");           // UNCONDITIONAL JUMP BY 21 TO 9a8c66
// Lock Coleoptera behind FE callbeads cutscene (fix triggers/outro skip)
DEF(60, 0x93d23b - 0x800000 +  6, "\x3f\x00");
DEF(61, 0x93d25a - 0x800000 +  2, "\x3f\x00");
DEF(62, 0x93d2cc - 0x800000 +  7, "\x3f\x00");
DEF(63, 0x93d4ac - 0x800000 + 12, "\x3f\x00");
DEF(64, 0x93cdb0 - 0x800000 +  2, "\x3f\x00");
DEF(65, 0x93ccee - 0x800000 + 11, "\x3f\x00");
// Move infinite callbead behind rocket in open world
DEF(66, 0x95d608 - 0x800000 +  2, "\x29\x00");
// Disable infinite callbead glitch (completely disable trigger for now)
DEF(67, 0x95d608 - 0x800000, "\x00");
// Desaturnate: don't check for char!=dog, but teleporter used instead
DEF(68, 0x9aef47 - 0x800000, "\x08\x85\x0d\x05\x02\x00\x00");
// Spawn Madrionus also if aegis is already defeated
DEF(69, 0x96dfd7 - 0x800000, "\x08\x05\x08\x04\x29\x05\x0b\x04\x27\x94");
// Allow to skip verm (put prison door to castle behind opening cells)
DEF(72, 0x98a486 - 0x800000 + 1, "\x87\xf7\x00");
// Allow entering west prison with dead dog by not assigning dog-died-script
DEF(73, 0x98b08b - 0x800000, "\x04\x03\x00");
// Remove horace DE stealing cutscene by changing script start to end
// ONLY PATCH IF BOSS RANDO IS ON, see also XX (make horace unskippable, TODO)
DEF(74, 0x9282c4 - 0x800000, "\xad\x42\x02");
// Add Text "Received Nothing!"; Could always be applied, but only required for boss drop rando
DEF(77, 0xc00000 - 0xc00000, "\xF9\x00\x28\x2E\x02\x82\xD2\x13\x1E\x0F");
// Change Horace spawn condition in camp to have DEs or DEs stolen
DEF(78,  0x96e02c + 1 - 0x800000, "\x05\x61\x00\x29\x05\x0d\x04\xa7");
DEF(78a, 0x96e089 + 1 - 0x800000, "\x05\x61\x00\x29\x05\x0d\x04\xa7");
DEF(78b, 0x96d6cf + 1 - 0x800000, "\x05\x61\x00\x29\x05\x0d\x04\xa7");
DEF(78c, 0x96d7b6 + 6 - 0x800000, "\x05\x61\x00\x29\x05\x0d\x04\x27");
DEF(78d, 0x96dbe7 + 1 - 0x800000, "\x05\x61\x00\x29\x05\x0d\x04\xa7");
// and fix horace telling you how many DEs you have
DEF(78e, 0x96dad1 + 1 - 0x800000, "\x05\x60\x00\x29\x30\x14\x14\xa7");
// Change Aegis spawn condition to have DEs or DEs stolen (applied after OW)
DEF(79, 0x95ea02 + 1 - 0x800000, "\x05\x61\x00\x29\x05\x0d\x04\x27");
DEF(80, 0x95ea22 + 1 - 0x800000, "\x05\x61\x00\x29\x05\x0d\x04\x27");
// Added/enhanced boss drops for boss drop rando
DEF(81, 0x96c113 - 0x800000,
    "\x29\x25\x41\x02" "\x29\x33\x3F\x00" "\x00"   // Magmar
    "\x29\x25\x41\x02" "\x0c\x25\x00\xb1" "\x00"   // Monk
    "\x29\x25\x41\x02" "\x0c\x70\x04\xb1" "\x00" // Footknight
    // 18 free bytes here
);
// Actual boss drops for boss drop rando, right after added drops (TODO: assert that)
#if 0 // this has been replaced by new boss drops
DEF(82, 0x96c125 - 0x800000,
    "\x52\x00\x00" "\x00" // Nothing
    "\x0c\x63\x00\xb1" "\x52\xbb\x05" "\x00" // Wheel
    "\x18\xe9\x01\xb4" "\x52\xec\x1c" "\x00" // Gladiator Sword
    "\x18\xe9\x01\xb6" "\x52\x7a\x1c" "\x00" // Crusader Sword
    "\x18\xe9\x01\xba" "\x52\xb8\x05" "\x00" // Spider Claw
    "\x18\xe9\x01\xbc" "\x52\x71\x0a" "\x00" // Bronze Axe
    "\x18\xe9\x01\xe2" "\x52\xc1\x05" "\x00" // Horn Spear
    "\x18\xe9\x01\xe4" "\x52\x29\x16" "\x00" // Bronze Spear
    "\x18\xe9\x01\xe6" "\x52\x89\x1c" "\x00" // Lance
    "\x14\xbf\x00\x07\xbf\x00\x29\x31\x9a" "\x52\xcb\x16" "\x00" // Honey
#if !progressive_armor
    "\x09\x07\xc7\x00\x94\x0a\x00" // Dino skin OR 100 Talons
    "\x14\xc7\x00\xb1" "\x52\xa8\x06" "\x04\x08\x00"
    "\x7c\xb0\x64\x00\x00" "\x52\xab\x06" "\x00"
#else
    "\x18\x39\x01\x84\xf4\x01" // Progressive breast armor instead
    "\xa3\x3e" "\xa3\x3f" // Call loot part1 + part2
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#endif
    "\x08\x85\x1d\x04\x16\x00" // Bazooka+Shells OR ...
    "\x18\xe9\x01\xea" "\x14\xed\x00\x07\xed\x00\x29\x3a\x9a" "\x52\x13\x1d" "\xa7\x80" "\x52\x16\x1d" "\x00"
    "\x08\x87\xcd\x00\x08\x00" // ... Shining Armor OR 5k Gold
    "\x14\xcd\x00\xb1" "\x52\x19\x1d" "\x00"
    "\x7c\xb6\x88\x13\x00" "\x52\x1c\x1d" "\x00"
    "\x7c\xb6\x10\x27\x00" "\x52\xa0\x1d" "\x00" // 10k Gold
    "\x14\xad\x00\x07\xad\x00\x29\x31\x9a" "\x52\xd5\x09" "\x00" // Mud Pepper
    );
#endif
// Rewrite boss drops for boss drop rando
DEF(83, 0x93d0b8 - 0x800000, // Thraxx
    "\x29\x25\x41\x02" "\x04\x00\x00" "\x29\x33\x3F\x00" "\xa7\x08" "\xa3\x00" "\x33\x78" "\xa3\x01");
DEF(84, 0x93d1db - 0x800000, // Coleoptera
    "\xa7\x14" "\x07\xb1\x57\x00" "\x3a" "\x5c\x88\x43\x02\xb2" "\xa3\x00" "\x33\x78" "\xa3\x01" "\x29\x25\x41\x02" "\x04\x00\x00");
DEF(85, 0x93dad7 - 0x800000, // Mammoth Graveyard Viper
    "\xa3\x00" "\x33\x78" "\xa3\x01" "\x29\x25\x41\x02" "\x04\x00\x00");
DEF(86, 0x94a775 - 0x800000, // Cave raptors
    "\x29\x25\x41\x02" "\x04\x17\x00");
DEF(87, 0x94ddd0 - 0x800000, // Salabog
    "\x29\x25\x41\x02" "\x04\x05\x00");
DEF(88, 0x99e533 - 0x800000, // Vigor
    "\x29\x25\x41\x02" "\x04\x00\x00" "\x29\x33\x3F\x00");
DEF(89, 0x97a434 - 0x800000, // Megataur, actually would not need changing
    "\x29\x25\x41\x02");
DEF(90, 0x95ab5d - 0x800000, // Rimsala, actually would not need changing
    "\x29\x25\x41\x02");
DEF(91, 0x97e512 - 0x800000, // Aquagoth
    "\x54\x01" "\xa3\x0d" "\x29\x25\x41\x02" "\x04\x05\x00" "\x00\x00\x00\x00\x00" "\x30\x44" "\xa7\x78" "\x55" "\x55");
DEF(92, 0x99c876 - 0x800000, // Bad Boys
    "\x80" "\xa3\x00" "\x33\x78" "\xa3\x01" "\x29\x25\x41\x02" "\x04\x00\x00");
DEF(93, 0x99ce98 - 0x800000, // Timberdrake
    "\x80" "\xa3\x00" "\x33\x78" "\xa3\x01" "\x29\x25\x41\x02" "\x04\x00\x00");
DEF(94, 0x9a807c - 0x800000, // Verminator
    "\x29\x25\x41\x02" "\x04\x34\x00");
DEF(95, 0x95a3d3 - 0x800000, // Tiny
    "\x29\x25\x41\x02" "\x04\x04\x00" "\x00\x00\x00\x00" "\x29\x33\x3f\x00" "\xa7\xf0" "\xa3\x00" "\x78\xd0\x00\x80\xb0");
DEF(96, 0x9a9279 - 0x800000, // Mungola
    "\x30\x44" "\x29\x25\x41\x02" "\x04\x01\x00" "\x00");
DEF(97, 0x958813 + 1 - 0x800000, // Sons of Something spawn:
    "\x05\x56\x04");            // check flag instead of weapon
DEF(98, 0x9589d7 - 0x800000, // Sons of Something drop pt1 (prize)
    "\x29\x25\x41\x02");
DEF(99, 0x9589e9 - 0x800000, // Sons of Something drop pt2 (text)
    "\x04\x00\x00");
DEF(100, 0x94931d - 0x800000, // Volcano Viper, sets new flag
    "\x0c\x24\x00\xb1" "\x29\x25\x41\x02" "\x29\x33\x3f\x00" "\x04\x0b\x00");
DEF(101, 0x94932c - 0x800000, // New Volcano Viper spawn trigger
    "\x0c\x36\x00\x05\x24\x00\x94" "\x00");
DEF(102, 0x949c73 - 0x800000, // Run new Volcano Viper spawn condition
    "\x29\x2c\x13\x01");
DEF(103, 0x94bd5d - 0x800000, // Magmar (see also patch 78)
    "\x29\x13\x41\x02");
DEF(104, 0x97a397 + 1 - 0x800000, // Mad Monk spawn condition (new flag)
    "\x85\x25\x00");
DEF(105, 0x97a157 - 0x800000, // Mad Monk drop (see also patch 78)
    "\xa3\x00" "\x33\x78" "\xa3\x01" "\x29\x1c\x41\x02" "\x04\x00\x00");
DEF(105b, 0x99d46e - 0x800000, // Footknight (see also patch 78)
    "\x29\x25\x41\x02");
// Ignore "no prior save file" flag during intro
DEF(106, 0x92e4ab + 4 - 0x800000, "\x00");
// Enter timber drake from east as if coming from south if he's not dead
// NOTE: this uses space freed from and has to be applied after patch 35
DEF(139, 0x928528 - 0x800000, "\x03\xc6\x02"); // point to 0x97c600+3
DEF(140, 0x97c600 + 3 - 0x800000,
    "\x09\x85\x2a\x04\x08\x00" // if not Timerdrake dead, skip 8
    "\xa3\x19" "\x22\x2d\x15\x20\x00" "\x00" // vanilla room change
    "\xa3\x00" "\xa3\x26" "\x22\x1b\x29\x20\x00" "\x00"); // custom room change
    // NOTE: 2 free bytes remaining
// remove option to save at mardonius' brother
DEF(141, 0x97a10c - 0x800000, "\x04\x03\x00\x00\x00\x00"); // 3 free Bytes
DEF(142, 0x97a14e - 0x800000, "\x04\x03\x00\x00\x00\x00"); // 3 free Bytes
// set DEs stolen flag when giving DEs away for rocket in tinker dialog
// NOTE: this uses space freed with patch 126, so may only be applied with bossdropamizer=on
// FIXME: we may want to ALWAYS apply this, not only for bossdropamizer
DEF(143, 0x998995 - 0x800000, "\x29\xbf\x8b\x01");
DEF(144, 0x958bbf - 0x800000, "\x0c\x26\x04\xb1" "\x0c\x0d\x04\xb1" "\x00");
// Don't give away single rocket part to disable double gauge
DEF(145, 0x998bda - 0x800000, "\x04\x00\x00");
// Avoid softlock if you have collected more than 3 rocket parts
DEF(146, 0x998d7e + 6 - 0x800000, "\x21"); // >= OP instead of == OP
// Don't despawn Rock north of Nobilia Market when Aegis is dead
DEF(147, 0x95cb0a + 1 - 0x800000, "\x30\x14\x14"); // !!FALSE instead of bit test
// Don't start vigor fight before crush cutscene
// NOTE: this uses space freed with patch 36
DEF(149a, 0x95d9dd - 0x800000, "\x29\x44\xc1\x02\x00\x00\x00\x00"); // Call away from sacred dog cutscene
DEF(149b, 0x97c144 - 0x800000, // New code (jumps back or not)
    "\x09\x85\x38\x00\x0c\x00" // If ! Doggo palace cutscene watched skip 12
    "\x0c\x2b\x00\xb0" // This and
    "\x0c\xb9\x04\xb0" // this is code removed from sacred dog cutscene
    "\x29\xe5\xd9\x01" // Call back
    "\x00" // END
);

