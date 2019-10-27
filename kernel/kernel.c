#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <tty.h>
#include <idt.h>
#include <pic.h>
#include <io.h>
#include <logger.h>
#include <gdt.h>
#include <timer.h>
#include <keyboard.h>
#include <memory.h>
#include "multiboot.h"
#include <frame_buffer.h>
#include <shutdown.h>
#include <panic.h>
#include <isr.h>
#include <defines.h>
#include <cpu_speed.h>

void kmain(multiboot_info_t* mbd, uint32_t magic) {
   init_logger();
   logf("Logger initialized\n");

   /* Check if the bootloader magic is correct */
   if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
      logf("Magic is incorrect...: %x\n", magic);
      while(1); // TODO: change to panic
   }

   logf("mbd->flags: %x\n", mbd->flags);
   if (mbd->flags & MULTIBOOT_INFO_MEMORY) {
      logf("mem_lower = %x\n", mbd->mem_lower);
      logf("mem_upper = %x\n", mbd->mem_upper);
   }

   /* Log a map of the regions of memory */
   if (mbd->flags & MULTIBOOT_INFO_MEM_MAP) {
      logf("mmap_addr = %x, mmap_length = %x\n", mbd->mmap_addr, mbd->mmap_length);
      multiboot_memory_map_t* map;

      for (map = (multiboot_memory_map_t*) mbd->mmap_addr;
            (uint32_t) map < (mbd->mmap_addr + mbd->mmap_length);
            map = (multiboot_memory_map_t*)((uint32_t) map + map->size + sizeof(map->size))) 
      {
         logf("base_addr_high = %x, base_addr_low = %x, length_high = %x, length_low = %x, type = %x\n",
               map->addr_upper,
               map->addr_lower,
               map->len_upper,
               map->len_lower,
               map->type);
      }

   }

   if (mbd->flags & (1 << 7)) {
      /* Safely read the drive regions */
      // TODO
   }

   init_gdt();
   logf("GDT initialized\n");

	init_pic();
   logf("PIC initialized\n");

   init_idt();
   logf("IDT initialized\n");
#ifdef STANDALONE
   clear_screen();
   logf("Terminal initialized\n");

   init_keyboard();
   logf("Keyboard intialized\n");
   logf("IDT and irq handlers operational\n");
#endif
#ifdef PAGING
   init_paging();
   logf("Paging initialized\n");
#endif
#ifdef TIMER
   init_timer(TIMER_FREQUENCY);
   logf("Timer initialized\n");
#endif
   __asm__ volatile("sti");
#ifdef STANDALONE
   printf("Ethan booted! Got ");
   printf(inttostr(memorycount()));
   printf("K mem total, ");
   printf(inttostr(lowmem()));
   printf("K lowmem, ");
   printf(inttostr(memorycount()-lowmem()));
   printf("K USABLE extended memory\n");
   printf(inttostr(discoverspeed()));
   printf("MHz CPU speed\n")
   printf(">>> ");
#endif
#ifdef TIMER
   waitm(300);
   logf("timer working");
#endif
   //clear_screen(); really shouldnt
   //shutdown(); that doesnt work, is page faulting
   //panic("TEST_PANIC", 0); works!
   while(1) {
   }
}
