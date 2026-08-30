// Runs verify.html in headless Chrome and prints what it found, so the port
// can be checked from a terminal like the macOS one's `make verify`.
//
//   node tools/headless.mjs                     # verify, print the result
//   node tools/headless.mjs shot 3 out.png      # screenshot one frame
//   node tools/headless.mjs smoke [out.png]     # run the intro itself briefly
//   FILE=build/east.html node … smoke           # ... from file://, no server
//
// Serves the repository root itself, because verify.html reaches sideways into
// ../macos-x86_64/build/linked/ for the reference frames.
import { spawn } from 'child_process';
import { createServer } from 'http';
import { readFile, writeFile } from 'fs/promises';
import { extname, join, normalize, resolve } from 'path';
import { fileURLToPath } from 'url';

const ROOT = fileURLToPath(new URL('../../..', import.meta.url));
const CHROME = process.env.CHROME ||
  '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
const PORT = 8731 + (process.pid % 200);
const TYPES = { '.html': 'text/html', '.js': 'text/javascript', '.wav': 'audio/wav',
                '.ppm': 'application/octet-stream', '.png': 'image/png' };

const server = createServer(async (req, res) => {
  const p = join(ROOT, normalize(decodeURIComponent(req.url.split('?')[0])));
  try {
    const b = await readFile(p);
    res.writeHead(200, { 'content-type': TYPES[extname(p)] || 'application/octet-stream' });
    res.end(b);
  } catch { res.writeHead(404); res.end(); }
});
await new Promise(r => server.listen(PORT, '127.0.0.1', r));

const mode = process.argv[2] || 'verify';
const url = process.env.FILE
  ? 'file://' + resolve(process.env.FILE)
  : `http://127.0.0.1:${PORT}/ports/webgl/` +
  (mode === 'shot' ? `shot.html?n=${process.argv[3] || 0}${process.env.DIFF ? '&diff=1' : ''}`
   : mode === 'smoke' ? 'index.html' : 'verify.html');

const chrome = spawn(CHROME, [
  '--headless=new', '--remote-debugging-port=0', '--no-first-run',
  '--user-data-dir=' + join(process.env.TMPDIR || '/tmp', 'lfte-headless-' + process.pid),
  // Software rendering by default so the frame comparison is reproducible;
  // GPU=1 runs on the real device, which is what `smoke` wants for timing.
  ...(process.env.GPU ? ['--use-angle=metal']
                      : ['--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader']),
  '--hide-scrollbars', '--window-size=1400,2400', 'about:blank',
], { stdio: ['ignore', 'pipe', 'pipe'] });

const wsUrl = await new Promise((resolve, reject) => {
  let buf = '';
  const t = setTimeout(() => reject(new Error('chrome did not start')), 20000);
  chrome.stderr.on('data', d => {
    buf += d;
    const m = buf.match(/ws:\/\/[^\s]+/);
    if (m) { clearTimeout(t); resolve(m[0].replace(/\/devtools\/browser\/.*/, '')); }
  });
});

const port = new URL(wsUrl).port;
const tab = await (await fetch(`http://127.0.0.1:${port}/json/new?${encodeURIComponent(url)}`,
                               { method: 'PUT' })).json();
const ws = new WebSocket(tab.webSocketDebuggerUrl);
await new Promise(r => ws.addEventListener('open', r));

let id = 0;
const pending = new Map();
ws.addEventListener('message', e => {
  const m = JSON.parse(e.data);
  if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); }
  if (m.method === 'Runtime.consoleAPICalled')
    console.log('  [page]', m.params.args.map(a => a.value ?? a.description).join(' '));
  if (m.method === 'Runtime.exceptionThrown')
    console.log('  [page error]', m.params.exceptionDetails.exception?.description ||
                                  m.params.exceptionDetails.text);
});
const send = (method, params = {}) => new Promise(r => {
  const n = ++id;
  pending.set(n, r);
  ws.send(JSON.stringify({ id: n, method, params }));
});
const evaluate = async expr => (await send('Runtime.evaluate',
  { expression: expr, returnByValue: true, awaitPromise: true })).result?.result?.value;

await send('Runtime.enable');
await send('Page.enable');

// The intro itself: wait for it to finish preparing, press start, let it run.
if (mode === 'smoke') {
  let errors = 0;
  ws.addEventListener('message', e => {
    const m = JSON.parse(e.data);
    if (m.method === 'Runtime.exceptionThrown') errors++;
  });
  const until = Date.now() + 120000;
  while (Date.now() < until && !(await evaluate('!document.getElementById("go").disabled')))
    await new Promise(r => setTimeout(r, 500));
  await evaluate('document.getElementById("go").click()');
  await new Promise(r => setTimeout(r, Number(process.env.SECS || 8) * 1000));
  const drawn = await evaluate('window.__drawn | 0');
  if (process.argv[3]) {
    const shot = await send('Page.captureScreenshot', { format: 'png' });
    await writeFile(process.argv[3], Buffer.from(shot.result.data, 'base64'));
    console.log('wrote %s', process.argv[3]);
  }
  console.log('frames drawn: %d, exceptions: %d', drawn, errors);
  ws.close(); chrome.kill(); server.close();
  process.exit(drawn > 0 && errors === 0 ? 0 : 1);
}

const deadline = Date.now() + 180000;
const key = mode === 'shot' ? '__shot' : '__verify';
let out;
while (Date.now() < deadline) {
  out = await evaluate(`window.${key} ?? null`);
  if (out) break;
  await new Promise(r => setTimeout(r, 500));
}

if (!out) {
  console.log('timed out waiting for the page');
} else if (mode === 'shot') {
  const png = Buffer.from(out.png, 'base64');
  await writeFile(process.argv[4] || 'shot.png', png);
  console.log('wrote %s (%d bytes)', process.argv[4] || 'shot.png', png.length);
} else {
  for (const f of out.frames || []) console.log('  ' + f.name.padEnd(12) + ' ' + f.stat);
  console.log('');
  for (const l of out.lines) console.log(l);
}

ws.close();
chrome.kill();
server.close();
process.exit(out && !out.unexpected?.length ? 0 : 1);
