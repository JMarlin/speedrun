#include "rom.h"
#include <stdio.h>
#include <malloc.h>

unsigned int tapeCounter = 0;
tapeDev** tapeTable = 0;

tapeDev* tape_create(unsigned short start, char* fileName) {

             unsigned char tempCell[2];
             int i;

             FILE* tapeFile = fopen(fileName, "rb");
             if(!tapeFile) {
                 printf("Could not load the tape file '%s'!\n", fileName);
                 return 0;
             }

             tapeTable = (tapeBlock**)realloc((void*)tapeTable, sizeof(tapeDev*)*(tapeCounter+1));
             tapeTable[tapeCounter] = (tapeDev*)malloc(sizeof(tapeDev));
             tapeTable[tapeCounter]->device = (deviceEntry*)malloc(sizeof(deviceEntry));
             tapeTable[tapeCounter]->device->startAddr = start;
             tapeTable[tapeCounter]->device->endAddr = start + 1; //mem[start] = control reg, mem[start+1] = data reg
             tapeTable[tapeCounter]->device->writeMethod = &tape_writer;
             tapeTable[tapeCounter]->device->readMethod = &tape_reader;
             tapeTable[tapeCounter]->buffer = (unsigned short*)malloc(sizeof(unsigned short)); //We'll dynamically realloc as we read the tape file

             //Read words from the tape file and keep increasing the buffer size until we hit EOF.
             for(tapeTable[tapeCounter]->tapeLength = 0; ;) {
                       if(!fread(&tempCell[0], sizeof(unsigned char), 1, tapeFile))
                           break;
                       if(!fread(&tempCell[1], sizeof(unsigned char), 1, tapeFile))
                           break;
                       tapeTable[tapeCounter]->tapeLength++;
                       tapeTable[tapeCounter]->buffer = (unsigned short*)realloc((void*)tapeTable[tapeCounter]->buffer, sizeof(unsigned short)*tapeTable[tapeCounter]->tapeLength);
                       tapeTable[tapeCounter]->buffer[tapeTable[tapeCounter]->tapeLength] = (((unsigned short)tempCell[0]) << 8) + (tempCell[1]);
             }

             fclose(tapeFile);

             return tapeTable[romCounter++];

}

//Writing to mem[start] sends one of the following commands to the tape controller:
//  0x0001 : step_forward, feed the tape by one position
//  0x0002 : step_back, rewind the tape by one position
//Reading from mem[start] gets the current controller status:
//  Bit 0 : at_start, the tape is currently at the first location
//  Bit 1 : at_end, we have reached the end of the tape
//  Bit 2 : command_success, the last command written has completed successfully
//  Bit 3 : command_fail, the last command written could not be completed
//Writing to mem[start + 1] punches the written value to the current tape location
//  Note: writing to the data register when the tape is at EOT will reallocate a new entry to the end of the tape buffer, then add the value
//Reading from mem[start + 1] reads the current value under the tape head
unsigned short tape_reader(unsigned short address) {

    int i;

#ifdef DEBUG
    printf("Reading from tape controller at 0x%04X\n", address);
#endif

    for(i = 0; i < tapeCounter; i++) {

        if(address <= tapeTable[i]->device->endAddr
           && address >= tapeTable[i]->device->startAddr) {
            //Do tapey things here
            return 0;
        }

    }

}

void tape_writer(unsigned short address, unsigned short data) {

}

//When unloading the ROM, we will write the current tape contents back to the tape file
void tape_unload(void) {

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
