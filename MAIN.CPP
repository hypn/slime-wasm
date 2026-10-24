#include <ctype.h>
#include <string.h>
#include "button.h"
#include <alloc.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include "mouse.h"
#include "vga.h"
#include <fstream.h>
#include <iostream.h>

TButton*button[10]  =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
clock_t start, end;

char*   banner;
void    n(void);

void    drawline(float zx1, float zy1, float zx2, float zy2, int col);

char    tlc         = DARKGRAY;

char    field[320][200];
// Original: `TMouse* mouse;` (NULL until main()). drawBar() runs before `new TMouse`
// and calls mouse->hide()/show() through the NULL pointer; on DOS that read the
// DS:0 data instead of faulting. Point it at a zeroed stub so it stays defined.
static char mouse_null_stub[256];
TMouse* mouse = (TMouse*) mouse_null_stub;
int     dens        = 2;
int     rainmode    = 0;
int     eraser      = 0;
int     drawmode    = 1;
int     frames      = 0;
int     frameRate   = -1;
int     paused      = 0;

void    putpixel(int x, int y, char c);
void    bar(int x1, int y1, int x2, int y2);
void    line(int x1, int y1, int x2, int y2);

void    load(void);

void clearLines(void)
{
    for (int x = 1; x < 299; x++)
    {
        for (int y = 1; y < 199; y++)
        {
            if (field[x][y] == 99)
            {
                field[x][y] = 0;
            }
        }
    }
}

void clearWater(void)
{
    int x;
    for (int x = 0; x < 300; x++)
    {
        for (int y = 0; y < 200; y++)
        {
            if (field[x][y] < 99)
            {
                field[x][y] = 0;
            }
        }
    }
    for (x = 0; x < 300; x++)
    {
        field[x][0] = 99;
        field[x][199] = 99;
    }
    for (int y = 0; y < 200; y++)
    {
        field[0][y] = 99;
        field[319][y] = 99;
    }
    rainmode = 0;
}

void check()
{
    for (int a = 0; a < 10; a++)
    {
        if (mouse->leftDown == 1 && mouse->oldLeftDown == 0)
        {
            if (button[a] != 0)
            {
                if (mouse->checkInside(button[a]->x1, button[a]->y1, button[a]->x2, button[a]->y2))
                {
                    if (a == 1)
                    {
                        button[1]->isDown = 1;
                        button[2]->isDown = 0;
                        button[3]->isDown = 0;
                        button[1]->paint();
                        button[2]->paint();
                        button[3]->paint();
                        eraser = 0;
                    }
                    if (a == 2)
                    {
                        button[1]->isDown = 0;
                        button[2]->isDown = 1;
                        button[3]->isDown = 0;
                        button[1]->paint();
                        button[2]->paint();
                        button[3]->paint();
                        eraser = 1;
                    }
                    if (a == 3)
                    {
                        button[1]->isDown = 0;
                        button[2]->isDown = 0;
                        button[3]->isDown = 1;
                        button[1]->paint();
                        button[2]->paint();
                        button[3]->paint();
                        eraser = 2;
                    }
                    if (a == 4)
                    {
                        button[4]->isDown = 1;
                        button[5]->isDown = 0;
                        button[4]->paint();
                        button[5]->paint();
                        drawmode = 1;
                    }
                    if (a == 5)
                    {
                        button[4]->isDown = 0;
                        button[5]->isDown = 1;
                        button[4]->paint();
                        button[5]->paint();
                        drawmode = 2;
                    }
                    button[a]->paint();
                }
            }
        }
        if (mouse->checkInside(button[a]->x1, button[a]->y1, button[a]->x2, button[a]->y2))
        {
            if (mouse->leftDown == 1 && mouse->oldLeftDown == 1)
            {
                if (button[a] != 0)
                {
                    if (button[a]->isDown == 0 && button[a]->isToggler == 1)
                    {
                        button[a]->isDown = 1;
                        button[a]->paint();
                    }
                }
            }
        }
        if (mouse->checkInside(button[a]->x1, button[a]->y1, button[a]->x2, button[a]->y2))
        {
            if (mouse->leftDown == 0 && mouse->oldLeftDown == 0)
            {
                if (button[a] != 0)
                {
                    if (button[a]->isDown == 1 && button[a]->isToggler == 1)
                    {
                        button[a]->isDown = 0;
                        button[a]->paint();
                    }
                }
            }
        }
        if (!(mouse->checkInside(button[a]->x1, button[a]->y1, button[a]->x2, button[a]->y2)))
        {
            if (mouse->leftDown == 1 && mouse->oldLeftDown == 1)
            {
                if (button[a] != 0)
                {
                    if (button[a]->isDown == 1 && button[a]->isToggler == 1)
                    {
                        button[a]->isDown = 0;
                        button[a]->paint();
                    }
                }
            }
        }
        if (mouse->checkInside(button[a]->x1, button[a]->y1, button[a]->x2, button[a]->y2))
        {
            if (button[a] != 0)
            {
                if (mouse->leftDown == 0 && mouse->oldLeftDown == 1 && button[a]->isToggler == 1 && button[a]->tag == 10)
                {
                    n();
                    button[a]->isDown = 0;
                    button[a]->paint();
                }
                if (mouse->leftDown == 0 && mouse->oldLeftDown == 1 && button[a]->isToggler == 1 && button[a]->tag == 11)
                {
                    paused = 1 - paused;
                }
                if (mouse->leftDown == 0 && mouse->oldLeftDown == 1 && button[a]->isToggler == 1 && button[a]->tag == 12)
                {
                    clearLines();
                }
                if (mouse->leftDown == 0 && mouse->oldLeftDown == 1 && button[a]->isToggler == 1 && button[a]->tag == 13)
                {
                    clearWater();
                }
                if (mouse->leftDown == 0 && mouse->oldLeftDown == 1 && button[a]->isToggler == 1 && button[a]->tag == 22)
                {
                    rainmode = 1 - rainmode;
                }
            }
        }
    }
}

