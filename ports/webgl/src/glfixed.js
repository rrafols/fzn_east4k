// ---------------------------------------------------------------------------
//  The fixed-function OpenGL 1.1 the intro was written against, on WebGL 2.
//
//  WebGL has none of what this intro uses: no glBegin, no matrix stack, no
//  display lists, no lighting, no glPolygonMode, no line width past 1.0, no
//  LINE_SMOOTH or POINT_SMOOTH.  Rather than rewrite the effects around what
//  WebGL does have, this file puts the old pipeline back, so frame.js and
//  draw.js can stay line-for-line transcriptions of the assembly - which
//  matters more than it sounds, because the intro leans on GL state carrying
//  from one effect to the next and from one frame to the next.  Blend mode is
//  never reset per frame, effect 2 inherits effect 1's modelview, and effect 5
//  draws with whatever glPolygonMode drawRubic left behind.  Emulating the
//  state machine gets all of that for free.
//
//  Transform and lighting happen here, on the CPU, in the same order the
//  fixed-function pipeline did them:
//
//    * triangles are handed to the GPU already in clip space, so WebGL's own
//      clipper and perspective-correct interpolation do the rest;
//    * wide lines and points become screen-space quads, which is what the
//      driver did with them - GL rasterises a width-8 line as an 8-pixel-wide
//      rectangle, and a smooth point as a circle - with LINE_SMOOTH and
//      POINT_SMOOTH coverage applied to *alpha only*, as GL does.  That last
//      detail is why the additively blended flares and lines look hard-edged
//      in the original and have to look hard-edged here.
//
//  Vertex counts are tiny (the biggest frame is a 30x30 quad mesh), so doing
//  it this way costs nothing and buys exactness.
// ---------------------------------------------------------------------------

const fr = Math.fround;

// -- enums, matching consts.inc ---------------------------------------------
export const GL = {
  DEPTH_BUFFER_BIT: 0x0100, COLOR_BUFFER_BIT: 0x4000,
  POINTS: 0, LINES: 1, LINE_STRIP: 3, QUADS: 7,
  ZERO: 0, ONE: 1, SRC_ALPHA: 0x0302, ONE_MINUS_SRC_ALPHA: 0x0303, DST_ALPHA: 0x0304,
  FRONT_AND_BACK: 0x0408,
  POINT_SMOOTH: 0x0B10, LINE_SMOOTH: 0x0B20, LIGHTING: 0x0B50,
  COLOR_MATERIAL: 0x0B57, DEPTH_TEST: 0x0B71, BLEND: 0x0BE2, TEXTURE_2D: 0x0DE1,
  LIGHT0: 0x4000, COMPILE: 0x1300, MODELVIEW: 0x1700, PROJECTION: 0x1701,
  LINE: 0x1B01, FILL: 0x1B02,
};

// ---------------------------------------------------------------------------
//  4x4 matrices, column-major as GL stores them, kept in single precision
//  because that is the only precision GL promises for them.
// ---------------------------------------------------------------------------
function matIdentity(m) {
  m.fill(0); m[0] = m[5] = m[10] = m[15] = 1;
  return m;
}

// dst = a * b
function matMul(dst, a, b) {
  for (let c = 0; c < 4; c++) {
    for (let r = 0; r < 4; r++) {
      let s = 0;
      for (let k = 0; k < 4; k++) s += a[k * 4 + r] * b[c * 4 + k];
      dst[c * 4 + r] = fr(s);
    }
  }
  return dst;
}

const tmpM = new Float32Array(16);
const tmpN = new Float32Array(16);

