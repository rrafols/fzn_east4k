#!/usr/bin/env python3
"""
Build the packed intro.

Layout of the single 4096-byte page:

    mach header + load commands
    loader stub  (see src/stub.asm)
    LZMA-compressed intro
    dyld bind opcodes for dlopen/dlsym
    zero padding

__TEXT is RWX and its vmsize runs past the file, so the stub unpacks the intro
into that zero-filled tail and jumps to it - no allocation, no second segment.

The bind stream normally has to live in __LINKEDIT, which would force a second
page and push the file past 4096.  Instead __LINKEDIT is declared as an empty
segment based at the image address, which makes dyld resolve its file offsets
against the first page, where the opcodes actually are.
"""
import struct, subprocess, sys, os, re, lzma, hashlib, json, shutil

VM        = 0x100000000
PAGE      = 0x1000
ROOT      = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

LC_SEGMENT_64     = 0x19
LC_SYMTAB         = 0x02
LC_DYSYMTAB       = 0x0b
LC_LOAD_DYLINKER  = 0x0e
LC_LOAD_DYLIB     = 0x0c
LC_MAIN           = 0x80000028
LC_DYLD_INFO_ONLY = 0x80000022
LC_BUILD_VERSION  = 0x32

BOUND = ['_dlsym', '_dlopen']           # order must match stub.asm

# oneKpaq packs about 8% smaller than LZMA, at the cost of an encoder run that
# takes a long while.  Results are cached by payload hash, and if the encoder is
# not around the build falls back to the system LZMA.
# Opt in with ONEKPAQ=/path/to/onekpaq; an encoder run takes a long time, so
# it is not the default.
ONEKPAQ = os.environ.get('ONEKPAQ', '')


def compress(raw, build):
    """-> (bytes, offset, shift, name).  offset/shift are 0 for LZMA."""
    if ONEKPAQ and os.path.exists(ONEKPAQ) and '--lzma' not in sys.argv:
        cache = os.path.join(build, 'okp-cache')
        os.makedirs(cache, exist_ok=True)
        key = hashlib.sha256(raw).hexdigest()[:16]
        meta, blob = os.path.join(cache, key + '.json'), os.path.join(cache, key + '.okp')
        if os.path.exists(meta) and os.path.exists(blob):
            m = json.load(open(meta))
            return open(blob, 'rb').read(), m['offset'], m['shift'], 'oneKpaq (cached)'
        src = os.path.join(build, 'okp-in.bin')
        open(src, 'wb').write(raw)
        print('  compressing with oneKpaq - this takes a while...', file=sys.stderr)
        r = subprocess.run([ONEKPAQ, '3', '1', src, blob],
                           cwd=os.path.dirname(ONEKPAQ), capture_output=True, text=True)
        m = re.search(r'offset=(\d+) shift=(\d+)', r.stdout or '')
        if r.returncode == 0 and m and os.path.exists(blob):
            off, sh = int(m.group(1)), int(m.group(2))
            json.dump({'offset': off, 'shift': sh}, open(meta, 'w'))
            return open(blob, 'rb').read(), off, sh, 'oneKpaq'
        print('  oneKpaq failed, falling back to LZMA', file=sys.stderr)
    packed = lzma.compress(raw, format=lzma.FORMAT_XZ, check=lzma.CHECK_NONE,
                           filters=[{'id': lzma.FILTER_LZMA2,
                                     'preset': 9 | lzma.PRESET_EXTREME,
                                     'lc': 0, 'lp': 0, 'pb': 0}])
    return packed, 0, 0, 'LZMA'


def uleb(v):
    out = bytearray()
    while True:
        b = v & 0x7f
        v >>= 7
        out.append(b | (0x80 if v else 0))
        if not v:
            return bytes(out)


def cstr_cmd(cmd, path, extra=b''):
    off = 12 + len(extra)
    data = path.encode() + b'\0'
    size = (off + len(data) + 7) & ~7
    out = struct.pack('<III', cmd, size, off) + extra + data
    return out + b'\0' * (size - len(out))


def read_map(path):
    syms, sect, sizes = {}, None, {}
    for line in open(path):
        if line.startswith('---- Section'):
            sect = line.split()[2]
            continue
        m = re.match(r'\s*([0-9A-F]+)\s+([0-9A-F]+)\s+(\S+)\s*$', line)
        if m:
            syms.setdefault(sect, {})[m.group(3)] = int(m.group(1), 16)
    return syms


def nasm(src, out, defines=(), extra=()):
    cmd = ['nasm', '-f', 'bin', '-I', os.path.join(ROOT, 'src') + '/',
                    '-I', os.path.join(ROOT, '..', '..', 'common', 'data') + '/']
    cmd += ['-D%s' % d for d in defines] + list(extra) + [src, '-o', out]
    subprocess.run(cmd, check=True)


