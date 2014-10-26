#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>

int deviceCounter = 0;
deviceEntry** deviceTable = NULL;
cpuState currentState;

void cpu_reset(void) {

     int i;

    currentState.pc = 0;
    currentState.sp = 0;
    currentState.sc = 0;
    currentState.ir = 0;
    currentState.flags = 0;
    currentState.itp = 0;
    
    for(i = 0; i < 10; i++){
        currentState.r[i] = 0;
    }
             
}

void cpu_addDevice(deviceEntry* newDevice) {
                       
    deviceTable = (deviceEntry**)realloc((void*)deviceTable, sizeof(deviceEntry*)*(deviceCounter+1));
    deviceTable[deviceCounter] = newDevice;
    ++deviceCounter;
                       
}

void cpu_unloadDevices(void) {

    free(deviceTable);
    
}

void cpu_memWrite(unsigned short address, unsigned short data) {
        
    int i;
    
    //printf("[%X] Writing 0x%04X to 0x%04X\n", currentState.pc, data, address);
        
    for(i = 0; i < deviceCounter; i++) {
            
            if(address <= deviceTable[i]->endAddr
               && address >= deviceTable[i]->startAddr) {
                deviceTable[i]->writeMethod(address, data);
                break;   
            }
            
    }
    
}           

unsigned short cpu_memRead(unsigned short address) {
        
    int i;
        
    //printf("[%X] Reading from 0x%04X\n", currentState.pc, address);    
        
    for(i = 0; i < deviceCounter; i++) {
            
            if(address <= deviceTable[i]->endAddr
               && address >= deviceTable[i]->startAddr) {
                return deviceTable[i]->readMethod(address);
            }
            
    }

    return 0;
    
}

int cpu_cycle(unsigned int requestedCycles) {
         
    int cycle;
         
    for(cycle = 0; (cycle < requestedCycles) && !(currentState.flags & FLAG_HALTED); cycle++) {

#ifdef DEBUG    
        printf("[0x%04X] Executing a cycle.\n", currentState.pc);
#endif //DEBUG
    
        currentState.ir = cpu_memRead(currentState.pc++);
        cpu_doTransfers();
    
    }
    
    if(currentState.flags & FLAG_HALTED)
        return 0;
    else    
        return requestedCycles;
         
}

void cpu_doTransfers(void) {

#ifdef DEBUG
     printf("Command 0x%04X.\n", currentState.ir);
#endif //DEBUG

     switch(IR_TO_CMD(currentState.ir)){
         
         case CMD_ADD:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                             + cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_SUB:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                             - cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_MLT:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                             * cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_DIV:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                             / cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_AND:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                             & cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_OR:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                             | cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_NOT:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         ~cpu_getReg(IR_TO_REGA(currentState.ir)));
         break;
         
         case CMD_XOR:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                             ^ cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_LSH:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                         << cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_RSH:
              cpu_setReg(IR_TO_REGA(currentState.ir),
                         cpu_getReg(IR_TO_REGA(currentState.ir))
                         >> cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_HLT:
              currentState.flags |= FLAG_HALTED;
         break;
         
         case CMD_CMP:
              {
                  int rA = cpu_getReg(IR_TO_REGA(currentState.ir));
                  int rB = cpu_getReg(IR_TO_REGB(currentState.ir));
                  currentState.flags &= ~(FLAG_HALTED | FLAG_IE);
                  if(rA > rB) currentState.flags |= FLAG_GT;
                  if(rA < rB) currentState.flags |= FLAG_LT;
                  if(rA == rB) currentState.flags |= FLAG_EQ;
              }
         break;
         
         case CMD_JMP:
              currentState.pc = cpu_memRead(currentState.pc);
         break;
         
         case CMD_JGT:
              if(currentState.flags & FLAG_GT)
                  currentState.pc = cpu_memRead(currentState.pc);
              else
                  currentState.pc++;
         break;
         
         case CMD_JLT:
              if(currentState.flags & FLAG_LT)
                  currentState.pc = cpu_memRead(currentState.pc);
              else
                  currentState.pc++;
         break;
         
         case CMD_JEQ:
              if(currentState.flags & FLAG_EQ)
                  currentState.pc = cpu_memRead(currentState.pc);
              else
                  currentState.pc++;
         break;
         
         case CMD_MOV:
              cpu_setReg(IR_TO_REGA(currentState.ir), cpu_getReg(IR_TO_REGB(currentState.ir)));
         break;
         
         case CMD_LIT:
              currentState.itp = cpu_memRead(currentState.pc++);
         break;
         
         case CMD_CAL:
              cpu_push(currentState.pc);
              currentState.pc = cpu_memRead(currentState.pc);
         break;
         
         case CMD_RET:
              currentState.pc = cpu_pop();
         break;

         case CMD_POP:
              cpu_setReg(IR_TO_REGA(currentState.ir), cpu_pop());
         break;

         case CMD_PSH:
              cpu_push(cpu_getReg(IR_TO_REGA(currentState.ir)));
         break;

         case CMD_INT:
              cpu_interrupt(IR_TO_INUM(currentState.ir));
         break;

         case CMD_DIN:
              currentState.flags &= ~FLAG_IE;
         break;

         case CMD_EIN:
              currentState.flags |= FLAG_IE;
         break;

         case CMD_LSP:
              currentState.sp = cpu_memRead(currentState.pc++);
         break;
         
         default:
         break;
         
     }

}

unsigned short cpu_getReg(unsigned char regNum){

    if(REG_TO_RNUM(regNum)) 
        if(REG_TO_IND(regNum)) 
            return cpu_memRead(currentState.r[REG_TO_RNUM(regNum) - 1]);
        else
            return currentState.r[REG_TO_RNUM(regNum) - 1];              
    else
        if(REG_TO_IND(regNum)) 
            return cpu_memRead(cpu_memRead(currentState.pc++));
        else
            return cpu_memRead(currentState.pc++);

}

void cpu_setReg(unsigned char regNum, unsigned short value){

    if(REG_TO_RNUM(regNum)) 
        if(REG_TO_IND(regNum)) 
            cpu_memWrite(currentState.r[REG_TO_RNUM(regNum) - 1], value);
        else
            currentState.r[REG_TO_RNUM(regNum) - 1] = value;              
    else
        if(REG_TO_IND(regNum)) 
            cpu_memWrite(cpu_memRead(currentState.pc++), value);

}

void cpu_push(unsigned short data) {

#ifdef DEBUG
    printf("PUSH %X\n", currentState.sp + currentState.sc);
#endif //DEBUG

    cpu_memWrite(currentState.sp + currentState.sc, data);
    currentState.sc += 1;
    
}

unsigned short cpu_pop(void) {
    
    currentState.sc -= 1;

#ifdef DEBUG    
    printf("POP %X\n", currentState.sp + currentState.sc);
#endif
    
    return cpu_memRead(currentState.sp + currentState.sc);
    
}

void cpu_interrupt(unsigned char interruptNumber) {
     
     if(currentState.flags & FLAG_IE) {
         cpu_push(currentState.pc);
         currentState.pc = cpu_memRead(currentState.itp + interruptNumber);
     }
     
}
