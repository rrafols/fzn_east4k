#!/usr/bin/env python3
"""Tile the rendered PPM frames into a single contact sheet for review."""
import sys, zlib, struct, glob

def readppm(p):
    d = open(p, 'rb').read()
    parts, i = [], 0
    while len(parts) < 4:
        while d[i:i+1].isspace(): i += 1
        j = i
        while not d[j:j+1].isspace(): j += 1
        parts.append(d[i:j]); i = j
    i += 1
    w, h = int(parts[1]), int(parts[2])
    return w, h, d[i:i+w*h*3]

def main(files, dst, cols=4, scale=3):
    w, h, _ = readppm(files[0])
    tw, th = w // scale, h // scale
    rows = (len(files) + cols - 1) // cols
    W, H = tw * cols, th * rows
    canvas = bytearray(W * H * 3)
    for n, f in enumerate(files):
        _, _, px = readppm(f)
        ox, oy = (n % cols) * tw, (n // cols) * th
        for y in range(th):
            sy = y * scale
            row = bytearray()
            for x in range(tw):
                o = (sy * w + x * scale) * 3
                row += px[o:o+3]
            o = ((oy + y) * W + ox) * 3
            canvas[o:o+len(row)] = row
    raw = b''.join(b'\x00' + bytes(canvas[y*W*3:(y+1)*W*3]) for y in range(H))
    def chunk(t, b):
        c = t + b
        return struct.pack('>I', len(b)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
    open(dst, 'wb').write(b'\x89PNG\r\n\x1a\n'
        + chunk(b'IHDR', struct.pack('>IIBBBBB', W, H, 8, 2, 0, 0, 0))
        + chunk(b'IDAT', zlib.compress(bytes(raw), 9)) + chunk(b'IEND', b''))
    print(f"{dst}: {len(files)} frames, {W}x{H}")

if __name__ == '__main__':
    main(sorted(glob.glob(sys.argv[1])), sys.argv[2])