export class GL1 {
  constructor(gl, width, height) {
    this.gl = gl;
    this.width = width;
    this.height = height;

    this.projection = matIdentity(new Float32Array(16));
    this.modelview = matIdentity(new Float32Array(16));
    this.matrixMode = GL.MODELVIEW;
    this.normalMatrix = new Float32Array(9);
    this.normalDirty = true;

    this.color = new Float32Array([1, 1, 1, 1]);
    this.normal = new Float32Array([0, 0, 1]);
    this.uv = new Float32Array([0, 0]);

    this.en = {
      [GL.BLEND]: false, [GL.DEPTH_TEST]: false, [GL.LIGHTING]: false,
      [GL.LIGHT0]: false, [GL.COLOR_MATERIAL]: false, [GL.LINE_SMOOTH]: false,
      [GL.POINT_SMOOTH]: false, [GL.TEXTURE_2D]: false,
    };
    this.blendSrc = GL.ONE;
    this.blendDst = GL.ZERO;
    this.lineW = 1;
    this.pointS = 1;
    this.polyMode = GL.FILL;
    this.texture = 0;
    this.viewportRect = [0, 0, width, height];

    // immediate mode
    this.prim = -1;
    this.vbuf = [];                       // vertices since glBegin
    this.lists = new Map();
    this.recording = null;

    // batches
    this.tri = new Buf(10);
    this.sp = new Buf(12);
    this.batchKind = 0;                   // 0 none, 1 triangles, 2 screen-space
    this.batchState = -1;
    this.serial = 0;

    this.textures = new Map();
    this.debug = false;                   // set to check glGetError per batch
    this.initGPU();
  }

  // -- state ----------------------------------------------------------------

  enable(cap) { this.setEnable(cap, true); }
  disable(cap) { this.setEnable(cap, false); }
  setEnable(cap, v) {
    if (this.en[cap] === v) return;
    // Lighting only changes the colours computed here, so it needs no flush.
    if (cap !== GL.LIGHTING && cap !== GL.COLOR_MATERIAL && cap !== GL.LIGHT0) {
      this.flush();
      this.serial++;
    }
    this.en[cap] = v;
  }

  blendFunc(s, d) {
    if (this.blendSrc === s && this.blendDst === d) return;
    this.flush();
    this.serial++;
    this.blendSrc = s; this.blendDst = d;
  }

  lineWidth(w) { if (this.lineW !== w) { this.flush(); this.lineW = w; } }
  pointSize(s) { if (this.pointS !== s) { this.flush(); this.pointS = s; } }
  polygonMode(face, mode) { this.polyMode = mode; }

  color3f(r, g, b) { this.color[0] = fr(r); this.color[1] = fr(g); this.color[2] = fr(b); this.color[3] = 1; }
  color4f(r, g, b, a) { this.color[0] = fr(r); this.color[1] = fr(g); this.color[2] = fr(b); this.color[3] = fr(a); }
  normal3f(x, y, z) { this.normal[0] = fr(x); this.normal[1] = fr(y); this.normal[2] = fr(z); }
  texCoord2f(u, v) {
    if (this.recording) { this.recording.push([0, u, v]); return; }
    this.uv[0] = fr(u); this.uv[1] = fr(v);
  }

  viewport(x, y, w, h) { this.viewportRect = [x, y, w, h]; }

  // -- matrices -------------------------------------------------------------

  cur() { return this.matrixMode === GL.PROJECTION ? this.projection : this.modelview; }
  touched() { if (this.matrixMode === GL.MODELVIEW) this.normalDirty = true; }

  setMatrixMode(m) { this.matrixMode = m; }
  loadIdentity() { matIdentity(this.cur()); this.touched(); }

  translatef(x, y, z) {
    const m = matIdentity(tmpM);
    m[12] = fr(x); m[13] = fr(y); m[14] = fr(z);
    const c = this.cur();
    c.set(matMul(tmpN, c, m));
    this.touched();
  }

  scalef(x, y, z) {
    const m = matIdentity(tmpM);
    m[0] = fr(x); m[5] = fr(y); m[10] = fr(z);
    const c = this.cur();
    c.set(matMul(tmpN, c, m));
    this.touched();
  }

  // glRotatef, built the way the GL spec spells it out.
  rotatef(angle, x, y, z) {
    const len = Math.sqrt(x * x + y * y + z * z);
    if (len === 0) return;
    x /= len; y /= len; z /= len;
    const a = angle * Math.PI / 180;
    const c = Math.cos(a), s = Math.sin(a), t = 1 - c;
    const m = matIdentity(tmpM);
    m[0] = fr(t * x * x + c);     m[4] = fr(t * x * y - s * z); m[8]  = fr(t * x * z + s * y);
    m[1] = fr(t * x * y + s * z); m[5] = fr(t * y * y + c);     m[9]  = fr(t * y * z - s * x);
    m[2] = fr(t * x * z - s * y); m[6] = fr(t * y * z + s * x); m[10] = fr(t * z * z + c);
    const cm = this.cur();
    cm.set(matMul(tmpN, cm, m));
    this.touched();
  }

