// Headless harness around dist/slime-node.js with a deterministic virtual clock.
const createSlime = require('../dist/slime-node.js');
module.exports = async function boot() {
  const M = await createSlime();
  const api = {
    M,
    init: M.cwrap('slime_init', null, []),
    step: M.cwrap('slime_step', 'number', []),
    mouse: M.cwrap('slime_mouse', null, ['number', 'number', 'number']),
    key: M.cwrap('slime_key', null, ['number']),
    present: M.cwrap('slime_present', 'number', []),
    vram: M.cwrap('slime_vram', 'number', []),
    setVirtualTime: M.cwrap('slime_set_virtual_time', null, ['number']),
    advance: M.cwrap('slime_advance_ms', null, ['number']),
    running: M.cwrap('slime_running', 'number', []),
    frame() { const p = api.present() >> 2; return new Uint8Array(M.HEAPU32.buffer.slice(p * 4, p * 4 + 64000 * 4)); },
    vramCopy() { const p = api.vram(); return M.HEAPU8.slice(p, p + 64000); },
    // run n sim steps, advancing the virtual clock at `sps` steps/second
    run(n, sps = 60) { for (let i = 0; i < n; i++) { api.advance(1000 / sps); api.step(); } },
    // mouse in screen pixels (320x200)
    pointer(x, y, buttons = 0) { api.mouse(x * 2, y, buttons); },
  };
  api.setVirtualTime(1);
  return api;
};
