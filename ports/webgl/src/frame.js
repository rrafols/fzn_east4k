// ---------------------------------------------------------------------------
//  The eight effects and the frame loop - a port of frame.inc.
//
//  Read this next to the assembly; it is meant to line up with it statement
//  for statement, including the parts that look wrong:
//
//    * effect 2 never calls glLoadIdentity before its mesh, so it inherits
//      whatever modelview effect 1 left behind - and effect 1 deliberately
//      ends with a glTranslatef after its last flare;
//    * effect 2's three line strips call glBegin three times and glEnd once,
//      so the second and third glBegin are errors that GL ignores and all
//      ninety vertices end up in one strip, joined across the gaps;
//    * effect 2's line strips read v_t1, which nothing in that loop sets - it
//      still holds cos(4.2 + lts) from the last row of the mesh above;
//    * blend mode, polygon mode and GL_LIGHTING are never reset per frame, so
//      each effect starts from wherever the previous one stopped.
//
//  All of that is load-bearing, and all of it comes out right because
//  glfixed.js keeps the state machine rather than flattening it.
// ---------------------------------------------------------------------------

import { GL } from './glfixed.js';
import { orderStuff } from './data.js';
import {
  st, px, MAX_PARTICLES, frand, drawCube, drawRubic, drawChin, advanceParticles,
} from './draw.js';
import { ORDERS, ROWS, ROW_MS } from './player.js';

const fr = Math.fround;

