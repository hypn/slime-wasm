#pragma once
#include <stdio.h>
#define DARKGRAY 8

#ifdef __cplusplus
extern "C" {
#endif
int  kbhit(void);
int  getch(void);
void gotoxy(int x, int y);
int  bios_cprintf(const char* fmt, ...);
#ifdef __cplusplus
}
#endif
// In mode 13h printf() goes through DOS/BIOS teletype output onto the graphics screen.
#undef  printf
#define printf bios_cprintf
