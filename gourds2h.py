#!/usr/bin/python

from sys import argv, version_info
import os.path
import csv
import codecs

from util import (tryint, str2flag, prizes, difficulty_modifiers, to_requirements, to_provides, romaddr, rom2scriptaddr,
                  word2bytes, bin2str, filename_to_include_guard)


DROP_SCRIPTS_START = 0xB18000

gourd_drop_struct = b"""struct gourd_drop_item {
    size_t pos; // script position
    size_t len; // script length
    const char* data; // script instructions
    uint32_t call_addr; // 24bit value to put into call for this drop
    const char* name; // name for spoiler log
    bool spoiler; // include in spoiler log
};
"""

gourd_data_struct = b"""struct gourd_data_item {
    size_t pos; // script address
    size_t len; // script length
    const char* data; // script instructions
    size_t call_target_addr; // address to patch in 24bit call dest
    const char* name; // name for spoiler log
};
"""

def gen_gourd_drop(addr, d):
    # [ prize, amount, extra+ before, extra+ after, provides, name ]
    code = []
    if d[0] >= 0:  # prize
        code += [0x17] + word2bytes(0x2391-0x2258) + word2bytes(d[0])
    if d[1] >=0:  # amount (money)
        code += [0x17] + word2bytes(0x2393-0x2258) + word2bytes(d[1])
    if d[2] >= 0:  # add (items) <- this is "add before loot", which is just amount
        code += [0x17] + word2bytes(0x2461-0x2258) + word2bytes(d[2])
    if d[3] >= 0:  # add (items) <- this is "add after loot", which is probably a bug in vanilla
        code += [0x17] + word2bytes(0x2461-0x2258) + word2bytes(d[3])
    code += [0x00] # end
    return (addr + len(code),
            b'{0x%06x, %2d, "%s", 0x%06x, "%s", %s}' % (
                romaddr(addr), len(code), bin2str(code).encode(), rom2scriptaddr(addr), d[5].encode(),
                b'true' if d[6] else b'false'))
    
def gen_gourd_data(g, short=False):
    # [ mapref, v2397, v2399, lootscript, startaddr, endaddr, missable, requires, name, flagval, special ]
    code = []
    if 'needs if' in g[10]:
        code += [0x08, 0x85] + word2bytes(g[9]) + word2bytes(g[5] - g[4] - 6)  # IF flag set skip
    callpos = len(code)
    code += [0x29, 0x00, 0x00, 0x00]  # call drop data
    if g[0] >= 0:  # has mapref
        code += [0x17] + word2bytes(0x2395-0x2258) + word2bytes(g[0])  # write mapref fast
    if g[1] >= 0:  # has v2397
        code += [0x17] + word2bytes(0x2397-0x2258) + word2bytes(g[1])  # write v2397 fast
    if g[2] >= 0 and g[2]<=0x0f and short:  # has v2399 and optimize for size
        code += [0x18] + word2bytes(0x2399-0x2258) + [0xb0 + g[2]]  # write v2399 slow
    elif g[2] >= 0:  # has v2399
        code += [0x17] + word2bytes(0x2399-0x2258) + word2bytes(g[2])  # write v2399 fast
    code += [0xa3, g[3]]  # call loot script
    if 'needs if' in g[10]:
        code += [0x0c] + word2bytes(g[9]) + [0x85, 0x90, 0x04]  # set flag if ($22ea & 0x01) else clear flag
    # TODO: handle next extra+
    blank = g[5] - g[4] - len(code)  # amount of remaining bytes
    if blank < 3:
        for i in range(0, blank):
            code += [0x4d]  # insert nops
    else:
        blank -= 3
        code += [0x04] + word2bytes(blank)  # insert unconditional branch
        if blank > 0:  # set first skipped/removed byte to 0 to make code easier to read
            code += [0x00]
    if len(code) > g[5] - g[4]:
        if short:
            print('Generated code "%s" does not fit into space (%d > %d)' % (bin2str(code), len(code), g[5] - g[4]))
        else:
            return gen_gourd_data(g, True)
    assert len(code) <= g[5] - g[4]
    return b'{0x%06x, %2d, "%s", 0x%06x, "%s"}' % (
        g[4], len(code), bin2str(code).encode(), g[4]+callpos+1, g[8].encode())

