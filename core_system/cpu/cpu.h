#ifndef CPU_H
#define CPU_H

#define IFIELD_CMD 0xFF00
#define IFIELD_REGA 0xF0
#define IFIELD_REGB 0xF
#define IFIELD_INUM 0xFF
#define IFIELD_IND 0x8
#define IFIELD_REGNUM 0x7

//#define DEBUG

#define CMD_ADD 0x0
#define CMD_SUB 0x1
#define CMD_MLT 0x2
#define CMD_DIV 0x3
#define CMD_AND 0x4
#define CMD_OR  0x5
#define CMD_NOT 0x6
#define CMD_XOR 0x7
#define CMD_LSH 0x8
#define CMD_RSH 0x9
#define CMD_HLT 0xA
#define CMD_CMP 0xB
#define CMD_JMP 0xC
#define CMD_JGT 0xD
#define CMD_JLT 0xE
#define CMD_JEQ 0xF
#define CMD_MOV 0x10
#define CMD_LIT 0x11
#define CMD_CAL 0x12
#define CMD_RET 0x13
#define CMD_POP 0x14
#define CMD_PSH 0x15
#define CMD_INT 0x16
#define CMD_DIN 0x17
#define CMD_EIN 0x18
#define CMD_LSP 0x19

#define FLAG_HALTED 0x8000
#define FLAG_GT     0x0001
#define FLAG_LT     0x0002
#define FLAG_EQ     0x0004
#define FLAG_IE     0x4000

#define IR_TO_CMD(ir) ((ir & IFIELD_CMD) >> 8)
#define IR_TO_REGA(ir) ((ir & IFIELD_REGA) >> 4)
#define IR_TO_REGB(ir) (ir & IFIELD_REGB)
#define IR_TO_INUM(ir) (ir & IFIELD_INUM)
#define REG_TO_IND(reg) ((reg & IFIELD_IND) >> 3)
#define REG_TO_RNUM(reg) ((reg & IFIELD_REGNUM))

typedef void (*writerFnc)(unsigned short address, unsigned short data);
typedef unsigned short (*readerFnc)(unsigned short address);

typedef struct deviceEntry {
        unsigned short startAddr;
        unsigned short endAddr;
        writerFnc writeMethod;
        readerFnc readMethod;
} deviceEntry; 

typedef struct cpuState {
        unsigned short r[7];
        unsigned short pc;
        unsigned short sp;
        unsigned short sc;
        unsigned short ir;
        unsigned short itp;
        unsigned short flags;
} cpuState;

void cpu_reset(void);
void cpu_addDevice(deviceEntry* newDevice);
void cpu_memWrite(unsigned short address, unsigned short data);     
unsigned short cpu_memRead(unsigned short address);
int cpu_cycle(unsigned int requestedCycles);
void cpu_doTransfers(void);
unsigned short cpu_getReg(unsigned char regNum);
void cpu_setReg(unsigned char regNum, unsigned short value);
void cpu_push(unsigned short data);
unsigned short cpu_pop(void);
void cpu_interrupt(unsigned char interruptNumber);
void cpu_unloadDevices(void);

#endif
