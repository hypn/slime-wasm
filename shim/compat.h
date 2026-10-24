// Force-included (-include compat.h) into every translation unit.
// Papers over the Turbo C++ 3.x / DOS runtime so the original sources compile nearly as-is.
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif
long         slime_clock(void);        // BIOS tick counter, 18.2065 Hz
int          slime_rand(void);         // Borland C RTL rand()
void         slime_srand(unsigned s);
void         slime_randomize(void);
#ifdef __cplusplus
}
#endif

#define clock()     slime_clock()
#undef  CLK_TCK
#define CLK_TCK     18.2
// Borland's <stdlib.h> definitions
#define random(num) ((int)(((long)slime_rand() * (num)) / 32768L))
#define randomize() slime_randomize()

#define far
#define near
#define huge
