#ifndef ROM_H
#define ROM_H

#include "../cpu/cpu.h"

typedef struct romBlock {
        deviceEntry* device;
        unsigned short* buffer;
} romBlock;

romBlock* rom_create(unsigned short start, unsigned short end, char* fileName);
unsigned short rom_reader(unsigned short address);
void rom_writer(unsigned short address, unsigned short data);
void rom_unload(void);

#endif
