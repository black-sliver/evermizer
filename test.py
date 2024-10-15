#!/usr/bin/python

from __future__ import print_function

import os
from sys import argv, stderr, stdout
import time
import binascii
import multiprocessing
from itertools import islice
from subprocess import Popen, PIPE
try: # python3
    from subprocess import DEVNULL
    proc_wait_has_timeout = True
except: # python2
    DEVNULL = open(os.devnull, 'w')
    proc_wait_has_timeout = False
try: # python3
    from tempfile import TemporaryDirectory
except: # python2
    from tempfile import mkdtemp
    from shutil import rmtree
    class TemporaryDirectory(object):
        def __init__(self, suffix="", prefix="tmp", dir=None):
            self._closed = False
            self.name = None # Handle mkdtemp raising an exception
            self.name = mkdtemp(suffix, prefix, dir)

        def __enter__(self):
            return self.name

        def __exit__(self, exc, value, tb):
            if self.name and not self._closed:
                try:
                    rmtree(self.name)
                except (TypeError, AttributeError) as ex:
                    return
                self._closed = True

        def __del__(self):
            self.__exit__(None,None,None) # this may fail during exit

has_waitpid = 'posix' in os.name

SPELLS = [
'Acid Rain','Atlas','Barrier','Call Up','Corrosion','Crush','Cure','Defend',
'Double Drain','Drain','Energize','Escape','Explosion','Fireball','Fire Power',
'Flash','Force Field','Hard Ball','Heal','Lance','Levitate','Lightning Storm',
'Miracle Cure','Nitro','One Up','Reflect','Regrowth','Revealer','Revive',
'Slow Burn','Speed','Sting','Stop','Super Heal'
]
BOSS_DROPS = [
'Diamond Eye','Wheel','Crusader Sword','Spider Claw','Bronze Axe','Horn Spear',
'Bronze Spear','Lance (Weapon)','Mud Pepper','Honey','Nothing',
'10,000 Gold Coins','Progressive vest',
'Gladiator Sword','Bazooka'
]
GOURD_DROPS = [
'Gauge','Knight Basher','Atom Smasher','Laser Lance',
#'30 Thunderballs','30 Particle Bombs or Nectar?','30 Cryo Blast',
'Progressive gloves','Progressive helmet','Progressive vest',
'Aura','Regenerate','Energy Core'
]
ALL_KEY_ITEMS_B = [
b'Wheel', b'Gauge', b'Diamond Eye', b'Energy Core',
b'Levitate', b'Revealer',
b'Gladiator Sword', b'Crusader Sword',
b'Spider Claw', b'Bronze Axe', b'Knight Basher', b'Atom Smasher',
b'Horn Spear', b'Bronze Spear', b'Lance (Weapon)', b'Laser Lance'
]
PRE_ACT4_DROPS = [ 'Gauge', 'Wheel' ]
SPELL_IDS = { name.encode('ascii'): index for (index,name) in enumerate(SPELLS) }
BOSS_DROP_IDS = { name.encode('ascii'): index for (index,name) in enumerate(BOSS_DROPS) }
GOURD_DROP_IDS = { name.encode('ascii'): index for (index,name) in enumerate(GOURD_DROPS) }
ACT4_GOURDS = range(274,285+1)
ACT4_ALCHEMY = [ SPELL_IDS[b'Call Up'], SPELL_IDS[b'Energize'],
                 SPELL_IDS[b'Force Field'], SPELL_IDS[b'Nitro'],
                 SPELL_IDS[b'Reflect'], SPELL_IDS[b'Stop']]
HIDDEN_GOURDS = [ 46, 75, 90, 91, 92, 93, 94, 96, 97, 98, 99,100,101,102,107,
                 108,118,162,163,164,170,178,190,191,192,193,194,199,200,201,
                 202,203,204,205,216,218,265,266,267,268,269,270] # from CSV
EARLY_GOURDS = [  0,  1,  4,  9, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 43, 47,
                 48, 49, 50, 51, 52,184,185,186,187,188,189] # from CSV
