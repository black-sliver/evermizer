#!/usr/bin/python

from sys import argv, version_info
import os.path

py2 = version_info[0]==2

def print_usage(code=1):
    print('Usage: ips2h.py [-a] <output.h> <input1.txt|ips> [<input2.txt|ips> ....]')
    print('  txt files have IPS filename(s) and #directives before "\\n--\\n"')
    print('  and copyright notice after.')
    print('  -a: append to file')
    exit(code)

def data2str(data, linelen=16, linesep=b'\n'):
    if len(data)<1: return '""';
    if py2: data = [ ord(b) for b in data ]
    return linesep.join (
        b'"\\x' + b'\\x'.join( ( b'%02x' % (b,) for b in data[i:i+linelen] ) ) + b'"'
        for i in range(0, len(data), linelen)
    )

def ips2h(src, name, n, offset=0):
    ret = []
    with open(src,'rb') as fips:
        hdr = fips.read(5)
        assert hdr==b'PATCH'
        block_hdr = fips.read(5)
        while True:
            assert len(block_hdr)>=3
            if block_hdr[:3] == b'EOF': break # this is one drawback of IPS
            assert len(block_hdr)==5
            if py2: block_hdr = [ ord(b) for b in block_hdr ]
            block_off = (block_hdr[0]<<16) + (block_hdr[1]<<8) + block_hdr[2] + offset
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

def main(dst_filename, append, *src_filenames):
    omode = 'ab' if append else 'wb'
    with open(dst_filename, omode) as fout:
        for src in src_filenames:
            n = 1
            fext = os.path.splitext(os.path.basename(src))[1]
            ext = fext.lower()
            fname = os.path.splitext(os.path.basename(src))[0]
            name = fname.upper().replace('-','_')
            fdir = os.path.dirname(src)
            if ext == '.ips':
                fout.write(b'/* %s%s */\n' % (fname.encode('ascii'), fext.encode('ascii')))
                blocks = ips2h(src, name, n)
                for block in blocks: fout.write(block)
                n += len(blocks)
                fout.write(b'\n')
            elif ext == '.txt':
                with open(src) as fin:
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
                            sources.append(line.rstrip('\r\n').split(' ')[0])
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
                        offset = 0
                        if ' ' in src:  # IPS patch is for headered rom?
                            tmp = src.split(' ')
                            assert(tmp[1] == 'headered')
                            src = tmp[0]
                            offset = -512
                        assert '../' not in src and '..\\' not in src
                        src = os.path.join(fdir, src)
                        blocks = ips2h(src, name, n, offset)
                        for block in blocks: fout.write(block)
                        n += len(blocks)
                fout.write(b'\n')
            else:
                    print('Unsupported file type: %s' % (ext,))
                    print_usage()

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
