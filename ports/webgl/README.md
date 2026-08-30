# Looking for the East — WebGL port

A port of the Fuzzion 4k intro *Looking for the East* (2003) to WebGL 2, from
the x86-64 macOS port. Same effects, same procedurally generated instruments,
same module. No build step, no dependencies — the intro is ES modules, served
as they are.

    make serve      # then open http://127.0.0.1:8000/ports/webgl/
    make bundle     # -> build/east.html, one file, opens without a server
    make pack       # -> build/east.min.html, the same thing at 12.6 KB

    make verify     # compare thirteen frames and the tune against the macOS port
    make audio      # just the tune, in node

This is a *preservation* port, not a 4k one: it is written to be as close to the
original as the platform allows, and the size of the result was not a
consideration.

## How close it gets

**The audio is bit-exact.** All 5,067,216 samples of the mixed tune are
identical to what the macOS port produces on real x87 hardware. That is not
the obvious outcome, and the rest of this section is mostly about why.

**The frames are not, and cannot be.** Two rasterisers do not agree byte for
byte. Measured against the macOS port's reference frames, on the eight of the
thirteen that are deterministic:

| | |
|---|---|
| root mean square error | 2.3 – 9.2 out of 255 |
| pixels differing at all | 4.4% – 32% |
| pixels off by more than 8/255 | 1,735 – 13,040 of 307,200 |
| **structurally different pixels** | **24 – 575, i.e. 0.008% – 0.19%** |

Nearly all of it is a one-pixel outline along every edge, where a wide line
lands a fraction of a pixel to one side. `verify.html` separates that from real
differences: a pixel counts as *structural* only when its colour falls outside
the range the reference covers in the 3×3 around it, so a shifted or
differently-weighted antialiased edge does not count but missing or mispositioned
geometry does. As a check that the measure has teeth, the five frames where
effect 1 appears — whose background lines are seeded from the clock, and so
differ between any two runs of the original too — score 10–16% structural.

## Why the audio was the hard part

The instrument generator keeps phase, frequency, amplitude and the two filter
state variables in x87 registers, which carry a 64-bit mantissa. Instrument 1
runs a sine at **3.1e12 Hz** for 4096 samples, so its phase accumulator reaches
about 1.3e16 — where a JavaScript double has an ulp of **2.0 radians** and the
sine argument is simply gone. WebAssembly does not help; it has no extended
precision either.

`src/x87.js` is therefore an 80-bit float, with a BigInt mantissa: every add,
multiply and divide is done exactly and rounded once to 64 bits, ties to even,
which is what the hardware does with the control word `finit` leaves behind.
`fsin` reduces against the same 66-bit approximation of π the 8087 uses
(`0x3243F6A8885A308D3 / 2^64`), because a correctly-reduced sine differs from
the hardware's by about 1e-4 radians at that magnitude. The reduction is exact
in BigInt; only the final |r| ≤ π/4 goes through a double sine, thirteen orders
of magnitude finer than the 16-bit sample it becomes.

Two integer details also turned out to matter, and both are visible in the
result. `fist`/`fistp` round to nearest even and store x87's *integer
indefinite* 0x8000 on overflow — which instruments 6 and 7 hit after about 438
samples, because their saw phase runs far past 32767, so their oscillator goes
silent and you hear the filter ringing on alone. And the saw wraps through a
signed byte taken from that 16-bit store, not from the phase directly.

Generating the instruments and mixing the tune takes about 0.8 s of BigInt
arithmetic, once, before the intro starts.

## What had to change

WebGL has none of what the intro uses. Rather than rewrite the effects around
what WebGL does have, `src/glfixed.js` puts the old pipeline back, so
`src/frame.js` and `src/draw.js` stay line-for-line transcriptions of
`frame.inc` and `draw.inc`.

| | fixed-function OpenGL 1.1 | here |
|---|---|---|
| geometry | `glBegin` / `glVertex3f` | collected, transformed on the CPU, batched |
| matrices | the matrix stack, `gluPerspective` | the same matrices, kept in single precision |
| lighting | `GL_LIGHT0` + `GL_COLOR_MATERIAL` | per-vertex, on the CPU, from the same defaults |
| the flare | a display list | recorded and replayed through the same vertex path |
| wide lines | `glLineWidth(8)` | screen-space rectangles, near-plane clipped |
| wireframe | `glPolygonMode(GL_LINE)` | the clipped polygon's edges, as lines |
| smoothing | `GL_LINE_SMOOTH`, `GL_POINT_SMOOTH` | coverage in the fragment shader, applied to alpha |
| resolution | 640×480, blitted up | an RGBA8 + depth24 framebuffer, blitted up |

