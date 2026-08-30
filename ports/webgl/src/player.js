// ---------------------------------------------------------------------------
//  Module playback - a port of player.inc, which is itself the macOS port's
//  replacement for the original's eight DirectSound buffers.
//
//  The whole tune is mixed up front into one 16-bit buffer, exactly as the
//  macOS port does, and the visuals then take their clock from playback
//  position rather than the other way round.  Mixing is single precision
//  throughout because the original's was: movss/mulss/addss, then a
//  round-to-nearest convert and a saturating pack to 16 bits.
// ---------------------------------------------------------------------------

import * as f from './x87.js';
import { orderList, patternList, channelList } from './data.js';
import { m_nBytes } from './data.js';
import { SAMPLE_STRIDE } from './sgen.js';

export const NCHANNELS = 8;
export const ORDERS = 63;
export const ROWS = 16;
export const ROW_MS = 114;
export const SND_RATE = 44100;
export const ROW_SAMPLES = Math.floor(SND_RATE * ROW_MS / 1000);   // 5027
export const TOTAL_SAMPLES = ORDERS * ROWS * ROW_SAMPLES;

const fr = Math.fround;

// -10000, -800, -700 and 0 hundredths of a dB, as the original tabulated them.
const volgain = [0.0, 0.398107, 0.446684, 1.0].map(fr);
const MASTERGAIN = fr(64.0);
const RATE_F = fr(44100.0);

// f[0] = 33152 / 127.71542846, each step x 1.05946309436, stored as an
// integer.  Ninety-six chained multiplies in extended precision, so it is
// built the same way here.
export function noteTable() {
  const t = new Int32Array(96);
  let v = f.fromNumber(fr(33152.0));
  const denom = f.fromNumber(fr(127.71542846));
  const step = f.fromNumber(fr(1.05946309436));
  for (let i = 0; i < 96; i++) {
    t[i] = f.toInt32(f.div(v, denom));
    v = f.mul(v, step);
  }
  return t;
}

export function renderSong(sampbuf) {
  const pcm = new Int16Array(TOTAL_SAMPLES);
  const freqtable = noteTable();

  // A voice is sample index (-1 idle), position, step, gain.
  const vSample = new Int32Array(NCHANNELS).fill(-1);
  const vPos = new Float32Array(NCHANNELS);
  const vStep = new Float32Array(NCHANNELS);
  const vGain = new Float32Array(NCHANNELS);

  let out = 0;
  for (let order = 0; order < ORDERS; order++) {
    const pat = orderList[order] * 8;
    for (let row = 0; row < ROWS; row++) {
      for (let ch = 0; ch < NCHANNELS; ch++) {
        const c = (patternList[pat + ch] << 24) >> 24;   // 255 = unused
        if (c < 0) continue;
        const base = c * 17;
        const instrument = channelList[base];
        const note = channelList[base + 1 + row];
        if (note === 0) continue;

        const semi = (note & 15) - 1;        // -1 keeps the pitch, 14 cuts
        const gain = volgain[(note >> 6) & 3];
        if (semi === 14) {
          vSample[ch] = -1; vPos[ch] = 0; vGain[ch] = 0;
        } else if (semi === -1) {
          vGain[ch] = gain;
        } else {
          const idx = (((note >> 4) & 3) + 4) * 12 + semi;
          vSample[ch] = instrument;
          vPos[ch] = 0;
          vStep[ch] = fr(fr(freqtable[idx]) / RATE_F);
          vGain[ch] = gain;
        }
      }

      for (let s = 0; s < ROW_SAMPLES; s++) {
        let acc = 0;
        for (let ch = 0; ch < NCHANNELS; ch++) {
          const idx = vSample[ch];
          if (idx < 0) continue;
          const pos = Math.trunc(vPos[ch]);          // cvttss2si
          if (pos >= m_nBytes[idx]) { vSample[ch] = -1; continue; }
          const v = sampbuf[idx * SAMPLE_STRIDE + pos] - 128;
          acc = fr(acc + fr(fr(v) * vGain[ch]));
          vPos[ch] = fr(vPos[ch] + vStep[ch]);
        }
        pcm[out++] = packss(roundTiesEven(fr(acc * MASTERGAIN)));
      }
    }
  }
  return pcm;
}

// cvtps2dq: convert with the current rounding mode, which finit leaves at
// round-to-nearest-even.
function roundTiesEven(x) {
  const r = Math.round(x);
  return (r - x === 0.5 && (r & 1)) ? r - 1 : r;
}

function packss(n) {
  return n > 32767 ? 32767 : n < -32768 ? -32768 : n;
}
