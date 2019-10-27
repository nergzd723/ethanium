#include <keyboard.h>
#include <shutdown.h>

void reboot()
{
    uint8_t temp;
    asm volatile ("cli");
    outb(KBRD_INTRFC, KBRD_RESET);
loop:
    asm volatile ("hlt"); /* if that didn't work, halt the CPU */
    goto loop; /* if a NMI is received, halt again */
}