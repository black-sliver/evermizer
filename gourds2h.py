#!/usr/bin/python

from sys import argv, version_info
import os.path
import csv
import codecs
try:
    from math import log2
except:
    from math import log
    def log2(x): return log(x,2)


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

def romaddr(addr):
    #if addr>0x800000: return addr-0x800000
    addr &= ~(0xc00000);
    return addr

def rom2scriptaddr(romaddr):
    romaddr &= ~(0x8000);
    romaddr -= 0x920000;
    return (romaddr&0x007fff) + ((romaddr&0x1ff0000)>>1);

def word2bytes(word):
    assert(word<=0xffff and word>=0x0000)
    return [(word>>0)&0xff, (word>>8)&0xff]

def bin2str(code):
    return ''.join( ( '\\x%02x' % (b,) for b in code ) )
    
def gen_gourd_drop(addr, d):
    # [ prize, amount, extra+ before, extra+ after, provides, name ]
    code = []
    if d[0]>=0: code += [ 0x17 ] + word2bytes(0x2391-0x2258) + word2bytes(d[0]) # prize
    if d[1]>=0: code += [ 0x17 ] + word2bytes(0x2393-0x2258) + word2bytes(d[1]) # amount (money)
    if d[2]>=0: code += [ 0x17 ] + word2bytes(0x2461-0x2258) + word2bytes(d[2]) # add (items) <- this is "add before loot", which is just amount
    if d[3]>=0: code += [ 0x17 ] + word2bytes(0x2461-0x2258) + word2bytes(d[3]) # add (items) <- this is "add after loot", which is probably a bug in vanilla
    code += [ 0x00 ] # end
    return addr+len(code), b'{0x%06x, %2d, "%s", 0x%06x, "%s", %s}' % (romaddr(addr), len(code), bin2str(code).encode(), rom2scriptaddr(addr), d[5].encode(), b'true' if d[6] else b'false')
    
def gen_gourd_data(g, short=False):
    # [ mapref, v2397, v2399, lootscript, startaddr, endaddr, missable, requires, name, flagval, special ]
    code = []
    if 'needs if' in g[10]:
        code += [ 0x08, 0x85 ] + word2bytes(g[9]) + word2bytes(g[5]-g[4]-6) # IF flag set skip
    callpos = len(code)
    code += [ 0x29, 0x00, 0x00, 0x00 ] # call drop data
    if g[0]>=0: # has mapref
        code += [ 0x17 ] + word2bytes(0x2395-0x2258) + word2bytes(g[0]) # write mapref fast
    if g[1]>=0: # has v2397
        code += [ 0x17 ] + word2bytes(0x2397-0x2258) + word2bytes(g[1]) # write v2397 fast
    if g[2]>=0 and g[2]<=0x0f and short: # has v2399 and size-optimize
        code += [ 0x18 ] + word2bytes(0x2399-0x2258) + [ 0xb0+g[2] ] # write v2399 slow
    elif g[2]>=0: # has v2399
        code += [ 0x17 ] + word2bytes(0x2399-0x2258) + word2bytes(g[2]) # write v2399 fast
    code += [ 0xa3, g[3] ] # call loot script
    if 'needs if' in g[10]:
        code += [ 0x0c ] + word2bytes(g[9]) + [ 0x85, 0x90, 0x04 ] # set flag if ($22ea & 0x01) else clear flag
    # TODO: handle next extra+
    blank = g[5]-g[4]-len(code) # amount of remaining bytes
    if blank<3:
        for i in range(0, blank):
            code += [ 0x4d ] # insert nops
    else:
        blank -= 3
        code += [ 0x04 ] + word2bytes(blank) # insert unconditional branch
        #for i in range(0, blank): code += [ 0x00 ] # clear remainder
        if blank>0: code += [ 0x00 ]
    if len(code)>g[5]-g[4]:
        if short:
            print('Generated code "%s" does not fit into space (%d>%d)' % (bin2str(code), len(code), g[5]-g[4]))
        else:
            return gen_gourd_data(g, True)
    assert(len(code)<=g[5]-g[4])
    return b'{0x%06x, %2d, "%s", 0x%06x, "%s"}' % (g[4], len(code), bin2str(code).encode(), g[4]+callpos+1, g[8].encode())

