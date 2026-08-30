#!/usr/bin/env python3
"""Compare two directories of rendered frames (and the mixed tune)."""
import sys, os, glob

RANDOM_FRAMES = {'shot04.ppm', 'shot05.ppm', 'shot06.ppm', 'shot09.ppm', 'shot0b.ppm'}

def main(a, b):
    diff = [os.path.basename(f) for f in sorted(glob.glob(os.path.join(a, 'shot*.ppm')))
            if open(f, 'rb').read() != open(os.path.join(b, os.path.basename(f)), 'rb').read()]
    n = len(glob.glob(os.path.join(a, 'shot*.ppm')))
    unexpected = [f for f in diff if f not in RANDOM_FRAMES]
    print('frames: %d compared, %d identical' % (n, n - len(diff)))
    print('differing: %s' % (', '.join(diff) or 'none'))
    print('  (effect 1 seeds its background lines from the clock, so those frames'
          '\n   are expected to differ between any two runs)')
    wav = [os.path.join(d, 'song.wav') for d in (a, b)]
    if all(os.path.exists(w) for w in wav):
        print('audio: %s' % ('identical' if open(wav[0],'rb').read() == open(wav[1],'rb').read()
                             else 'DIFFERENT'))
    if unexpected:
        print('\nUNEXPECTED differences: %s' % ', '.join(unexpected))
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1], sys.argv[2]))
