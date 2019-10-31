#include <stdint.h>
#include <io.h>
#include <timer.h>
#include <sound.h>

 static void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (uint8_t) (Div) );
 	outb(0x42, (uint8_t) (Div >> 8));
 
        //And play the sound using the PC speaker
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(0x61, tmp | 3);
 	}
 }
 
 //make it shutup
 static void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;
 	outb(0x61, tmp);
 }
 
static void playnote(uint32_t freq, uint32_t time){
	play_sound(freq);
	waitm(time);
	nosound();
}

 //Make a beep
 void beep() {
	 asm volatile("sti");
 	 playnote(C6, 100);
 }