INFREQUENT_GOURDS = [ 174,175,176,177 ] # Halls NE has the wing situation
KNIGHT_BASHER_REQUIRED_BOSSES = [ 13 ] # Mungola, once we mix pools
KNIGHT_BASHER_REQUIRED_GOURDS = [ 195,196,197,198 ]
KNIGHT_BASHER_REQUIRED_ALCHEMY = [ SPELL_IDS[b'Lightning Storm'] ]
BRONZE_AXE_REQUIRED_BOSSES = [ 7 ] # Rimsala
BRONZE_AXE_REQUIRED_GOURDS = [ 118,121,122,123,124,125,140 ] # once we mix pools
LEVITATE_REQUIRED_BOSSES = [ 12,15,16 ] # Tiny, Volcano Viper, Magmar
LEVITATE_REQUIRED_GOURDS = [ 107,108 ] + list(range(73,94+1)) # this is with fixed sequence
LEVITATE_REQUIRED_ALCHEMY = [ SPELL_IDS[b'Speed'] ]
REVEALER_REQUIRED_BOSSES = [ 6,17 ] # Megataur, Mad Monk
REVEALER_REQUIRED_GOURDS = [] # TODO
REVEALER_REQUIRED_ALCHEMY = [ SPELL_IDS[b'Fireball'] ]
UNREACHABLE_BOSS_DROPS =    [[ 'Knight Basher', KNIGHT_BASHER_REQUIRED_BOSSES ],
                             [ 'Bronze Axe', BRONZE_AXE_REQUIRED_BOSSES ],
                             [ 'Levitate', LEVITATE_REQUIRED_BOSSES ],
                             [ 'Revealer', REVEALER_REQUIRED_BOSSES ]]
UNREACHABLE_GOURD_DROPS =   [[ 'Knight Basher', KNIGHT_BASHER_REQUIRED_GOURDS ],
                             [ 'Bronze Axe', BRONZE_AXE_REQUIRED_GOURDS ],
                             [ 'Levitate', LEVITATE_REQUIRED_GOURDS ],
                             [ 'Revealer', REVEALER_REQUIRED_GOURDS ]]
UNREACHABLE_ALCHEMY_DROPS = [[ 'Knight Baser', KNIGHT_BASHER_REQUIRED_ALCHEMY ],
                             [ 'Levitate', LEVITATE_REQUIRED_ALCHEMY ],
                             [ 'Revealer', REVEALER_REQUIRED_ALCHEMY ]]


class Colors:
    PASS = '\033[92m'
    FAIL = '\033[91m'
    BOLD = '\033[1m'
    END  = '\033[0m'


class SeedGenerator(object):
    def __init__(self, cmdline, limit=100):
        self.limit = limit
        self.count = 0
        self.cmdline = cmdline
        
    def __iter__(self):
        self.count = 0
        return self
        
    def __next__(self):
        return self.next()
    def next(self):
        if self.count >= self.limit: raise StopIteration()
        self.count += 1
        seed = binascii.b2a_hex(os.urandom(8)).decode('ascii')
        return self.cmdline + [seed]


class Test(object):
    did_throw = False
    completed = False
    result = False

    def __init__(self):
        self.checks = []

    def run(self):
        raise NotImplementedError("class Test used without subclass")


