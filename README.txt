HOWTO:
0. ROM has to be NTSC/US and can be a 3MB .SFC file with or without 512B header
1. Visit the in-browser version, unpack the exe or compile main.c
2. Drag and drop your rom into the in-browser field or onto the .exe
3a In-browser version should present you checkboxes in your browser and offer
   the patched ROM as "download" or show messages in an alert box.
3b Exe should ask you for settings and spit out success or error in a "dos"
   window, create a file starting with "Evermizer" in source ROM's directory
   and print out the complete filename.
4. Checksum is wrong, but neither FXPAK nor tested emulators seem to care
5. You can skip the intro by pressing start (even without a prior save)
6. You have to loot gourds/chests, check alchemists or kill Thraxx to get a
   weapon that can cut grass to get access to the Windwalker.
7. If you have questions or need help, continue reading below, check the guide
   at https://github.com/black-sliver/evermizer/blob/master/guide.md
   or join the discord.

CHANGES SO FAR:
  This is still far from complete.
  Crude user interface to select seed and settings
    Default output filename includes settings and seed number
    Option to save a spoiler log
  Command line interface:
    Allow batch processing
    Allow to specify complete output filename with -o
    Allow to specify output directory with -d
    Run the program without any file/arguments to see possible arguments
    Run the program in UI mode or with --settings to see possible settings
  Energy Core:
    Vanilla places it in its original location under the chessboard
    Shuffle converts the vanilla Energy Core to be a chest
    Fragment converts it to a chest and places multiple fragments into
      random chests. Use pool (see below) to shuffle them to other places.
      Trade for the real Energy Core at Prof. Ruffleberg.
  Ingredienizer:
    Randomize ingredient requirements
    Scales with difficulty
    Goes crazy with "Full"
  Alchemizer: "On" randomizes where you get which alchemy and
    Make sure Atlas is obtainable and usable in Easy and Normal
    Make sure Levitate and Revealer is obtainable and usable when required
    Newly learned alchemy is now preselected/highlighted for alchemy selection
    Strong Heart only teaches one formula, the other one is in the market
    "Pool" shuffles spells with other enabled pools
    (Dialogs still unchanged)
  Boss dropamizer: "On" randomizes what each boss drops and
    Horace will not steal your DEs
    Thraxx will always drop a weapon for now
    Bridges in pyramid won't collapse
    Horace spawn and dialog is changed (tied to DEs in inventory or DEs lost)
    Tinker taking away your diamond eyes sets flag to not lock-out Aegis
    If gourdomizer is on as well, you can go back through Big Bug after Thraxx
    "Pool" shuffles boss drops with other enabled pools
  Gourdomizer: "On" shuffles gourd/pot/chest drops, no gourds are missable and
    "Pool" shuffles gourd drops with other enabled pools
  Mixed Pool Strategy: when pooling is enabled
    "Balance" keeps the amount of key items in each pool constant
    "Random" may redistribute key items randomly
    "Bosses" will move all key items to bosses
  Sniffamizer: "On" shuffles, "Full" randomizes dog-sniff ingredients
  Callbeadmizer: "On" swaps call bead rings, "Full" shuffles individual spells
  Doggomizer: "On" swaps, "Full" randomizes the dog in your party
    Act1-3 only because toaster evasion sound hardlocks for some music tracks
  Pupdunk: Act0 doggo everywhere!
  Musicmizer: randomize music, in testing stage
  Sequence breaks: Fix, allow or "in logic" (possibly requiring it)
    Desolarify: Act1 first rock skip is disabled (Levitate required)
    Desaturnate: require energy core and use of teleporter before final boss hatch is open
    Disable double gauge: don't take away rocket parts until you have them all
  Out of bounds: Fix, allow or "in logic" (possibly requiring it)
    This will not enable Saturn skip nor rock skip, however volcano shop skips rock skip.
  Fix cheats:
    Remove infinite call bead glitch
  All accessible:
    Make sure all checks/areas/bosses can be reached
  Fix infinite ammo:
    Fixes the bug that Particle Bomb and Cryo-Blast do not drain
  Fix atlas glitch:
    Fixes stat underflow when dying with active stat boosts
  Fix wings glitch:
    Fixes wings granting invincibility if they "did not work"
  Money%:
    1 to 2500, Enemies give that % of money
  Exp%:
    1 to 2500, Enemies, weapons and formulas give that % experience points
  Shorter dialogs:
    Cut some dialogs and cutscenes. This is work in progress.
  Short boss rush:
    Start boss rush at Metal Magmar. Cut enemy HP in half.
  Turdo Mode:
    Replaces all offensive spells, weakens weapons, reduces enemies' Magic Def.
  Open world: Unlock windwalker in every fire pit (slashing weapon required)
    Grant access to inner volcano
    Grant access to east swamp
    Fix palace cutscenes
    Lock Barrier (Horace inside palace) + gates behind Aegis (instead of WW)
    Make both Act3 towns filled with people
    Moved ticket lady bazooka shells behind dog race (in both towns)
    Fixed lance not appearing. He now appears in ivory as well as ebon keep
      Still not 100%, look into this again, weird camera glitch in Act3 houses
    Locked east draw bridge behind Mungola
    Disabled exit from Naris to chessboard if Mungola is alive
    Fixed reverse Timberdrake
    Disabled exit from castle to Verminator if Verminator is not dead
    Fixed prison sound crash
    Killing Verminator is optional, prison door unlocks when prison is cleared
    Keep dog after Magmar (room change required to make him visible)
    Play Tinker WW cutscene once, regardless of progress
    Landing skip converted to a regular landing
    Act2 dog statue and dialogues moved behind aegis
    Vigor fight won't start if you did not jump down the cliff at blimp
    Act1 Fire Eyes cutscenes play in order (dog naming skip as in vanilla)
    Act1 top-of-volcano guy fixed
    Act3 Mungola HP / fight fixed
    Act1 move Coleoptera behind talking to FE after killing Thraxx
    Aquagoth is accessible once. Oglin cave gourd can't be randomized
    Madrinus will (still) spawn after Aegis is defeated
    Move call bead glitch behind rocket
    Don't despawn North Market rock when Aegis is dead
    Replaced FE Village Windwalker checks with Magmar-dead checks
    Progress Act3 NPCs based on Castle crumbled instead of WW (ongoing effort)
  Vanilla bugs fixed:
    Fix softlock when entering west prison with dead dog
    Fix softlock when opening mosquito cell last (exit will be bottom left cell)
    Fix softlock when you have more than 3 rocket parts
    Fix texts disappearing when you fall down in Ruins NW room
    Removed option to save at Madronius' Brother
    Fix softlock when leaving tiny's lair
    Saving with Atlas, Defend or Speed does not ruin your stats
  Fix missables:
    Directly reward Queen's Key to inventory
    Make Revealer and Escape still obtainable when Horace is gone
    All rooms with gourds/chests/pots can be re-entered with gourdomizer on
  Some quality of life improvements are always enabled, most notably:
    Preselect the correct spell in alchemy selection screen
    Bazooka charge/energize and stat display fix
    Multiple level-ups from one enemy
  Uses tinymt64 for platform-independend RNG. See tinymt64.h for license
  Allow for ROM to grow to 4MB if required
  Allow skipping of intro with no prior save file


