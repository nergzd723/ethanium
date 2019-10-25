#include <frame_buffer.h>
char[] lastcommbuf;
char[] terminal_buf;
void charbridge(char c){
    switch(c)
    {
        case '\b':
            fb_backspace();
            break;
        case '\n':
            fb_newline();
            break;
        default:
            fb_write_byte(c);
            break;
        lastcommbuf += c;
        terminal_buf += c;
        
    }
}