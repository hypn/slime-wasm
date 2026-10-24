#ifndef _INT33_H_
#define _INT33_H_
// Replacement for the INT 33h mouse driver calls MOUSE.CPP used. Implemented in port/.
void int33_set_x_range(int lo, int hi);          // AX=7
void int33_set_y_range(int lo, int hi);          // AX=8
void int33_show_cursor(void);                    // AX=1
void int33_hide_cursor(void);                    // AX=2
void int33_get_status(unsigned* buttons, unsigned* x, unsigned* y);  // AX=3
void int33_set_position(int x, int y);           // AX=4
#endif
