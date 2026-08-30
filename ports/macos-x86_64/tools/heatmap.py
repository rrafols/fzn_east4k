#!/usr/bin/env python3
"""
Where the packed bytes actually go.

Raw size is misleading for a packed intro - a long run of identical instructions
costs almost nothing once compressed, while a short run of unique bytes costs a
lot. So this measures each block's *incremental* cost under the same LZMA
settings the packer uses: compress the payload prefix by prefix and take the
delta. That is the real number of bytes a piece of code contributes.

Writes build/heatmap.png and prints a per-region table.
"""
import sys, os, re, zlib, struct, lzma

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import font5x7

ROOT  = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BLOCK = 32
FILTERS = [{'id': lzma.FILTER_LZMA2, 'preset': 9 | lzma.PRESET_EXTREME,
            'lc': 0, 'lp': 0, 'pb': 0}]


def clen(data):
    return len(lzma.compress(data, format=lzma.FORMAT_XZ,
                             check=lzma.CHECK_NONE, filters=FILTERS))


def regions(mapfile, total):
    syms, sect = {}, None
    for line in open(mapfile):
        if line.startswith('---- Section'):
            sect = line.split()[2]
            continue
        m = re.match(r'\s*([0-9A-F]+)\s+([0-9A-F]+)\s+(\S+)\s*$', line)
        if m and sect == '.text':
            syms[m.group(3)] = int(m.group(1), 16)
    tops = sorted((a, n) for n, a in syms.items() if '.' not in n)
    out = []
    for i, (a, n) in enumerate(tops):
        end = tops[i+1][0] if i+1 < len(tops) else total
        if end > a:
            out.append((n, a, end))
    return out


GROUPS = [
    ('constants',    lambda n: n.startswith('K_') or n in ('kbase', 'kend')),
    ('instruments',  lambda n: n.startswith('m_')),
    ('glyph data',   lambda n: n in ('chin1', 'chin2', 'fznchar')),
    ('music data',   lambda n: n in ('orderList', 'patternList', 'channelList')),
    ('arrangement',  lambda n: n == 'orderStuff'),
    ('float pool',   lambda n: n in ('cubekey', 'volgain', 'mastergain', 'rate_f',
                                     'c105', 'cDiv', 'c33152', 'f480', 'one_i',
                                     'd_fovy', 'd_znear', 'd_zfar', 'd_rate',
                                     'd_aspect', 'pfattr', 'asbd', 'dmask')),
]


def group(regs):
    """Collapse the many tiny symbols into meaningful buckets."""
    out = {}
    for name, a, b in regs:
        label = None
        for g, pred in GROUPS:
            if pred(name):
                label = g
                break
        if label is None:
            label = name if (b - a) >= 40 else 'misc'
        out.setdefault(label, []).append((a, b))
    return out


