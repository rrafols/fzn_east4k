// Renders the tune and compares it with the macOS port's song.wav, which is
// the same mix produced by the same arithmetic on real x87 hardware.
//
//   node tools/check_audio.mjs [path/to/song.wav]
import { readFileSync } from 'fs';
import { fileURLToPath } from 'url';
import { genSamples } from '../src/sgen.js';
import { renderSong, TOTAL_SAMPLES } from '../src/player.js';

const ref = process.argv[2] ||
  fileURLToPath(new URL('../../macos-x86_64/build/linked/song.wav', import.meta.url));

let t = Date.now();
const sampbuf = genSamples();
console.log('instruments: %d ms', Date.now() - t);
t = Date.now();
const pcm = renderSong(sampbuf);
console.log('mix:         %d ms  (%d samples)', Date.now() - t, pcm.length);

let wav;
try { wav = readFileSync(ref); }
catch { console.log('\nno reference at %s - build it with `make verify` in ports/macos-x86_64', ref); process.exit(0); }

const got = pcm;
const want = new Int16Array(wav.buffer, wav.byteOffset + 44, (wav.length - 44) >> 1);
console.log('reference:   %d samples', want.length);

const n = Math.min(got.length, want.length);
let diff = 0, maxAbs = 0, sumAbs = 0, firstAt = -1;
for (let i = 0; i < n; i++) {
  const d = Math.abs(got[i] - want[i]);
  if (d) { diff++; sumAbs += d; if (d > maxAbs) maxAbs = d; if (firstAt < 0) firstAt = i; }
}
console.log('\ndiffering samples: %d / %d (%s%%)', diff, n, (100 * diff / n).toFixed(6));
if (diff) {
  console.log('first at:          %d (%.3f s)', firstAt, firstAt / 44100);
  console.log('max |delta|:       %d of 32768', maxAbs);
  console.log('mean |delta|:      %s (over differing samples)', (sumAbs / diff).toFixed(3));
  for (let i = firstAt; i < Math.min(firstAt + 8, n); i++)
    console.log('  [%d] got %d want %d', i, got[i], want[i]);
} else {
  console.log('identical.');
}