void drawBar()
{
    bar(300, 0, 319, 199);
    char    i[16][16];
    int     x, l;
    button[0] = new TButton;
    button[0]->x1 = 301;
    button[0]->y1 = 2;
    button[0]->x2 = 318;
    button[0]->y2 = 19;
    button[0]->tag = 22;
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            if (random(5) == 1)
            {
                i[x][y] = 1;
            }
            else
            {
                i[x][y] = 0;
            }
        }
    }
    button[0]->setBlitMap((void *) (&i));
    button[0]->isToggler = 1;
    button[0]->paint();
    button[1] = new TButton;
    button[1]->x1 = 301;
    button[1]->y1 = 27;
    button[1]->x2 = 318;
    button[1]->y2 = 44;
    button[1]->tag = 0;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 1;
        }
    }
    i[5][8] = 2;
    i[6][8] = 2;
    i[7][8] = 2;
    i[8][8] = 2;
    i[9][8] = 2;
    i[10][8] = 2;
    i[11][8] = 2;
    i[8][5] = 2;
    i[8][6] = 2;
    i[8][7] = 2;
    i[8][8] = 2;
    i[8][9] = 2;
    i[8][10] = 2;
    i[8][11] = 2;
    button[1]->setBlitMap(&i);
    button[1]->isDown = 1;
    button[1]->paint();
    button[2] = new TButton;
    button[2]->x1 = 301;
    button[2]->y1 = 47;
    button[2]->x2 = 318;
    button[2]->y2 = 64;
    button[2]->tag = 0;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 0;
        }
    }
    i[7][3] = 2;
    i[8][3] = 2;
    i[9][3] = 2;
    i[10][3] = 2;
    i[11][3] = 2;
    i[12][3] = 2;
    i[13][3] = 2;
    i[3][3] = 15;
    i[4][4] = 15;
    i[5][5] = 15;
    i[6][6] = 15;
    i[7][7] = 15;
    i[8][8] = 15;
    i[9][9] = 15;
    i[10][10] = 15;
    i[11][11] = 15;
    i[12][12] = 15;
    i[13][13] = 15;
    button[2]->setBlitMap((void *) (&i));
    button[2]->paint();
    button[3] = new TButton;
    button[3]->x1 = 301;
    button[3]->y1 = 67;
    button[3]->x2 = 318;
    button[3]->y2 = 84;
    button[3]->tag = 0;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 1;
        }
    }
    i[7][3] = 2;
    i[8][3] = 2;
    i[9][3] = 2;
    i[10][3] = 2;
    i[11][3] = 2;
    i[12][3] = 2;
    i[13][3] = 2;
    button[3]->setBlitMap((void *) (&i));
    button[3]->paint();
    button[4] = new TButton;
    button[4]->x1 = 301;
    button[4]->y1 = 92;
    button[4]->x2 = 318;
    button[4]->y2 = 109;
    button[4]->isDown = 1;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 0;
        }
    }
    i[8][4] = 2;
    i[9][4] = 2;
    i[10][4] = 2;
    i[11][4] = 2;
    i[12][4] = 2;
    i[13][4] = 2;
    i[14][4] = 2;
    i[11][1] = 2;
    i[11][2] = 2;
    i[11][3] = 2;
    i[11][4] = 2;
    i[11][5] = 2;
    i[11][6] = 2;
    i[11][7] = 2;
    i[3][3] = 15;
    i[4][4] = 15;
    i[5][5] = 15;
    i[6][6] = 15;
    i[7][7] = 15;
    i[8][8] = 15;
    i[9][9] = 15;
    i[10][10] = 15;
    i[11][11] = 15;
    i[12][12] = 15;
    i[13][13] = 15;
    button[4]->setBlitMap((void *) (&i));
    button[4]->paint();
    button[5] = new TButton;
    button[5]->x1 = 301;
    button[5]->y1 = 112;
    button[5]->x2 = 318;
    button[5]->y2 = 129;
    button[5]->tag = 0;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 0;
        }
    }
    i[8][4] = 2;
    i[9][4] = 2;
    i[10][4] = 2;
    i[11][4] = 2;
    i[12][4] = 2;
    i[13][4] = 2;
    i[14][4] = 2;
    i[11][1] = 2;
    i[11][2] = 2;
    i[11][3] = 2;
    i[11][4] = 2;
    i[11][5] = 2;
    i[11][6] = 2;
    i[11][7] = 2;
    i[5][6] = 15;
    i[6][7] = 15;
    i[8][8] = 15;
    i[7][8] = 15;
    i[6][8] = 15;
    i[6][7] = 15;
    i[9][9] = 15;
    button[5]->setBlitMap((void *) (&i));
    button[5]->paint();
    button[6] = new TButton;
    button[6]->x1 = 301;
    button[6]->y1 = 137;
    button[6]->x2 = 318;
    button[6]->y2 = 154;
    button[6]->tag = 11;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 0;
        }
    }
    for (int l = 4; l < 13; l++)
    {
        i[4][l] = 10;
    }
    i[5][4] = 10;
    i[6][4] = 10;
    i[7][4] = 10;
    i[8][4] = 10;
    i[9][5] = 10;
    i[9][6] = 10;
    i[9][7] = 10;
    i[8][8] = 10;
    i[7][8] = 10;
    i[6][8] = 10;
    i[5][8] = 10;
    button[6]->setBlitMap((void *) (&i));
    button[6]->isToggler = 1;
    button[6]->paint();

    button[7] = new TButton;
    button[7]->x1 = 301;
    button[7]->y1 = 181;
    button[7]->x2 = 318;
    button[7]->y2 = 198;
    button[7]->tag = 10;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 0;
        }
    }
    for (l = 4; l < 13; l++)
    {
        i[4][l] = 4;
        i[12][l] = 4;
    }
    i[5][5] = 4;
    i[6][6] = 4;
    i[7][7] = 4;
    i[8][8] = 4;
    i[9][9] = 4;
    i[10][10] = 4;
    i[11][11] = 4;
    button[7]->setBlitMap((void *) (&i));
    button[7]->isToggler = 1;
    button[7]->paint();

    button[8] = new TButton;
    button[8]->x1 = 301;
    button[8]->y1 = 158;
    button[8]->x2 = 318;
    button[8]->y2 = 166;
    button[8]->tag = 12;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 16;
        }
    }
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 7; y++)
        {
            i[x][y] = 15 * random(2);
        }
    }
    button[8]->setBlitMap((void *) (&i));
    button[8]->isToggler = 1;
    button[8]->paint();

    button[9] = new TButton;
    button[9]->x1 = 301;
    button[9]->y1 = 169;
    button[9]->x2 = 318;
    button[9]->y2 = 177;
    button[9]->tag = 13;
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            i[x][y] = 16;
        }
    }
    for (x = 0; x < 16; x++)
    {
        for (int y = 0; y < 7; y++)
        {
            i[x][y] = 1 * random(2);
        }
    }
    button[9]->setBlitMap((void *) (&i));
    button[9]->isToggler = 1;
    button[9]->paint();
}

