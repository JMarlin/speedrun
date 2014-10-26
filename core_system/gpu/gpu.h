#ifndef GPU_H
#define GPU_H

#include "../cpu/cpu.h"

deviceEntry* gpu_create(unsigned short vbase);
void gpu_unload(void);
void gpu_redraw(void);
void gpu_write(unsigned short address, unsigned short data);
unsigned short gpu_read(unsigned short address);

#endif //GPU_H
