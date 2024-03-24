try:
    from math import log2
except:
    from math import log
    def log2(x): return log(x,2)


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

def tryint(s):
    try:
        return int(s, 0)
    except:
        return -1

def str2flag(s):
    try:
        addr,bm = s.split('&')
        assert addr[0]=='$'
        assert bm[0]=='0' and bm[1]=='x'
        addr,bm = int(addr[1:],16), int(bm,0)
        return ((addr-0x2258)<<3) + int(log2(bm))
    except (AssertionError, ValueError):
        return -1  # TODO: move catch to main

def to_provides_or_requires(s, nothing, macro):
    s = s.strip()
    if s == '':
        return nothing
    data = []
    for one in s.split(','):
        p = requirements[one.strip()]  # str -> enum
        if '*' in p:  # multiplier
            p = p.split('*')
            data.append('%s,%s' % (p[0],p[1]))
        else:
            data.append('1,%s' % (p,))
    return '%s%dN(' % (macro, len(data),) + ', '.join(data) + ')'

def to_requirements(s):
    return to_provides_or_requires(s, 'NOTHING_REQUIRED', 'REQ')

def to_provides(s):
    return to_provides_or_requires(s, 'NOTHING_PROVIDED', 'PVD')

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

def filename_to_include_guard(filename):
    import os.path
    return os.path.basename(filename).upper().replace('.', '_').encode()
