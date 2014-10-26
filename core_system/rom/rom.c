#include "rom.h"
#include <stdio.h>
#include <malloc.h>

unsigned int romCounter = 0;
romBlock** romTable = 0;

romBlock* rom_create(unsigned short start, unsigned short end, char* fileName) {
             
             unsigned char tempCell[2];
             int i;
             
             FILE* romFile = fopen(fileName, "rb");
             if(!romFile) {
                 printf("Could not load the ROM image '%s'!\n", fileName);
                 return 0;
             }
             
             romTable = (romBlock**)realloc((void*)romTable, sizeof(romBlock*)*(romCounter+1));
             romTable[romCounter] = (romBlock*)malloc(sizeof(romBlock));
             romTable[romCounter]->device = (deviceEntry*)malloc(sizeof(deviceEntry));
             romTable[romCounter]->device->startAddr = start;
             romTable[romCounter]->device->endAddr = end;
             romTable[romCounter]->device->writeMethod = &rom_writer;
             romTable[romCounter]->device->readMethod = &rom_reader;
             romTable[romCounter]->buffer = (unsigned short*)malloc(sizeof(unsigned short)*(end - start));
             
             
             for(i = 0; i < (end - start); i++) {
                       if(!fread(&tempCell[0], sizeof(unsigned char), 1, romFile))
                           break;
                       if(!fread(&tempCell[1], sizeof(unsigned char), 1, romFile))
                           break;
                       romTable[romCounter]->buffer[i] = (((unsigned short)tempCell[0]) << 8) + (tempCell[1]);
             }
             
             if(fgetc(romFile) != EOF)
                 printf("The ROM image '%s' is larger than %d bytes and has been truncated.\n", fileName, end-start);
             
             fclose(romFile);
                          
             return romTable[romCounter++];
             
}

unsigned short rom_reader(unsigned short address) {
     
    int i;

#ifdef DEBUG    
    printf("Reading from rom at 0x%04X\n", address);
#endif
     
    for(i = 0; i < romCounter; i++) {
            
        if(address <= romTable[i]->device->endAddr
           && address >= romTable[i]->device->startAddr) {
            return romTable[i]->buffer[address-romTable[i]->device->startAddr];
        }
           
    }
     
}

void rom_writer(unsigned short address, unsigned short data) {
                  
}

void rom_unload(void) {

    int i;

    for(i = 0; i < romCounter; i++) {
        free(romTable[i]->device);
        romTable[i]->device = NULL;
        free(romTable[i]);
        romTable[i] = NULL;
    }

    free(romTable);
    romTable = NULL;

}
