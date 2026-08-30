// Runs the minified build and the module build side by side and compares what
// they compute: the eight instruments, the whole mixed tune, the flare texture
// and the seeded particle field.  Minifiers are where a port like this would
// quietly lose its arithmetic, so this is checked rather than assumed.
//
//   python3 tools/pack.py --check && node tools/check_packed.mjs
import { readFileSync } from 'fs';
import { fileURLToPath } from 'url';
import { genSamples } from '../src/sgen.js';
import { renderSong, noteTable } from '../src/player.js';
import { makeFlare, seedParticles, px } from '../src/draw.js';

const path = fileURLToPath(new URL('../build/east.check.js', import.meta.url));
const packed = new Function(readFileSync(path, 'utf8') + ';return globalThis.__x')();

const dv = new DataView(new ArrayBuffer(4));
const fnv = (feed) => {
  let h = 2166136261 >>> 0;
  feed(b => { h ^= b & 0xff; h = Math.imul(h, 16777619) >>> 0; });
  return h >>> 0;
};
const bytes = a => fnv(p => { for (const v of a) p(v); });
const words = a => fnv(p => { for (const v of a) { p(v); p(v >> 8); } });
const floats = a => fnv(p => { for (const v of a) { dv.setFloat32(0, v); for (let b = 0; b < 4; b++) p(dv.getUint8(b)); } });

const cases = [];
const add = (name, run) => cases.push([name, run]);

add('instruments', m => bytes(m.buf));
add('mixed tune', m => words(m.pcm));
add('note table', m => words(m.notes));
add('flare texture', m => floats(m.flare));
add('particle seed', m => floats(m.px));

const build = (api) => {
  const buf = api.genSamples();
  const pcm = api.renderSong(buf);
  api.seedParticles();
  return { buf, pcm, notes: api.noteTable(), flare: api.makeFlare(), px: api.px };
};

const a = build({ genSamples, renderSong, noteTable, makeFlare, seedParticles, px });
const b = build(packed);

let bad = 0;
for (const [name, run] of cases) {
  const x = run(a), y = run(b);
  const ok = x === y;
  if (!ok) bad++;
  console.log('  %s %s  %s', ok ? 'ok  ' : 'FAIL', name.padEnd(16),
              ok ? x.toString(16) : x.toString(16) + ' vs ' + y.toString(16));
}
console.log(bad ? '\n%d of %d differ' : '\nminified build is identical (%d of %d differ)',
            bad, cases.length);
process.exit(bad ? 1 : 0);