def main(dst_filename, src_filename, rom_filename=None):
    rom = None
    name = ''
    src = ''

    if rom_filename:
        with open(rom_filename, 'rb') as f:
            rom = bytearray(f.read())
            if len(rom)>3*1024*1024+512 or len(rom)<3*1024*1024:
                rom = None
                print('Invalid ROM')
            elif len(rom)>3*1024*1024:
                rom = rom[len(rom)-3*1024*1024:]
            if rom:
                name = rom[0xFFC0:0xFFC0+20].decode('ASCII')
                print('ROM loaded: %s' % (name,))

    with codecs.open(src_filename, 'r', encoding='utf-8') as fin:
        if version_info[0] == 2:  # sadly py2 CSV is bytes, py3 CSV is unicode
            src = fin.read().encode('ascii', 'replace').replace('\r\n', '\n')
        else:
            src = fin.read().replace('\r\n', '\n')
        data = csv.reader(src.split('\n'))
        next(data)  # skip header
        rownr = 1
        checknr = 0
        flagvals = []
        locations = []
        drops = []
        for row in data:
            rownr += 1
            # since the csv has more data than required, we try to verify stuff
            if r'unreachable' in row[17].lower():
                continue
            try:
                csv_checknr = int(row[0])
                assert csv_checknr == checknr
                checknr += 1
            except Exception:
                print('Bad check number in row %d: "%s"' % (rownr, row[0]))
                raise
            if not tryint(row[6]) in prizes:
                print('Bad item id in row %d: "%s"' % (rownr, row[6]))
            elif not prizes[tryint(row[6])].lower() in row[7].lower():
                print('Bad item in row %d: "%s" ^= "%s" != "%s"' % (
                    rownr, row[6], prizes[tryint(row[6])], row[7].lower()))
            elif tryint(row[6]) == 0x1 and tryint(row[8]) < 1:
                print('Missing amount for money in row %d' % (rownr,))
            elif tryint(row[6]) == 0x1 and tryint(row[9]) > 0:
                print('Extra amount does not apply to money in row %d' % (rownr,))
            elif tryint(row[6]) != 0x1 and tryint(row[8]) > 0:
                print('Amount only applies to money in row %d' % (rownr,))
            else:
                startaddr, endaddr = romaddr(tryint(row[14])), romaddr(tryint(row[15]))
                mapref, v2397, v2399 = tryint(row[4]), tryint(row[11]), tryint(row[12])
                hasv239x = v2397 >= 0 or v2399 >= 0
                altloot = tryint(row[13]) > 0
                missable = tryint(row[18]) > 0 or row[18] == r'missable'
                requires = to_requirements(row[17])
                provides = to_provides(row[19])
                lootscript = tryint(row[13]) if altloot else 0x3a
                flagval = str2flag(row[5])
                locname = row[1]
                itemname = row[7]
                spoiler = tryint(row[6]) < 0x200 or tryint(row[6]) > 0x2ff  # skip spoiler log for ingredients
                if tryint(itemname.split(' ')[0]) < 1:  # add count if not included in text
                    if tryint(row[8]) > 1:
                        itemname = str(tryint(row[8])) + ' ' + itemname
                    elif tryint(row[9]) > 0:
                        itemname = str(tryint(row[9]) + 1) + ' ' + itemname
                special = row[16].lower()  # special instructions for code generation
                try:
                    difficulty = difficulty_modifiers[row[20].lower()]  # used for cyberscore
                except KeyError:
                    difficulty = 0

                if startaddr < 1 or endaddr < 1:
                    print('Bad address in row %d' % (rownr,))
                    exit(1)
                if (endaddr - startaddr < 6 or
                      (mapref >= 0 and endaddr - startaddr < 11) or
                      (hasv239x and endaddr - startaddr < 21)):
                    print('Bad address span in row %d' % (rownr,))
                    exit(1)
                if rom and rom[endaddr] != 0x0c and rom[endaddr] != 0x00:
                    # NOTE: 00 is only valid for "needs IF", but we don't check that here
                    print('End address is not INSTR 0c or 00 in row %d' % (rownr,))
                    exit(1)
                if rom and rom[startaddr] not in (0x17, 0x18, 0x08, 0x09, 0xbc) and itemname != 'Energy Core':
                    # NOTE: energy core is not a valid chest
                    # NOTE: bc is only valid for "needs IF", but we don't check that here
                    print('Start address is not WRITE or bc for %s in row %d' % (itemname, rownr,))
                    exit(1)
                if flagval < 0:
                    print('No such flag "%s" in row %d' % (row[5], rownr))
                    exit(1)
                if flagval in flagvals:
                    print('Duplicate flag "%s" in row %d' % (row[5], rownr))
                    exit(1)

                # ok, add to list
                flagvals.append(flagval)
                locations.append([mapref, v2397, v2399, lootscript, startaddr, endaddr, missable, requires, locname,
                                  flagval, special, difficulty])
                drops.append([tryint(row[6]), tryint(row[8]), tryint(row[9]), tryint(row[10]), provides, itemname,
                              spoiler])

        print('%d locations and %d drops loaded from %d rows' % (len(locations), len(drops), rownr,))
        include_guard = filename_to_include_guard(dst_filename)
        with open(dst_filename, 'wb') as fout:
            fout.write(b'#if defined CHECK_TREE\n')
            for i in range(0, len(locations)):
                fout.write(b'    {0, CHECK_GOURD,%3d, %d, %2d, %-49s NOTHING_PROVIDED},\n' % (
                    i, 1 if locations[i][6] else 0, locations[i][11], (locations[i][7]+',').encode()))
            fout.write(b'#elif defined DROP_TREE\n')
            for i in range(0, len(drops)):
                if drops[i][4] == 'NOTHING_PROVIDED':
                    continue
                fout.write(b'    {CHECK_GOURD,%3d, %s},\n' % (i, (drops[i][4]).encode()))
            fout.write(b'#elif !defined %s\n' % include_guard)
            fout.write(b'#define %s\n' % include_guard)
            fout.write(b'#include <stdbool.h>\n')
            fout.write(b'#include <stdint.h>\n')
            fout.write(b'#include "util.h"\n')
            fout.write(gourd_data_struct)
            fout.write(gourd_drop_struct)
            fout.write(b'const static struct gourd_data_item gourd_data[] = {\n')
            for i in range(0, len(locations)):
                try:
                    fout.write(b'    %s,\n' % (gen_gourd_data(locations[i]),))
                except:
                    import traceback
                    print('Error generating gourd data for: ')
                    from pprint import pprint
                    pprint(locations[i])
                    traceback.print_stack()
            fout.write(b'};\n')
            fout.write(b'const static struct gourd_drop_item gourd_drops_data[] = {\n')
            addr = DROP_SCRIPTS_START
            for i in range(0, len(drops)):
                addr, drop = gen_gourd_drop(addr, drops[i])
                fout.write(b'    %s,\n' % (drop,))
            fout.write(b'};\n')
            fout.write(b'_Static_assert(ARRAY_SIZE(gourd_data) == ARRAY_SIZE(gourd_drops_data), "Bad gourd data");\n')
            fout.write(b'#endif // %s\n' % (include_guard,))

if __name__ == '__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("output", metavar="output.h")
    parser.add_argument("input", metavar="input.csv")
    parser.add_argument("rom", metavar="rom.csv", nargs='?', help="for verification only")
    args = parser.parse_args()
    main(args.output, args.input, args.rom)
