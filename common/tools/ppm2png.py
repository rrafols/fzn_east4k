#!/usr/bin/env python3
"""Minimal PPM(P6) -> PNG converter, so rendered frames can be inspected directly."""
import sys, zlib, struct

def main(src, dst):
    d = open(src, 'rb').read()
    parts, i = [], 0
    while len(parts) < 4:
        while d[i:i+1].isspace(): i += 1
        if d[i:i+1] == b'#':
            while d[i:i+1] != b'\n': i += 1
            continue
        j = i
        while not d[j:j+1].isspace(): j += 1
        parts.append(d[i:j]); i = j
    i += 1
    w, h = int(parts[1]), int(parts[2])
    px = d[i:i + w*h*3]
    raw = b''.join(b'\x00' + px[y*w*3:(y+1)*w*3] for y in range(h))

    def chunk(t, b):
        c = t + b
        return struct.pack('>I', len(b)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)

    open(dst, 'wb').write(
        b'\x89PNG\r\n\x1a\n'
        + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
        + chunk(b'IDAT', zlib.compress(raw, 9))
        + chunk(b'IEND', b''))

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
