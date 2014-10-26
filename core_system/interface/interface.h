#ifndef INTERFACE_H
#define INTERFACE_H

#include "../cpu/cpu.h"

#define KEYBUF_SIZE 255

typedef struct interfaceChip {
    deviceEntry* device; 
    unsigned int bufIndex;
    unsigned char keyBuffer[KEYBUF_SIZE];
} interfaceChip;

interfaceChip* interface_create(unsigned short baseAddress);
void interface_pushKeystroke(unsigned char keyCode);
int interface_keyReady(void);
unsigned short interface_getNextKeys(void);
void interface_pollKeyboard(void);
void interface_write(unsigned short address, unsigned short data);
unsigned short interface_read(unsigned short address);
void interface_unload(void);

#endif /* INTERFACE_H */
