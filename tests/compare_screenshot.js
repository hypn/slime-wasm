// Compares our panel/banner/cursor-free regions against the original DOSBox screenshot (2x scaled).
const fs = require('fs'), path = require('path');
const boot = require('./harness'), png = require('./png');
(async () => {
  const ref = png.decode(fs.readFileSync(path.join(__dirname, '..', 'screenshot.png')));
  const s = await boot(); s.init(); s.run(1);
  // the screenshot was taken mid-scroll; find the banner offset that matches it best
  let f = s.frame(), best = [1e9, 0];
  for (let k = 0; k < 9000; k++) {
    s.run(1, 60); f = s.frame(); let bad = 0;
    for (let y = 8; y < 16; y++) for (let x = 8; x < 296; x++) { const o = (y * 2 * ref.w + x * 2) * 4, i = (y * 320 + x) * 4; if (Math.abs(ref.rgba[o] - f[i]) > 4) bad++; }
    if (bad < best[0]) best = [bad, k];
  }
  console.log('best banner offset step', best[1], 'with', best[0], 'differing text pixels');
  const s2 = await boot(); s2.init(); s2.run(1); s2.run(best[1] + 1, 60); f = s2.frame();
  const region = (name, x0, y0, x1, y1) => {
    let bad = 0, n = 0;
    for (let y = y0; y < y1; y++) for (let x = x0; x < x1; x++) {
      const o = (y * 2 * ref.w + x * 2) * 4, i = (y * 320 + x) * 4; n++;
      // DOSBox scales the 6-bit DAC slightly differently (168 vs 170), so allow a small tolerance
      if ([0, 1, 2].some(c => Math.abs(ref.rgba[o + c] - f[i + c]) > 4)) bad++;
    }
    console.log(name.padEnd(28), `${bad}/${n} pixels differ`);
  };
  console.log('screenshot', ref.w, 'x', ref.h);
  region('panel (x300..319, y0..199)', 300, 0, 320, 199);
  region('top border rows 0..7', 0, 0, 300, 8);
  region('banner text rows 8..15', 8, 8, 296, 16);
  region('rule rows 16..19', 0, 16, 300, 19);
})();