export function drawFrame(g, ms) {
  st.ts = fr(ms / 20);

  const order = Math.floor(ms / (ROW_MS * ROWS));
  if (order >= ORDERS) return false;
  st.order = order;
  st.flags = orderStuff[order];

  g.clearColorf(fr(0.6), 0, 0, 1);
  g.clear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT);
  g.enable(GL.BLEND);
  g.enable(GL.LIGHT0);
  g.enable(GL.COLOR_MATERIAL);
  g.enable(GL.LINE_SMOOTH);
  g.enable(GL.POINT_SMOOTH);
  g.setMatrixMode(GL.PROJECTION);
  g.loadIdentity();
  g.perspective(80.0, 4 / 3, 0.1, 100.0);
  g.setMatrixMode(GL.MODELVIEW);
  g.loadIdentity();

  // ==== 1 - lit rubik grid over random background lines, plus flares =======
  if (st.flags & 1) {
    g.lineWidth(5);
    g.color3f(1, fr(0.9), 0);
    g.begin(GL.LINES);
    for (let n = 0; n < 25; n++) {
      // The only nondeterminism in the visuals: this PRNG was reseeded from
      // the clock just before the first frame.
      const y = fr(frand(17));
      g.vertex3f(-30, y, -20);
      g.vertex3f(30, y, -20);
    }
    g.end();

    g.enable(GL.DEPTH_TEST);
    g.enable(GL.LIGHTING);
    g.lineWidth(8);
    g.color3f(1, 0, fr(0.2));
    g.disable(GL.BLEND);

    drawRubic(g, 1, 1, 0, 0);

    g.disable(GL.LIGHTING);
    g.disable(GL.DEPTH_TEST);
    g.enable(GL.TEXTURE_2D);
    g.enable(GL.BLEND);
    g.blendFunc(GL.ONE, GL.ONE);
    g.color3f(1, 1, 1);

    for (let n = 0; n < MAX_PARTICLES; n++) {
      const p = px[n];
      g.loadIdentity();
      g.translatef(p, 3, fr(-4.2));
      g.callList(1);
      g.loadIdentity();
      g.translatef(-p, -3, fr(-4.2));          // xor of the sign bit
      g.callList(1);
    }

    g.disable(GL.TEXTURE_2D);
    g.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
    g.lineWidth(8);
    g.color3f(0, 0, 0);
    g.translatef(0, 0, 10);                    // left on the modelview
  }

  // ==== 2 - lit sine sheet plus three line strips ==========================
  if (st.flags & 2) {
    st.lts = fr(st.ts / 18);

    g.translatef(0, 0, fr(-3.2));
    g.rotatef(fr(st.lts * 10), 0, 0, 1);

    g.enable(GL.LIGHTING);
    g.color3f(fr(0.8), fr(0.4), fr(0.2));
    g.begin(GL.QUADS);
    for (let i = -15; i < 15; i++) {
      st.i = i;
      const yf = i * fr(0.3);                  // kept unrounded, as fst does
      st.y = fr(yf);
      st.t1 = fr(Math.cos(yf + st.lts));
      st.t2 = fr(Math.sin(yf + (st.lts + fr(0.3))));

      for (let j = -15; j < 15; j++) {
        st.j = j;
        const xf = j * fr(0.3);
        st.x = fr(xf);
        st.k1 = fr(Math.sin(xf + st.lts));
        st.k2 = fr(Math.cos(xf + (st.lts + fr(0.3))));

        const x = st.x, y = st.y, x3 = fr(st.x + fr(0.3)), y3 = fr(st.y + fr(0.3));
        g.normal3f(x, y, fr(st.k1 + st.t1));
        g.vertex3f(x, y, fr(st.k1 + st.t1));
        g.vertex3f(x3, y, fr(st.k2 + st.t1));
        g.vertex3f(x3, y3, fr(st.k2 + st.t2));
        g.vertex3f(x, y3, fr(st.k1 + st.t2));
      }
    }
    g.end();

    g.disable(GL.DEPTH_TEST);
    g.disable(GL.LIGHTING);
    g.lineWidth(8);
    g.color3f(1, 0, 0);
    g.loadIdentity();
    g.translatef(0, 0, fr(-3.2));

    for (let i = 0; i < 3; i++) {
      st.i = i;
      g.begin(GL.LINE_STRIP);                  // ignored after the first
      for (let j = -15; j < 15; j++) {
        st.j = j;
        const xf = j * fr(0.3);
        st.x = fr(xf);
        const s = Math.sin(xf + st.lts + i * 30);
        const z = fr(s);
        const y = fr(st.x + st.x + st.t1);     // t1 is the mesh's last row
        g.vertex3f(fr(s + i), y, z);
      }
    }
    g.end();
  }

  // ==== 3 - the tall spinning cube with flares either side =================
  if (st.flags & 4) {
    g.enable(GL.LIGHTING);
    g.enable(GL.DEPTH_TEST);
    g.blendFunc(GL.DST_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
    g.color3f(1, 0, fr(0.2));
    g.loadIdentity();

    st.x = fr(Math.sin(st.ts / 64) * 20);
    g.rotatef(st.x, 0, 0, 1);
    g.translatef(0, 0, -5);
    g.scalef(1, 8, 1);
    g.rotatef(st.ts, 1, 0, 1);
    drawCube(g);

    g.disable(GL.LIGHTING);
    g.polygonMode(GL.FRONT_AND_BACK, GL.LINE);
    g.lineWidth(8);
    g.color3f(0, 0, 0);
    drawCube(g);
    g.polygonMode(GL.FRONT_AND_BACK, GL.FILL);

    g.disable(GL.DEPTH_TEST);
    g.enable(GL.TEXTURE_2D);
    g.color3f(1, 1, 1);
    g.blendFunc(GL.ONE, GL.ONE);

    for (let n = 0; n < MAX_PARTICLES; n++) {
      g.loadIdentity();
      g.rotatef(st.x, 0, 0, 1);
      g.translatef(-3, fr(px[n] - st.x / 10), -5);
      g.callList(1);
      g.translatef(6, 0, 0);
      g.callList(1);
    }
    g.disable(GL.TEXTURE_2D);
  }

  // ==== 4 - wide rubik grid with orbiting flares and streaks ===============
  if (st.flags & 8) {
    g.blendFunc(GL.DST_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
    g.lineWidth(2);
    g.color3f(1, fr(0.4), 0);

    drawRubic(g, 2, 3, -7, 0);

    g.enable(GL.TEXTURE_2D);
    g.color3f(1, 1, 1);
    for (let i = 0; i < MAX_PARTICLES; i++) {
      st.i = i;
      g.loadIdentity();
      g.translatef(fr(1.5), 0, 0);
      g.rotatef(fr(Math.sin(i) * 180 + st.ts), 0, 0, 1);
      g.translatef(fr(-0.8), 0, px[i]);
      g.callList(1);
      g.translatef(fr(1.6), 0, 0);
      g.callList(1);
    }

    g.disable(GL.TEXTURE_2D);
    g.lineWidth(1);
    g.color4f(1, 1, 1, fr(0.2));
    g.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);

    for (let i = 0; i < MAX_PARTICLES; i++) {
      st.i = i;
      g.loadIdentity();
      g.translatef(fr(1.5), 0, 0);
      g.rotatef(fr(Math.sin(i) * 180 + st.ts), 0, 0, 1);
      g.begin(GL.LINES);
      g.vertex3f(fr(-0.8), 0, px[i]);
      g.vertex3f(fr(0.8), 0, px[i]);
      g.end();
    }
  }

  // ==== 5 - dense wireframe grid, flare swarm and the smeared glyph ========
  if (st.flags & 16) {
    st.lts = fr(st.ts / 5);
    g.lineWidth(1);

    drawRubic(g, 9, 9, 0, 1);

    g.blendFunc(GL.ONE, GL.ONE);
    g.enable(GL.TEXTURE_2D);
    g.color3f(1, 1, 1);

    for (let i = 0; i < MAX_PARTICLES; i++) {
      st.i = i;
      g.loadIdentity();
      g.rotatef(fr(Math.sin(i) * 180 * 17 + st.lts), 0, 0, 1);
      g.translatef(px[i], 0, px[i]);
      g.callList(1);
    }

    g.disable(GL.TEXTURE_2D);
    g.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
    g.lineWidth(8);
    g.color4f(1, 1, 1, fr(0.3));

    for (let i = 0; i < 12; i++) {
      st.i = i;
      g.loadIdentity();
      g.translatef(0, 0, -5);
      g.rotatef(fr(Math.sin(-(st.lts * 4)) * 180 + i), 0, 0, 1);
      g.scalef(3, 5, 3);
      drawChin(g, 1);
    }
  }

  // ==== 6 - the two glyphs swept around the screen =========================
  if (st.flags & 32) {
    st.k1 = fr(Math.cos(st.ts) * 5);           // fsincos: cos first, then sin
    st.k2 = fr(Math.sin(st.ts) * 5);

    g.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
    g.lineWidth(8);
    g.color4f(1, 1, 1, fr(0.05));

    st.t1 = fr(-4.0);
    for (let which = 0; which < 2; which++) {
      const ty = which === 0 ? st.k1 : st.k2;
      for (let i = 0; i < 50; i++) {
        st.i = i;
        g.loadIdentity();
        g.rotatef(fr(i / 8), 0, 0, 1);
        g.translatef(st.t1, ty, -10);
        g.scalef(4, 6, 3);
        drawChin(g, 1 - which);
      }
      st.t1 = fr(4.0);
    }
  }

  // ==== 7 - flares on a bobbing ring ======================================
  if (st.flags & 64) {
    g.color3f(1, 1, 1);
    g.blendFunc(GL.ONE, GL.ONE);
    g.enable(GL.TEXTURE_2D);

    for (let i = 0; i < MAX_PARTICLES; i++) {
      st.i = i;
      g.loadIdentity();
      g.rotatef(fr(Math.sin(px[i] / 10) * 180 + st.ts), 0, 0, 1);
      g.translatef(4, 0, fr(Math.sin((st.ts + i) / 18) * 8 - 10));
      g.callList(1);
    }
    g.disable(GL.TEXTURE_2D);
  }

  // ==== 8 - the group logo, drawn as fat smeared points ====================
  if (st.flags & 128) {
    g.color4f(1, 1, fr(0.5), fr(0.1));
    g.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
    g.pointSize(37);

    for (let i = 0; i < 50; i++) {
      st.i = i;
      g.loadIdentity();
      g.translatef(0, 0, -15);
      g.rotatef(fr(i / 8), 0, 0, 1);
      g.scalef(4, 6, 3);
      drawChin(g, 8);
      drawChin(g, 9);
    }
  }

  advanceParticles();
  g.flush();
  return true;
}