  // gluPerspective, which is where the intro's 80-degree field of view and its
  // hard-coded 4:3 come from.
  perspective(fovy, aspect, zNear, zFar) {
    const f = 1 / Math.tan(fovy * Math.PI / 360);
    const m = tmpM;
    m.fill(0);
    m[0] = fr(f / aspect);
    m[5] = fr(f);
    m[10] = fr((zFar + zNear) / (zNear - zFar));
    m[11] = -1;
    m[14] = fr(2 * zFar * zNear / (zNear - zFar));
    const c = this.cur();
    c.set(matMul(tmpN, c, m));
    this.touched();
  }

  updateNormalMatrix() {
    // Inverse transpose of the modelview's upper-left 3x3.  Nothing enables
    // GL_NORMALIZE, so the result is used unnormalised - which is exactly why
    // the cube scaled to (1, 8, 1) in effect 3 shades the way it does.
    const m = this.modelview, n = this.normalMatrix;
    const a = m[0], b = m[4], c = m[8];
    const d = m[1], e = m[5], f2 = m[9];
    const g = m[2], h = m[6], i = m[10];
    const A = e * i - f2 * h, B = f2 * g - d * i, C = d * h - e * g;
    let det = a * A + b * B + c * C;
    if (det === 0) det = 1;
    const id = 1 / det;
    // inverse = adj/det; then transpose.  Row-major here: n[row*3+col].
    n[0] = A * id;               n[1] = B * id;               n[2] = C * id;
    n[3] = (c * h - b * i) * id; n[4] = (a * i - c * g) * id; n[5] = (b * g - a * h) * id;
    n[6] = (b * f2 - c * e) * id; n[7] = (c * d - a * f2) * id; n[8] = (a * e - b * d) * id;
    this.normalDirty = false;
  }

  // -- immediate mode -------------------------------------------------------

  begin(mode) {
    if (this.recording) { this.recording.push([2, mode]); return; }
    // glBegin inside a glBegin is GL_INVALID_OPERATION and is ignored.  Effect
    // 2 does exactly that - three glBegin(GL_LINE_STRIP) against one glEnd -
    // so its three strips are really one, joined across the gaps.  Keep it.
    if (this.prim >= 0) return;
    this.prim = mode;
    this.vbuf.length = 0;
  }

  vertex3f(x, y, z) {
    if (this.recording) { this.recording.push([1, x, y, z]); return; }
    const mv = this.modelview, p = this.projection;
    x = fr(x); y = fr(y); z = fr(z);

    // eye = MV * v
    const ex = mv[0] * x + mv[4] * y + mv[8] * z + mv[12];
    const ey = mv[1] * x + mv[5] * y + mv[9] * z + mv[13];
    const ez = mv[2] * x + mv[6] * y + mv[10] * z + mv[14];
    const ew = mv[3] * x + mv[7] * y + mv[11] * z + mv[15];
    // clip = P * eye
    const cx = p[0] * ex + p[4] * ey + p[8] * ez + p[12] * ew;
    const cy = p[1] * ex + p[5] * ey + p[9] * ez + p[13] * ew;
    const cz = p[2] * ex + p[6] * ey + p[10] * ez + p[14] * ew;
    const cw = p[3] * ex + p[7] * ey + p[11] * ez + p[15] * ew;

    let r = this.color[0], g = this.color[1], b = this.color[2];
    const a = this.color[3];
    if (this.en[GL.LIGHTING]) {
      if (this.normalDirty) this.updateNormalMatrix();
      const n = this.normalMatrix, nx = this.normal[0], ny = this.normal[1], nz = this.normal[2];
      // Only the eye-space z of the normal matters: LIGHT0's default position
      // is (0, 0, 1, 0), a directional light straight down +Z, already unit
      // length, and nothing in the intro ever calls glLight*.
      const nez = n[6] * nx + n[7] * ny + n[8] * nz;
      // emission 0 + ambient(0.2) * C + max(N.L, 0) * diffuse(1) * C
      const k = 0.2 + (nez > 0 ? nez : 0);
      r = r * k; g = g * k; b = b * k;
      if (r > 1) r = 1; if (g > 1) g = 1; if (b > 1) b = 1;
    }
    this.vbuf.push(cx, cy, cz, cw, r, g, b, a, this.uv[0], this.uv[1]);
  }

