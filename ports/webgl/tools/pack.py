#!/usr/bin/env python3
"""Pack the intro into the smallest self-contained HTML file it will go into.

    python3 tools/pack.py             -> build/east.min.html
    python3 tools/pack.py --check     -> build/east.check.js, the same pipeline
                                         with the internals still reachable

The pipeline is the js13k one:

    flatten -> terser -> roadroller -> a minimal HTML shell

*Flatten* is the interesting step.  build/east.html (tools/bundle.py) keeps a
little module registry, which means every cross-module call stays a property
lookup on a name no minifier may touch.  Here the modules are concatenated into
one scope instead, so terser can mangle every name in the intro down to one or
two letters.  That is only safe because the port has exactly one cross-module
name collision - `const fr = Math.fround`, written identically in four files -
and the checks below refuse to continue if another one ever appears.

Nothing here changes any arithmetic: terser runs with its unsafe passes off, in
particular `unsafe_math`, which would be free to reassociate the float
expressions the whole port is built on.  `make pack-verify` re-checks that from
the other end, against the same reference the rest of the port is checked
against.
"""
import os, re, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..'))
SRC = os.path.join(ROOT, 'src')
BUILD = os.path.join(ROOT, 'build')

# -M is roadroller's context memory.  Higher packs better, but its wasm path
# overflows a 32-bit offset well before the documented 1024 limit, so this stays
# at a value the real payload has been seen to survive.
ROADROLLER = ['-O2', '-M', '384', '-D']

MODULES = ['data.js', 'x87.js', 'sgen.js', 'player.js', 'glfixed.js',
           'draw.js', 'frame.js', 'intro.js']

# Declared identically in several modules; hoisted to the top of the flat scope.
SHARED = {'fr': 'const fr = Math.fround;'}

RE_NAMED = re.compile(r"import\s*\{(.*?)\}\s*from\s*'(?:\./)?(?:src/)?([\w.]+)';", re.S)
RE_STAR = re.compile(r"import\s*\*\s*as\s+(\w+)\s+from\s*'(?:\./)?(?:src/)?([\w.]+)';")
RE_TOPLEVEL = [
    re.compile(r'^(?:export\s+)?(?:const|let|var)\s+([A-Za-z_$][\w$]*)', re.M),
    re.compile(r'^(?:export\s+)?(?:async\s+)?function\s+([A-Za-z_$][\w$]*)', re.M),
    re.compile(r'^(?:export\s+)?class\s+([A-Za-z_$][\w$]*)', re.M),
]


def toplevel_names(src):
    n = set()
    for p in RE_TOPLEVEL:
        n |= set(p.findall(src))
    return n


