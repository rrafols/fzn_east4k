// ---------------------------------------------------------------------------
//  Start-up, in the order intro.asm does it.
//
//  The order is not incidental: the particle field is seeded while holdrand is
//  still zero, and only afterwards is the PRNG reseeded from the clock.  That
//  is why the particles are the same every run and effect 1's background lines
//  are not.
// ---------------------------------------------------------------------------

import { GL1, GL } from './glfixed.js';
import { st, seedParticles, makeFlare, buildFlareList, FLARE_DIM } from './draw.js';
import { genSamples } from './sgen.js';
import { renderSong, TOTAL_SAMPLES, SND_RATE } from './player.js';

export const RENDER_W = 640;
export const RENDER_H = 480;

export function createGL(canvas) {
  const gl = canvas.getContext('webgl2', {
    alpha: false, antialias: false, depth: false, stencil: false,
    preserveDrawingBuffer: false, powerPreference: 'high-performance',
  });
  if (!gl) throw new Error('WebGL 2 is required');
  const g = new GL1(gl, RENDER_W, RENDER_H);

  // The flare texture lives in texture object 0, the default texture, which is
  // where the intro leaves it bound.
  g.bindTexture(GL.TEXTURE_2D, 0);
  g.texParameteri(GL.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  g.texImage2DFloat(FLARE_DIM, FLARE_DIM, makeFlare());
  buildFlareList(g);
  return g;
}

// Instruments then the mix; ~0.8 s of BigInt arithmetic, so it is worth
// yielding around.
export async function renderAudio(onProgress) {
  onProgress?.('generating instruments');
  await frameTick();
  const sampbuf = genSamples();
  onProgress?.('mixing the tune');
  await frameTick();
  const pcm = renderSong(sampbuf);
  return pcm;
}

export function seed(clockMs) {
  seedParticles();
  st.holdrand = clockMs | 0;         // the low half of the start-up clock
}

export function audioBuffer(ctx, pcm) {
  const buf = ctx.createBuffer(1, TOTAL_SAMPLES, SND_RATE);
  const ch = buf.getChannelData(0);
  for (let i = 0; i < pcm.length; i++) ch[i] = pcm[i] / 32768;
  return buf;
}

const frameTick = () => new Promise(r => requestAnimationFrame(r));
