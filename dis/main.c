#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* The speedrun dissasembler
 *     This is an application to disassemble blobs of assembled/compiled machine code for
 * the speedrun architecture. It has the follwing usage:
 *    "dis filename.bin [-d]"
 *    filename.bin: The name of the file to be disassembled
 *    -d: Optional flag to dump the full listing to stdout and exit
 *    If the -d option is not supplied, dis runs in an interactive mode which
 *    disassembles 6 lines at a time
 */

#define IFIELD_CMD 0xFF00
#define IFIELD_REGA 0xF0
#define IFIELD_REGB 0xF
#define IFIELD_INUM 0xFF
#define IFIELD_IND 0x8
#define IFIELD_REGNUM 0x7

#define IR_TO_CMD(ir) ((ir & IFIELD_CMD) >> 8)
#define IR_TO_REGA(ir) ((ir & IFIELD_REGA) >> 4)
#define IR_TO_REGB(ir) (ir & IFIELD_REGB)
#define IR_TO_INUM(ir) (ir & IFIELD_INUM)
#define REG_TO_IND(reg) ((reg & IFIELD_IND) >> 3)
#define REG_TO_RNUM(reg) (reg & IFIELD_REGNUM)

#define TYP_TWOREG 0
#define TYP_ONEREG 1
#define TYP_IMMONLY 2
#define TYP_NOREG 3
#define TYP_INT 4

const char* base_command[] = {"ADD",
                              "SUB",
                              "MLT",
                              "DIV",
                              "AND",
                              "OR",
                              "NOT",
                              "XOR",
                              "LSH",
                              "RSH",
                              "HLT",
                              "CMP",
                              "JMP",
                              "JGT",
                              "JLT",
                              "JEQ",
                              "MOV",
                              "LIT",
                              "CAL",
                              "RET",
                              "POP", 
                              "PSH",
                              "INT",
                              "DIN",
                              "EIN",
                              "LSP"};
                              
const int command_type[] = {TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_TWOREG,
                            TYP_NOREG,
                            TYP_TWOREG,
                            TYP_IMMONLY,
                            TYP_IMMONLY,
                            TYP_IMMONLY,
                            TYP_IMMONLY,
                            TYP_TWOREG,
                            TYP_NOREG,
                            TYP_IMMONLY,
                            TYP_NOREG,
                            TYP_ONEREG, 
                            TYP_ONEREG,
                            TYP_INT,
                            TYP_NOREG,
                            TYP_NOREG,
                            TYP_IMMONLY };

void dis_loadData(char* fileName, int* dataSize, unsigned short* data[]) {
    //Open the file name, read through to EOF to get the file size, malloc that 
    //many bytes, then read the file into that malloc'd area and return the size
    //and the malloc'd pointer
    int i = 0;
    
    FILE* binFile = fopen(fileName, "rb");
    if(!binFile) {
        printf("Could not load the image '%s'!\n", fileName);
        (*data) = NULL;
        return;
    }
    
    (*dataSize) = 0;
    while(fgetc(binFile) != EOF) ++(*dataSize);
    rewind(binFile);
    
    (*data) = (unsigned short*)malloc((*dataSize)%2?(*dataSize)+1:(*dataSize));
    for( ; i < (*dataSize); ++i) {
        (*data)[i++/2] = fgetc(binFile) << 8;
        if(i < (*dataSize))
            (*data)[i/2] += fgetc(binFile) & 0xFF;
    }
    
    fclose(binFile);
    
    (*dataSize) = (*dataSize)%2?((*dataSize)+1)/2:(*dataSize)/2;
        
}

int dis_isNumber(char* testString) {
    
    int isNumber = 1;
    int hexString = 0;
    int strIndex = 0;
    
    if(testString[0] == '0' && (testString[1] == 'x' || testString[1] == 'X')) {
        strIndex = 2;
        hexString = 1;
    }
    
    for(; testString[strIndex] != 0 && ((testString[strIndex] >= '0' && testString[strIndex] <= '9') || (hexString && ((testString[strIndex] >= 'a' && testString[strIndex] <= 'f') || (testString[strIndex] >= 'A' && testString[strIndex] <= 'F')))); strIndex++);
    
    return testString[strIndex] == 0;
    
}

int dis_valueOf(char* numString) {
    int base = 10;
    int value = 0;
    int place = 1;
    int offset = 0;
    int working;
    int places; 
 
    for(places = 0; numString[places]; places++);
 
    if(numString[0] == '0' && (numString[1] == 'x' || numString[1] == 'X')) {
        places -= 2;
        offset = 2;
        base = 16;
    }
        
    for( ; places-place+1; place++) {
         if(numString[places+offset-place] >= 'A' && numString[places+offset-place] <= 'F')
             working = numString[places+offset-place] - 'A' + 10;
             
         if(numString[places+offset-place] >= 'a' && numString[places+offset-place] <= 'f')
             working = numString[places+offset-place] - 'a' + 10;
             
         if(numString[places+offset-place] >= '0' && numString[places+offset-place] <= '9')
             working = numString[places+offset-place] - '0';    
         
         working *= (int)pow((double)base, (double)(place-1)); 
         value += working;
    }
    
    return value;
    
}

void dis_readCommand(char* cmdBuffer, int size) {
    int tmpChar;
    int bufferIndex = 0;

    if(cmdBuffer == NULL) {
        printf("\nERROR: Could not allocate memory for user input!\n");
        return;
    }
    
    while(1) {
    
        tmpChar = getchar();
    
        if(tmpChar == '\n' || tmpChar == EOF) { 
            cmdBuffer[bufferIndex] = 0;
            break;
        }else{
            if(bufferIndex < (size-1) && tmpChar > 0)
                cmdBuffer[bufferIndex++] = (char)tmpChar;
        }
            
    
    } 
    
 
}

