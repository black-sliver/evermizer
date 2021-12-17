# Multiworld Roadmap

Multiworld is functional.

Uses [Archipelago](https://github.com/ArchipelagoMW/Archipelago) ("AP") for multi-game multiworld generation and synchronization\
See [black-sliver / Archipelago / soe](https://github.com/black-sliver/archipelago/tree/soe) for seed generation\
See [black-sliver / pyevermizer](https://github.com/black-sliver/pyevermizer) for the interface to Archipelago\
See [black-sliver / apbpatch](https://github.com/black-sliver/apbpatch) for ROM patching\
See [black-sliver / ap-soeclient](https://github.com/black-sliver/ap-soeclient) for AP client

Below are the items that are still to be done.

## Additional features for AP

* Traps (items)

* Death link (similar to items)

* Copy seed/slot from ROM to RAM

* Ingredienizer in AP to put alchemy in logic

* Show remote player and/or item name

## Semi-related features

* Add memory block to signal which call bead spells have been viewed by the player\
  (NOTE: if the player never opened the CB menu, the spells should not show up in the tracker)
  
* Reserve space for Extended market timer

* Reserve space for Energy Core fragments (unused trade good?)

* Add magic number + version + seed settings to ROM 

* Copy magic number + version + seed settings from ROM to RAM (for RA+Snes9x)

* Add flag "--hide-settings" that gets written to ROM so the tracker does not spoil (Mystery seed)

## Steps

### ROM generation

_Status: PARTIAL_

* place ID sequence in ROM: **done**
* memcpy ID sequence to RAM: **TODO**
* use `placement.txt` for item placement: **done**
* see other changes below: **TODO**

### Reserve/assign memory

_Status: PARTIAL_

see [wiki/Memory-Layout#ram-layout](https://github.com/black-sliver/evermizer/wiki/Memory-Layout#ram-layout)

* Alchemy spots: 1bit per spell, so we can map 1:1; this needs to be bit-accessible by scripts
  **done**
* Call bead visibility; this will be written by ASM code **TODO**
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
  **RESERVED**
* Energy core fragments (maybe the 1 unused word in trade goods)
  **RESERVED**
* 16bit integer for "last received item sequence number"
  **done**
* Some bytes for multiworld communication
  **done**

### Change item and location definition

_Status: UNCERTAIN_

* Move boss locations, drops and addresses to `bosses.json?
* Add utilities `bosses2h.py?
* Same for alchemy?

### Change how "unrandom" works

_Stauts: TODO_

At the moment if something is not randomized, it is not touched. Instead we
should always apply the same modifications, but have the values fixed, to allow
more options without making the code unreadable.

### Call bead changes for the tracker

_Status: TODO_

Implement setting above bits when opening the corresponding ring menu.

### Clean up and fix non-AP generation

_Status: TODO_

* Remove old patches

### Add remote player/item names

_Status: TODO_

In a future version we may want to display who sent or received an item.