def build_loadcmds(stub_off, file_size, bind_off, bind_size, vmsize, entry):
    dylink = cstr_cmd(LC_LOAD_DYLINKER, '/usr/lib/dyld')
    dylib  = cstr_cmd(LC_LOAD_DYLIB, '/usr/lib/libSystem.B.dylib',
                      struct.pack('<III', 0, 0x10000, 0x10000))
    parts = [
        struct.pack('<II16sQQQQIIII', LC_SEGMENT_64, 72, b'__PAGEZERO',
                    0, VM, 0, 0, 0, 0, 0, 0),
        struct.pack('<II16sQQQQIIII', LC_SEGMENT_64, 72, b'__TEXT',
                    VM, vmsize, 0, file_size, 7, 7, 0, 0),
        # Empty, based at the image: dyld resolves LC_DYLD_INFO offsets against
        # this, so the opcodes can sit inside the first page.
        struct.pack('<II16sQQQQIIII', LC_SEGMENT_64, 72, b'__LINKEDIT',
                    VM, 0, 0, 0, 7, 1, 0, 0),
        struct.pack('<IIIIIIIIIIII', LC_DYLD_INFO_ONLY, 48, 0, 0,
                    bind_off, bind_size, 0, 0, 0, 0, 0, 0),
        struct.pack('<IIIIII', LC_SYMTAB, 24, 0, 0, 0, 0),
        # Without a platform version macOS treats the binary as ancient and
        # hands OpenGL the legacy driver plugin, which rasterises slightly
        # differently.  Twenty-four bytes to get the modern one.
        struct.pack('<IIIIII', LC_BUILD_VERSION, 24, 1, 13 << 16, 13 << 16, 0),
        struct.pack('<II18I', LC_DYSYMTAB, 80, *([0] * 18)),
        dylink,
        struct.pack('<IIQQ', LC_MAIN, 24, entry, 0),
        dylib,
    ]
    cmds = b''.join(parts)
    hdr = struct.pack('<IiiIIIII', 0xfeedfacf, 0x01000007, 3, 2,
                      len(parts), len(cmds), 0x85, 0)
    return hdr + cmds


def main():
    # --offscreen packs the frame-dump build instead, which exercises the whole
    # loader path without taking over the display.
    offscreen = '--offscreen' in sys.argv
    build = os.path.join(ROOT, 'build')
    os.makedirs(build, exist_ok=True)
    payload_bin = os.path.join(build, 'payload.bin')
    payload_map = os.path.join(build, 'payload.map')

    defs = ['TINY=1'] + (['OFFSCREEN=1'] if offscreen else [])
    nasm(os.path.join(ROOT, 'src', 'intro.asm'), payload_bin, defs)
    raw = open(payload_bin, 'rb').read()
    syms = read_map(payload_map)
    entry = syms['.text']['_start']
    bss_end = syms['.bss']['bss_end']       # exact end of the zero-filled tail

    packed, okp_off, okp_shift, packer = compress(raw, build)
    use_okp = 1 if packer.startswith('oneKpaq') else 0

    # The stub's length is independent of the values baked into it, so a couple
    # of passes settle the layout: stub length -> packed offset -> file size ->
    # unpack address (which must clear the mapped file, or the decompressor
    # would overwrite its own input).
    stub_off = len(build_loadcmds(0, 0, 0, 0, 0, 0))
    stub_bin = os.path.join(build, 'stub.bin')
    packed_off, destva = 0, VM + PAGE
    for _ in range(6):
        nasm(os.path.join(ROOT, 'src', 'stub.asm'), stub_bin, [
            'STUBVA=%d' % (VM + stub_off),
            'DESTVA=%d' % destva,
            'RAWSZ=%d' % len(raw),
            'PACKEDVA=%d' % (VM + stub_off + packed_off),
            'PACKEDSZ=%d' % len(packed),
            'ENTRY=%d' % entry,
            'PACKEDOFF=%d' % okp_off,
            'ONEKPAQ_DECOMPRESSOR_SHIFT=%d' % okp_shift,
            'USE_ONEKPAQ=%d' % use_okp,
        ])
        stub = open(stub_bin, 'rb').read()
        total = max(PAGE, stub_off + len(stub) + len(packed) + 32)
        # oneKpaq reads thirteen bytes below the destination and needs them
        # zero, so keep clear of the last file page.
        new_dest = VM + ((total + PAGE - 1) & ~(PAGE - 1)) + 16
        if packed_off == len(stub) and destva == new_dest:
            break
        packed_off, destva = len(stub), new_dest

    body = stub + packed
    bind_off_rel = stub_off + len(body)

    stub_syms = read_map(os.path.join(build, 'stub.map'))['.text']
    p_dlsym_off = stub_syms['p_dlsym'] - VM        # file offset of the slot pair
    ops = bytearray([0x11, 0x51, 0x70 | 1]) + uleb(p_dlsym_off)
    for name in BOUND:
        ops.append(0x40)
        ops += name.encode() + b'\0'
        ops.append(0x90)
    ops.append(0x00)

    file_size = max(PAGE, stub_off + len(body) + len(ops))
    # bss_end is an address in the flat image, so it already covers the code
    vmsize = ((destva - VM) + bss_end + PAGE - 1) & ~(PAGE - 1)

    hdr = build_loadcmds(stub_off, file_size, bind_off_rel, len(ops), vmsize,
                         stub_off)
    assert len(hdr) == stub_off, (len(hdr), stub_off)

    out = bytearray(b'\0' * file_size)
    out[0:len(hdr)] = hdr
    out[stub_off:stub_off + len(body)] = body
    out[bind_off_rel:bind_off_rel + len(ops)] = ops

    target = os.path.join(build, 'east_shots') if offscreen else os.path.join(ROOT, 'east')
    open(target, 'wb').write(bytes(out))
    os.chmod(target, 0o755)

    print('  header + load commands : %5d' % len(hdr))
    print('  loader stub            : %5d   (%s)' % (len(stub), packer))
    print('  intro, packed          : %5d   (%d raw, %.1f%%)'
          % (len(packed), len(raw), 100 * len(packed) / len(raw)))
    print('  dyld bind opcodes      : %5d' % len(ops))
    print('  ---------------------------------')
    print('  %-22s : %5d bytes' % (os.path.basename(target), file_size))
    if file_size > PAGE:
        print('  (%d over the 4096-byte page)' % (file_size - PAGE))
    else:
        print('  (%d bytes of the page still free)' % (PAGE - file_size))


if __name__ == '__main__':
    main()
