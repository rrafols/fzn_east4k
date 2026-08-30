# What the intro does

Notes for anyone writing another port. This is the behaviour both existing ports
implement; the arithmetic is identical in each, only the platform layer differs.

## Timing

The tune is 63 orders of 16 rows at **114 ms per row**, so 1824 ms per order and
about 115 seconds total. Two clocks drive everything, both derived from
milliseconds since start:

    ts    = ms / 20            animation time, ~50 units per second
    order = ms / (114 * 16)    which order is playing; the intro ends at 63

The original ran the music on a thread that slept 114 ms per row while the
visuals used `GetTickCount`; deriving both from one clock is simpler and drifts
less.

`orderStuff[order]` is a bitmask of which effects draw this frame — bit 0 is
effect 1, bit 7 is effect 8. Several run at once, in order, over the same frame.

## Per frame

Clear to **(0.6, 0, 0)**, enable `BLEND`, `LIGHT0`, `COLOR_MATERIAL`,
`LINE_SMOOTH`, `POINT_SMOOTH`, then `gluPerspective(80, 4/3, 0.1, 100)` and an
identity modelview. Nothing calls `glLight*`, so the default `GL_LIGHT0` applies:
white diffuse, directional along +Z. Blend mode is *not* reset per frame — each
effect inherits whatever the last one set, and the original depends on that.

## The effects

1. **Rubik grid** — 25 background lines at random heights (width 5, yellow),
   then a lit 3x3x3 grid of cubes, then flare billboards at y=+/-3, z=-4.2.
2. **Sine sheet** — a lit 30x30 quad mesh, z from `sin`/`cos` of position and
   time, then three red line strips over it.
3. **Tall cube** — one cube scaled (1, 8, 1), drawn solid then again as a black
   wireframe, with flares either side.
4. **Wide grid** — the rubik grid at scale (2, 3), offset -7, plus orbiting
   flares and streak lines.
5. **Dense grid** — the grid at scale (9, 9), outline pass only, a flare swarm,
   and the glyph smeared twelve times.
6. **Glyphs** — the two characters swept around on `cos`/`sin` of time, each
   drawn 50 times at alpha 0.05 to smear them.
7. **Flare ring** — 100 billboards on a bobbing ring of radius 4.
8. **Logo** — the Fuzzion mark, drawn 50 times as fat smeared points.

## Particles

100 of them, one float each, seeded *before* the PRNG is seeded from the clock,
so the field is identical every run. Position starts in [-20, 20], velocity in
[0, 0.25]; each frame `px += vel`, wrapping to -20 once it passes 20. The
background lines in effect 1 use the clock-seeded PRNG and so differ per run —
that is the only nondeterminism in the visuals.

## Instruments

Eight, generated at start-up from the parameters in `samples.inc`: an oscillator
(sine, saw, or square, optionally rectified) through a resonant low-pass.

    phase += freq;  freq += dfreq
    val = osc(phase) * amp;  amp += damp
    sp += (val - ps) * c;  ps += sp;  out = ps;  sp *= r

Output is unsigned 8-bit at a base rate of 8363 Hz. **The x87 details matter**:
one instrument has a frequency of 3.1e12, where x87 `fsin` and a C `sin()`
diverge completely, and the saw wraps through a signed byte via `fistp`. A port
that uses a different maths library will not sound the same.

Note frequencies come from a table of 96 semitones: `f[0] = 33152/127.71542846`,
each step x 1.05946309436. A note byte is `semitone | octave<<4 | volume<<6`;
semitone 0 means "retrigger at this volume", 15 means note cut. Volume indexes
-10000, -800, -700, 0 hundredths of a dB, i.e. gains of 0, 0.398, 0.447, 1.

## Resolution

The original forced 640x480, and the look depends on it: `glLineWidth` and
`glPointSize` are in **pixels**, so running at the display's resolution makes
every outline and point proportionally thinner, and the alpha smears far too
faint. Render at 640x480 and scale the result up.
