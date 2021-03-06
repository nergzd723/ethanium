#include <sb16.h>
#include <stdbool.h>
#include <io.h>
#include <timer.h>
#include <logger.h>

bool sound_blaster = false;

unsigned short read_DSP(){
    return inb(DSP_READ);
}

void write_DSP(unsigned char byte){
    outb(DSP_WRITE, byte);
}

void reset_DSP(void) {
          outb(DSP_RESET, 1);
          waitm(3); //wait 3  microseconds
          outb(DSP_RESET, 0);
 
          if(read_DSP()==0xAA) { 
                    sound_blaster=true; 
          }
}


void sb16_init(void) {
        logf("SB16 driver fired!\n");
          reset_DSP();
 
          //if DSP doesnt exist
          if(sound_blaster==false) {
                    logf("No soundblaster detected\n");
                    return;
          }

          //get DSP version
          write_DSP(DSP_CMD_VERSION);
          int sb16_version_major=read_DSP();
          int sb16_version_minor=read_DSP();
          logf("Soundblaster initialized!");
          logf("\n");
 
}