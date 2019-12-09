#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include <stdint.h>

#define FB_COLS 90
#define FB_ROWS 60
#define FB_CELLS FB_COLS * FB_ROWS

#define FB_BLACK 0
#define FB_BLUE 1
#define FB_GREEN 2
#define FB_CYAN 3
#define FB_RED 4
#define FB_MAGENTA 5
#define FB_BROWN 6
#define FB_LIGHT_GREY 7
#define FB_DARK_GREY 8
#define FB_LIGHT_BLUE 9
#define FB_LIGHT_GREEN 10
#define FB_LIGHT_CYAN 11
#define FB_LIGHT_RED 12
#define FB_LIGHT_MAGENTA 13
#define FB_LIGHT_BROWN 14
#define FB_WHITE 15

void clear_screen();
void fb_backspace();
void fb_write_byte(uint8_t b);
void fb_newline();
void fill_screen(unsigned char color);
int printf (const char * format, ...);
int row();
char* inttostr( int zahl );
void downarrowp();
void uparrowp();
void leftarrowp();
void rightarrowp();
void reset_cursor();
void fb_write_cell(unsigned int cell, char c, unsigned char fg, unsigned char bg);
void move_cursor_to_pos(unsigned short pos);

#endif /* INCLUDE_FRAME_BUFFER_H */