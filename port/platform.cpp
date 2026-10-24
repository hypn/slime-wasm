// Platform layer replacing the DOS hardware Slime talked to:
//   VGA mode 13h framebuffer + default DAC palette, BIOS 8x8 text output,
//   INT 33h mouse, keyboard (kbhit/getch), BIOS tick clock and Borland rand().
// JS drives it through the slime_* exports below.
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "vga.h"
#include "int33.h"
#include "font8x8.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define EXPORT extern "C" EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT extern "C"
#endif

// Defined in MAIN.CPP
void slime_start(void);
void slime_stop(void);
int  water_step(void);

// ---------------------------------------------------------------- video
unsigned char vga_vram[64000];
static uint8_t  dac[256][3];            // 8-bit RGB
static uint32_t rgba[64000];            // presented frame (ImageData byte order)
static int      video_mode = 3;

// IBM VGA default 256-colour palette (what the BIOS loads on mode 13h set).
static void load_default_palette()
{
    static const uint8_t ega[16][3] = {
        {0,0,0},{0,0,42},{0,42,0},{0,42,42},{42,0,0},{42,0,42},{42,21,0},{42,42,42},
        {21,21,21},{21,21,63},{21,63,21},{21,63,63},{63,21,21},{63,21,63},{63,63,21},{63,63,63}};
    int six[256][3];
    for (int i = 0; i < 16; i++) for (int c = 0; c < 3; c++) six[i][c] = ega[i][c];
    static const uint8_t gray[16] = {0,5,8,11,14,17,20,24,28,32,36,40,45,50,56,63};
    for (int i = 0; i < 16; i++) for (int c = 0; c < 3; c++) six[16 + i][c] = gray[i];
    // 9 groups of 24 hues: (max, min); the first group steps are 0,16,32,48,63
    static const int grp[9][2] = {{63,0},{63,31},{63,45},{28,0},{28,14},{28,20},{16,0},{16,8},{16,11}};
    for (int g = 0; g < 9; g++)
    {
        int mx = grp[g][0], mn = grp[g][1];
        int st[5];
        for (int k = 0; k < 5; k++)
            st[k] = (g == 0) ? (k == 4 ? 63 : k * 16) : mn + (mx - mn) * k / 4;
        for (int h = 0; h < 24; h++)
        {
            int r, gr, b, k = h % 4;
            switch (h / 4)
            {
            case 0: r = st[k];     gr = mn;        b = mx;        break;  // blue -> magenta
            case 1: r = mx;        gr = mn;        b = st[4 - k]; break;  // -> red
            case 2: r = mx;        gr = st[k];     b = mn;        break;  // -> yellow
            case 3: r = st[4 - k]; gr = mx;        b = mn;        break;  // -> green
            case 4: r = mn;        gr = mx;        b = st[k];     break;  // -> cyan
            default:r = mn;        gr = st[4 - k]; b = mx;        break;  // -> blue
            }
            six[32 + g * 24 + h][0] = r; six[32 + g * 24 + h][1] = gr; six[32 + g * 24 + h][2] = b;
        }
    }
    for (int i = 0; i < 256; i++)
        for (int c = 0; c < 3; c++)
        {
            int v = six[i][c];
            dac[i][c] = (uint8_t)((v << 2) | (v >> 4));
        }
}

void bios_set_video_mode(int mode)
{
    video_mode = mode;
    memset(vga_vram, 0, sizeof(vga_vram));
    if (mode == 0x13)
        load_default_palette();
}

// ------------------------------------------- BIOS teletype text (mode 13h)
static int text_x = 1, text_y = 1;      // 1-based, as conio's gotoxy
static const int TEXT_FG = 7;           // conio default text attribute (LIGHTGRAY)

extern "C" void gotoxy(int x, int y) { text_x = x; text_y = y; }

extern "C" int bios_cprintf(const char* fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (video_mode != 0x13)
        return n;
    for (const char* p = buf; *p; p++)
    {
        if (text_x > 40) { text_x = 1; text_y++; }       // 40 columns in mode 13h
        if (text_y > 25) text_y = 25;                    // (no scrolling needed here)
        const uint8_t* g = bios_font8x8[(uint8_t)*p];
        int px = (text_x - 1) * 8, py = (text_y - 1) * 8;
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++)
                vga_vram[(py + r) * 320 + px + c] = (g[r] & (0x80 >> c)) ? TEXT_FG : 0;
        text_x++;
    }
    return n;
}

// ---------------------------------------------------------------- mouse
static int mx = 159, my = 99;           // driver coordinates: x 0..639, y 0..199
static int xlo = 0, xhi = 639, ylo = 0, yhi = 199;
static unsigned mbuttons = 0;
static int cursor_count = -1;           // INT 33h show/hide counter; visible when 0

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void int33_set_x_range(int lo, int hi) { xlo = lo; xhi = hi; mx = clampi(mx, xlo, xhi); }
void int33_set_y_range(int lo, int hi) { ylo = lo; yhi = hi; my = clampi(my, ylo, yhi); }
void int33_show_cursor(void) { if (cursor_count < 0) cursor_count++; }
void int33_hide_cursor(void) { cursor_count--; }
void int33_get_status(unsigned* b, unsigned* x, unsigned* y) { *b = mbuttons; *x = mx; *y = my; }
void int33_set_position(int x, int y) { mx = clampi(x, xlo, xhi); my = clampi(y, ylo, yhi); }