  end() {
    if (this.recording) { this.recording.push([3]); return; }
    if (this.prim < 0) return;
    const v = this.vbuf, n = v.length / 10;
    switch (this.prim) {
      case GL.QUADS:
        for (let i = 0; i + 3 < n; i += 4) {
          if (this.polyMode === GL.LINE) this.emitPolyLines(v, [i, i + 1, i + 2, i + 3]);
          else { this.emitTri(v, i, i + 1, i + 2); this.emitTri(v, i, i + 2, i + 3); }
        }
        break;
      case GL.LINES:
        for (let i = 0; i + 1 < n; i += 2) this.emitLine(v, i, i + 1);
        break;
      case GL.LINE_STRIP:
        for (let i = 0; i + 1 < n; i++) this.emitLine(v, i, i + 1);
        break;
      case GL.POINTS:
        for (let i = 0; i < n; i++) this.emitPoint(v, i);
        break;
    }
    this.prim = -1;
    this.vbuf.length = 0;
  }

  // -- display lists --------------------------------------------------------

  newList(id, mode) { this.recording = []; this.listId = id; }
  endList() { this.lists.set(this.listId, this.recording); this.recording = null; }
  callList(id) {
    const ops = this.lists.get(id);
    if (!ops) return;
    for (const o of ops) {
      if (o[0] === 1) this.vertex3f(o[1], o[2], o[3]);
      else if (o[0] === 0) this.texCoord2f(o[1], o[2]);
      else if (o[0] === 2) this.begin(o[1]);
      else this.end();
    }
  }

  // -- primitive emission ---------------------------------------------------

  emitTri(v, i0, i1, i2) {
    this.want(1);
    const b = this.tri;
    b.push10(v, i0 * 10);
    b.push10(v, i1 * 10);
    b.push10(v, i2 * 10);
  }

  // A wide line is a screen-space rectangle from p0 to p1, which is what the
  // rasteriser made of it.  Clip against the near plane first: the tall cube in
  // effect 3 runs straight through the eye.
  emitLine(v, i0, i1) {
    const s = clipNear(v, i0 * 10, i1 * 10);
    if (s) this.lineQuad(s[0], s[1]);
  }

  lineQuad(a, b) {
    const p0 = this.toWindow(a), p1 = this.toWindow(b);
    if (!p0 || !p1) return;
    const dx = p1[0] - p0[0], dy = p1[1] - p0[1];
    const len = Math.hypot(dx, dy);
    if (len === 0) return;
    const half = this.lineW / 2;
    const pad = half + 1;                          // room for the coverage ramp
    const nx = -(dy / len) * pad, ny = (dx / len) * pad;
    this.want(2);
    const q = this.sp;
    const sm = this.en[GL.LINE_SMOOTH] ? 1 : 0;
    q.line(p0, a, nx, ny, -pad, half, 0, sm);
    q.line(p0, a, -nx, -ny, pad, half, 0, sm);
    q.line(p1, b, -nx, -ny, pad, half, 0, sm);
    q.line(p0, a, nx, ny, -pad, half, 0, sm);
    q.line(p1, b, -nx, -ny, pad, half, 0, sm);
    q.line(p1, b, nx, ny, -pad, half, 0, sm);
  }

  emitPoint(v, i) {
    const o = i * 10;
    if (v[o + 3] <= 0 || v[o + 2] + v[o + 3] < 0) return;
    const src = v.slice(o, o + 10);
    const p = this.toWindow(src);
    if (!p) return;
    const half = this.pointS / 2, pad = half + 1;
    this.want(2);
    const q = this.sp;
    const sm = this.en[GL.POINT_SMOOTH] ? 1 : 0;
    q.point(p, src, -pad, -pad, half, 1, sm);
    q.point(p, src,  pad, -pad, half, 1, sm);
    q.point(p, src,  pad,  pad, half, 1, sm);
    q.point(p, src, -pad, -pad, half, 1, sm);
    q.point(p, src,  pad,  pad, half, 1, sm);
    q.point(p, src, -pad,  pad, half, 1, sm);
  }

  // glPolygonMode(GL_LINE): the polygon is clipped, then its boundary is drawn
  // as lines.  Only the near plane needs clipping here - clipping against the
  // sides would put an edge along the screen border, which the original does
  // not show.
  emitPolyLines(v, idx) {
    let poly = idx.map(i => v.slice(i * 10, i * 10 + 10));
    poly = clipPolyNear(poly);
    if (poly.length < 2) return;
    for (let i = 0; i < poly.length; i++)
      this.lineQuad(poly[i], poly[(i + 1) % poly.length]);
  }

