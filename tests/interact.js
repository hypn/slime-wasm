// Scripted session: draw lines, pour water, turn rain on, pause, quit. Dumps PNGs to tests/out/.
const fs = require('fs'), path = require('path'), assert = require('assert');
const boot = require('./harness'), png = require('./png');
(async () => {
  const s = await boot();
  const out = path.join(__dirname, 'out'); fs.mkdirSync(out, { recursive: true });
  const save = n => fs.writeFileSync(path.join(out, n), png.encode(s.frame(), 320, 200, 2));
  const L = 1, R = 2;
  s.init(); s.run(2);

  // straight line (drawmode 1): drag with left button, release
  s.pointer(40, 40, 0); s.run(2);
  s.pointer(40, 40, L); s.run(2);
  for (let i = 1; i <= 20; i++) { s.pointer(40 + i * 8, 40 + i * 4, L); s.run(1); }
  s.pointer(200, 120, 0); s.run(2);
  // pour water with right button above the line
  s.pointer(120, 40, R); s.run(1);
  for (let i = 0; i < 400; i++) s.run(1);
  s.pointer(120, 40, 0); s.run(300);
  save('water.png');

  // cursor over panel -> driver arrow visible
  s.pointer(309, 30, 0); s.run(4);
  assert(s.M.cwrap('slime_cursor_visible', 'number', [])() === 1, 'arrow should show over the panel');
  save('panel_cursor.png');

  // rain button (button 0 at 301..318, 2..19)
  s.pointer(309, 10, L); s.run(2); s.pointer(309, 10, 0); s.run(2);
  s.pointer(100, 100, 0); s.run(600);
  save('rain.png');

  // pause then clear-all (N button) then quit with space
  s.key('p'.charCodeAt(0)); s.run(2);
  s.key('p'.charCodeAt(0)); s.run(2);
  s.pointer(309, 190, L); s.run(2); s.pointer(309, 190, 0); s.run(3);
  save('cleared.png');
  s.key(32); s.run(1);
  assert(s.running() === 0, 'space should quit');
  save('exited.png');
  console.log('interact ok');
})();
