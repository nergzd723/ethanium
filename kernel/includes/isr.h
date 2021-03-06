#ifndef __ISR_H__
#define __ISR_H__

#include <stdint.h>

struct context {
   uint32_t ds;                  // Data segment selector
   uint32_t edi, esi, ebp, useless_esp, ebx, edx, ecx, eax; // Pushed by pusha.
   uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
   uint32_t eip, cs, eflags, esp, ss; // Pushed by the processor automatically.
};
typedef struct{
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;

typedef struct context context_t;

typedef void (*isr_t)(context_t*);
void register_interrupt_handler(uint8_t n, isr_t handler);
extern void int32(unsigned char intnum, regs16_t *regs);

#endif