/* Interactive commands:
 * any decimal number = begin listing from that address
 * any hex number proceeded by a 0x = begin listing from that address
 * no input = proceed to next page
 * quit = exit the program
 */
void dis_updateFromUser(int* currentAddress) {   
    char* userString;
    userString = (char*)malloc(7);
    
    while(1) {    
        printf("dis[%04X]>", *currentAddress);
        
        dis_readCommand(userString, 7);
        
        if(userString == NULL) {
            *currentAddress = -1;
            return;
        }
            
        if(userString[0] == 0)
            return;
        
        if(!strcmp("quit", userString)) {
            *currentAddress = -1;
            return;
        }
        
        if(dis_isNumber(userString)) {
            *currentAddress = dis_valueOf(userString);
            return;        
        }
        
        printf("Command '%s' not recognized.\n", userString);
     
    }
    
    free(userString);
    
}

//This is the meat of dis. It parses the command starting at 'address' and
//returns the disassembled string for the instruction or data starting at that location.
//As a side effect, it ratchets up address based on the number of words needed
//to decode the command and places that number of words in instSize
char* dis_getLine(unsigned short* data, int dataSize, int* address, int* instSize) {
      
      static char basic[] = "Basic command";
      static char ext[] = "Extended command";
      static char output[80];
      unsigned char commandNumber;
      unsigned char regA, regB;
      unsigned long sourceAddress;
      unsigned long destAddress;
      
      *instSize = 1;
      
      if(*address >= dataSize || address < 0)
          return NULL;
      
      commandNumber = IR_TO_CMD(data[*address]);
      if(commandNumber < 0x20){
                       
          sprintf(output, "%s", base_command[commandNumber]);
                       
          if(command_type[commandNumber] == TYP_ONEREG || command_type[commandNumber] == TYP_TWOREG){
              regA = IR_TO_REGA(data[*address]);
              
              if(REG_TO_IND(regA))
                  if(REG_TO_RNUM(regA))
                      sprintf(output + strlen(output), " [r%d]", REG_TO_RNUM(regA) - 1);
                  else
                      sprintf(output + strlen(output), " [%04X]", data[(*address)+((*instSize)++)]);
              else 
                  if(REG_TO_RNUM(regA))
                      sprintf(output + strlen(output), " r%d", REG_TO_RNUM(regA) - 1);
                  else
                      sprintf(output + strlen(output), " INVALID_DESTINATION");           
              
              if(command_type[commandNumber] == TYP_TWOREG) {
                  regB = IR_TO_REGB(data[*address]);
                  
                  if(REG_TO_IND(regB))
                      if(REG_TO_RNUM(regB))
                          sprintf(output + strlen(output), ", [r%d]", REG_TO_RNUM(regB) - 1);
                      else
                          sprintf(output + strlen(output), ", [%04X]", data[(*address)+((*instSize)++)]);
                  else 
                      if(REG_TO_RNUM(regB))
                          sprintf(output + strlen(output), ", r%d", REG_TO_RNUM(regB) - 1);
                      else
                          sprintf(output + strlen(output), ", %04X", data[(*address)+((*instSize)++)]);
                          
              }
              
          }
          
          if(command_type[commandNumber] == TYP_INT){
              sprintf(output + strlen(output), " %X", IR_TO_INUM(data[*address]));
          }
          
          if(command_type[commandNumber] == TYP_IMMONLY){
              sprintf(output + strlen(output), " %04X", data[(*address)+((*instSize)++)]);
          }
          
      }else{
          sprintf(output, "DW %04X", data[*address]);
      }
                
      *address += *instSize;
      return &output[0];
       
}

int main(int argc, char *argv[])
{
  
    const char usageMessage[] = "Usage:\tdis filename.bin [-d]\n";
    int address = 0;
    int interactive = 1;
    unsigned short* data;
    int dataSize, lineCount, instSize, i;
    char* currentLine;
  
    if(argc < 2 || argc > 3) {
        printf(usageMessage);
        return 0;
    }
    
    if(argc == 3) {
        if(!strcmp(argv[2], "-d")) {
            interactive = 0;
        }else{
            printf(usageMessage);
            return 0;
        }
    }
        
    dis_loadData(argv[1], &dataSize, &data);
    if(data == NULL)
        return 0;
    
    while(1) {
           
                
            
        //Get user input
        if(interactive)
            dis_updateFromUser(&address);
                    
        //Quit if the user said so
        if(address < 0)
            return 0;
        
        //I made this convoluted and attempted to do what I needed with zero braces just because fuck it
        //There, just like python! Kinda!
        for(lineCount = 0; (lineCount < 6) && ((currentLine = dis_getLine(data, dataSize, &address, &instSize)) != NULL); lineCount++)
            if(interactive)
                for(i = 0; i < instSize; i++)
                    if(i)
                        printf("[%04X] %04X\n",  address - (instSize - i), data[address - (instSize - i)]);
                    else
                        printf("[%04X] %04X : %s\n",  address - (instSize - i), data[address - (instSize - i)], currentLine);
            else
                printf("%s\n", currentLine);    
                
        if(!currentLine)
            if(interactive)
                printf("Dis has reached the end of the file.\n");
            else
                return 0;
    
    }
  
  //data should be free()'d when we're done, but, as my mantra goes, fuck it. The OS will take care of that minor detail.
  
}
