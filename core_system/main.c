#include "cpu/cpu.h"
#include "ram/ram.h"
#include "rom/rom.h"
#include "gpu/gpu.h"
#include "interface/interface.h"
#include <stdio.h>
#include <SDL/SDL.h>

int main(int argc, char* argv[]) {

    int done;
    SDL_Event event;

    //Start SDL
    SDL_Init( SDL_INIT_EVERYTHING );
    atexit(SDL_Quit);
    printf("SDL started.\n");
    
    deviceEntry* gpu = gpu_create(0xFC00);
    ramBlock* mainRAM = ram_create(0x1000, 0xF7FF);
    romBlock* monitorROM = rom_create(0x0000, 0x0FFF, "monitor.rom");
    interfaceChip* userInterface = interface_create(0xFFFE);
    
    cpu_addDevice(mainRAM->device);
    cpu_addDevice(monitorROM->device);
    cpu_addDevice(gpu);
    cpu_addDevice(userInterface->device);
    
    cpu_reset();
        
    while(cpu_cycle(2000)){
                        
        interface_pollKeyboard();
        gpu_redraw(); //<- this is slowing execution down a lot. We should really put it on a 30-60hz timer.   
                        
    }
    gpu_redraw(); 
    
    //MessageBox(0, "CPU halted.", "Notice", 0);   replace with motif call 
    
    
    done = 0;
    while(!done)
    while ( SDL_PollEvent(&event) )
                 {
                        switch (event.type)
                        {
 
                                case SDL_MOUSEMOTION:
                                        //keycombo |= MOUSEMOVE;
                                        break;
                                case SDL_MOUSEBUTTONDOWN:
                                        //keycombo |= MOUSEPRESS;
                                        break;
                                case SDL_KEYDOWN:
                                        switch( event.key.keysym.sym )
                                        {
                                                case SDLK_a:
                                                        done = 1;
                                                break;
                                                default: break;
                                        }
                                break;

                                case SDL_QUIT:
                                        done = 1;
                                        break;
                                default:
                                        break;
                        }
                }      
    
    ram_unload();  
    rom_unload();
    interface_unload();
    gpu_unload();
    cpu_unloadDevices();
    
    SDL_Quit();
        
    return 0;

}
