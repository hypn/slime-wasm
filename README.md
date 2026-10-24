# Slime (DOS)

### About:
Welcome to SLIME! This little program is intended for use as a watery/slimey substance simulator.

### Instructions:
Draw lines with your left mouse button, create fluid with your right mouse button, and use the toolbar on the side screen to make it rain, change mouse button functionality, pause, remove lines or remove water.


You will see there is a panel on the right with buttons on it.  The button at the top is the rain button. Clicking it turns the rain on or off. The next three buttons are your right-click buttons. Use these to decide what you want the simulator to do when you right-click the mouse button.


The next two buttons affect what happens when you left-click the mouse button. So, the button under the rain button (button number 2) adds water when you right-click. This is apparent by the plus in the blue background, which symbolised adding water. Button 3, the button with the minus symbol and a line means removing lines. You can use this to remove any lines you dont want. Button 4 is the button you will use to remove some water in a small area. It's useful for clearing up where the water has messed or spilled. The button under that, button number 5, is the button that you will use to add straight lines to the scene. The button underneath that one is the add freeform line. Use this to 'scribble' lines all over the place.


There are two smaller buttons under that. The white one is used when you want to erase all lines, but keep the water. The blue one is used when you want to erase all water without losing any lines. The larger button underneath (with an N on it) can be used to clear both lines and water at the same time. It will also stop the rain if it was on. You can use 'p' to pause if you dont want to click the button.


To quit the simulation, hit space. Thats it for now. Enjoy yourself.


**DOSBox config:** Set CPU Core to "dynamic" and CPU Cycles to "max" (or run "core=dynamic" and "cycles=max" in the command prompt before running "slime.exe")

### Screenshot:
![Slime-dos screenshot](screenshot.png "Slime-dos screenshot")

### Info:
**Created:** 2001


**Operating System:** Dos 6


**Compile with:** Borland Turbo C++ 3.x (make sure your "Options" -> "Directories" paths for "Libraries" and "Includes" are correct)

## WebAssembly port

The original code recompiled with Emscripten; only the DOS hardware layer is replaced.
The simulation, rendering, button panel, banner and mouse logic are the original code.

    ./build.sh web      # dist/index.html + slime.js + slime.wasm (serve dist/ over http)
    tests/run.sh        # node build + screenshot comparison + scripted session (PNGs in tests/out/)

Needs Emscripten (`~/emsdk`, or set `EMSDK_DIR`).

### Layout
- `*.CPP`, `*.H`  the original sources, patched minimally (`git diff` against the DOS fork point)
- `shim/`   fake `dos.h`/`conio.h`/`alloc.h`/`mem.h`/`fstream.h`/`iostream.h` + `compat.h` (force-included)
- `port/`   `platform.cpp`: VGA mode 13h VRAM + default DAC palette, INT 33h mouse, BIOS 8x8 text,
            keyboard queue, BIOS tick clock, Borland `rand()`; `font8x8.h` (IBM BIOS font)
- `web/`    thin page: canvas, pointer/keyboard events, requestAnimationFrame pacing

### Patches to the original source
- `putpixel`: `asm` write to `0xA000:offs` -> `vga_vram[offs]`.
- `gr()`/`tx()`: INT 10h mode sets -> `bios_set_video_mode()`.
- `MOUSE.CPP`: each INT 33h `asm` block -> an `int33_*` call (same driver semantics: show/hide counter, ranges, x/2).
- `MOUSE.H`: `const X = 1;` -> `const int X = 1;` (implicit int).
- Old Turbo C++ scoping let `for (int x ...)` leak `x` into the function, and later loops reuse it as
  `for (x = ...)`. Those loops are untouched; `x` (and `l` in `drawBar`) is declared once at the top of
  `clearWater`, `drawBar`, `n` and `water_step` so they still compile.
- `water()`'s `while (!mustExit)` loop -> `water_init()` + `water_step()` (one iteration), locals made static;
  `main()` -> `slime_start()`/`slime_stop()`. The loop body is unchanged.
- `TMouse* mouse` points at a zeroed stub instead of NULL: `drawBar()` calls `mouse->hide()` before `new TMouse`
  (read DS:0 on DOS).
- `load()` returns if `c:\bitmap.bmp` can't be opened (the original read uninitialised stack).

### Fidelity notes
- Verified against `screenshot.png`: panel, borders, banner text and rule match pixel for pixel
  (`tests/compare_screenshot.js`), including the random button noise, which depends on Borland's `rand()`
  (seed 1, never randomized) and on `render()` consuming a `random()` during `gr()`.
- Speed: DOS ran the loop unthrottled, so it depended on CPU. Here it is `SPS` iterations/s (default 60, `?sps=`
  in the URL); render is every 3rd iteration as in the original.
- Pixel shape: 4:3 like a CRT by default; a link under the canvas switches to square pixels.
- Space quits (as original) and shows "press any key to restart", which boots a fresh instance.
- Drop a BMP on the page, then press `L`, to use the hidden bitmap loader (`c:\bitmap.bmp` in the original).
