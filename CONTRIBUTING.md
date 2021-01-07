# Contributing

Development is on github, so please send pull requests there.

When submitting changes, you agree that it will be released under the project's
license and made sure you have the author's permission to do so.

For "regular" patches, see patches/LICENSE in addition.

IPS patches are seen as blobs, not code. They are to be accompanied by a .txt
file.

## Writing a randomization

* Addresses, consts and helpers should be defined in `data.h`.
* Patches should get the ROM to a state that only requires changing numbers.
* Actual implementation is inside `main()`. Consider how the rerolling works.
* Use helper functions like `shuffle_u8` from `rng.h` for randomization.
* Use `{read,write}{8,16,24}(location+rom_off, ...)` to read/write
* Don't forget the `rom_off` as this allows using ROMs with and without headers

## Adding an option/flag

Please see `struct option options` and macros below it in `main.c`.

## Evermizer patch system

The goal of the patch system is to generate actual instructions based on the
input data (the actual patches), not a loop over some data.

We have macros
  1. `DEF(name,data,loc)` to create (local) variables from string and location
  2. `APPLY(name)` to apply these local variables to the buffer
  3. `DEF_LOC(name,loc)` to create just the address variable, used to duplicate patches to a different address.

`DEF_LOC()` should probably be replaced by `DEF_DUP()` or a different mechanism in `patches/*.txt`.

By having `char[] patch` and `size_t location` as local variables, we get the
benefit of inlining the `memcpy`s during compilation. While an array of structs
could in theory be detected by the compiler as being the same, it will not get
optimized the same way (size becomes an actual variable?).

Instead we define additional macros `APPLY_<name>()` in `gen.h` for readability
that expand to `APPLY(<name>1);`...`APPLY(<name>n);` which expand to `memcpy`s.

The only downside of `APPLY_<name>` is that the `gen.h` can now only be included
after definition of the `APPLY()` macro.

Patches come from 2 places in code:
  1. `patches.h` (old) hand-written `DEF()`s
  2. `gen.h` (new) auto-generated `DEFS()`s

Patches in `gen.h` come from 2 places:
  1. `everscript2h.py patches/*.txt`
  2. `ips2h.py ips/*.txt`

### patches/*.txt format

TODO: add description

### ips/*.txt format

TODO: add description

## Writing a patch

Read above and check examples in `patches/` or `ips/`.

We generate two versions of the binary: *open world* (no rando) and *evermizer*.
If the patch is only to be included in one of them, please use
`#if(n)def NO_RANDO` in the `.txt` as well as `main.c`.

NOTE: *open world* with open world = off will become a bug-fixed vanilla version, if we ever implement open world as an actual option.

## Adding a patch

First create a patch in `patches/` or `ips/`, this will automatically be picked
up by the Makefile.

Then add the appropriate `APPLY()` or `APPLY_<name>` calls to `main.c`.
Set `grow=true` if it writes beyond 3MB.

## Upstreaming changes

Please create **one commit per feature/fix** that include
* changes to patches/ or ips/
* changes to main.c
* changes to data.h
* a note in README.txt if appropriate
* an entry in changelog.txt