def tryint(s):
    try:
        return int(s, 0)
    except:
        return -1

def str2flag(s):
    try:
        addr,bm = s.split('&')
        assert(addr[0]=='$')
        assert(bm[0]=='0' and bm[1]=='x')
        addr,bm = int(addr[1:],16),int(bm,0)
        return ((addr-0x2258)<<3) + int(log2(bm))
    except:
        return -1 # TODO: move catch to main

prizes = {
    -1: 'invalid',
    0x0:'nothing', 0x1:'moniez',
    0x101:'chocobo egg',
    0x200:'wax', 0x201:'water', 0x202:'vinegar', 0x203:'root', 0x204:'oil',
    0x205:'mud pepper', 0x206:'mushroom', 0x207:'meteorite', 0x208:'limestone',
    0x209:'iron', 0x20a:'gunpowder', 0x20b:'grease', 0x20c:'feather',
    0x20d:'ethanol', 0x20e:'dry ice', 0x20f:'crystal', 0x210:'clay',
    0x211:'brimstone', 0x212:'bone', 0x213:'atlas medallion', 0x214:'ash',
    0x215:'acorn',
    0x401:'grass vest', 0x408:'gold-plated vest', 0x40b:'titanium vest',
    0x40e:'shell hat', 0x417:'old reliable',
    0x41a:'mammoth guard', 0x41f:'iron bracer', 0x423:'protector ring',
    0x800:'petal', 0x801:'nectar', 0x802:'honey', 0x803:'biscuit',
    0x804:'wings', 0x805:'herb', 0x806:'pixie dust', 0x807:'call bead',
    # rando-only:
    0x117:'knight basher', 0x118:'atom smasher', 0x11c:'laser lance',
    0x125:'gauge',
    0x128:'energy core',
    0x131:'amulet of annihilation',   
    0x141:'regenerate', 0x142:'aura',
    0x1e1:'thunderball', 0x1e2:'particle bomb', 0x1e3:'cryo blast',
    0x1f1:'colosseum vest', 0x1f2:'colosseum gloves', 0x1f3:'colosseum helmet',
    0x1f4:'progressive vest', 0x1f5:'progressive gloves', 0x1f6:'progressive helmet',
}

requirements = {
    'weapon': 'P_WEAPON',
    'non-sword': 'P_NON_SWORD',
    'levitate': 'P_LEVITATE',
    'bronze axe+': 'P_BRONZE_AXE_PLUS',
    'real bronze axe+': 'P_REAL_BRONZE_AXE_PLUS',  # not circumvented by OoB
    'bronze axe': 'P_BRONZE_AXE',
    'bronze spear+': 'P_BRONZE_SPEAR_PLUS',
    'aegis': 'P_AEGIS_DEAD',
    'pyramid': 'P_PYRAMID_ACCESSIBLE',
    'knight basher': 'P_KNIGHT_BASHER',
    'knight basher+': 'P_KNIGHT_BASHER_PLUS',
    'jaguar ring': 'P_JAGUAR_RING',
    # 'queen\'s key': 'P_QUEENS_KEY',  # replaced for OoB
    'revealer': 'P_REVEALER',
    'rocket': 'P_ROCKET',
    'gauge': 'P_GAUGE',
    'energy core': 'P_ENERGY_CORE',
    'volcano entered': 'P_VOLCANO_ENTERED',
    'act1 weapon': 'P_ACT1_WEAPON',
    'act2 weapon': 'P_ACT2_WEAPON',
    'act3 weapon': 'P_ACT3_WEAPON',
    'act4 weapon': 'P_ACT4_WEAPON',
    'armor': 'P_ARMOR',
    'ammo': 'P_AMMO',
    'glitched ammo': 'P_GLITCHED_AMMO',
    'call bead': 'P_CALLBEAD',
    '2x call bead': '2*P_CALLBEAD',
    '3x call bead': '3*P_CALLBEAD',
    'wings': 'P_WINGS',
    'gauge access': 'P_GAUGE_ACCESS',
    'queen\'s key chests access': 'P_QUEENS_KEY_CHESTS_ACCESS',
    'east crustacia chests access': 'P_ECRUSTACIA_CHESTS_ACCESS',
    'oglin cave access': 'P_OGLIN_CAVE_ACCESS',
    'energy core access': 'P_ENERGY_CORE_ACCESS',
}

