#!/usr/bin/env python3
"""Flatten the intro into one self-contained HTML file.

index.html and src/ already depend on nothing outside this directory and fetch
nothing at run time, so the port is portable as it stands - but ES modules are
fetched, and a browser refuses to fetch them over file://.  This writes a
single build/east.html with every module inlined as a classic script, which
opens by double-clicking, with no server and no other file beside it.

The transform is deliberately small: it handles only the four import/export
forms the port actually uses, and fails loudly on anything else rather than
quietly producing a file that half works.
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..'))
SRC = os.path.join(ROOT, 'src')
OUT = os.path.join(ROOT, 'build', 'east.html')

MODULES = ['data.js', 'x87.js', 'sgen.js', 'player.js', 'glfixed.js',
           'draw.js', 'frame.js', 'intro.js']

# A module registry small enough to read: each module is a function that fills
# its own exports the first time something asks for it.
RUNTIME = """\
// Bundled by tools/bundle.py - the modules below are src/*.js, unchanged apart
// from their import and export statements.  Edit those, not this file.
const __mods = {}, __done = {};
const __def = (name, fn) => { __mods[name] = fn; };
const __req = (name) => {
  if (!__done[name]) { __done[name] = {}; __mods[name](__done[name], __req); }
  return __done[name];
};
"""

RE_NAMED = re.compile(r"import\s*\{(.*?)\}\s*from\s*'(?:\./)?(?:src/)?([\w.]+)';", re.S)
RE_STAR = re.compile(r"import\s*\*\s*as\s+(\w+)\s+from\s*'(?:\./)?(?:src/)?([\w.]+)';")
RE_EXPORT = re.compile(r'^export\s+(?:(const|class)\s+(\w+)|(?:async\s+)?function\s+(\w+))',
                       re.M)
RE_ANY_IMPORT = re.compile(r'^\s*import\b', re.M)
RE_ANY_EXPORT = re.compile(r'^\s*export\b', re.M)


def rewrite(src, what):
    """Turn one module's source into a body plus the names it exports."""
    names = [m.group(2) or m.group(3) for m in RE_EXPORT.finditer(src)]

    # `export let`/`export var` would need live bindings, which this registry
    # does not give; nothing in the port uses them, so refuse rather than guess.
    for bad in re.finditer(r'^export\s+(let|var|default|\*)', src, re.M):
        sys.exit('%s: unsupported `export %s`' % (what, bad.group(1)))

    src = RE_NAMED.sub(lambda m: "const {%s} = __req('%s');" % (m.group(1).strip(), m.group(2)), src)
    src = RE_STAR.sub(lambda m: "const %s = __req('%s');" % (m.group(1), m.group(2)), src)
    src = re.sub(r'^export\s+', '', src, flags=re.M)

    leftover = RE_ANY_IMPORT.search(src) or RE_ANY_EXPORT.search(src)
    if leftover:
        line = src[:leftover.start()].count('\n') + 1
        sys.exit('%s:%d: import/export form the bundler does not handle' % (what, line))
    return src, names


def main():
    parts = [RUNTIME]
    for name in MODULES:
        body, names = rewrite(open(os.path.join(SRC, name)).read(), 'src/' + name)
        parts.append("__def('%s', function (exports, __req) {\n%s\nObject.assign(exports, "
                     "{ %s });\n});\n" % (name, body, ', '.join(names)))

    page = open(os.path.join(ROOT, 'index.html')).read()
    m = re.search(r'<script type="module">(.*?)</script>', page, re.S)
    if not m:
        sys.exit('index.html: could not find the module script')
    body, _ = rewrite(m.group(1), 'index.html')

    # The page script uses top-level await, which a classic script cannot.
    parts.append('(async () => {\n%s\n})();\n' % body)

    html = page[:m.start()] + '<script>\n' + '\n'.join(parts) + '</script>' + page[m.end():]
    html = html.replace('<title>Looking for the East</title>',
                        '<title>Looking for the East</title>\n<!-- self-contained build: '
                        'open this file directly, no server needed -->')

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    open(OUT, 'w').write(html)
    print('wrote %s (%.1f KB)' % (OUT, len(html.encode()) / 1024))
    return 0


if __name__ == '__main__':
    sys.exit(main())
