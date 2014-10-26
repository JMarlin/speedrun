#include "interface.h"
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>

interfaceChip ichip;

interfaceChip* interface_create(unsigned short baseAddress) {
    
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
     
     static int charTyped = 0;
     
     if(!charTyped){
         interface_pushKeystroke('X');
         charTyped = 1;
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