Three of those are worth spelling out.

**Transform and lighting happen on the CPU**, in the pipeline's order, and
triangles reach the GPU already in clip space so WebGL's own clipper and
perspective-correct interpolation finish the job. Vertex counts are tiny — the
biggest frame is a 30×30 quad mesh — so this costs nothing and buys exactness.
It also means `GL_NORMALIZE` staying off is free: the cube scaled to (1, 8, 1)
in effect 3 shades exactly as wrongly as it should.

**Line and point smoothing multiply *alpha*, not colour**, which is what GL
does. It is why the additively blended flares and lines look hard-edged in the
original, and they have to keep looking that way.

**Rendering happens at 640×480 and is scaled up**, because `glLineWidth` and
`glPointSize` are in pixels and the whole look is calibrated to that. The
framebuffer is RGBA8 with a real alpha channel, matching the reference build's,
because effects 3 and 4 blend against destination alpha.

## The state machine, and the bugs in it

Emulating GL state rather than flattening it is what makes the port faithful,
because the intro leans on state carrying between effects and between frames:

* effect 2 never calls `glLoadIdentity` before its mesh, so it inherits
  whatever modelview effect 1 left behind — and effect 1 deliberately ends with
  a `glTranslatef` after its last flare;
* effect 2's three line strips call `glBegin` three times against one `glEnd`,
  so the second and third are `GL_INVALID_OPERATION` and are ignored, and all
  ninety vertices end up in **one** strip, joined across the gaps. That is what
  the crossing red lines in the original are;
* effect 2's line strips read `v_t1`, which nothing in that loop sets — it
  still holds `cos(4.2 + lts)` from the last row of the mesh above;
* blend mode, polygon mode and `GL_LIGHTING` are never reset per frame;
* `drawRubic` overwrites `v_k1`, `v_k2` and `v_t1`, and leaves the current
  colour at black.

All of it is reproduced.

## One correction to `common/doc/effects.md`

That file says the particles' "position starts in [-20, 20]". The code is
`fild 40; call frand; fisub 40`, which is **[-60, -19]** — they drift in from
off-screen and only reach the [-20, 20] band the effects are framed around
after a few hundred frames of wrapping. The reference frames confirm it:
`shot04.ppm` renders effect 1 on its own at frame 4 and has no flares in it at
all, because they are all still far off to the left. The velocity range in that
file, [0, 0.25], is right.

This matters for anyone else porting the intro, because the offscreen
verification frames advance the field once per rendered frame, so the seed is
what those frames actually show.

## Verifying it

`make verify` renders the thirteen frames from `offscreen.inc`'s `shot_ms`
table in a headless browser and compares them with
`../macos-x86_64/build/linked/`, then mixes the tune and compares it with that
directory's `song.wav`. Build those first, with `make verify` in
`../macos-x86_64`.

`verify.html` is the same thing in a browser, with the reference, this port's
frame, and an amplified difference side by side. `shot.html?n=3` renders one
frame, `&diff=1` renders the difference instead.

The five effect-1 frames are expected to differ, exactly as `common/tools/compare.py`
expects them to between two runs of the macOS port.

The harness drives `beginFrame` / `drawFrame` / `present` — the intro's own
loop — rather than just rendering frames, and it renders with `debug` on so a
dropped draw call raises instead of quietly producing a wrong frame. Both of
those are there because of a bug that got past the first version of it:
`present()` left the framebuffer's own texture bound to texture unit 0, and
because the triangle shader references its sampler statically, every following
frame's untextured geometry — the cubes, the sine sheet — was a framebuffer
feedback loop, which the browser detects and silently drops. Lines and points
use a different program with no sampler, so they kept drawing, and the result
was a wireframe intro with no fills from the second frame on.

The verification could not see it, because it rendered frames and read them
back without ever calling `present()`. Fixing the bug is one bound texture;
the useful part was closing the gap, so the harness now runs the same sequence
the intro does. Reintroducing the bug now fails `make verify`.