  toWindow(a) {
    const w = a[3];
    if (!(w > 0)) return null;
    const [vx, vy, vw, vh] = this.viewportRect;
    return [
      (a[0] / w * 0.5 + 0.5) * vw + vx,
      (a[1] / w * 0.5 + 0.5) * vh + vy,
      a[2] / w,
    ];
  }

  // -- batching -------------------------------------------------------------

  // Every state change that the GPU can see bumps a serial, so want() - which
  // runs once per primitive - is an integer compare rather than a rebuild of
  // the state.  Draw order is preserved because any such change flushes first.
  want(kind) {
    if (this.batchKind !== kind || this.batchState !== this.serial) {
      this.flush();
      this.batchKind = kind;
      this.batchState = this.serial;
      this.batchBlendOn = this.en[GL.BLEND];
      this.batchSrc = this.blendSrc;
      this.batchDst = this.blendDst;
      this.batchDepth = this.en[GL.DEPTH_TEST];
      this.batchTex = this.en[GL.TEXTURE_2D] ? this.texture : -1;
    }
  }

  // -- the GPU side ---------------------------------------------------------

  initGPU() {
    const gl = this.gl;

    this.progTri = program(gl, VS_TRI, FS_TRI);
    this.progSP = program(gl, VS_SP, FS_SP);
    this.progBlit = program(gl, VS_BLIT, FS_BLIT);
    this.uUseTex = gl.getUniformLocation(this.progTri, 'uUseTex');
    this.uTex = gl.getUniformLocation(this.progTri, 'uTex');
    this.uSize = gl.getUniformLocation(this.progSP, 'uSize');
    this.uBlitTex = gl.getUniformLocation(this.progBlit, 'uTex');

    this.bufTri = gl.createBuffer();
    this.vaoTri = gl.createVertexArray();
    gl.bindVertexArray(this.vaoTri);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.bufTri);
    attrib(gl, this.progTri, 'aPos', 4, 10, 0);
    attrib(gl, this.progTri, 'aColor', 4, 10, 4);
    attrib(gl, this.progTri, 'aUV', 2, 10, 8);