class PopenTest(Test):
    capture_stderr = False
    
    def __init__(self, generator, process_done, checks, process_cleanup=None):
        super(PopenTest, self).__init__()
        self.generator = generator
        self.process_done = process_done
        self.process_cleanup = process_cleanup
        self.checks = checks

    def run(self):
        self.completed = False
        max_workers = multiprocessing.cpu_count() #+ 1
        fstderr = PIPE if self.capture_stderr else DEVNULL
        processes = (Popen(cmd, stdout=PIPE, stderr=fstderr) for cmd in self.generator)
        running = list(islice(processes, max_workers)) # initial processes
        while running and not self.completed:
            wpid,wres = (-1,None)
            try:
                if has_waitpid: wpid,wres = os.waitpid(-1,0) # wait for any child to signal
            except (ChildProcessError): pass # this happens when the process failed immediately
            for i, p in enumerate(running):
                if p.pid == wpid: res = wres
                else: res = p.poll()
                if res is not None: # process finished
                    output = p.stdout.read(-1)
                    if self.capture_stderr:
                        stderrbuf = p.stderr.read(-1)
                        if stderrbuf: print(stderrbuf)
                    try:
                        if self.process_done: self.process_done(res, output)
                    except Exception as ex:
                        self.result = False
                        self.completed = True
                        self.did_throw = True
                        print(str(ex), end=' ')
                        stdout.flush()
                        break
                    self.result = all(check(res, output) for check in self.checks)
                    self.completed = self.result
                    if self.process_cleanup: self.process_cleanup(res, output)
                    if self.completed:
                        del running[i]
                        break
                    running[i] = next(processes, None) # start next
                    if running[i] is None: # no new processes
                        del running[i]
                        break
        for i, p in enumerate(running):
            res = p.wait()
            output = p.stdout.read(-1)
            del running[i]
            if self.process_cleanup: self.process_cleanup(res, output)
        
        self.completed = True
        return self.result


class LogicError(Exception):
    pass


class EvermizerTest(PopenTest):
    def __init__(self, exe, args, checks, limit=1000):
        self.popen_count = 0
        self.popen_failed = 0
        cmdline = [exe] + args
        super(EvermizerTest,self).__init__(SeedGenerator(cmdline, limit),
                self.proc_done, checks, self.proc_cleanup)

    @staticmethod
    def log_and_rom(o):
        log = None
        rom = None
        for line in o.split(b'\n'):
            s = line.lstrip()
            if s.startswith(b'Rom saved as '):
                rom = s.rstrip()[13:-1].rstrip()
            elif s.startswith(b'Spoiler log saved as '):
                log = s.rstrip()[21:-1].rstrip()
            if log and rom: break
        return log,rom
        
    def proc_done(self, r, o):
        self.popen_count += 1
        if r != 0:
            self.popen_failed += 1
            if self.popen_failed > 2 and self.popen_count < 2*self.popen_failed:
                raise LogicError('(settings seem unbeatable)')
            return False
    
    def proc_cleanup(self, r, o):
        for fn in self.log_and_rom(o):
            if fn:
                os.unlink(fn)