## Layout

```
index.html          the intro
verify.html         the frame and audio comparison, in a browser
shot.html           one frame, or one difference
src/intro.js        start-up, in intro.asm's order
src/frame.js        the eight effects and the frame loop  (frame.inc)
src/draw.js         cube, rubik grid, glyphs, flare, rand/frand  (draw.inc)
src/glfixed.js      fixed-function OpenGL 1.1 on WebGL 2
src/sgen.js         instrument synthesis  (sgen.inc)
src/player.js       module playback and mixing  (player.inc)
src/x87.js          80-bit extended precision, and fsin's 66-bit pi
src/data.js         generated - do not edit
tools/extract_data.py   ../../common/data -> src/data.js, through nasm
tools/bundle.py         everything above -> one self-contained build/east.html
tools/pack.py           the same, flattened + terser + roadroller, 12.6 KB
tools/check_packed.mjs  runs the minified build against the module build
tools/check_audio.mjs   mix the tune in node, compare with song.wav
tools/headless.mjs      run verify.html / shot.html / the intro in headless Chrome
```

`src/data.js` is generated rather than hand-transcribed: `tools/extract_data.py`
assembles `../../common/data/` with nasm and reads the bytes back, so whatever
the assembler makes of `031-2`, the `NT()` macro and `256-70-15` is what ends up
in the port. Regenerate it with `make data`.

## Running it standalone

`index.html` and `src/` reference nothing outside this directory and fetch
nothing at run time, so the intro is already self-contained: copy those two
onto any static host, or into any other project, and it runs. `src/data.js` is
generated ahead of time, so `common/data/` is not needed either — only
`make data` and the verification pages reach outside.

What does *not* work is opening `index.html` off the disk, because a browser
will not fetch ES modules over `file://`. `make bundle` solves that by
inlining every module into a single **70 KB `build/east.html`** that opens by
double-clicking, with no server and nothing beside it. `tools/bundle.py`
handles only the four import/export forms the port uses and stops with an error
on anything else, rather than quietly emitting a file that half works; the
module sources are otherwise untouched.

`make pack` does the same thing as small as it will go — the js13k pipeline:

| | bytes |
|---|---|
| flattened into one scope | 67,705 |
| [terser](https://terser.org) | 30,404 |
| [roadroller](https://github.com/lifthrasiir/roadroller) `-O2 -M 384 -D` | 11,843 |
| **`build/east.min.html`**, with the page around it | **12,942** |

For comparison, `build/east.html` gzipped is 22,217 — the packed file is
smaller than the readable one compressed, and it needs no server to decompress
it.

Flattening is what makes the rest pay off. The readable bundle keeps a small
module registry, so every cross-module call stays a property lookup on a name
no minifier may touch; `tools/pack.py` concatenates the modules into one scope
instead, and terser can then mangle every name in the intro. That is only safe
because the port has exactly one cross-module name collision — `const fr =
Math.fround`, written identically in four files — and pack.py refuses to
continue if another appears, if a namespace alias survives its rewrite, or if a
module shadows a name it imports. (`player.js` used to declare a local `div`
over `x87.div`, which is exactly that bug; the check is why it is not one now.)

Nothing in the pipeline is allowed to touch the arithmetic: terser runs with
its `unsafe` passes off, `unsafe_math` in particular, which would be free to
reassociate the float expressions the whole port rests on. `make pack-verify`
checks that from the other end, running the minified build and the module build
side by side and comparing the instruments, the whole mixed tune, the note
table, the flare texture and the seeded particle field. All five hashes match.

Two things were tried and measured and are *not* used, because both came out
bigger: inlining the `GL` enum as numeric literals (12,094), and encoding the
byte tables as strings instead of decimal arrays (12,201). Terser already
shortens `GL` to two characters, and roadroller's context model handles
repeated property names and digit runs better than either substitute.

## Requirements

WebGL 2 — any current browser. The module build must be served over HTTP; the
bundled and packed ones need not be. `make verify` additionally wants node 18+
and Chrome (set `CHROME=` to point elsewhere); `make pack` fetches terser and
roadroller through `npx`, so its first run needs the network.

## Credits

Original intro by **Fuzzion**, 2003 — code by Raimon Ràfols, music `searching
east`. Released at Bcnparty. This port keeps the original data files intact.
