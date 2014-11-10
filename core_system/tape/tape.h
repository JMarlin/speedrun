#ifndef TAPE_H
#define TAPE_H

#include "../cpu/cpu.h"

typedef struct tapeDev {
        unsigned short statusReg;
        unsigned short tapeLength;
        deviceEntry* device;
        unsigned short* buffer;
} romBlock;

tapeDev* tape_create(unsigned short start, unsigned short end, char* fileName);
unsigned short tape_reader(unsigned short address);
void tape_writer(unsigned short address, unsigned short data);
void tape_unload(void);

#endif