class DropTest(EvermizerTest):
    """Test if all checks get all drops for given settings and difficulty"""

    def __init__(self, exe, rom, wdir, difficulty, settings='', limit=10000):
        self.difficulty = difficulty
        self.settings = settings+'l'
        self.checks = [
            self.check_spells,
            self.check_boss_drops,
            self.check_gourd_drops
        ]
        # [dim1][dim2]=n: dim1=location, dim2=drop, value=count
        # pre-allocate spells
        self.spells = [ [ 0 for _ in SPELLS ] for _ in SPELLS ]
        # create empty arrays for boss drops and gourds
        self.boss_drops = []
        self.gourd_drops = []
        super(DropTest, self).__init__(exe, ['-b', '-d', wdir, '--dry-run', rom,
                                       self.difficulty + self.settings],
                                       self.checks, limit=limit)
    
    def parse_log(self, fn):
        BLOCK_NONE, BLOCK_SPELL, BLOCK_BOSS, BLOCK_GOURD = 0, 1, 2, 3
        block = BLOCK_NONE
        p_boss_drop = 0
        p_gourd_drop = 0
        p_spell_location = 0
        with open(fn, 'rb') as f:
            for line in f:
                s = line.lstrip()
                if s.startswith(b'Spell '):
                    block = BLOCK_SPELL
                    p_spell_location = s.index(b'Location')
                elif s.startswith(b'Alchemist '):
                    block = BLOCK_NONE
                elif s.startswith(b'Boss '):
                    block = BLOCK_BOSS
                    tmp = s.split(b'Boss', 1)[1]
                    p_boss_drop = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Boss')  # left of 'Boss' + right of 'Boss' + len('Boss')
                elif s.startswith(b'Gourd '):
                    block = BLOCK_GOURD
                    tmp = s.split(b'Gourd', 1)[1]
                    p_gourd_drop = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Gourd')  # left of 'Gourd' + right of 'Gourd' + len('Gourd')
                elif not s.startswith(b'('):
                    continue # drops start with (id)
                elif block == BLOCK_SPELL:
                    s = s.split(b' ', 1)[1]
                    spell = s[:17].rstrip()
                    loc = s[p_spell_location:].rstrip()
                    n_spell = SPELL_IDS[spell]
                    n_loc = SPELL_IDS[loc]
                    self.spells[n_loc][n_spell] += 1
                elif block == BLOCK_BOSS:
                    n_loc = int(s.split(b' ', 1)[0][1:-1])
                    n_drop = BOSS_DROP_IDS[line[p_boss_drop:].rstrip()]
                    while len(self.boss_drops)<=n_loc:
                        self.boss_drops.append( [0 for _ in BOSS_DROPS ] )
                    self.boss_drops[n_loc][n_drop] += 1
                elif block == BLOCK_GOURD:
                    n_loc = int(s.split(b' ', 1)[0][1:-1])
                    s_drop = line[p_gourd_drop:].rstrip()
                    if s_drop not in GOURD_DROP_IDS: continue
                    n_drop = GOURD_DROP_IDS[s_drop]
                    while len(self.gourd_drops)<=n_loc:
                        self.gourd_drops.append( [0 for _ in GOURD_DROPS ] )
                    self.gourd_drops[n_loc][n_drop] += 1
    
    def proc_done(self, r, o):
        super(DropTest, self).proc_done(r, o)
        log, rom = self.log_and_rom(o)
        if log:
            self.parse_log(log)
    
    def check_spells(self, r=0, o=None, report=False):
        if r != 0:
            return False
        res = True
        for loc,loc_name in enumerate(SPELLS):
            for drop,drop_name in enumerate(SPELLS):
                n = self.spells[loc][drop]
                if n < 1:
                    if '4' in self.settings:
                        ignore = False
                        for name,locs in UNREACHABLE_ALCHEMY_DROPS:
                            if drop_name == name and loc in locs:
                                ignore = True
                        if ignore: continue
                    if self.difficulty in ['e'] and drop_name in ['Atlas'] and loc in ACT4_ALCHEMY:
                        continue
                    if report:
                        print('  no %s in %s' % (drop_name, loc_name))
                        res = False
                    else:
                        return False
                elif n>0 and '4' in self.settings:
                    for name,locs in UNREACHABLE_ALCHEMY_DROPS:
                        if drop_name == name and loc in locs:
                            if report:
                                print('  %2d %s in #%d' % (n, drop_name, loc_name))
                                res = False
                            else:
                                return False
        return res
        
    def check_boss_drops(self, r=0, o=None, report=False):
        if r != 0:
            return False
        res = True
        for loc,drops in enumerate(self.boss_drops):
            for drop,n in enumerate(drops):
                if n<1:
                    if '4' in self.settings:
                        ignore = False
                        for name,locs in UNREACHABLE_BOSS_DROPS:
                            if BOSS_DROPS[drop] == name and loc in locs:
                                ignore = True
                        if ignore: continue
                    if report:
                        print('  no %s in #%d' % (BOSS_DROPS[drop], loc))
                        res = False
                    else:
                        return False
                # current version does not require all bosses to be accessible
        return res
    
    def check_gourd_drops(self, r=0, o=None, report=False):
        if r != 0:
            return False
        res = True
        for drop,drop_name in enumerate(GOURD_DROPS):
            for loc,drops in enumerate(self.gourd_drops):
                n = drops[drop]
                if n<1:
                    if drop_name in PRE_ACT4_DROPS and loc in ACT4_GOURDS:
                        continue
                    if self.difficulty in ['e'] and loc in HIDDEN_GOURDS:
                        continue
                    if self.difficulty in ['n','h'] and loc in EARLY_GOURDS:
                        continue  # reduce required seeds, covered by e,x
                    if self.difficulty in ['h'] and loc in INFREQUENT_GOURDS:
                        continue  # reduce required seeds, covered by e,n,x
                    if '4' in self.settings:
                        ignore = False
                        for name,locs in UNREACHABLE_GOURD_DROPS:
                            if drop_name == name and loc in locs:
                                ignore = True
                        if ignore: continue
                    if report:
                        print('  no %s in #%d' % (drop_name, loc))
                        res = False
                    else:
                        return False
                elif n>0 and '4' in self.settings:
                    for name,locs in UNREACHABLE_GOURD_DROPS:
                        if drop_name == name and loc in locs:
                            if report:
                                print('  %2d %s in #%d' % (n, drop_name, loc))
                                res = False
                            else:
                                return False
                    
        return res