    this.bufSP = gl.createBuffer();
    this.vaoSP = gl.createVertexArray();
    gl.bindVertexArray(this.vaoSP);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.bufSP);
    attrib(gl, this.progSP, 'aPos', 3, 12, 0);
    attrib(gl, this.progSP, 'aColor', 4, 12, 3);
    attrib(gl, this.progSP, 'aCoord', 2, 12, 7);
    attrib(gl, this.progSP, 'aParam', 3, 12, 9);
    gl.bindVertexArray(null);

    // The intro renders at 640x480 because glLineWidth and glPointSize are in
    // pixels and the look is calibrated to that resolution; the result is
    // scaled up afterwards.  RGBA8 and a 24-bit depth buffer match the
    // reference build's framebuffer object exactly, which matters because
    // effect 3 and effect 4 blend against destination alpha.
    this.fbo = gl.createFramebuffer();
    this.fboTex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, this.fboTex);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA8, this.width, this.height, 0,
                  gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    this.fboDepth = gl.createRenderbuffer();
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.fboDepth);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT24, this.width, this.height);
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.fbo);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.fboTex, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.fboDepth);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // Bound whenever the intro has no texture enabled.  The fragment shader
    // references its sampler statically, so *something* has to be bound; if
    // that something is left over from present() it is this framebuffer's own
    // texture, and the draw becomes a feedback loop that the browser drops.
    this.blankTex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, this.blankTex);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE,
                  new Uint8Array([255, 255, 255, 255]));
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

    this.quadBuf = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.quadBuf);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      -1, -1, 0, 0,  1, -1, 1, 0,  1, 1, 1, 1,
      -1, -1, 0, 0,  1, 1, 1, 1,  -1, 1, 0, 1]), gl.STATIC_DRAW);
    this.vaoBlit = gl.createVertexArray();
    gl.bindVertexArray(this.vaoBlit);
    attrib(gl, this.progBlit, 'aPos', 2, 4, 0);
    attrib(gl, this.progBlit, 'aUV', 2, 4, 2);
    gl.bindVertexArray(null);

    gl.disable(gl.CULL_FACE);
    gl.disable(gl.DITHER);
    gl.depthFunc(gl.LESS);
  }

  // -- textures -------------------------------------------------------------
  //
  //  The intro leaves the flare in texture object 0, the default texture,
  //  which WebGL does not have; names are mapped to real objects here.

  texObj(name) {
    let t = this.textures.get(name);
    if (!t) {
      t = this.gl.createTexture();
      this.textures.set(name, t);
      this.gl.bindTexture(this.gl.TEXTURE_2D, t);
      this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.NEAREST_MIPMAP_LINEAR);
    }
    return t;
  }

  bindTexture(target, name) {
    if (this.texture === name) return;
    this.flush();
    this.serial++;
    this.texture = name;
  }

  texParameteri(target, pname, value) {
    const gl = this.gl;
    this.flush();
    gl.bindTexture(gl.TEXTURE_2D, this.texObj(this.texture));
    gl.texParameteri(gl.TEXTURE_2D, pname, value);
  }

  // GL_RGBA is a fixed-point internal format, so the float values are clamped
  // and quantised on upload - which the flare needs, its red channel peaking
  // at 1.6.
  texImage2DFloat(w, h, floats) {
    const gl = this.gl;
    this.flush();
    const px = new Uint8Array(w * h * 4);
    for (let i = 0; i < px.length; i++) {
      const v = floats[i];
      px[i] = Math.round((v < 0 ? 0 : v > 1 ? 1 : v) * 255);
    }
    gl.bindTexture(gl.TEXTURE_2D, this.texObj(this.texture));
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, px);
  }

  // -- frame ----------------------------------------------------------------

  beginFrame() {
    const gl = this.gl;
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.fbo);
    gl.viewport(0, 0, this.width, this.height);
    gl.disable(gl.SCISSOR_TEST);
  }

  clearColorf(r, g, b, a) { this.clearCol = [r, g, b, a]; }

  clear(mask) {
    const gl = this.gl;
    this.flush();
    const c = this.clearCol || [0, 0, 0, 0];
    gl.clearColor(c[0], c[1], c[2], c[3]);
    gl.colorMask(true, true, true, true);
    gl.depthMask(true);
    gl.clear((mask & GL.COLOR_BUFFER_BIT ? gl.COLOR_BUFFER_BIT : 0) |
             (mask & GL.DEPTH_BUFFER_BIT ? gl.DEPTH_BUFFER_BIT : 0));
  }

  flush() {
    const gl = this.gl;
    const kind = this.batchKind;
    if (kind === 0) return;
    const buf = kind === 1 ? this.tri : this.sp;
    if (buf.n === 0) { this.batchKind = 0; this.batchState = -1; return; }

    if (this.batchBlendOn) { gl.enable(gl.BLEND); gl.blendFunc(this.batchSrc, this.batchDst); }
    else gl.disable(gl.BLEND);
    if (this.batchDepth) { gl.enable(gl.DEPTH_TEST); gl.depthMask(true); }
    else { gl.disable(gl.DEPTH_TEST); gl.depthMask(false); }

    if (kind === 1) {
      gl.useProgram(this.progTri);
      gl.bindVertexArray(this.vaoTri);
      gl.bindBuffer(gl.ARRAY_BUFFER, this.bufTri);
      gl.bufferData(gl.ARRAY_BUFFER, buf.a.subarray(0, buf.n), gl.STREAM_DRAW);
      const useTex = this.batchTex >= 0;
      gl.uniform1i(this.uUseTex, useTex ? 1 : 0);
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, useTex ? this.texObj(this.batchTex) : this.blankTex);
      gl.uniform1i(this.uTex, 0);
      gl.drawArrays(gl.TRIANGLES, 0, buf.n / 10);
    } else {
      gl.useProgram(this.progSP);
      gl.bindVertexArray(this.vaoSP);
      gl.bindBuffer(gl.ARRAY_BUFFER, this.bufSP);
      gl.bufferData(gl.ARRAY_BUFFER, buf.a.subarray(0, buf.n), gl.STREAM_DRAW);
      gl.uniform2f(this.uSize, this.width, this.height);
      gl.drawArrays(gl.TRIANGLES, 0, buf.n / 12);
    }
    if (this.debug) {
      const e = gl.getError();
      if (e) throw new Error('GL error 0x' + e.toString(16) + ' after a ' +
                             (kind === 1 ? 'triangle' : 'screen-space') + ' batch');
    }
    buf.reset();
    this.batchKind = 0;
    this.batchState = -1;
  }

  // Scale the 640x480 frame into a 4:3 box on the canvas, letterboxed, the way
  // the macOS port blits its corner of the back buffer to the display.
  present(canvasW, canvasH) {
    const gl = this.gl;
    this.flush();
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.viewport(0, 0, canvasW, canvasH);
    gl.disable(gl.BLEND);
    gl.disable(gl.DEPTH_TEST);
    gl.depthMask(false);
    gl.clearColor(0, 0, 0, 1);
    gl.clear(gl.COLOR_BUFFER_BIT);

    let w = Math.floor(canvasH * 4 / 3), h = canvasH;
    if (w > canvasW) { w = canvasW; h = Math.floor(canvasW * 3 / 4); }
    gl.viewport((canvasW - w) >> 1, (canvasH - h) >> 1, w, h);

    gl.useProgram(this.progBlit);
    gl.bindVertexArray(this.vaoBlit);
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.fboTex);
    gl.uniform1i(this.uBlitTex, 0);
    gl.drawArrays(gl.TRIANGLES, 0, 6);
    gl.bindVertexArray(null);
    gl.bindTexture(gl.TEXTURE_2D, null);      // never leave the target sampled
  }

  // The verification path: the same RGB bytes glReadPixels hands back, bottom
  // row first, so they can go straight into a PPM.
  readPixels() {
    const gl = this.gl;
    this.flush();
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.fbo);
    const rgba = new Uint8Array(this.width * this.height * 4);
    gl.readPixels(0, 0, this.width, this.height, gl.RGBA, gl.UNSIGNED_BYTE, rgba);
    const rgb = new Uint8Array(this.width * this.height * 3);
    for (let i = 0, o = 0; i < rgba.length; i += 4) {
      rgb[o++] = rgba[i]; rgb[o++] = rgba[i + 1]; rgb[o++] = rgba[i + 2];
    }
    return rgb;
  }
}

