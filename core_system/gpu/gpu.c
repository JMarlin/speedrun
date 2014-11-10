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
             source.x = 8*(long)((vram[i] & (0xFF << ((1-j)*8))) >> ((1-j)*8) );
             //source.x = 8*'A';
             dest.y = (((i*2)+j)/80)*16;
             dest.x = (((i*2)+j)%80)*8;
             if(SDL_BlitSurface(font, &source, screen, &dest) != 0) {
                 printf("Blit failed: %s\n", SDL_GetError());
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
    screen = SDL_SetVideoMode( 640, 400, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
    if(screen == NULL) {
        printf("Failed to set screen.");
        return NULL;
    }

    bitmap = SDL_LoadBMP("gpu/system_font.bmp");
    if(bitmap == NULL) {
        printf("Failed to load image.");
        return NULL;
    }

    font = SDL_DisplayFormat(bitmap);
    if(font == NULL) {
        printf("Failed to convert image.");
        return NULL;
    }

    for(i = 0; i < 1000; i++) {
    	vram[i] = 'A';
	    vram[i] += ((unsigned short)'A' << 8) & 0xFF00;
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