class BossOnlyTest(EvermizerTest):
    """Test if all key items are on bosses for given difficulty"""

    def __init__(self, exe, rom, wdir, difficulty, limit=10000):
        self.difficulty = difficulty
        self.settings = 'lABGO'
        self.checks = []  # we check while parsing logs
        super(BossOnlyTest, self).__init__(exe, ['-b', '-d', wdir, '--dry-run', rom,
                                           self.difficulty + self.settings],
                                           self.checks, limit=limit)

    def parse_log(self, fn):
        BLOCK_NONE, BLOCK_ALCHEMIST, BLOCK_BOSS, BLOCK_GOURD = 0, 1, 2, 3
        block = BLOCK_NONE
        p = 0
        boss_drops = []
        with open(fn, 'rb') as f:
            for line in f:
                s = line.lstrip()
                if s.startswith(b'Spell '):
                    block = BLOCK_NONE
                elif s.startswith(b'Alchemist '):
                    block = BLOCK_ALCHEMIST
                    tmp = s.split(b'Alchemist', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Alchemist')  # left of 'Alchemist' + right of 'Alchemist' + len('Alchemist')
                elif s.startswith(b'Boss '):
                    block = BLOCK_BOSS
                    tmp = s.split(b'Boss', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Boss')  # left of 'Boss' + right of 'Boss' + len('Boss')
                elif s.startswith(b'Gourd '):
                    block = BLOCK_GOURD
                    tmp = s.split(b'Gourd', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Gourd')  # left of 'Gourd' + right of 'Gourd' + len('Gourd')
                elif not s.startswith(b'('):
                    continue # drops start with (id)
                elif block == BLOCK_ALCHEMIST:
                    s_loc = line[:p].rstrip()
                    s_drop = line[p:].rstrip()
                    if s_drop in ALL_KEY_ITEMS_B:
                        raise Exception('Encountered %s at %s in %s' % (s_drop, s_loc, fn))
                elif block == BLOCK_BOSS:
                    s_drop = line[p:].rstrip()
                    boss_drops.append(s_drop)
                elif block == BLOCK_GOURD:
                    s_loc = line[:p].rstrip()
                    s_drop = line[p:].rstrip()
                    if s_drop in ALL_KEY_ITEMS_B:
                        raise Exception('Encountered %s at %s in %s' % (s_drop, s_loc, fn))

        for item in ALL_KEY_ITEMS_B:
            if item not in boss_drops:
                raise Exception('%s missing from bosses (%s) in %s' % (item, b', '.join(boss_drops), fn))

    def proc_done(self, r, o):
        super(BossOnlyTest, self).proc_done(r, o)
        log, rom = self.log_and_rom(o)
        if log:
            self.parse_log(log)


class GlitchLevitateTest(EvermizerTest):
    """Test that Levitate is considered accessible even though it's locked by levitate"""

    def __init__(self, exe, rom, wdir, difficulty, settings, limit=10000):
        self.difficulty = difficulty
        self.settings = settings
        self.checks = [
            self.check_levitate
        ]
        self.found_glitch_levitate = False
        super(GlitchLevitateTest, self).__init__(exe, ['-b', '-d', wdir, '--dry-run', rom,
                                                 self.difficulty + self.settings],
                                                 self.checks, limit=limit)

    def parse_log(self, fn):
        # TODO: unitfy parse_log
        BLOCK_NONE, BLOCK_ALCHEMIST, BLOCK_BOSS, BLOCK_GOURD = 0, 1, 2, 3
        block = BLOCK_NONE
        p = 0
        with open(fn, 'rb') as f:
            for line in f:
                s = line.lstrip()
                if s.startswith(b'Spell '):
                    block = BLOCK_NONE
                elif s.startswith(b'Alchemist '):
                    block = BLOCK_ALCHEMIST
                    tmp = s.split(b'Alchemist', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Alchemist')  # left of 'Alchemist' + right of 'Alchemist' + len('Alchemist')
                elif s.startswith(b'Boss '):
                    block = BLOCK_BOSS
                    tmp = s.split(b'Boss', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Boss')  # left of 'Boss' + right of 'Boss' + len('Boss')
                elif s.startswith(b'Gourd '):
                    block = BLOCK_GOURD
                    tmp = s.split(b'Gourd', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Gourd')  # left of 'Gourd' + right of 'Gourd' + len('Gourd')
                elif not s.startswith(b'('):
                    continue # drops start with (id)
                elif block == BLOCK_ALCHEMIST:
                    s_drop = line[p:].rstrip()
                    if s_drop == b'Levitate':
                        n_loc = int(s.split(b' ', 1)[0][1:-1])
                        if n_loc in LEVITATE_REQUIRED_ALCHEMY:
                            self.found_glitch_levitate = True
                elif block == BLOCK_BOSS:
                    s_drop = line[p:].rstrip()
                    if s_drop == b'Levitate':
                        n_loc = int(s.split(b' ', 1)[0][1:-1])
                        if n_loc in LEVITATE_REQUIRED_BOSSES:
                            self.found_glitch_levitate = True
                elif block == BLOCK_GOURD:
                    s_drop = line[p:].rstrip()
                    if s_drop == b'Levitate':
                        n_loc = int(s.split(b' ', 1)[0][1:-1])
                        if n_loc in LEVITATE_REQUIRED_GOURDS:
                            self.found_glitch_levitate = True

    def proc_done(self, r, o):
        super(GlitchLevitateTest, self).proc_done(r, o)
        log, rom = self.log_and_rom(o)
        if log:
            self.parse_log(log)

    def check_levitate(self, r=0, o=None, report=False):
        return self.found_glitch_levitate


class RockSkipLevitateTest(GlitchLevitateTest):
    """Test that all is considered accessible even though sequence breaking is required for levitate"""

    def __init__(self, exe, rom, wdir, difficulty, limit=10000):
        settings = 'r4lABGoJ'
        super(RockSkipLevitateTest, self).__init__(exe, rom, wdir, difficulty, settings, limit)


class OoBLevitateTest(GlitchLevitateTest):
    """Test that all is considered accessible even though OoB is required for levitate"""

    def __init__(self, exe, rom, wdir, difficulty, limit=10000):
        settings = 'l4ABGoU'
        super(OoBLevitateTest, self).__init__(exe, rom, wdir, difficulty, settings, limit)


class OoBGaugeTest(EvermizerTest):
    """Test that a oob-only-accessible key item is valid"""

    def __init__(self, exe, rom, wdir, difficulty, limit=10000):
        self.difficulty = difficulty
        self.settings = 'lABGoU'
        self.checks = [
            self.check_oob_required
        ]
        self.found_seed = None
        super(OoBGaugeTest, self).__init__(exe, ['-b', '-d', wdir, '--dry-run', rom,
                                           self.difficulty + self.settings],
                                           self.checks, limit=limit)

    def parse_log(self, fn):
        # TODO: unitfy parse_log
        BLOCK_NONE, BLOCK_ALCHEMIST, BLOCK_BOSS, BLOCK_GOURD = 0, 1, 2, 3
        block = BLOCK_NONE
        levitate_behind_levitate = False
        gauge_behind_levitate = False
        gauge_in_volcano = False
        p = 0
        with open(fn, 'rb') as f:
            for line in f:
                s = line.lstrip()
                if s.startswith(b'Spell '):
                    block = BLOCK_NONE
                elif s.startswith(b'Alchemist '):
                    block = BLOCK_ALCHEMIST
                    tmp = s.split(b'Alchemist', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Alchemist')  # left of 'Alchemist' + right of 'Alchemist' + len('Alchemist')
                elif s.startswith(b'Boss '):
                    block = BLOCK_BOSS
                    tmp = s.split(b'Boss', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Boss')  # left of 'Boss' + right of 'Boss' + len('Boss')
                elif s.startswith(b'Gourd '):
                    block = BLOCK_GOURD
                    tmp = s.split(b'Gourd', 1)[1]
                    p = len(line)-len(s) + len(tmp)-len(tmp.lstrip()) + len('Gourd')  # left of 'Gourd' + right of 'Gourd' + len('Gourd')
                elif not s.startswith(b'('):
                    continue # drops start with (id)
                elif block == BLOCK_ALCHEMIST:
                    n_loc = int(s.split(b' ', 1)[0][1:-1])
                    if n_loc in LEVITATE_REQUIRED_ALCHEMY:
                        s_drop = line[p:].rstrip()
                        if s_drop == b'Levitate':
                            levitate_behind_levitate = True
                        elif s_drop == b'Gauge':
                            gauge_behind_levitate = True
                elif block == BLOCK_BOSS:
                    n_loc = int(s.split(b' ', 1)[0][1:-1])
                    if n_loc in LEVITATE_REQUIRED_BOSSES:
                        s_drop = line[p:].rstrip()
                        if s_drop == b'Levitate':
                            levitate_behind_levitate = True
                        elif s_drop == b'Gauge':
                            gauge_behind_levitate = True
                elif block == BLOCK_GOURD:
                    n_loc = int(s.split(b' ', 1)[0][1:-1])
                    if n_loc in LEVITATE_REQUIRED_GOURDS:
                        s_drop = line[p:].rstrip()
                        if s_drop == b'Levitate':
                            levitate_behind_levitate = True
                        elif s_drop == b'Gauge':
                            gauge_behind_levitate = True

        if levitate_behind_levitate and gauge_behind_levitate:
            self.found_seed = fn

    def proc_done(self, r, o):
        super(OoBGaugeTest, self).proc_done(r, o)
        log, rom = self.log_and_rom(o)
        if log:
            self.parse_log(log)

    def check_oob_required(self, r=0, o=None, report=False):
        return bool(self.found_seed)


class ExecTest(Test):
    """Test if execution succeeds for given settings"""

    def __init__(self, exe, rom, wdir, args=[], difficulty='', settings='', seed=None, expect=True):
        super(ExecTest, self).__init__()
        self.expect = expect
        self.popen_count = 0
        self.popen_failed = 0
        self.checks = []
        self.cmd = [exe, '-b', '-d', wdir, '--dry-run',] + args + [
                    rom, difficulty + settings]
        if seed is not None: self.cmd.append(seed)

    def run(self, stdout=DEVNULL, stderr=DEVNULL, save_proc=False):
        proc = Popen(self.cmd, stdout=stdout, stderr=stderr)
        if save_proc:
            self.proc = proc
        self.popen_count = 1
        try:
            res = proc.wait(*[10000] if proc_wait_has_timeout else [])
        except Exception as ex:
            import traceback
            traceback.print_exc(ex)
            self.popen_failed = 1
            return False
        if res != 0:
            self.popen_failed = 1
            return not self.expect
        return self.expect


class JsonExecTest(ExecTest):
    """Test if output of an exec is valid json"""

    def run(self):
        res = super(JsonExecTest, self).run(stdout=PIPE, save_proc=True)
        if res != self.expect:
            return res
        try:
            output = self.proc.stdout.read(-1)
            import json
            json.loads(output)
        except Exception as ex:
            if ex.__class__.__name__ in ('JSONDecodeError', 'TypeError'):
                return not self.expect  # bad json
            import traceback
            traceback.print_exc(ex)
            return False  # other problem
        return self.expect


def print_usage(exe):
    print('Usage: %s [--verbose|--silent] [--more|--less] <path/to/evermizer.exe> <path/to/soe.sfc>' % (exe,))


if __name__ == '__main__':
    from time import sleep

    n = 1
    verbose = False
    silent = False
    less = False
    more = False
    while True:
        if   len(argv)-n>0 and argv[n] == '--verbose': verbose = True
        elif len(argv)-n>0 and argv[n] == '--silent': silent = True
        elif len(argv)-n>0 and argv[n] == '--more': more = True
        elif len(argv)-n>0 and argv[n] == '--less': less = True
        else: break
        n += 1
    if len(argv)-n != 2 or (silent and verbose) or (more and less):
        print_usage(argv[0])
        exit(1)
    
    exe = argv[n]
    rom = argv[n+1]
    for fn in (exe,rom):
        if not os.path.isfile(fn):
            print("No such file: %s" % (fn,), file=stderr)
            exit(1)
    if 'posix' in os.name and not os.access(exe, os.X_OK):
        print("File not executable: %s" % (exe,), file=stderr)
        exit(1)
    
    done = 0
    failed = 0
    difficulties = ['e','n','h','x'] if more else ['x'] if less else ['e','n','x']
    variations = ['','4','J','U']
    # FIXME: speed up hard seed generation so we don't have to skip it

    with TemporaryDirectory() as wdir:
        def make_exec_test(args, difficulty='e', settings='', seed='1', expect=True, cls=ExecTest):
            return cls(exe, rom, wdir, args, difficulty, settings, seed, expect)

        tests = []
        tests += [
            ['Exec Invalid', make_exec_test(['--invalid'], expect=False)],
            ['Exec Money%', make_exec_test(['--money','200'])],
            ['Exec Exep%', make_exec_test(['--exp','200'])],
            ['Exec Required Fragments', make_exec_test(['--required-fragments','99'], 'x', 'Z')],
            ['Exec Available Fragments', make_exec_test(['--available-fragments','99'], 'x', 'Z')],
            ['Exec settings.json', make_exec_test(['--settings.json'], cls=JsonExecTest)],
        ]
        tests += [
            ['Drops %s%s' % (difficulty, settings), DropTest(exe, rom, wdir, difficulty, settings)]
            for difficulty in difficulties for settings in variations
        ]
        tests += [
            ['Boss-only %s' % (difficulty,), BossOnlyTest(exe, rom, wdir, difficulty)]
            for difficulty in ['e', 'h']
        ]
        tests += [
            ['Rock skip Levitate', RockSkipLevitateTest(exe, rom, wdir, 'x')],
            ['OoB Levitate', OoBLevitateTest(exe, rom, wdir, 'x')],
            ['OoB Gauge', OoBGaugeTest(exe, rom, wdir, 'x')],
        ]
        for i, [name, test] in enumerate(tests):
            print('%sTEST%3d%s:' % (Colors.BOLD, i, Colors.END), end=' ')
            stdout.flush()
            teststart = time.time()
            passed = test.run()
            testend = time.time()
            done += 1
            if not passed: failed += 1
            if not silent:
                result = 'PASSED' if passed else 'FAILED'
                result_color = Colors.PASS if passed else Colors.FAIL
                print('%s%s%s: %s' % (result_color, result, Colors.END, name))
                if not passed:
                    for check in test.checks:
                        if not check() and not test.did_throw:
                            check(report=True)
            
            if verbose:
                print ('    %d/%d seeds failed in %.1fs' % (
                        test.popen_failed, test.popen_count,
                        testend-teststart,))

        sleep(.1)  # avoid "Directory not empty"

    if done>0 and failed==0 and not silent:
        print('%sALL %d TESTS PASSED%s' % (Colors.BOLD + Colors.PASS, done, Colors.END))
    elif not silent:
        print('%s%d/%d TESTS FAILED%s' % (Colors.BOLD + Colors.FAIL, failed, done, Colors.END))
    
    if failed>0: exit(1)