// ---------------------------------------------------------------------------
//  Shaders.  All the pipeline work has happened by the time anything gets
//  here: the triangle program just passes clip coordinates through, and the
//  screen-space program turns a pixel offset into the coverage the smooth
//  line and point rules ask for.
// ---------------------------------------------------------------------------

const VS_TRI = `#version 300 es
in vec4 aPos; in vec4 aColor; in vec2 aUV;
out vec4 vColor; out vec2 vUV;
void main() { vColor = aColor; vUV = aUV; gl_Position = aPos; }`;

const FS_TRI = `#version 300 es
precision highp float;
in vec4 vColor; in vec2 vUV;
uniform sampler2D uTex; uniform bool uUseTex;
out vec4 o;
void main() {
  vec4 c = vColor;
  if (uUseTex) c *= texture(uTex, vUV);    // GL_MODULATE, the default tex env
  o = c;
}`;

const VS_SP = `#version 300 es
in vec3 aPos; in vec4 aColor; in vec2 aCoord; in vec3 aParam;
uniform vec2 uSize;
out vec4 vColor; out vec2 vCoord; out vec3 vParam;
void main() {
  vColor = aColor; vCoord = aCoord; vParam = aParam;
  gl_Position = vec4(aPos.x / uSize.x * 2.0 - 1.0,
                     aPos.y / uSize.y * 2.0 - 1.0, aPos.z, 1.0);
}`;

const FS_SP = `#version 300 es
precision highp float;
in vec4 vColor; in vec2 vCoord; in vec3 vParam;
out vec4 o;
void main() {
  float halfExtent = vParam.x;
  // lines measure across the segment, points measure radially
  float d = vParam.y < 0.5 ? abs(vCoord.y) : length(vCoord);
  float cov = vParam.z > 0.5 ? clamp(halfExtent + 0.5 - d, 0.0, 1.0)
                             : (d <= halfExtent ? 1.0 : 0.0);
  if (cov <= 0.0) discard;
  // GL applies antialiasing coverage to alpha, not to colour - so a smooth
  // line under GL_ONE/GL_ONE blending has hard edges, and has to keep them.
  o = vec4(vColor.rgb, vColor.a * cov);
}`;

const VS_BLIT = `#version 300 es
in vec2 aPos; in vec2 aUV; out vec2 vUV;
void main() { vUV = aUV; gl_Position = vec4(aPos, 0.0, 1.0); }`;

