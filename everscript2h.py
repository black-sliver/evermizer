#!/usr/bin/python

from __future__ import print_function
from sys import argv, version_info
import os.path

py2 = version_info[0]==2
if py2:
    def F(s):
        return s.replace('%b', '%s')
else:
    def F(s):
        return s


def print_usage(code=1):
    print('Usage: [-a] everscript2h.py <output.h> <input1.txt> [<input2.txt> ....]')
    print('  -a: append to file')
    exit(code)

def main(dst_filename, append, *src_filenames):
    spaces = { False: b'', True: b'    ' }
    omode = 'ab' if append else 'wb'
    with open(dst_filename, omode) as fout:
        for src in src_filenames:
            n = 1
            name = os.path.splitext(os.path.basename(src))[0].upper()
            in_set = False
            set_name = ''
            set_items = []
            with open(src) as fin:
                in_patch = False
                line_no = 0
                for line in fin:
                  line_no += 1
                  try:
                    if line.lstrip().startswith('#set'):
                        in_set = True
                        set_name = line.lstrip()[4:].lstrip().split('//')[0].rstrip()
                        if set_name == '': set_name = name
                        set_items = []
                        continue
                    if line.lstrip().startswith('#'):
                        fout.write(line.encode('ascii'))
                        continue
                    if line.startswith('@'):
                        if in_patch:
                            fout.write(b');\n');
                            in_patch = False
                            if len(line.strip())==1: continue # end of patch
                        patchname = name
                        in_patch = True
                        if n>1: patchname += str(n)
                        n += 1
                        if '+' in line:
                            a,b = line[1:].strip().split('+',1)
                            addr = int(a,0) + int(b,0)
                        else:
                            addr = int(line[1:].strip(),0)
                        if addr>=0xc00000: # hirom, fast, "raw"
                            fout.write(F(b'DEF(%b, 0x%06x - 0xC00000,\n') % (patchname.encode('ascii'), addr))
                        elif addr>=0x800000: # hirom, fast, "script"
                            fout.write(F(b'DEF(%b, 0x%06x - 0x800000,\n') % (patchname.encode('ascii'), addr))
                        else: # assume absolute address
                            fout.write(F(b'DEF(%b, 0x%06x,\n') % (patchname.encode('ascii'), addr))
                        if in_set: set_items.append(F(b'%b') % (patchname.encode('ascii'),))
                        in_patch = True
                        continue
                    if '//' in line:
                        data,comment = line.rstrip().split('//',1)
                    else:
                        data,comment = line.strip(),''
                    if ':' in data: # label
                        comment = data+' '+comment
                        data = ''
                    else:
                        data = data.strip()
                    if comment.strip()=='': comment==''
                    if comment and not data:
                        fout.write(b'%s//%s\n' % (spaces[in_patch],comment.encode('ascii')))
                    elif data and data[0]=='"' and data[-1]=='"':
                        if comment!='': comment=' //'+comment
                        fout.write(b'%s%s%s\n' % (spaces[in_patch], data.encode('ascii'), comment.encode('ascii')))
                    elif data:
                        if comment!='': comment=' //'+comment
                        b = []
                        for a in data.split(' '):
                            if len(a) == 2: # byte
                                b.append(a)
                            elif len(a) == 4: # word
                                b += [a[2:], a[:2]]
                            elif len(a) == 6: # long addr
                                b += [a[4:], a[2:4], a[:2]]
                            elif len(a) == 8: # dword
                                b += [a[6:], a[4:6], a[2:4], a[:2]]
                            else:
                                raise Exception('Invalid data: ' + str(data))
                        fout.write(b'%s"\\x%s"%s\n' % (spaces[in_patch], '\\x'.join(b).encode('ascii'), comment.encode('ascii')))
                  except Exception as e:
                    print('In %s:%d' % (os.path.basename(src), line_no))
                    raise(e)
                    
                if in_patch:
                    fout.write(b');\n');
                
                if in_set and set_name != '':
                    # add define to apply all previous since #set directive
                    fout.write(F(b'#define APPLY_%b() do {') % (set_name.encode('ascii'),))
                    i = 0
                    for i in range(len(set_items)):
                        fout.write( b'\\\n    ' if i%4==0 else b' ' )
                        fout.write(F(b'APPLY(%b);') % (set_items[i],))
                    fout.write(b'} while(false)\n')

if __name__ == '__main__':
    # TODO: rewrite with argparse
    if len(argv)<3 or (argv[1] == '-a' and len(argv)<4): print_usage()
    if (argv[1] == '-a'):
        append = True
        first_src = 3
    else:
        append = False
        first_src = 2
    dst = os.path.splitext(argv[first_src-1])[0]+'.h' # FIXME: splitext still required?
    main(dst, append, *argv[first_src:])
