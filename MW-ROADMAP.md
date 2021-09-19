# Multiworld Roadmap

## Required features for MW

* Mix spells and items (levitate and revealer are progression items)

* Temp. var in "loot item" to decide if an animation should be played (own item),\
  or a simple unframed text should be displayed (received item)
  
* Routine (on NMI?) to award items through memory writes

* Either use Farrak's client or write our own between SoE and AP

* Counter to (re-)synchronize received items

* Flags for checked alchemy locations (in addition to flags for received spells)

* Plando-like interface to have item placement come from AP

* **or** Place "remote item X" in each spot and let AP do the rest

## Semi-related features

* Add memory block to signal which call bead spells have been viewed by the player\
  (NOTE: if the player never opened the CB menu, the spells should not show up in the tracker)
  
* Reserve space for Extended market timer

* Reserve space for Energy Core fragments (unused trade good?)

* Add magic number + version + seed settings to ROM 

* Copy magic number + version + seed settings from ROM to RAM (for RA+Snes9x)

* Add flag "--hide-settings" that gets written to ROM so the tracker does not spoil

## Steps

### Add LGPL text + info in readme

_Status: TODO_

### Reserve/assign memory

_Status: TODO_

see [wiki/Memory-Layout#ram-layout](https://github.com/black-sliver/evermizer/wiki/Memory-Layout#ram-layout)

* Alchemy spots: 1bit per spell, so we can map 1:1; this needs to be bit-accessible by scripts
* Call bead visibility; this will be written by ASM code
  * FIXME: for chaos the tracker will still have to read mapping from ROM
  * either 4x4 bits: full enum for each slot -> tracker only reads these 2B
    * Unknown
    * FE
    * Horace
    * Horace + Aura
    * Horace + Regenerate
    * Horace + Aura + Regenerate
    * Queen
    * Prof.
  * or 4x3 bits: enum for each slot -> only if Aura/Reg. spoil the slot in text
    * Unknown
    * FE
    * Horace
    * Queen
    * Prof.
  * or 4x 1 bit for "menu visibility" + 2x 1 bit for aura/regenerate -> tracker needs to check ROM as well
* Extended market timer: 1-2 additional bytes for the upper part
* Energy core fragments (maybe the 1 unused word in trade goods)
* 16bit integer for "last received item sequence number"
* Some bytes for multiworld communication

### SRAM-save, SRAM-load

_Status: TODO_

### Call bead changes for the tracker

_Status: TODO_

Implement setting above bits when opening the corresponding ring menu.

### Convert spells to items

_Status: TODO_

* Assign item IDs
* Call loot script instead of rewarding spell directly
* Change dialogs (TBD)
* Use spell location flags instead of spell flags in alchemists' scripts

### Change item and location definition

_Status: TODO_

* Move boss locations, drops and addresses to bosses.json
* Add utility bosses2h.py
* Add utility to merge bosses.json, gourds.csv and alchemy.json for AP
* Generate code on the fly?

### Implement receiving of items

_Status: TODO_

### Implement sending of items

_Status: TODO_

### MW item placement

_Status: TODO_

* use `bosses.json` for both the rando and AP

### AP client

_Status: TODO_

### AP generation interface

_Status: TODO_

* use `bosses.json` for both the rando and AP

### Change how "unrandom" works

_Stauts: TODO_

At the moment if something is not randomized, it is not touched. Instead we
should always apply the same modifications, but have the values fixed, to allow
more options without making the code unreadable.

