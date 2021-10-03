# Multiworld Roadmap

Uses [Archipelago](https://github.com/ArchipelagoMW/Archipelago) ("AP") for multi-game multiworld generation and synchronization\
See [black-sliver / Archipelago / soe](https://github.com/black-sliver/archipelago/tree/soe) for seed generation\
See [black-sliver / pyevermizer](https://github.com/black-sliver/pyevermizer) for the interface to Archipelago

## Required features for AP

* Mix spells and items (levitate and revealer are progression items)

* Temp. var in "loot item" to decide if an animation should be played (own item),\
  or a simple unframed text should be displayed (received item)
  
* Routine (on NMI?) to (schedule script to) award items through memory writes

* Client that sends out and receives items from the AP server

* Counter to (re-)synchronize received items

* Flags for checked alchemy locations (in addition to flags for received spells)

* Plando-like interface to have item placement come from AP

## Semi-related features

* Add memory block to signal which call bead spells have been viewed by the player\
  (NOTE: if the player never opened the CB menu, the spells should not show up in the tracker)
  
* Reserve space for Extended market timer

* Reserve space for Energy Core fragments (unused trade good?)

* Add magic number + version + seed settings to ROM 

* Copy magic number + version + seed settings from ROM to RAM (for RA+Snes9x)

* Add flag "--hide-settings" that gets written to ROM so the tracker does not spoil (Mystery seed)

## Steps

### Add LGPL text + info in readme

_Status: **DONE**_

### Add missing item names

_Status: **DONE**_

### AP item placement

_Status: PARTIAL_

* Logic/placement: **done**
* Placement output: **TODO**

### AP interface

_Status: PARTIAL_

AP will have to
* generate a `placement.txt`: **TODO**
* generate an ID sequence for seed/slot: **done**

`main()` will have to
* take a `placement.txt`: **done**
* take an ID sequence: **done**

### ROM generation

_Status: PARTIAL_

* place ID sequence in ROM: **done**
* memcpy ID sequence to RAM: **TODO**
* use `placement.txt` for item placement: **TODO**
* see other changes below: **TODO**

### Reserve/assign memory

_Status: TODO_

see [wiki/Memory-Layout#ram-layout](https://github.com/black-sliver/evermizer/wiki/Memory-Layout#ram-layout)

* Alchemy spots: 1bit per spell, so we can map 1:1; this needs to be bit-accessible by scripts
* Call bead visibility; this will be written by ASM code
  * FIXME: for chaos the tracker will still have to read mapping from ROM
  * 4x4 bits: full enum for each slot -> tracker only reads these 2B
    * Unknown
    * FE
    * Horace
    * Horace + Aura
    * Horace + Regenerate
    * Horace + Aura + Regenerate
    * Queen
    * Prof.
* Extended market timer: 1-2 additional bytes for the upper part
* Energy core fragments (maybe the 1 unused word in trade goods)
* 16bit integer for "last received item sequence number"
* Some bytes for multiworld communication

### SRAM-save, SRAM-load

_Status: TODO_

We need to change the addresses and sizes when the game loads/saves
from/to SRAM to accommodate for the additional values above.

### Convert spells to items

_Status: TODO_

* Assign item IDs
* Call loot script instead of rewarding spell directly
* Change dialogs (TBD)
* Use spell location flags instead of spell flags in alchemists' scripts

### Change item and location definition

_Status: TODO_

* Move boss locations, drops and addresses to `bosses.json?
* Add utilities `bosses2h.py?
* Same for alchemy?
* Generate code on the fly (required to mix drops)?

### Implement receiving of remote items in game

_Status: TODO_

### AP client

_Status: TODO_
* this automatically sends items to the server
* this has to receive, forward and resync remote items to the game

### Change how "unrandom" works

_Stauts: TODO_

At the moment if something is not randomized, it is not touched. Instead we
should always apply the same modifications, but have the values fixed, to allow
more options without making the code unreadable.

### Call bead changes for the tracker

_Status: TODO_

Implement setting above bits when opening the corresponding ring menu.