const FS_BLIT = `#version 300 es
precision highp float;
in vec2 vUV; uniform sampler2D uTex; out vec4 o;
void main() { o = vec4(texture(uTex, vUV).rgb, 1.0); }`;

function shader(gl, type, src) {
  const s = gl.createShader(type);
  gl.shaderSource(s, src);
  gl.compileShader(s);
  if (!gl.getShaderParameter(s, gl.COMPILE_STATUS))
    throw new Error(gl.getShaderInfoLog(s) + '\n' + src);
  return s;
}

function program(gl, vs, fs) {
  const p = gl.createProgram();
  gl.attachShader(p, shader(gl, gl.VERTEX_SHADER, vs));
  gl.attachShader(p, shader(gl, gl.FRAGMENT_SHADER, fs));
  gl.linkProgram(p);
  if (!gl.getProgramParameter(p, gl.LINK_STATUS))
    throw new Error(gl.getProgramInfoLog(p));
  return p;
}

function attrib(gl, prog, name, size, stride, offset) {
  const loc = gl.getAttribLocation(prog, name);
  if (loc < 0) return;
  gl.enableVertexAttribArray(loc);
  gl.vertexAttribPointer(loc, size, gl.FLOAT, false, stride * 4, offset * 4);
}

// ---------------------------------------------------------------------------
//  Growable float buffers for the two batch kinds.
// ---------------------------------------------------------------------------
class Buf {
  constructor(stride) {
    this.stride = stride;
    this.a = new Float32Array(stride * 4096);
    this.n = 0;
  }
  room(k) {
    if (this.n + k <= this.a.length) return;
    const b = new Float32Array(Math.max(this.a.length * 2, this.n + k));
    b.set(this.a.subarray(0, this.n));
    this.a = b;
  }
  push10(v, o) {
    this.room(10);
    for (let i = 0; i < 10; i++) this.a[this.n++] = v[o + i];
  }
  // screen-space vertex: x, y, z, rgba, coord.xy, halfExtent, kind, smooth
  line(p, src, ox, oy, d, half, kind, sm) {
    this.room(12);
    const a = this.a;
    a[this.n++] = p[0] + ox; a[this.n++] = p[1] + oy; a[this.n++] = p[2];
    a[this.n++] = src[4]; a[this.n++] = src[5]; a[this.n++] = src[6]; a[this.n++] = src[7];
    a[this.n++] = 0; a[this.n++] = d;
    a[this.n++] = half; a[this.n++] = kind; a[this.n++] = sm;
  }
  point(p, src, ox, oy, half, kind, sm) {
    this.room(12);
    const a = this.a;
    a[this.n++] = p[0] + ox; a[this.n++] = p[1] + oy; a[this.n++] = p[2];
    a[this.n++] = src[4]; a[this.n++] = src[5]; a[this.n++] = src[6]; a[this.n++] = src[7];
    a[this.n++] = ox; a[this.n++] = oy;
    a[this.n++] = half; a[this.n++] = kind; a[this.n++] = sm;
  }
  reset() { this.n = 0; }
}

// ---------------------------------------------------------------------------
//  Near-plane clipping in homogeneous clip space (z + w >= 0).
// ---------------------------------------------------------------------------
const NEAR_EPS = 1e-7;

function clipNear(v, o0, o1) {
  const a = v.slice(o0, o0 + 10), b = v.slice(o1, o1 + 10);
  const da = a[2] + a[3], db = b[2] + b[3];
  if (da >= NEAR_EPS && db >= NEAR_EPS) return [a, b];
  if (da < NEAR_EPS && db < NEAR_EPS) return null;
  const t = da / (da - db);
  const m = lerp10(a, b, t);
  return da >= NEAR_EPS ? [a, m] : [m, b];
}

function clipPolyNear(poly) {
  const out = [];
  for (let i = 0; i < poly.length; i++) {
    const a = poly[i], b = poly[(i + 1) % poly.length];
    const da = a[2] + a[3], db = b[2] + b[3];
    if (da >= NEAR_EPS) out.push(a);
    if ((da >= NEAR_EPS) !== (db >= NEAR_EPS)) out.push(lerp10(a, b, da / (da - db)));
  }
  return out;
}

function lerp10(a, b, t) {
  const m = new Array(10);
  for (let i = 0; i < 10; i++) m[i] = a[i] + (b[i] - a[i]) * t;
  return m;
}
