#include "../cpu/cpu.h"
#include "gpu.h"
#include <SDL/SDL.h>

deviceEntry gpuDevice; 
SDL_Surface *font, *bitmap;
SDL_Surface *screen;
unsigned short vram[1000];

void gpu_redraw(void) {
   
     SDL_Rect source, dest;
     int i, j;
     

     source.x = 0;
     source.y = 0;
     source.w = 8;
     source.h = 16;
     dest.x = 0;
     dest.y = 0;
     dest.w = 8;
     dest.h = 16;
     
     for(i = 0; i < 1000; i++) {
         for(j = 0; j < 2; j++) {
             source.x = 8*((vram[i] & (0xFF << ((1-j)*8))) >> ((1-j)*8) );
             dest.y = (((i*2)+j)/80)*16;
             dest.x = (((i*2)+j)%80)*8;
             if(SDL_BlitSurface(font, &source, screen, &dest) != 0) {
                 //MessageBox(0, "Blit failed", "Oops", 0); replace with motif call
	     }
         }
     }
     
     SDL_Flip(screen);
     
}

deviceEntry* gpu_create(unsigned short vbase) {
    
    int i;
    
    gpuDevice.startAddr = vbase;
    gpuDevice.endAddr = vbase + 999;
    gpuDevice.writeMethod = &gpu_write;
    gpuDevice.readMethod = &gpu_read;               
                    
    //Set up screen
    screen = SDL_SetVideoMode( 640, 400, 32, SDL_SWSURFACE | SDL_DOUBLEBUF );
    bitmap = SDL_LoadBMP("gpu/system_font.bmp");
    if(bitmap == NULL) {
        //MessageBox(0, "Failed to load image.", "Oops", 0); replace with motif call
        return NULL;
    }
    
    font = SDL_DisplayFormat(bitmap);
    SDL_FreeSurface(bitmap);
    if(font == NULL) {
        //MessageBox(0, "Failed to convert image.", "Oops", 0); replace with motif call
        return NULL;
    }
    
    for(i = 0; i < 1000; i++) {
    	vram[i] = i++ & 0xFF;
	vram[i] += (i << 8) & 0xFF;
    }
     
    return &gpuDevice;
    
}

void gpu_unload(void) {
    SDL_FreeSurface(font);
}

void gpu_write(unsigned short address, unsigned short data) {
     
    vram[address-gpuDevice.startAddr] = data;
    
}

unsigned short gpu_read(unsigned short address) {

    return vram[address-gpuDevice.startAddr];
    
}
