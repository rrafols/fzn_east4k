// ---------------------------------------------------------------------------
//  Geometry helpers and the small maths routines - a port of draw.inc.
//
//  Every value the original hands to OpenGL, or keeps in one of its globals,
//  goes through a `dd` slot, so it is rounded to single precision at that
//  point.  fr() marks each of those, and the arithmetic in between is done in
//  doubles, which are wider than the x87 registers the original used and round
//  to the same float.
// ---------------------------------------------------------------------------

import { GL } from './glfixed.js';
import { cubepack, chin1, chin2, fznchar } from './data.js';

const fr = Math.fround;

// The original's globals.  v_i, v_j and v_k are 16-bit; the rest are floats.
export const st = {
  ts: 0, lts: 0, k1: 0, k2: 0, t1: 0, t2: 0, x: 0, y: 0,
  i: 0, j: 0, k: 0, flags: 0, order: 0, holdrand: 0,
};

export const MAX_PARTICLES = 100;
export const px = new Float32Array(MAX_PARTICLES);
export const pvel = new Float32Array(MAX_PARTICLES);

// The Microsoft C rand() the original seeded its particles with.
export function rand() {
  st.holdrand = (Math.imul(st.holdrand, 214013) + 2531011) | 0;
  return st.holdrand & 0x7fff;
}

// max * (rand()/32000 - 0.5)
export function frand(max) {
  return max * (rand() / 32000 - fr(0.5));
}

// Called before the clock seeds holdrand, so the field is the same every run.
// The position really does start at [-60, -19]: `fild 40; frand; fisub 40`.
// It takes a few hundred frames of drifting before the particles wrap into the
// [-20, 20] band the effects are framed around.
export function seedParticles() {
  for (let i = 0; i < MAX_PARTICLES; i++) {
    px[i] = fr(frand(40) - 40);
    pvel[i] = fr((frand(1) + fr(0.5)) / 4);
  }
}

// px += vel each frame, wrapping to -20 once it passes 20.
export function advanceParticles() {
  for (let i = 0; i < MAX_PARTICLES; i++) {
    const n = px[i] + pvel[i];
    px[i] = fr(n < 20 ? n : -20);
  }
}

// -- the cube ---------------------------------------------------------------

// Six faces of (normal + 4 vertices), two bits per component.
export const cubef = (() => {
  const key = [0.0, 1.0, -1.0, 0.0];
  const c = new Float32Array(90);
  for (let i = 0; i < 90; i++) c[i] = key[(cubepack[i >> 2] >> ((i & 3) * 2)) & 3];
  return c;
})();

export function drawCube(g) {
  g.begin(GL.QUADS);
  let p = 0;
  for (let f = 0; f < 6; f++) {
    g.normal3f(cubef[p], cubef[p + 1], cubef[p + 2]); p += 3;
    for (let v = 0; v < 4; v++) {
      g.vertex3f(cubef[p], cubef[p + 1], cubef[p + 2]); p += 3;
    }
  }
  g.end();
}

// 3x3x3 grid of cubes, drawn solid and then again as black wireframe.  An
// outlineOnly pass skips straight to the second, which is what effect 5 wants.
// Note that it leaves glPolygonMode back at FILL but the colour at black, and
// that it overwrites v_k1, v_k2 and v_t1 on the way through - both of which
// the effects that follow it depend on.
export function drawRubic(g, xScale, yScale, xOff, outlineOnly) {
  st.k1 = fr(xScale);
  st.k2 = fr(yScale);
  st.t1 = fr(xOff - 5);

  let rub = outlineOnly;
  for (let pass = 0; pass < 2; pass++) {
    if (!rub) {
      for (st.k = -1; st.k <= 1; st.k++) {
        for (st.j = -1; st.j <= 1; st.j++) {
          for (st.i = -1; st.i <= 1; st.i++) {
            g.loadIdentity();
            g.translatef(st.t1, 0, -10);
            g.scalef(st.k1, st.k2, 1);
            g.rotatef(st.ts, 0, 1, 1);
            g.translatef(fr(st.i * fr(2.2)), fr(st.j * fr(2.2)), fr(st.k * fr(2.2)));
            drawCube(g);
          }
        }
      }
    }
    g.polygonMode(GL.FRONT_AND_BACK, GL.LINE);
    g.color3f(0, 0, 0);
    rub = 0;
  }
  g.polygonMode(GL.FRONT_AND_BACK, GL.FILL);
}

// -- the glyphs -------------------------------------------------------------

// 0 is the second character, 1 the first, 8 and 9 the group mark - which is
// drawn twice, once as lines and once as fat points.
export function drawChin(g, idx) {
  let data, n;
  if (idx === 0) { data = chin2; n = 7 * 2; }
  else if (idx >= 8) { data = fznchar; n = 3 * 2; }
  else { data = chin1; n = 10 * 2; }

  g.begin(idx === 9 ? GL.POINTS : GL.LINES);
  for (let v = 0; v < n; v++) {
    const ex = ((data[v * 2] - 128) << 24) >> 24;
    const ey = ((data[v * 2 + 1] - 128) << 24) >> 24;
    g.vertex3f(fr(ex * 2 / 256), fr(-(ey * 2) / 256), 0);
  }
  g.end();
}

// -- the lens flare ---------------------------------------------------------

export const FLARE_DIM = 128;

// Two gaussians, one wide and one tight, combined into a slightly different
// curve per channel.  Red peaks at 1.6 and is clamped on upload, as GL_RGBA
// clamps it.
export function makeFlare() {
  const t = new Float32Array(FLARE_DIM * FLARE_DIM * 4);
  let o = 0;
  for (let row = FLARE_DIM; row > 0; row--) {
    const dy = row - 64;
    let ci = 64;
    for (let col = 0; col < FLARE_DIM; col++, ci--) {
      const d = Math.sqrt(dy * dy + ci * ci);
      const ff = Math.exp(-(d * d) * fr(0.005)) * fr(0.6);
      const ssi = Math.exp(-(d * d) * fr(0.8)) * fr(0.4);
      t[o + 0] = fr(ff + ff + ssi);            // R, peaking at 1.6
      t[o + 1] = fr(fr(0.7) * ff + ssi);       // G
      t[o + 2] = fr(fr(0.25) * ff + ssi);      // B
      t[o + 3] = fr(ff + ssi);                 // A
      o += 4;
    }
  }
  return t;
}

// The billboard quad the flare is drawn with, compiled into list 1.
export function buildFlareList(g) {
  g.newList(1, GL.COMPILE);
  g.begin(GL.QUADS);
  g.texCoord2f(0, 0); g.vertex3f(-2, -2, 0);
  g.texCoord2f(1, 0); g.vertex3f(2, -2, 0);
  g.texCoord2f(1, 1); g.vertex3f(2, 2, 0);
  g.texCoord2f(0, 1); g.vertex3f(-2, 2, 0);
  g.end();
  g.endList();
}
