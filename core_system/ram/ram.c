#include "ram.h"
#include <stdio.h>
#include <malloc.h>

unsigned int ramCounter = 0;
ramBlock** ramTable = 0;

ramBlock* ram_create(unsigned short start, unsigned short end) {
             
             ramTable = (ramBlock**)realloc((void*)ramTable, sizeof(ramBlock*)*(ramCounter+1));
             ramTable[ramCounter] = (ramBlock*)malloc(sizeof(ramBlock));
             ramTable[ramCounter]->device = (deviceEntry*)malloc(sizeof(deviceEntry));
             ramTable[ramCounter]->device->startAddr = start;
             ramTable[ramCounter]->device->endAddr = end;
             ramTable[ramCounter]->device->writeMethod = &ram_writer;
             ramTable[ramCounter]->device->readMethod = &ram_reader;
             ramTable[ramCounter]->buffer = (unsigned short*)malloc(sizeof(unsigned short)*(end - start));
                          
             return ramTable[ramCounter++];
             
}

unsigned short ram_reader(unsigned short address) {
     
    int i;

#ifdef DEBUG     
    printf("Reading from ram at 0x%04X\n", address);
#endif     
     
    for(i = 0; i < ramCounter; i++) {
            
        if(address <= ramTable[i]->device->endAddr
           && address >= ramTable[i]->device->startAddr) {
            return ramTable[i]->buffer[address-ramTable[i]->device->startAddr];
        }
           
    }
    
    return 0xFFFF;
     
}

void ram_writer(unsigned short address, unsigned short data) {
        
    int i;

#ifdef DEBUG         
    printf("Writing to ram at 0x%04X\n", address);
#endif
         
    for(i = 0; i < ramCounter; i++) {
            
        if(address <= ramTable[i]->device->endAddr
           && address >= ramTable[i]->device->startAddr) {
            ramTable[i]->buffer[address-ramTable[i]->device->startAddr] = data;
            break;
        }
           
    }
         
}

void ram_unload(void) {

    int i;

    for(i = 0; i < ramCounter; i++) {
        free(ramTable[i]->device);
        ramTable[i]->device = NULL;
        free(ramTable[i]);
        ramTable[i] = NULL;
    }

    free(ramTable);
    ramTable = NULL;

}
