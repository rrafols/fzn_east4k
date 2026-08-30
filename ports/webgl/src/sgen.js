// ---------------------------------------------------------------------------
//  Instrument synthesis - a direct port of sgen.inc.
//
//  Eight instruments, each an oscillator (sine, saw, or square, optionally
//  rectified) through a resonant low-pass, written out as unsigned 8-bit at a
//  base rate of 8363 Hz.  Every value the original keeps in an x87 register
//  stays in x87.js's extended precision here, because the arithmetic is what
//  the sound is: instrument 1 runs a sine at 3.1e12 Hz and instruments 6 and 7
//  drive their saw phase far past what a 16-bit store can hold, so both the
//  precision and the overflow behaviour are load-bearing.
// ---------------------------------------------------------------------------

import * as f from './x87.js';
import { m_nBytes, m_tLfo, m_Freq, m_dFreq, m_Amp, m_dAmp, m_c, m_r } from './data.js';

export const NSAMPLES = 8;
export const SAMPLE_STRIDE = 20000;      // sampbuf: NSAMPLES * 20000 bytes

export function genSamples() {
  const buf = new Uint8Array(NSAMPLES * SAMPLE_STRIDE);
  for (let i = 0; i < NSAMPLES; i++) gensample(buf, i);
  return buf;
}

function gensample(buf, i) {
  const nBytes = m_nBytes[i];
  const lfo = m_tLfo[i];
  const dfreq = f.fromNumber(m_dFreq[i]);
  const damp = f.fromNumber(m_dAmp[i]);
  const c = f.fromNumber(m_c[i]);
  const r = f.fromNumber(m_r[i]);

  let sp = f.ZERO;                        // filter velocity
  let ps = f.ZERO;                        // filter position, and the output
  let phase = f.ZERO;
  let amp = f.fromNumber(m_Amp[i]);
  let freq = f.fromNumber(m_Freq[i]);

  let p = i * SAMPLE_STRIDE;
  for (let n = 0; n < nBytes; n++) {
    phase = f.add(phase, freq);
    freq = f.add(freq, dfreq);

    let val = phase;
    if (lfo & 1) {
      val = f.sin(val);
    } else {
      // The ramp wraps through a signed byte - but by way of a 16-bit store,
      // so once the phase leaves int16 range x87 writes 0x8000 instead and the
      // low byte, and with it the oscillator, goes to zero.  Instruments 6 and
      // 7 do exactly that after ~438 samples, and the filter rings on alone.
      const w = f.toInt16(val);
      val = f.fromNumber((w << 24) >> 24);
    }
    if (lfo & 2) val = f.fromNumber(f.toInt16(val) & 0x80);
    if (lfo & 4) val = f.abs(val);
    if (lfo & 8) val = f.add(val, f.abs(val));

    val = f.mul(val, amp);
    amp = f.add(amp, damp);

    val = f.mul(f.sub(val, ps), c);       // (val - ps) * c
    sp = f.add(sp, val);
    ps = f.add(ps, sp);
    const out = f.toInt16(ps);            // fist, no pop: sp is scaled after
    sp = f.mul(sp, r);

    buf[p++] = ((out & 0xff) + 127) & 0xff;
  }
}
