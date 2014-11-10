#include "interface.h"
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>

interfaceChip ichip;

//Stolen from http://www.gamedev.net/topic/394223-sdl-converting-keypress-data-to-ascii/
char getUnicodeValue( SDL_KeyboardEvent* key )
{
    // magic numbers courtesy of SDL docs :)
    const int INTERNATIONAL_MASK = 0xFF80, UNICODE_MASK = 0x7F;

    int uni = key->keysym.unicode;

    if( uni == 0 ) // not translatable key (like up or down arrows)
    {
        // probably not useful as string input
        // we could optionally use this to get some value
        // for it: SDL_GetKeyName( key );
        return 0;
    }

    if( ( uni & INTERNATIONAL_MASK ) == 0 )
    {
        if( SDL_GetModState() & KMOD_SHIFT )
        {
            return (char)(toupper(uni & UNICODE_MASK));
        }
        else
        {
            return (char)(uni & UNICODE_MASK);
        }
    }
    else // we have a funky international character. one we can't read :(
    {
        // we could do nothing, or we can just show some sign of input, like so:
        return '?';
    }
}

interfaceChip* interface_create(unsigned short baseAddress) {

    SDL_EnableUNICODE( SDL_ENABLE );

    ichip.device = (deviceEntry*)malloc(sizeof(deviceEntry));
    ichip.bufIndex = 0;
    ichip.device->startAddr = baseAddress;
    ichip.device->endAddr = baseAddress + 1;
    ichip.device->writeMethod = &interface_write;
    ichip.device->readMethod = &interface_read;

    return &ichip;

}

void interface_unload(void) {

    free(ichip.device);
    ichip.device = NULL;

}

void interface_pushKeystroke(unsigned char keyCode) {

     if(ichip.bufIndex < KEYBUF_SIZE) {
         ichip.bufIndex += 1;
         ichip.keyBuffer[(ichip.bufIndex)-1] = keyCode;
     }

}

int interface_keyReady() {

    return ichip.bufIndex==0?0:1;

}

unsigned short interface_getNextKeys(void) {

    unsigned short workingReg;

    if(ichip.bufIndex == 0)
        return 0;

    if(ichip.bufIndex = 1)
        return ichip.keyBuffer[(ichip.bufIndex--)-1] << 8;

    workingReg = (ichip.keyBuffer[(ichip.bufIndex)-1] << 8) + ichip.keyBuffer[(ichip.bufIndex)-2];
    ichip.bufIndex -= 2;

    return workingReg;

}

void interface_pollKeyboard(void) {

     SDL_Event event;

     SDL_PollEvent(&event);
     switch (event.type) {

      case SDL_MOUSEMOTION:
        //keycombo |= MOUSEMOVE;
        break;
      case SDL_MOUSEBUTTONDOWN:
        //keycombo |= MOUSEPRESS;
        break;
      case SDL_KEYDOWN:
        interface_pushKeystroke((unsigned char)getUnicodeValue(&event.key));
        break;
      case SDL_QUIT:
        //done = 1;
        break;
      default:
        break;
     }


}

void interface_write(unsigned short address, unsigned short data) {

    //printf("Writing to interface chip.\n");

    unsigned char writtenBytes[2];
    int i;

    writtenBytes[0] = (data & 0xFF00) >> 8;
    writtenBytes[1] = data & 0xFF;

    for(i = 0; i < 2; i++)
        if(writtenBytes[i]) printf("%c", writtenBytes[i]); //printf("Printing character '%c'\n", writtenBytes[i]);

}

unsigned short interface_read(unsigned short address) {

    //printf("Reading from interface chip.\n");

    if(address & 0x1)
        return interface_getNextKeys();
    else
        return interface_keyReady();

}
