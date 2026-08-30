// ---------------------------------------------------------------------------
//  x87 80-bit extended precision, emulated.
//
//  The instrument generator (sgen.inc) keeps phase, freq, amp and the two
//  filter state variables in x87 registers, which are 64-bit-mantissa extended
//  precision.  One instrument has a frequency of 3.1e12 and runs for 4096
//  samples, so its phase accumulator reaches ~1.3e16 - where a JavaScript
//  double has an ulp of 2.0 radians and the sine argument is simply gone.
//  Extended precision has an ulp of 0.001 there, so it still carries a signal.
//  That is the whole reason this file exists.
//
//  A value is { s, m, k }: sign 0/1, a BigInt mantissa with exactly 64 bits
//  set (2^63 <= m < 2^64), and a binary exponent, meaning (-1)^s * m * 2^k.
//  Zero is m = 0n.  Every operation is done exactly on the BigInts and then
//  rounded once to 64 bits, round-to-nearest-even, which is what the hardware
//  does with the control word `finit` leaves behind.
//
//  Only the sound generator needs this.  Every value the visuals hand to
//  OpenGL is stored to a 32-bit float first (`fstp dword`), so plain doubles
//  plus Math.fround reproduce those exactly; see fround32 at the bottom.
// ---------------------------------------------------------------------------

const M64 = (1n << 64n) - 1n;

export const ZERO = { s: 0, m: 0n, k: 0 };

function bitLength(m) {
  let n = 0;
  while (m >= 0x100000000n) { m >>= 32n; n += 32; }
  return n + (32 - Math.clz32(Number(m)));
}

// value = (-1)^s * m * 2^k, rounded to a 64-bit mantissa (ties to even).
function norm(s, m, k) {
  if (m === 0n) return ZERO;
  const nb = bitLength(m);
  if (nb > 64) {
    const sh = BigInt(nb - 64);
    const lost = m & ((1n << sh) - 1n);
    const half = 1n << (sh - 1n);
    m >>= sh;
    k += nb - 64;
    if (lost > half || (lost === half && (m & 1n) === 1n)) {
      m += 1n;
      if (m > M64) { m >>= 1n; k += 1; }        // carried out of the top bit
    }
  } else if (nb < 64) {
    m <<= BigInt(64 - nb);
    k -= 64 - nb;
  }
  return { s, m, k };
}

export function add(a, b) {
  if (a.m === 0n) return b;
  if (b.m === 0n) return a;
  // Line both mantissas up at the smaller exponent.  If they are more than
  // 200 bits apart the smaller one lands far below the larger's rounding
  // boundary and cannot move it, so the larger is already the answer.
  const [hi, lo] = a.k >= b.k ? [a, b] : [b, a];
  const d = hi.k - lo.k;
  if (d > 200) return hi;
  const mh = hi.m << BigInt(d);
  const ml = lo.m;
  if (hi.s === lo.s) return norm(hi.s, mh + ml, lo.k);
  if (mh === ml) return ZERO;
  return mh > ml ? norm(hi.s, mh - ml, lo.k) : norm(lo.s, ml - mh, lo.k);
}

export function sub(a, b) {
  return add(a, neg(b));
}

export function mul(a, b) {
  if (a.m === 0n || b.m === 0n) return ZERO;
  return norm(a.s ^ b.s, a.m * b.m, a.k + b.k);
}

// fdiv: shift the numerator up so the integer quotient carries the full
// mantissa, keep the remainder as a sticky bit, and round once.
export function div(a, b) {
  if (a.m === 0n) return ZERO;
  const n = a.m << 72n;
  const q = n / b.m;
  const qq = (n % b.m) === 0n ? q : (q | 1n);
  return norm(a.s ^ b.s, qq, a.k - b.k - 72);
}

export function neg(a) {
  return a.m === 0n ? ZERO : { s: a.s ^ 1, m: a.m, k: a.k };
}

export function abs(a) {
  return a.s ? { s: 0, m: a.m, k: a.k } : a;
}

// -- conversions ------------------------------------------------------------

const dv = new DataView(new ArrayBuffer(8));

