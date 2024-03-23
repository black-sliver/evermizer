#!/usr/bin/python

from sys import argv, version_info
import os.path
import csv
import codecs

from util import (tryint, str2flag, prizes, difficulty_modifiers, to_requirements, to_provides, romaddr, rom2scriptaddr,
                  word2bytes, bin2str, filename_to_include_guard)


DROP_SCRIPTS_START = 0xB18000

sniff_data_struct = b"""struct sniff_data_item {
    size_t call_pos; // position to put the call to setup script in (pool)
    uint8_t call_length; // length of the instruction to replace by call
    uint16_t item; // vanilla item ID
    size_t item_pos; // position to put the item id in (shuffle)
    bool missable;
    bool excluded;
    const char* location_name; // name for spoiler log
};
"""

def gen_sniff_data(drop, loc):
    # [ prize, amount, _, _, provides, name ]
    # [ _, _, _, _, startaddr, endaddr, missable, requires, name, flagval, special ]
    return b'{0x%06x, %d, 0x%03x, 0x%06x, %s, %s, "%s"}' % (
        loc[4], loc[5] - loc[4], drop[0], loc[5] - 2, b'true' if loc[6] else b'false', b'true' if loc[10] else b'false',
        loc[8].encode())

def main(dst_filename, src_filename, rom_filename=None):
    # sniff spots are a lot simpler than gourds because we never want to rewrite them,
    # only either replace a 16bit int, or replace a single write by a single call.
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
            if not row:
                continue
            # since the csv has more data than required, we try to verify stuff
            # NOTE: opposed to gourds, we keep unreachable in here and mark them as such to keep the numbering
            #       additionally, there are also unreachable and respawning locations that need to be expluded
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
            else:
                startaddr, endaddr = romaddr(tryint(row[14])), romaddr(tryint(row[15]))
                missable = tryint(row[18]) > 0 or row[18] == r'missable'
                requires = to_requirements(row[17])
                provides = to_provides(row[19])
                flagval = str2flag(row[5])
                locname = row[1]
                itemname = row[7]
                spoiler = tryint(row[6]) < 0x200 or tryint(row[6]) > 0x2ff  # skip spoiler log for ingredients
                if tryint(itemname.split(' ')[0]) < 1:  # add count if not included in text
                    if tryint(row[8]) > 1:
                        itemname = str(tryint(row[8])) + ' ' + itemname
                    elif tryint(row[9]) > 0:
                        itemname = str(tryint(row[9]) + 1) + ' ' + itemname
                special = row[16].lower()  # special instructions
                missable = missable or (special == "unreachable")
                try:
                    difficulty = difficulty_modifiers[row[20].lower()]  # used for cyberscore
                except KeyError:
                    if row[20]:
                        raise
                    difficulty = 0

                if startaddr < 1 or endaddr < 1:
                    print('Bad address in row %d' % (rownr,))
                    exit(1)
                if endaddr - startaddr < 4:
                    print('Bad address span in row %d' % (rownr,))
                    exit(1)
                if rom and rom[endaddr] not in (0x17, 0xa3):
                    # NOTE: for sniff, endaddr points to mapref assignment, not end, since we keep most of the script
                    print('End address is %02x, not INSTR 17 or a3 in row %d' % (rom[endaddr], rownr,))
                    exit(1)
                if rom and rom[startaddr] not in (0x17, 0x18, 0x08, 0x09, 0xbc) and itemname != 'Energy Core':
                    # NOTE: energy core is not a valid chest
                    # NOTE: bc is only valid for "needs IF", but we don't check that here
                    print('Start address is not WRITE or bc for %s in row %d' % (itemname, rownr,))
                    exit(1)
                if flagval < 0:
                    print('No such flag "%s" in row %d' % (row[5], rownr))
                    exit(1)
                if flagval in flagvals and special != 'respawns':  # respawning ones need special handling anyway
                    print('Duplicate flag "%s" in row %d' % (row[5], rownr))
                    exit(1)

                # ok, add to list
                flagvals.append(flagval)
                locations.append([None, None, None, None, startaddr, endaddr, missable, requires, locname,
                                  None, special, difficulty])
                drops.append([tryint(row[6]), tryint(row[8]), tryint(row[9]), tryint(row[10]), provides, itemname,
                              spoiler])

        print('%d locations and %d drops loaded from %d rows' % (len(locations), len(drops), rownr,))
        include_guard = filename_to_include_guard(dst_filename)
        with open(dst_filename, 'wb') as fout:
            fout.write(b'#if defined CHECK_TREE\n')
            for i in range(0, len(locations)):
                fout.write(b'    {0, CHECK_SNIFF,%3d, %d, %2d, %-49s NOTHING_PROVIDED},\n' % (
                    i, 1 if locations[i][6] else 0, locations[i][11], (locations[i][7]+',').encode()))
            fout.write(b'#elif defined DROP_TREE\n')
            for i in range(0, len(drops)):
                if drops[i][4] == 'NOTHING_PROVIDED':
                    continue
                assert False, "Vanilla should not have progression in sniff spots"
                fout.write(b'    {CHECK_GOURD,%3d, %s},\n' % (i, (drops[i][4]).encode()))
            fout.write(b'#elif !defined %s\n' % (include_guard,))
            fout.write(b'#define %s\n' % (include_guard,))
            fout.write(sniff_data_struct)
            fout.write(b'const static struct sniff_data_item sniff_data[] = {\n')
            for i in range(0, len(locations)):
                try:
                    fout.write(b'    %s,\n' % (gen_sniff_data(drops[i], locations[i]),))
                except:
                    print('Error generating sniff data for: ')
                    from pprint import pprint
                    pprint(drops[i])
                    pprint(locations[i])
                    raise
            fout.write(b'};\n')
            fout.write(b'#endif // %s\n' % (include_guard,))

if __name__ == '__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("output", metavar="output.h")
    parser.add_argument("input", metavar="input.csv")
    parser.add_argument("rom", metavar="rom.csv", nargs='?', help="for verification only")
    args = parser.parse_args()
    main(args.output, args.input, args.rom)