def main():
    # assemble the release payload ourselves: the verify build differs
    import subprocess
    payload = os.path.join(ROOT, 'build', 'payload.bin')
    mapfile = os.path.join(ROOT, 'build', 'payload.map')
    subprocess.run(['nasm', '-f', 'bin', '-I', os.path.join(ROOT, 'src') + '/',
                    '-I', os.path.join(ROOT, '..', '..', 'common', 'data') + '/',
                    '-DTINY=1', os.path.join(ROOT, 'src', 'intro.asm'),
                    '-o', payload], check=True, cwd=ROOT,
                   stderr=subprocess.DEVNULL)
    data = open(payload, 'rb').read()
    regs = group(regions(mapfile, len(data)))

    # incremental compressed cost of every block, measured in context
    print('measuring %d blocks...' % ((len(data)+BLOCK-1)//BLOCK), file=sys.stderr)
    costs, prev = [], clen(b'')
    for off in range(0, len(data), BLOCK):
        cur = clen(data[:off+BLOCK])
        costs.append(max(0, cur - prev))
        prev = cur
    packed = clen(data)

    # Roll up per region, splitting each block's cost across the regions it
    # spans in proportion to how many bytes each owns - otherwise a four-byte
    # constant sharing a block gets charged for the whole thing.
    acc = {}
    for name, spans in regs.items():
        tot = 0.0
        raw = 0
        for a, b in spans:
            raw += b - a
            for i in range(a // BLOCK, min(len(costs), (b + BLOCK - 1) // BLOCK + 1)):
                lo, hi = i * BLOCK, min(len(data), (i + 1) * BLOCK)
                ov = max(0, min(b, hi) - max(a, lo))
                if ov and hi > lo:
                    tot += costs[i] * ov / float(hi - lo)
        acc[name] = (tot, raw)
    table = sorted(((c, r, n) for n, (c, r) in acc.items()), reverse=True)

    print()
    print('%-16s %8s %10s %9s %8s' % ('region', 'raw', 'packed', 'bits/byte', '% packed'))
    print('-' * 56)
    for comp, raw, name in table:
        print('%-16s %8d %10.0f %9.2f %7.1f%%'
              % (name, raw, comp, 8.0*comp/raw if raw else 0, 100.0*comp/packed))
    print('-' * 56)
    print('%-16s %8d %10d %9.2f' % ('total', len(data), packed, 8.0*packed/len(data)))

    render(data, costs, table, packed, regs,
           os.path.join(ROOT, 'build', 'heatmap.png'))


def render(data, costs, table, packed, regs, path):
    COLS = 48
    CELL = 12
    rows = (len(costs) + COLS - 1) // COLS
    bar_h = 14
    top_h = 24 + len(table) * bar_h + 20
    W = max(COLS * CELL + 40, 560)
    H = top_h + 26 + rows * CELL + 46
    img = bytearray(b'\x14\x16\x1c' * (W * H))

    def put(x, y, c):
        if 0 <= x < W and 0 <= y < H:
            o = (y * W + x) * 3
            img[o:o+3] = bytes(c)

    def box(x, y, w, h, c):
        for yy in range(y, y+h):
            for xx in range(x, x+w):
                put(xx, yy, c)

    def ramp(t):
        """blue (cheap) -> green -> yellow -> red (expensive)"""
        t = max(0.0, min(1.0, t))
        stops = [(0.0,(40,70,160)),(0.35,(40,150,120)),(0.6,(230,200,60)),(1.0,(220,60,50))]
        for i in range(len(stops)-1):
            a, ca = stops[i]; b, cb = stops[i+1]
            if a <= t <= b:
                f = (t-a)/(b-a)
                return tuple(int(ca[k]+(cb[k]-ca[k])*f) for k in range(3))
        return stops[-1][1]

    white = (235, 238, 245); grey = (120, 128, 140)
    font5x7.draw(put, 12, 8, 'PACKED SIZE BY REGION', white, 2)

    widest = max(c for c, _, _ in table) or 1
    y = 30
    for comp, raw, name in table:
        font5x7.draw(put, 12, y + 3, name[:14].upper(), white, 1)
        bx = 12 + 15*6 + 4
        bw = int((W - bx - 96) * comp / widest)
        box(bx, y, max(1, bw), 9, ramp(min(1.0, (8.0*comp/raw)/8.0 if raw else 0)))
        # a faint outline showing what it costs uncompressed
        rw = int((W - bx - 96) * raw / (max(r for _, r, _ in table) or 1))
        for xx in range(bx, bx + max(1, rw)):
            put(xx, y + 10, (70, 76, 88))
        font5x7.draw(put, W - 90, y + 3, '%d' % comp, white, 1)
        font5x7.draw(put, W - 46, y + 3, '%d%%' % round(100.0*comp/packed), grey, 1)
        y += bar_h

    y += 14
    font5x7.draw(put, 12, y, 'EVERY %d BYTES, COST IN BITS PER BYTE' % BLOCK, white, 1)
    y += 14
    for i, c in enumerate(costs):
        cx = 20 + (i % COLS) * CELL
        cy = y + (i // COLS) * CELL
        box(cx, cy, CELL-1, CELL-1, ramp((8.0*c/BLOCK)/8.0))
    # tick where each region starts, so the grid can be read against the bars
    starts = sorted(a for spans in regs.values() for a, _ in spans)
    for a in starts:
        i = a // BLOCK
        if i >= len(costs):
            continue
        cx = 20 + (i % COLS) * CELL
        cy = y + (i // COLS) * CELL
        for k in range(CELL - 1):
            put(cx - 1, cy + k, (250, 250, 250))

    y2 = y + rows * CELL + 10
    font5x7.draw(put, 20, y2, 'CHEAP', grey, 1)
    for i in range(160):
        box(20 + 40 + i, y2, 1, 7, ramp(i/159.0))
    font5x7.draw(put, 20 + 40 + 166, y2, 'COSTLY', grey, 1)
    font5x7.draw(put, 20, y2 + 14,
                 'TOTAL %d RAW  %d PACKED' % (len(data), packed), white, 1)

    raw = b''.join(b'\x00' + bytes(img[r*W*3:(r+1)*W*3]) for r in range(H))
    def ch(t, b):
        c = t + b
        return struct.pack('>I', len(b)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
    open(path, 'wb').write(b'\x89PNG\r\n\x1a\n'
        + ch(b'IHDR', struct.pack('>IIBBBBB', W, H, 8, 2, 0, 0, 0))
        + ch(b'IDAT', zlib.compress(bytes(raw), 9)) + ch(b'IEND', b''))
    print('\n%s  %dx%d' % (path, W, H))


if __name__ == '__main__':
    main()