void n(void)
{
    int x;
    for (int x = 0; x < 320; x++)
    {
        for (int y = 0; y < 200; y++)
        {
            field[x][y] = 0;
        }
    }
    for (x = 0; x < 300; x++)
    {
        field[x][0] = 99;
        field[x][199] = 99;
    }
    for (int y = 0; y < 200; y++)
    {
        field[0][y] = 99;
        field[299][y] = 99;
    }
    rainmode = 0;
}

void plug(void)
{
    int e   = mouse->x - 10;
    for (int u = 0; u < 320; u++)
    {
        field[u][199] = 99;
    }
    for (int h = e; h < e + 20; h++)
    {
        if ((h > 0) && (h < 299))
        {
            field[h][199] = 100;
        }
    }
}

void unPlug()
{
    for (int u = 0; u < 320; u++)
    {
        field[u][199] = 99;
    }
}

void swap(int* a, int* b)
{
    int v   = *a;
    *a = *b;
    *b = v;
}

int sign(int j)
{
    if (j < 0)
    {
        return -1;
    }
    else if (j > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void line(float zx1, float zy1, float zx2, float zy2, int st)
{
    int x1, y1, x2, y2;
    if (zx1 <= zx2)
    {
        x1 = zx1;
        y1 = zy1;
        x2 = zx2;
        y2 = zy2;
    }
    else
    {
        x2 = zx1;
        y2 = zy1;
        x1 = zx2;
        y1 = zy2;
    }
    if ((x1 == x2) && (y1 == y2))
    {
        return;
    }
    float   dx  = x2 - x1;
    float   dy  = y2 - y1;
    if (fabs(dx) > fabs(dy))
    {
        float   iy  = dy / dx;
        float   cy  = y1;
        if (x1 > x2)
        {
            swap(&x1, &x2);
        }
        for (int x = x1; x <= x2; x++)
        {
            cy += iy;
            if (x < 300)
            {
                if (st == 1)
                {
                    field[x][int(cy)] = 99;
                }
                else
                {
                    putpixel(x, int(cy), tlc);
                }
            }
        }
    }
    else
    {
        float   ix  = fabs(dx / dy);
        float   cx  = x1;
        if (y1 > y2)
        {
            swap(&y1, &y2);
            ix = -ix;
            cx = x2;
        }
        for (int y = y1; y <= y2; y++)
        {
            cx += ix;
            if (cx < 300)
            {
                if (st == 1)
                {
                    field[int(cx)][y] = 99;
                }
                else
                {
                    putpixel(cx, y, tlc);
                }
            }
        }
    }
}

void bar(int x1, int y1, int x2, int y2)
{
    for (int x = x1; x <= x2; x++)
    {
        for (int y = y1; y <= y2; y++)
        {
            if (x >= 0 && x <= 319 && y >= 0 && y <= 199)
            {
                putpixel(x, y, 7);
            }
        }
    }
}

void addWater()
{
    int t   = 4;
    int mmx = mouse->x;
    int mmy = mouse->y;
    for (int qx = -t; qx <= t; qx++)
    {
        for (int qy = -t*2; qy < 0; qy++)
        {
            if ((qy + mmy) >= 0)
            {
                if (qx + mmx<319 && qx + mmx>0 && qy + mmy > 0 && qy + mmy < 200)
                {
                    if (field[qx + mmx][qy + mmy] < 99)
                    {
                        field[qx + mmx][qy + mmy] = random(5);
                    }
                }
            }
        }
    }
}

void killWall()
{
    int sx  = mouse->x - 5;
    int sy  = mouse->y - 5;
    for (int x = sx; x < sx + 10; x++)
    {
        for (int y = sy; y < sy + 10; y++)
        {
            if (x > 0 && x<299 && y>0 && y < 199 && field[x][y] == 99)
            {
                field[x][y] = 0;
            }
        }
    }
}

void killWater()
{
    int sx  = mouse->x - 5;
    int sy  = mouse->y - 5;
    for (int x = sx; x < sx + 10; x++)
    {
        for (int y = sy; y < sy + 10; y++)
        {
            if (x > 0 && x<300 && y>0 && y < 199 && field[x][y] < 99)
            {
                field[x][y] = 0;
            }
        }
    }
}

void putpixel(int x, int y, char c)
{
    if (x<0 || x>319 || y<0 || y>199)
    {
        return;
    }
    static unsigned offs;
    offs = x + y * 320;
    static unsigned char    d;
    d = c;
    vga_vram[offs] = d;
}

void render(void)
{
    const float pc  = 10;
    frames++;
    end = clock();
    if (((end - start) / CLK_TCK) > 1 / pc)
    {
        static int  bannerpos   = 0;
        char        cc[40]      = "";
        strncpy(cc, banner + bannerpos, 36);
        bannerpos++;
        if (bannerpos > strlen(banner))
        {
            bannerpos = 0;
        }
        start = clock();
        end = clock();
        frameRate = frames * pc + random(5) - 2;
        frames = 0;
        gotoxy(2, 2);
        printf("%s", cc);
        drawline(0, 0, 299, 0, 15);
        drawline(299, 0, 299, 17, 15);
        drawline(0, 0, 0, 17, 15);
        drawline(1, 8, 1, 14, 0);
        drawline(2, 8, 2, 14, 0);
        drawline(3, 8, 3, 14, 0);
        drawline(4, 8, 4, 14, 0);
        drawline(5, 8, 5, 14, 0);
        drawline(6, 8, 6, 14, 0);
        drawline(7, 8, 7, 14, 0);
        drawline(296, 7, 296, 17, 0);
        drawline(297, 7, 297, 17, 0);
        drawline(298, 7, 298, 17, 0);
        drawline(299, 0, 299, 17, 15);
        drawline(1, 1, 298, 1, 0);
        drawline(1, 2, 298, 2, 0);
        drawline(1, 3, 298, 3, 0);
        drawline(1, 4, 298, 4, 0);
        drawline(1, 5, 298, 5, 0);
        drawline(1, 6, 298, 6, 0);
        drawline(1, 7, 298, 7, 0);
        drawline(1, 16, 295, 16, 0);
        drawline(1, 17, 295, 17, 0);
    }

    for (int x = 0; x < 300; x++)
    {
        for (int y = 18; y < 200; y++)
        {
            if (field[x][y] == 99)
            {
                putpixel(x, y, 15);
            }
            else if (field[x][y] != 0)
            {
                putpixel(x, y, (field[x][y] + 1) / 2 + 103);
            }
            else
            {
                putpixel(x, y, 0);
            }
            if (y == 18)
            {
                putpixel(x, 18, 15);
            }
        }
    }
    if (mouse->x >= 299)
    {
        mouse->show();
    }
    if (mouse->x < 299)
    {
        mouse->hide();
        putpixel(mouse->x, mouse->y, 14);
        putpixel(mouse->x + 1, mouse->y, 14);
        putpixel(mouse->x, mouse->y + 1, 14);
        putpixel(mouse->x, mouse->y - 1, 14);
        putpixel(mouse->x - 1, mouse->y, 14);
    }
}

void gr(void)
{
    bios_set_video_mode(0x13);
    n();
    render();
}

void tx(void)
{
    bios_set_video_mode(0x3);
}

static long water_frames;
static long water_s;
static int  water_mustExit;
static int  water_x1, water_y1, water_x2, water_y2;
static int  water_hasLeft;

void water_init(void)
{
    water_frames = 0;
    water_s = clock();
    water_mustExit = 0;
    water_hasLeft = 0;
}

// One iteration of the original `while (!mustExit)` loop in water().
// Returns 0 once the user has asked to quit.
int water_step(void)
{
    long&   frames      = water_frames;
    long&   s           = water_s;
    int&    mustExit    = water_mustExit;
    int&    x1          = water_x1;
    int&    y1          = water_y1;
    int&    x2          = water_x2;
    int&    y2          = water_y2;
    int&    hasLeft     = water_hasLeft;
    char    q, l, u, d, b, r = 0;
    int     x;
    {
        if (kbhit())
        {
            char    ch  = getch();
            ch = tolower(ch);
            if (ch == ' ')
            {
                mustExit = 1;
            }
            if (ch == 'p')
            {
                paused = 1 - paused;
            }
            if (ch == 'l')
            {
                load();
            }
        }
        mouse->update();
        check();
        if (rainmode == 1 && !paused)
        {
            for (int u = 1; u < 319; u++)
            {
                if (random(100) == 1)
                {
                    field[u][1] = 5;
                }
            }
        }
        if (mouse->rightDown == 1)
        {
            if (eraser == 0)
            {
                addWater();
            }
            if (eraser == 1)
            {
                killWall();
            }
            if (eraser == 2)
            {
                killWater();
            }
        }
        if (drawmode == 1)
        {
            if ((mouse->leftDown == 1) && (mouse->oldLeftDown == 0) && mouse->x < 300)
            {
                hasLeft = 1;
                x1 = mouse->x;
                y1 = mouse->y;
            }
            if ((mouse->leftDown == 1) && (mouse->oldLeftDown == 1) && hasLeft == 1 && mouse->x < 300)
            {
                line(x1, y1, mouse->x, mouse->y, 0);
            }
            if ((mouse->leftDown == 0) && (mouse->oldLeftDown == 1) && hasLeft == 1 && mouse->x < 300)
            {
                hasLeft = 0;
                x2 = mouse->x;
                y2 = mouse->y;
                line(x1, y1, x2, y2, 1);
            }
        }
        else
        {
            static int  mayDraw = 1;
            if ((mouse->leftDown == 1) && (mouse->oldLeftDown == 0) && mouse->x < 300)
            {
                mayDraw = 1;
                hasLeft = 1;
                x1 = mouse->x;
                y1 = mouse->y;
            }
            if ((mouse->leftDown == 1) && (mouse->oldLeftDown == 1) && hasLeft == 1 && mayDraw == 1)
            {
                line(x1, y1, mouse->x, mouse->y, 1);
                x1 = mouse->x;
                y1 = mouse->y;
            }
            if ((mouse->leftDown == 0) && (mouse->oldLeftDown == 1))
            {
                mayDraw = 0;
            }
        }
        frames++;
        if (!paused)
        {
            for (int x = 1; x < 319; x++)
            {
                for (int y = 198; y > 0; y--)
                {
                    if (field[x][y] < 0)
                    {
                        field[x][y] = 0;
                    }
                    if (field[x][y + 1] == 100)
                    {
                        field[x][y] = 0;
                    }
                    if ((field[x][y] > 0) && (field[x][y] < 99))
                    {
                        field[x][y]--;
                        u = field[x][y - 1];
                        d = field[x][y + 1];
                        l = field[x - 1][y];
                        r = field[x + 1][y];
                        q = d;
                        b = 2;
                        if (u < q)
                        {
                            q = u;
                            b = 1;
                        }
                        if (l < q)
                        {
                            q = l;
                            b = 3;
                        }
                        if (r < q)
                        {
                            q = r;
                            b = 4;
                        }
                        if (b == 1)
                        {
                            if (field[x][y - 1] < 97)
                            {
                                field[x][y - 1]++;
                            }
                        }
                        if (b == 2)
                        {
                            if (field[x][y + 1] < 97)
                            {
                                field[x][y + 1]++;
                            }
                        }
                        if (b == 3)
                        {
                            if (field[x - 1][y] < 97)
                            {
                                field[x - 1][y]++;
                            }
                        }
                        if (b == 4)
                        {
                            if (field[x + 1][y] < 97)
                            {
                                field[x + 1][y]++;
                            }
                        }
                    }
                }
            }
            int k   = 2;
            for (x = 318; x > 0; x--)
            {
                for (int y = 198; y >= 1; y--)
                {
                    if ((field[x][y] > 0) && (field[x][y] < 99))
                    {
                        u = field[x][y - 1];
                        d = field[x][y + 1];
                        l = field[x - 1][y];
                        r = field[x + 1][y];
                        q = d;
                        b = 2;
                        if (l < q)
                        {
                            q = l;
                            b = 3;
                        }
                        if (r < q)
                        {
                            q = r;
                            b = 4;
                        }
                        if (u < q)
                        {
                            q = u;
                            b = 1;
                        }
                        if (b == 1)
                        {
                            if (field[x][y - 1] < 97)
                            {
                                field[x][y - 1] += k;
                                field[x][y] -= k;
                            }
                        }
                        if (b == 2)
                        {
                            if (field[x][y + 1] < 97)
                            {
                                field[x][y + 1] += k;
                                field[x][y] -= k;
                            }
                        }
                        if (b == 3)
                        {
                            if (field[x - 1][y] < 97)
                            {
                                field[x - 1][y] += k;
                                field[x][y] -= k;
                            }
                        }
                        if (b == 4)
                        {
                            if (field[x + 1][y] < 97)
                            {
                                field[x + 1][y] += k;
                                field[x][y] -= k;
                            }
                        }
                    }
                }
            }
        }
        static int  nn  = 0;
        nn++;
        if (nn % 3 == 0)
        {
            render();
            nn = 0;
        }
    }
    s = -(s - clock());
    return !mustExit;
}

void test()
{
    for (int x = 104; x < 256; x++)
    {
        for (int y = 0; y < 20; y++)
        {
            putpixel(x, y, x);
        }
    }
    getch();
}

void ran(void)
{
    randomize();
    int t   = random(20) + 5;
    int ms  = 200;
    for (int gg = 0; gg < t; gg++)
    {
        float   x1, y1, x2, y2;
        x1 = random(258) + 1;
        y1 = random(118) + 1;
        x2 = x1 + random(ms) - ms / 2;
        y2 = y1 + random(ms) - ms / 2;
        line(x1, y1, x2, y2, 1);
    }
}

void    doText(char* b);

void slime_start(void)
{
    banner = (char *) farmalloc(3000);
    doText(banner);
    tx();
    gr();
    start = clock();
    drawBar();
    mouse = new TMouse();
    mouse->hide();
    water_init();
}

void slime_stop(void)
{
    delete mouse;
    tx();
    farfree(banner);
}

void load(void)
{
    fstream r;
    r.open("c:\\bitmap.bmp", ios::in | ios::binary);
    if (!r.is_open())
    {
        return;
    }
    char    u[1024] =
    {
        0
    };
    r.read(u, 54);
    r.read(u, 1024);
    for (int y = 199; y >= 0; y--)
    {
        for (int x = 0; x < 320; x++)
        {
            char    rr;
            r.read(&rr, 1);
            if (x > 0 && x<299 && y>0 && y < 199)
            {
                if (rr != 0)
                {
                    rr = 99;
                }
                field[x][y] = rr;
            }
        }
    }
}









