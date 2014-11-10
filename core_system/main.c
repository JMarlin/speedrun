#include "./cpu/cpu.h"
#include "./ram/ram.h"
#include "./rom/rom.h"
#include "./gpu/gpu.h"
#include "./interface/interface.h"
#include <stdio.h>
#include <SDL/SDL.h>

int main(int argc, char* argv[]) {

    int done;
    deviceEntry* gpu;
    ramBlock* mainRAM;
    romBlock* monitorROM;
    interfaceChip* userInterface;

    //Start SDL
    SDL_Init( SDL_INIT_EVERYTHING );
    SDL_ShowCursor( SDL_DISABLE );
    atexit(SDL_Quit);
    printf("SDL started.\n");

    gpu = gpu_create(0xFC00);
    if(gpu == NULL)
      return 0;
    mainRAM = ram_create(0x1000, 0xF7FF);
    monitorROM = rom_create(0x0000, 0x0FFF, "monitor.rom");
    userInterface = interface_create(0xFFFE);

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

    printf("CPU halted.", "Notice");

    ram_unload();
    rom_unload();
    interface_unload();
    gpu_unload();
    cpu_unloadDevices();

    SDL_Quit();

    return 0;

}