// Default INT 33h graphics cursor (arrow), hot spot (0,0)
static const uint16_t cur_screen[16] = {0x3FFF,0x1FFF,0x0FFF,0x07FF,0x03FF,0x01FF,0x00FF,0x007F,
                                        0x003F,0x001F,0x01FF,0x00FF,0x30FF,0xF87F,0xF87F,0xFCFF};
static const uint16_t cur_cursor[16] = {0x0000,0x4000,0x6000,0x7000,0x7800,0x7C00,0x7E00,0x7F00,
                                        0x7F80,0x7C00,0x6C00,0x4600,0x0600,0x0300,0x0300,0x0000};

// ------------------------------------------------------------- keyboard
static int keyq[64], key_head, key_tail;
extern "C" int kbhit(void) { return key_head != key_tail; }
extern "C" int getch(void)
{
    if (key_head == key_tail) return 0;
    int c = keyq[key_head];
    key_head = (key_head + 1) % 64;
    return c;
}

// ---------------------------------------------------------- time / random
static double now_ms()
{
#ifdef __EMSCRIPTEN__
    return emscripten_get_now();
#else
    return 0;
#endif
}
static int    virtual_time;
static double virtual_ms;

extern "C" long slime_clock(void)
{
    // BIOS ticks count from midnight, so they are already large at program start.
    // (render() relies on that: its first call with start==0 always takes the timed
    // branch and consumes a random() - which the button noise pattern depends on.)
    double ms = (virtual_time ? virtual_ms : now_ms()) + 12.0 * 3600 * 1000;
    return (long)(ms * 18.2065 / 1000.0);
}

static uint32_t seed = 1;               // Borland RTL default seed
extern "C" void slime_srand(unsigned s) { seed = s; }
extern "C" int  slime_rand(void)
{
    seed = seed * 0x015A4E35UL + 1;
    return (seed >> 16) & 0x7FFF;
}
extern "C" void slime_randomize(void) { seed = (uint32_t)now_ms(); }

// ------------------------------------------------------------- exports
static int running;

EXPORT void slime_init(void)
{
    key_head = key_tail = 0;
    slime_start();
    running = 1;
}

// One iteration of the original main loop. Returns 0 after the user quit with space.
EXPORT int slime_step(void)
{
    if (!running) return 0;
    if (!water_step())
    {
        slime_stop();
        running = 0;
    }
    return running;
}

// x, y in driver units (x 0..639, y 0..199); buttons: bit0 left, bit1 right, bit2 middle
EXPORT void slime_mouse(int x, int y, int buttons)
{
    mx = clampi(x, xlo, xhi);
    my = clampi(y, ylo, yhi);
    mbuttons = buttons & 7;
}

EXPORT void slime_key(int ch)
{
    int next = (key_tail + 1) % 64;
    if (next != key_head) { keyq[key_tail] = ch; key_tail = next; }
}

// Converts VRAM through the DAC, overlays the driver's cursor, returns 320x200 RGBA.
EXPORT uint32_t* slime_present(void)
{
    uint32_t lut[256];
    for (int i = 0; i < 256; i++)
        lut[i] = 0xFF000000u | (dac[i][2] << 16) | (dac[i][1] << 8) | dac[i][0];
    if (video_mode == 0x13)
        for (int i = 0; i < 64000; i++) rgba[i] = lut[vga_vram[i]];
    else
        for (int i = 0; i < 64000; i++) rgba[i] = 0xFF000000u;
    if (video_mode == 0x13 && cursor_count >= 0)
    {
        int cx = mx / 2, cy = my;
        for (int r = 0; r < 16; r++)
            for (int c = 0; c < 16; c++)
            {
                int x = cx + c, y = cy + r;
                if (x < 0 || x > 319 || y < 0 || y > 199) continue;
                uint16_t bit = 0x8000 >> c;
                if (cur_screen[r] & bit) continue;       // screen mask 1 = keep background
                rgba[y * 320 + x] = lut[(cur_cursor[r] & bit) ? 15 : 0];
            }
    }
    return rgba;
}

EXPORT int       slime_running(void) { return running; }
EXPORT uint8_t*  slime_vram(void)    { return vga_vram; }
EXPORT int       slime_cursor_visible(void) { return cursor_count >= 0; }
EXPORT void      slime_set_virtual_time(int on) { virtual_time = on; }
EXPORT void      slime_advance_ms(int ms) { virtual_ms += ms; }
EXPORT void      slime_seed(unsigned s) { seed = s; }
