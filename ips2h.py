#!/usr/bin/python3

from sys import argv
import os.path

def print_usage(code=1):
    print('Usage: ips2h.py [-a] <output.h> <input1.txt|ips> [<input2.txt|ips> ....]')
    print('  txt files have IPS filename(s) and #directives before "\\n--\\n"')
    print('  and copyright notice after.')
    print('  -a: append to file')
    exit(code)

def data2str(data, linelen=16, linesep=b'\n'):
    if len(data)<1: return '""';
    return linesep.join (
        b'"\\x' + b'\\x'.join( ( b'%02x' % (b,) for b in data[i:i+linelen] ) ) + b'"'
        for i in range(0, len(data), linelen)
    )

def ips2h(src, name, n):
    ret = []
    with open(src,'rb') as fips:
        hdr = fips.read(5)
        assert hdr==b'PATCH'
        block_hdr = fips.read(5)
        while True:
            assert len(block_hdr)>=3
            if block_hdr[:3] == b'EOF': break # this is one drawback of IPS
            assert len(block_hdr)==5
            block_off = (block_hdr[0]<<16) + (block_hdr[1]<<8) + block_hdr[2]
            block_len = (block_hdr[3]<<8) + block_hdr[4]
            if block_len==0:
                print('RLE not implemented!')
                assert False
            else:
                data = fips.read(block_len)
                assert len(data) == block_len
                partname = name
                if n>1: partname += '%d' % (n,)
                else: partname += ' '
                fmt = b'%s, 0x%06x, %s' if len(data)<10 else b'%s, 0x%06x,\n    %s\n'
                ret.append( (b'DEF('+fmt+b');\n') % (
                        partname.encode('ascii'), block_off, data2str(data,linesep=b'\n    ') ) );
                n += 1
            block_hdr = fips.read(5)
        assert(len(fips.read(4)) in (3,0)) # we don't allow excess bytes
    return ret

if __name__ == '__main__':
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
            fext = os.path.splitext(os.path.basename(argv[i]))[1]
            ext = fext.lower()
            fname = os.path.splitext(os.path.basename(argv[i]))[0]
            name = fname.upper().replace('-','_')
            fdir = os.path.dirname(argv[i])
            if ext == '.ips':
                fout.write(b'/* %s%s */\n' % (fname.encode('ascii'), fext.encode('ascii')))
                blocks = ips2h(src, name, n)
                for block in blocks: fout.write(block)
                n += len(blocks)
                fout.write(b'\n')
            elif ext == '.txt':
                with open(argv[i]) as fin:
                    # find copyright notice
                    in_notice = False
                    sources = []
                    for line in fin:
                        if line.rstrip('\r\n') == '--':
                            fout.write(b'/*\n%s\n' % (', '.join(sources).encode('ascii'),))
                            in_notice = True
                        elif in_notice and line.rstrip('\r\n') == '':
                            break # we only include the first section in .h
                        elif in_notice:
                            fout.write(line.replace('*/', '* /').encode('ascii'))
                        elif line.rstrip('\r\n') != '' and not line.startswith('#'):
                            sources.append(line.rstrip('\r\n'))
                    if in_notice: fout.write(b'*/\n')
                    # rewind file
                    fin.seek(0)
                    # find patches/directives
                    for line in fin:
                        if line.lstrip().startswith('#'):
                            fout.write(line.encode('ascii'))
                            continue
                        if line.rstrip('\r\n') == '--':
                            break # end of patch section of txt file
                        src = line.rstrip('\r\n')
                        assert '../' not in src and '..\\' not in src
                        src = os.path.join(fdir, src)
                        blocks = ips2h(src, name, n)
                        for block in blocks: fout.write(block)
                        n += len(blocks)
                fout.write(b'\n')
            else:
                    print('Unsupported file type: %s' % (ext,))
                    print_usage()