export function fromNumber(x) {
  if (x === 0 || !isFinite(x)) return ZERO;
  const s = x < 0 ? 1 : 0;
  x = Math.abs(x);
  dv.setFloat64(0, x);
  const be = (dv.getUint32(0) >>> 20) & 0x7ff;
  if (be === 0) {                                // subnormal; scale into range
    const scaled = fromNumber(x * 18446744073709551616.0);   // 2^64
    return { s, m: scaled.m, k: scaled.k - 64 };
  }
  const e = be - 1023;                            // 2^e <= x < 2^(e+1)
  const mi = x * Math.pow(2, 52 - e);             // exact, in [2^52, 2^53)
  return norm(s, BigInt(mi), e - 52);
}

export function toNumber(a) {
  if (a.m === 0n) return 0;
  const v = Number(a.m) * Math.pow(2, a.k);
  return a.s ? -v : v;
}

// fist/fistp word: round to nearest even, and on overflow store the x87
// "integer indefinite" 0x8000 - which the instruments genuinely hit, because
// the saw oscillators run their phase far past 32767.
export function toInt16(a) {
  const n = toIntRoundNearestEven(a);
  if (n === null || n > 32767n || n < -32768n) return 0x8000;
  return Number(n) & 0xffff;
}

// fistp dword: the note table is built with one, and its 96 chained multiplies
// are the other place extended precision is doing real work.
export function toInt32(a) {
  const n = toIntRoundNearestEven(a);
  if (n === null || n > 2147483647n || n < -2147483648n) return -2147483648;
  return Number(n);
}

function toIntRoundNearestEven(a) {
  if (a.m === 0n) return 0n;
  let m = a.m, k = a.k;
  if (k >= 0) {
    if (k > 80) return null;                      // certainly out of range
    m <<= BigInt(k);
  } else {
    const sh = BigInt(-k);
    if (-k > 130) return 0n;
    const lost = m & ((1n << sh) - 1n);
    const half = 1n << (sh - 1n);
    m >>= sh;
    if (lost > half || (lost === half && (m & 1n) === 1n)) m += 1n;
  }
  return a.s ? -m : m;
}

// -- fsin -------------------------------------------------------------------
//
//  The 8087 reduces its argument against a 66-bit approximation of pi, not the
//  real thing, and at 1.3e16 that is a 1e-4 radian difference from a correctly
//  reduced sine - visible in the low bits of the sample bytes.  So reduce
//  against the same constant it uses:
//
//      pi/4 ~ 0.C90FDAA2 2168C234 C  (hex), i.e. pi ~ 0x3243F6A8885A308D3 / 2^64
//
//  The reduction itself is done exactly in BigInt; only the final small
//  argument, |r| <= pi/4, goes through a double sine, where 53 bits are
//  thirteen orders of magnitude finer than the 16-bit sample it becomes.

const PI66 = 0x3243F6A8885A308D3n;               // pi * 2^64, truncated to 66 bits

export function sin(a) {
  if (a.m === 0n) return ZERO;
  // fsin leaves the argument alone (and sets C2) when |x| >= 2^63.  Nothing
  // here reaches that, but the intro would want the hardware's answer if it did.
  if (a.k >= 0) return a;

  // N = nearest integer to x / (pi/2) = 2x / pi66
  const sh = a.k + 1 + 64;
  let num = a.m, den = PI66;
  if (sh >= 0) num <<= BigInt(sh); else den <<= BigInt(-sh);
  let N = (2n * num + den) / (2n * den);          // round half up; ties are moot
  if (a.s) N = -N;

  // r = x - N * pi66/2, scaled by 2^S so both terms are integers
  const S = Math.max(65, -a.k);
  const xs = (a.s ? -a.m : a.m) << BigInt(a.k + S);
  const rs = xs - ((N * PI66) << BigInt(S - 65));
  const r = Number(rs) * Math.pow(2, -S);

  const q = Number(((N % 4n) + 4n) % 4n);
  const v = q === 0 ? Math.sin(r)
          : q === 1 ? Math.cos(r)
          : q === 2 ? -Math.sin(r)
          :           -Math.cos(r);
  return fromNumber(v);
}