difficulty_modifiers = {
    'dumb':   1,
    'far':    1,
    'hidden': 2,
    'early': -1,
}

def to_provides_or_requires(s, nothing, macro):
    s = s.strip()
    if s == '': return nothing
    ss = s.split(',')
    data = []
    for c in ss:
        p = requirements[c.strip()]
        if '*' in p:
            p = p.split('*')
            data.append('%s,%s' % (p[0],p[1]))
        else:
            data.append('1,%s' % (p,))
    #while len(data)<4:
    #    data.append('{0,P_NONE}')
    return '%s%dN(' % (macro, len(data),) + ', '.join(data) + ')'

def to_requirements(s):
    return to_provides_or_requires(s, 'NOTHING_REQUIRED', 'REQ')

def to_provides(s):
    return to_provides_or_requires(s, 'NOTHING_PROVIDED', 'PVD')

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
        if version_info[0]==2: # sadly py2 CSV is bytes, py3 CSV is unicode
            src = fin.read().encode('ascii', 'replace').replace('\r\n', '\n')
        else:
            src = fin.read().replace('\r\n', '\n')
        data = csv.reader(src.split('\n'))
        next(data) # skip header
        rownr=1
        checknr=0
        flagvals = []
        locations = []
        drops = []
        for row in data:
            rownr+=1
            # since the csv has more data than required, we try to verify stuff
            if r'unreachable' in row[17].lower():
                continue
            try:
                csv_checknr = int(row[0])
                assert(csv_checknr == checknr)
                checknr += 1
            except Exception as ex:
                print('Bad check number in row %d: "%s"' % (rownr,row[0]))
                raise ex
            if not tryint(row[6]) in prizes:
                print('Bad item id in row %d: "%s"' % (rownr, row[6]))
            elif not prizes[tryint(row[6])].lower() in row[7].lower():
                print('Bad item in row %d: "%s" ^= "%s" != "%s"' % (rownr,
                    row[6], prizes[tryint(row[6])], row[7].lower()))
            elif tryint(row[6])==0x1 and tryint(row[8])<1:
                print('Missing amount for money in row %d' % (rownr,))
            elif tryint(row[6])==0x1 and tryint(row[9])>0:
                print('Extra amount does not apply to money in row %d' % (rownr,))
            elif tryint(row[6])!=0x1 and tryint(row[8])>0:
                print('Amount only applies to money in row %d' % (rownr,))
            else:
                startaddr,endaddr = romaddr(tryint(row[14])),romaddr(tryint(row[15]))
                mapref,v2397,v2399 = tryint(row[4]),tryint(row[11]),tryint(row[12])
                hasv239x = v2397>=0 or v2399>=0
                altloot = tryint(row[13])>0
                missable = tryint(row[18])>0 or (tryint(row[18])<0 and row[18]==r'missable')
                requires = to_requirements(row[17])
                provides = to_provides(row[19])
                lootscript = tryint(row[13]) if altloot else 0x3a
                flagval = str2flag(row[5])
                locname = row[1]
                itemname = row[7]
                spoiler = tryint(row[6])<0x200 or tryint(row[6])>0x2ff # skip spoiler log for ingredients
                if tryint(itemname.split(' ')[0]) < 1: # add count if not included in text
                    if tryint(row[8]) > 1:
                        itemname = str(tryint(row[8])) + ' ' + itemname
                    elif tryint(row[9]) > 0:
                        itemname = str(tryint(row[9]) + 1) + ' ' + itemname
                special = row[16].lower() # special instructions for code generation
                difficulty = 0
                try: difficulty = difficulty_modifiers[row[20].lower()]
                except: pass # used for cyberscore
                if startaddr<1 or endaddr<1:
                    print('Bad address in row %d' % (rownr,))
                    exit(1)
                #elif endaddr-startaddr<4 or (mapref>=0 and endaddr-startaddr<9) or (hasv239x and endaddr-startaddr<19): # <- call loot script in drop
                elif endaddr-startaddr<6 or (mapref>=0 and endaddr-startaddr<11) or (hasv239x and endaddr-startaddr<21): # <- call loot script in trigger
                    print('Bad address span in row %d' % (rownr,))
                    exit(1)
                elif rom and rom[endaddr] != 0x0c and rom[endaddr] != 0x00: # note 00 is only valid if "needs IF" is set
                    print('End address is not INSTR 0c or 00 in row %d' % (rownr,))
                    exit(1)
                elif rom and rom[startaddr] not in (0x17,0x18,0x08,0x09,0xbc) and itemname != 'Energy Core':
                    # NOTE: energy core is not a valid chest
                    print('Start address is not WRITE or bc for %s in row %d' % (itemname, rownr,)) # note bc is only valid if "needs IF" is set
                    exit(1)
                #elif rom and rom[startaddr] in (0x08,0x09) and rom[startaddr+1] == flagval&0xff and rom[startaddr+2] == flagval>>8:
                #    print('Start address is flag test in row %d' % (rownr,))
                #    exit(1)
                elif flagval<0:
                    print('No such flag "%s" in row %d' % (row[5], rownr))
                    exit(1)
                elif flagval in flagvals:
                    print('Duplicate flag "%s" in row %d' % (row[5], rownr))
                    exit(1)
                else: # ok, add to list
                    flagvals.append(flagval)
                    locations.append([mapref,v2397,v2399,lootscript,startaddr,endaddr,missable,requires,locname,flagval,special,difficulty])
                    drops.append([tryint(row[6]),tryint(row[8]),tryint(row[9]),tryint(row[10]), provides, itemname, spoiler])
        print('%d locations and %d drops loaded from %d rows' % (len(locations),len(drops),rownr,))
        #from pprint import pprint
        #pprint(locations)
        #pprint(drops)
        with open(dst_filename, 'wb') as fout:
            fout.write(b'#if defined CHECK_TREE\n')
            for i in range(0, len(locations)):
                fout.write(b'    {0, CHECK_GOURD,%3d, %d, %2d, %-49s NOTHING_PROVIDED},\n' % (i, 1 if locations[i][6] else 0, locations[i][11], (locations[i][7]+',').encode()))
            fout.write(b'#elif defined DROP_TREE\n')
            for i in range(0, len(drops)):
                if drops[i][4] == 'NOTHING_PROVIDED': continue
                fout.write(b'    {CHECK_GOURD,%3d, %s},\n' % (i, (drops[i][4]).encode()))
            fout.write(b'#elif !defined GOURDS_H_INCLUDED\n')
            fout.write(b'#define GOURDS_H_INCLUDED\n')
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
            fout.write(b'#endif\n')

if __name__ == '__main__':
    # TODO: rewrite with argparse
    if len(argv)<3 or len(argv)>4:
        print('Usage: gourds2h.py <output.h> <input.csv> [<rom.sfc to verify against>]')
        exit(1)
    dst = os.path.splitext(argv[1])[0]+'.h' # FIXME: do we still need the splitext?
    src = argv[2]
    rom = argv[3] if len(argv)>3 else None
    main(dst, src, rom)
