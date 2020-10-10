#!/usr/bin/python3

from sys import argv
import os.path

def print_usage(code=1):
    print('Usage: [-a] everscript2h.py <output.h> <input1.txt> [<input2.txt> ....]')
    print('  -a: append to file')
    exit(code)

if __name__ == '__main__':
    spaces = { False: b'', True: b'    ' }
    if len(argv)<3 or (argv[1] == '-a' and len(argv)<4): print_usage()
    if (argv[1] == '-a'):
        omode = 'ab'
        firstinput = 3
    else:
        omode = 'wb'
        firstinput = 2
    with open(os.path.splitext(argv[firstinput-1])[0]+'.h',omode) as fout:
        for i in range(firstinput,len(argv)):
            n = 1
            name = os.path.splitext(os.path.basename(argv[i]))[0].upper()
            with open(argv[i]) as fin:
                in_patch = False
                for line in fin:
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
                            fout.write(b'DEF(%b, 0x%06x - 0xC00000,\n' % (patchname.encode('ascii'), addr))
                        elif addr>=0x800000: # hirom, fast, "script"
                            fout.write(b'DEF(%b, 0x%06x - 0x800000,\n' % (patchname.encode('ascii'), addr))
                        else: # assume absolute address
                            fout.write(b'DEF(%b, 0x%06x,\n' % (patchname.encode('ascii'), addr))
                        in_patch = True
                        continue
                    if '//' in line:
                        data,comment = line.rstrip().split('//',1)
                    else:
                        data,comment = line.strip(),''
                    if ':' in data:
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
                        b = data.split(' ')
                        fout.write(b'%s"\\x%s"%s\n' % (spaces[in_patch], '\\x'.join(b).encode('ascii'), comment.encode('ascii')))
                if in_patch:
                    fout.write(b');\n');

