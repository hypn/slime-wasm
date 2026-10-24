#ifndef _VGA_H_
#define _VGA_H_
extern unsigned char vga_vram[64000];            // mode 13h framebuffer (was segment 0xA000)
void bios_set_video_mode(int mode);              // INT 10h AX=0013h / 0003h
#endif
