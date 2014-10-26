#ifndef RAM_H
#define RAM_H

#include "../cpu/cpu.h"

typedef struct ramBlock {
        deviceEntry* device;
        unsigned short* buffer;
} ramBlock;

ramBlock* ram_create(unsigned short start, unsigned short end);
unsigned short ram_reader(unsigned short address);
void ram_writer(unsigned short address, unsigned short data);
void ram_unload(void);

#endif