STILL TO BE FIXED:
  Show info to the player if market timer is over but Vigor won't spawn yet
    you have to jump down the ravine next to Blimp's cave
  Act3 can have camera location bugs in houses until blindly talked to Lance
    saving and loading is a work-around
  A lot of vanilla crashes/softlocks
  See discord and github for ideas and feature requests


LINUX AND MAC BUILD INSTRUCTIONS:
  run make

WINDOWS BUILD INSTRUCTIONS:
  Run make for mingw, or create a VS project with main.c as (only) source file.
  The github source tree requires you to have python3 in PATH to build gen.h.
  The zip distribution includes a pre-built gen.h.

EMSCRIPTEN BUILD INSTRUCTIONS:
  Make sure emscripten/emcc is configured and in PATH. run make wasm
  You may need to set some env variables, like EM_CONFIG=~/.emscripten

CREDITS:
  Written by black_sliver
  Some patches by assassin17 - http://assassin17.brinkster.net
  Thanks to all people pushing or commenting on the project, providing ideas or
    artwork, testing it, pointing out bugs and looking into SNES/SoE stuff
  ayame li, colin, cyb3r, darkmoon2321, dot, esmo88, flarezenyu, fronk,
  greenambler, metasigma, mirapoix, masterknightdh, neagix, nyrambler,
  queenanne, scarrfish, skarsnikus, solarcell007, turbo, ventuz, zheal
  Thanks to vr-interactive for providing the website's new background image
  Thanks to sferath for providing the mascot - https://deviantart.com/sferath
  Thanks to unforgottenhero for listening to turbo.
  Special thanks to elwismw for existing. #hereforelwis

LICENSE:
  The source tree and generated binaries are distributed under the terms of
  GPL v3 - see LICENSE file - or alternatively under the terms of
  LGPL v3 if built into a DLL - see lgpl-3.0.txt.
  The patches found in patches/ directory are in public domain unless
  noted otherwise. See patches/LICENSE file.
  For the patches found in ips/, see the corresponding .txt files.