def strip_comments_and_strings(src):
    """Blank out comments and string bodies, so name checks only see code."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if c == '/' and i + 1 < n and src[i + 1] == '/':
            j = src.find('\n', i)
            j = n if j < 0 else j
            out.append(' ' * (j - i)); i = j
        elif c == '/' and i + 1 < n and src[i + 1] == '*':
            j = src.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(' ' * (j - i)); i = j
        elif c in '"\'`':
            j = i + 1
            while j < n and src[j] != c:
                j += 2 if src[j] == '\\' else 1
            j = min(j + 1, n)
            out.append(' ' * (j - i)); i = j
        else:
            out.append(c); i += 1
    return ''.join(out)


def flatten(page_script_omitted=False):
    bodies, owner, ALIASES = [], {}, set()
    for name in MODULES:
        raw = open(os.path.join(SRC, name)).read()
        code = strip_comments_and_strings(raw)

        # What this module declares at the top level, minus the shared names.
        mine = toplevel_names(code) - set(SHARED)
        for d in sorted(mine):
            if d in owner:
                sys.exit('src/%s: `%s` is also declared in src/%s - flattening '
                         'would collide.  Rename one, or add it to SHARED if '
                         'the two definitions are identical.' % (name, d, owner[d]))
            owner[d] = name
        bodies.append((name, raw, code, mine))

    parts = [SHARED[k] for k in sorted(SHARED)]
    for name, raw, code, mine in bodies:
        # `import * as f` becomes nothing: after flattening, f.add IS add.  The
        # module must not shadow any name it reaches through the alias, or the
        # rewrite would silently bind to the wrong thing - player.js used to
        # declare a local `div` over x87's, which is exactly that bug.
        # Found in the raw source, not the stripped copy: stripping blanks the
        # module path, and the pattern needs it.  The clash analysis below still
        # uses the stripped copy, so comments cannot trigger a false alarm.
        for alias, mod in RE_STAR.findall(raw):
            ALIASES.add(alias)
            used = set(re.findall(r'\b' + alias + r'\.(\w+)', code))
            clash = used & (toplevel_names(code) | set(re.findall(
                r'\b(?:const|let|var)\s+([A-Za-z_$][\w$]*)', code)))
            if clash:
                sys.exit('src/%s: declares %s, which it also reaches through '
                         '`%s.` - flattening would shadow it.'
                         % (name, ', '.join(sorted(clash)), alias))
            raw = re.sub(r'\b' + alias + r'\.(\w+)', r'\1', raw)

        raw = RE_STAR.sub('', raw)
        raw = RE_NAMED.sub('', raw)              # names are simply in scope now
        raw = re.sub(r'^export\s+', '', raw, flags=re.M)
        for k, decl in SHARED.items():
            raw = raw.replace(decl + '\n', '')
        parts.append(raw)

    page = open(os.path.join(ROOT, 'index.html')).read()
    m = re.search(r'<script type="module">(.*?)</script>', page, re.S)
    if not page_script_omitted:
        body = RE_NAMED.sub('', m.group(1))
        parts.append('(async () => {%s})();' % body)

    js = '\n'.join(parts)
    leftover = re.search(r'^\s*(?:import|export)\b', js, re.M)
    if leftover:
        sys.exit('a module statement survived flattening at offset %d' % leftover.start())
    # A namespace alias that survived the rewrite would be a free variable, and
    # would only show up when that code path ran.  Look for it now instead.
    stripped = strip_comments_and_strings(js)
    for alias in ALIASES:
        if re.search(r'\b' + alias + r'\s*\.', stripped):
            sys.exit('`%s.` survived flattening - the namespace rewrite missed a use'
                     % alias)
    return js, page, m


def run(tool, args, data):
    exe = shutil.which('npx')
    if not exe:
        sys.exit('npx not found; npm is needed for terser and roadroller')
    src = os.path.join(BUILD, '_in.js')
    open(src, 'w').write(data)
    r = subprocess.run([exe, '-y', tool] + args + [src], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit('%s failed:\n%s' % (tool, r.stderr[-3000:]))
    os.remove(src)
    return r.stdout


def main():
    check = '--check' in sys.argv
    os.makedirs(BUILD, exist_ok=True)
    # The check build leaves out the page script, which only touches the DOM,
    # and exposes the parts that do arithmetic so node can compare them.
    js, page, m = flatten(page_script_omitted=check)

    if check:
        # Keep the internals reachable under known names, so tools/check_packed
        # can exercise the minified code rather than trusting it.
        js += ('\nglobalThis.__x = { genSamples, renderSong, makeFlare, '
               'seedParticles, px, noteTable };\n')

    # One closure, so the intro defines nothing in the global scope at all.
    # That is what makes roadroller's -D safe: its decoder is then free to use
    # short global names without any chance of colliding with the intro's.
    js = '(()=>{%s})()' % js

    raw = len(js.encode())
    # No `unsafe*` passes: unsafe_math may reassociate float arithmetic, and the
    # arithmetic is the thing being preserved.
    small = run('terser@5', [
        '--compress', 'passes=4,pure_getters,booleans_as_integers,unsafe=false',
        '--mangle', 'toplevel',
        '--format', 'quote_style=1,wrap_func_args=false',
    ], js)

    if check:
        out = os.path.join(BUILD, 'east.check.js')
        open(out, 'w').write(small)
        print('flattened %d -> terser %d  ->  %s' % (raw, len(small.encode()), out))
        return 0

    # The minified-but-readable stage is kept too: it is the one to debug with
    # if the packed file ever misbehaves.
    open(os.path.join(BUILD, 'east.min.js'), 'w').write(small)

    packed = run('roadroller@2', ROADROLLER, small)

    # The shell carries the page's own markup and CSS; roadroller already has
    # the script, so what is left is as small as it can usefully be.
    css = re.search(r'<style>(.*?)</style>', page, re.S).group(1)
    css = re.sub(r'\s+', ' ', re.sub(r'/\*.*?\*/', '', css, flags=re.S)).strip()
    css = re.sub(r'\s*([{}:;,>])\s*', r'\1', css).replace(';}', '}')
    body = page[page.index('<canvas'):m.start()].strip()
    body = re.sub(r'>\s+<', '><', body)

    html = ('<!doctype html><meta charset=utf-8><title>Looking for the East</title>'
            '<meta name=viewport content="width=device-width,initial-scale=1">'
            '<style>%s</style>%s<script>%s</script>' % (css, body, packed))

    out = os.path.join(BUILD, 'east.min.html')
    open(out, 'w').write(html)
    print('flattened %8d' % raw)
    print('terser    %8d' % len(small.encode()))
    print('roadroller%8d' % len(packed.encode()))
    print('%-10s%8d  %s' % ('html', len(html.encode()), out))
    return 0


if __name__ == '__main__':
    sys.exit(main())
