#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#define TYP_DATA 5

const unsigned char command_value[] = {0x0,
                                       0x1,
                                       0x2,
                                       0x3,
                                       0x4,
                                       0x5,
                                       0x6,
                                       0x7,
                                       0x8,
                                       0x9,
                                       0xA,
                                       0xB,
                                       0xC,
                                       0xD,
                                       0xE,
                                       0xF,
                                       0x10,
                                       0x11,
                                       0x12,
                                       0x13,
                                       0x14, 
                                       0x15,
                                       0x16,
                                       0x17,
                                       0x18,
                                       0x19,
                                       0x0};

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
                              "LSP",
                              "DW", "\0" };
                              
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
                            TYP_IMMONLY,
                            TYP_DATA };


//These will be created and added to the list of symbols
//either when a symbol label is encountered or when 
//a symbol which has not yet been added is found in the code
typedef struct sct_symbol {
        char* name;
        int resolved;
        unsigned short address;
} symbol;

//These will be added to the list of symbols each time
//the assembler comes across the usage of a symbol in the source
typedef struct sct_instance {
        symbol* sym;
        unsigned short address;
} instance;

instance** instanceList = NULL;
int instanceCount = 0;
symbol** symbolList = NULL;
int symbolCount = 0;

symbol* findSymbol(char* symbolName) {
        
        int i;
        
        i = 0;
        if(symbolList != NULL)
            for( ; i < symbolCount; i++) { 
                if(!strcmp(symbolList[i]->name, symbolName))
                    break;
            }
                    
        if(i == symbolCount || symbolList == NULL)
            return NULL;
        else
            return symbolList[i];        
        
}

symbol* addSymbol(char* symbolName, int resolved, unsigned short address) {
    int i;
    
    symbol* newSymbol = (symbol*)malloc(sizeof(symbol));
    if(newSymbol == NULL) {
        printf("Error: Out of symbol memory.\n");
        return NULL;
    }
    
    newSymbol->name = (char*)malloc(strlen(symbolName) + 1);
    strcpy(newSymbol->name, symbolName);
    newSymbol->resolved = resolved;
    newSymbol->address = address;
    
    symbolList = (symbol**)realloc(symbolList, sizeof(symbol*)*(symbolCount + 1));
    if(symbolList == NULL) {
        printf("Error: Out of symbol list memory.\n");
        return NULL;
    }
    
    symbolList[symbolCount] = newSymbol;
    
    return symbolList[symbolCount++];
    
}

int as_isNumber(char* testString, int allowHex) {
    
    int isNumber = 1;
    int strIndex = 0;
        
    for(; testString[strIndex] != 0 && ((testString[strIndex] >= '0' && testString[strIndex] <= '9') || (allowHex && ((testString[strIndex] >= 'a' && testString[strIndex] <= 'f') || (testString[strIndex] >= 'A' && testString[strIndex] <= 'F')))); strIndex++);
    
    return testString[strIndex] == 0;
    
}

int as_valueOf(char* numString, int base) {
    int value = 0;
    int place = 1;
    int offset = 0;
    int working;
    int places; 
 
    for(places = 0; numString[places]; places++);
         
    for( ; places-place+1; place++) {
         if(numString[places-place] >= 'A' && numString[places-place] <= 'F')
             working = numString[places+offset-place] - 'A' + 10;
             
         if(numString[places-place] >= 'a' && numString[places-place] <= 'f')
             working = numString[places+offset-place] - 'a' + 10;
             
         if(numString[places-place] >= '0' && numString[places-place] <= '9')
             working = numString[places+offset-place] - '0';    
         
         working *= (int)pow((double)base, (double)(place-1)); 
         value += working;
    }
    
    return value;
    
}

instance* addInstance(char* symbolName, unsigned short address) {
    
    instance* newInstance = (instance*)malloc(sizeof(instance));
    if(newInstance == NULL) {
        printf("Error: Could not allocate a new instance structure.\n");
        return NULL;
    }
    
    symbol* parentSymbol = findSymbol(symbolName);
    if(parentSymbol == NULL) {
        parentSymbol = addSymbol(symbolName, 0, 0);
    }
    
    newInstance->sym = parentSymbol;
    newInstance->address = address;
    
    instanceList = (instance**)realloc(instanceList, sizeof(instance*) * (instanceCount + 1));
    if(instanceList == NULL) {
        printf("Error: Out of instance list memory.\n");
        return NULL;
    }
    
    instanceList[instanceCount] = newInstance;
    return instanceList[instanceCount++];
    
}

int as_isLabel(char* token) {
    return strlen(token) ? token[strlen(token)-1] == ':' : 0;
}

void as_labelToSymbolName(char* label) {
    label[strlen(label)-1] = 0;    
}

int as_isCommand(char* token) {

    int i;
    
    for(i = 0; strlen(base_command[i]) && (strcmp(base_command[i], token) != 0); i++);
    
    return strlen(base_command[i]) == 0 ? -1 : i;

}

void as_dumpWhitespace(FILE* sourceFile, int* lineEnd) {

    int tempc;
    
     while(1) {
        tempc = fgetc(sourceFile);
        if(tempc != ' ' &&
           tempc != '\t' &&
           tempc != '\r' &&
           tempc != '\f')
            break;
    }

    if(tempc == '\n')
        *lineEnd = 1;
 
    if(tempc == EOF) {
        *lineEnd = 2;
    }
 
    if(tempc != EOF && tempc != '\n')
        fseek(sourceFile, -1, SEEK_CUR);

}

int as_nextToken(FILE* sourceFile, char* tokenBuffer, int bufsz, int* lineEnd) {
    
    int bufIndex, tempc;
    
    if(tokenBuffer == NULL || bufsz == 0) {
        printf("Error: invalid token buffer.\n");
        return 0;
    }
    
    tokenBuffer[bufIndex = 0] = 0;
    
    as_dumpWhitespace(sourceFile, lineEnd);
    if(*lineEnd) return 1;
    
    while(bufIndex < bufsz - 1) {
        tempc = fgetc(sourceFile);
        if(tempc == EOF ||
           tempc == ' ' ||
           tempc == '\n' ||
           tempc == '\t' ||
           tempc == '\r' ||
           tempc == '\f')
            break;
        tokenBuffer[bufIndex++] = tempc;
    }
    
    if(tempc == EOF){
        *lineEnd = 2;
        return;
    }
    
    if(bufIndex == bufsz - 1)
        printf("Warning: token buffer overflow.");
    
    tokenBuffer[bufIndex] = 0;
    
    if(tempc == '\n') *lineEnd = 1;
    
    return 1; 
    
}

void as_nextLineTokens(FILE* sourceFile, char** tokens, int* tokenCount, int* eof) {

    int tempc;
    int endLine;

    *tokenCount = 0;

    while(1) {
        endLine = 0;
        as_nextToken(sourceFile, tokens[(*tokenCount)], 80, &endLine);
        if(tokens[(*tokenCount)] == NULL) {
            tokenCount = 0;
        }else if(strlen(tokens[(*tokenCount)]))
                (*tokenCount)++;
        if(endLine) {
            *eof = (endLine == 2);
            return;
        }
    }

}

int as_registerNumber(char* regName) {
    
    int value;
    
    if((regName[0] != 'r' && regName[0] != 'R') || strlen(regName) != 2 || !as_isNumber(regName+1, 0)) {
        return -1;
    }
         
    value = as_valueOf(regName+1, 10) + 1;
    return value > 7 ? -1 : value;
    
}

int as_writeObject(unsigned short* memory, int binSize, char* objectName) {

    FILE* outFile;
    char* stringTable;
    unsigned short *stringOffset, offsetValue, i, j;
    int stringTableLen, stringCount;

    if(!(outFile = fopen(objectName, "wb"))) {
        printf("Error: Could not open output file %s.\n", objectName);
        return 0;
    }

    //Assemble the string table and a list of offsets for each string
    for(stringTableLen = stringCount = 0; stringCount < symbolCount; stringCount++)
        stringTableLen += strlen(symbolList[stringCount]->name) + 1;
    stringTable = (char*)malloc(stringTableLen);
    stringOffset = (unsigned short*)malloc(sizeof(unsigned short)*stringCount);
    for(offsetValue = i = 0; i < symbolCount; i++) {
        stringOffset[i] = offsetValue;
        strcpy(stringTable+offsetValue, symbolList[i]->name); 
        offsetValue += strlen(symbolList[i]->name) + 1;
    }
    
    //Write the magic number
    fputc('S', outFile);
    fputc('F', outFile);
    
    //Write the code size
    fputc((binSize & 0xFF00) >> 8, outFile);
    fputc((binSize & 0xFF), outFile);
    
    //Write the symbol count
    fputc((symbolCount & 0xFF00) >> 8, outFile);
    fputc((symbolCount & 0xFF), outFile);
    
    //Write the relocation count
    fputc((instanceCount & 0xFF00) >> 8, outFile);    
    fputc((instanceCount & 0xFF), outFile);
    
    //Write the code
    for(i = 0; i < binSize; i++) {
        fputc((memory[i] & 0xFF00) >> 8, outFile);    
        fputc((memory[i] & 0xFF), outFile);
    }
    
    //Write the symbol entries
    for(i = 0; i < symbolCount; i++) {
        if(symbolList[i]->resolved)
            fputc(0xFF, outFile);
        else
            putc(0x00, outFile);
        fputc((symbolList[i]->address & 0xFF00) >> 8, outFile);    
        fputc((symbolList[i]->address & 0xFF), outFile);
        fputc((stringOffset[i] & 0xFF00) >> 8, outFile);    
        fputc((stringOffset[i] & 0xFF), outFile);
    }
        
    //Write the relocation entries
    for(i = 0; i < instanceCount; i++) {
        for(j = 0; j < symbolCount; j++) {
            if(instanceList[i]->sym == symbolList[j]) {
                fputc((j & 0xFF00) >> 8, outFile);
                fputc((j & 0xFF), outFile);
                break;
            }
        }
        fputc((instanceList[i]->address & 0xFF00) >> 8, outFile);    
        fputc((instanceList[i]->address & 0xFF), outFile);
    }
    
    //Write the string table
    for(i = 0; i < offsetValue; i++) {
        fputc(stringTable[i], outFile);
    }

    fclose(outFile);

}

int as_translateLine(FILE* sourceFile, int* address, unsigned short* memory, int* lineNumber) {

    unsigned short currentCell;
    char** tokens;
    int tokenCount, eof, i, j, commandIndex, regA, regB, words;
    symbol* tempSymbol;
        
    words = 1;
    
    (*lineNumber)++;
    
    tokens = (char**)malloc(sizeof(char*)*20);
    if(tokens == NULL){
        printf("Error: Could not initialize token table.\n");
        return -1;
    }
    
    for(i = 0; i < 20; i++)
        if((tokens[i] = (char*)malloc(sizeof(char*)*80))==NULL){
            printf("Error: could not allocate token buffer #%d.\n", i);
            for(j = 0; j < i; j++)
                free(tokens[j]);
            free(tokens);
            return -1;
        }
        
    as_nextLineTokens(sourceFile, tokens, &tokenCount, &eof);
    if(tokens == NULL)
        return -1;
    if(!tokenCount)
        if(eof)
            return 0;
        else
            return 1;
    
    for( i = 0; as_isLabel(tokens[i]) && i < tokenCount; i++) {
        as_labelToSymbolName(tokens[i]);
        tempSymbol = findSymbol(tokens[i]);
        if(tempSymbol == NULL)
            addSymbol(tokens[i], 1, *address);
        else {
            if(tempSymbol->resolved){ 
                printf("Error: multiple declaration of symbol '%s' at line %d.\n", tokens[i], *lineNumber);
                return -1;
            }else{
                tempSymbol->resolved = 1;
                tempSymbol->address = *address;
            }
        }
    }
        
    if(i == tokenCount) return 1;
    
    if( (commandIndex = as_isCommand(tokens[i])) < 0) { 
        printf("Error: unrecognized command '%s' at line %d.\n", tokens[i], *lineNumber);
        return -1;     
    }
    
    currentCell = command_value[commandIndex] << 8;

    i++;
    if(i == tokenCount)
        if(command_type[commandIndex] == TYP_NOREG) {
            memory[*address] = currentCell;
            *address += words;
    
            for(j = 0; j < 20; j++)
                free(tokens[j]);
            free(tokens);                          
            return 1; 
        }else{
            printf("Error: missing operand, immediate or address at line %d.\n", *lineNumber);
            return -1;  
        }
    
    if(command_type[commandIndex] == TYP_ONEREG || command_type[commandIndex] == TYP_TWOREG) {
        
        if(command_type[commandIndex] == TYP_TWOREG)
            if(tokens[i][strlen(tokens[i])-1] != ',') {
                printf("Error: expected comma immediately following first operand at line %d.\n", *lineNumber);
                return -1;
            }else{
                tokens[i][strlen(tokens[i])-1] = 0;
            }
        
        regA = 0;
        if(tokens[i][0] == '[' && tokens[i][strlen(tokens[i])-1] == ']' ) {
            regA = 0x8;
            tokens[i]++; //Get rid of first brace
            tokens[i][strlen(tokens[i])-1] = 0; //And the end brace
        }
        
        if(as_isNumber(tokens[i], 1)) {
            if(REG_TO_IND(regA)) {
                memory[*address + words++] = 0xFFFF & as_valueOf(tokens[i], 16);
            }else{
                printf("Error: destination operand at line number %d is an immediate.\n", *lineNumber);
                return -1;
            }
        }else if(as_registerNumber(tokens[i]) >= 0) {
                regA |= as_registerNumber(tokens[i]);
              }else{
                if(REG_TO_IND(regA)) {
                    addInstance(tokens[i], *address + 1);
                    memory[*address + words++] = 0;
                }else{
                    printf("Error: destination operand at line number %d is an immediate.\n", *lineNumber);
                    return -1;
                }
        }
                
        
        currentCell |= regA << 4;        
        
        if(command_type[commandIndex] == TYP_TWOREG) {
            i++;
            if(i == tokenCount) {
                printf("Error: missing operand, immediate or address at line %d.\n", *lineNumber);
                return -1;  
            }
                    
            regB = 0;                                              
            if(tokens[i][0] == '[' && tokens[i][strlen(tokens[i])-1] == ']' ) {
                regB = 0x8;
                tokens[i]++; //Get rid of first brace
                tokens[i][strlen(tokens[i])-1] = 0; //And the end brace
            }
            
            if(as_isNumber(tokens[i], 1)) {
                memory[*address + words++] = 0xFFFF & as_valueOf(tokens[i], 16);
            }else if(as_registerNumber(tokens[i]) >= 0) {
                    regB |= as_registerNumber(tokens[i]);
                  }else{
                      addInstance(tokens[i], *address + 1);
                      memory[*address + words++] = 0;
                  }
            
            currentCell |= regB;
            
        }
    }

    if(command_type[commandIndex] == TYP_IMMONLY || command_type[commandIndex] == TYP_INT || command_type[commandIndex] == TYP_DATA) {
    
        if(tokens[i][0] == '[' && tokens[i][strlen(tokens[i]-1)] == ']' ) {
            printf("Error: Indirect addressing is not compatible with the command at line %d.\n", *lineNumber);
            return -1;
        }else{
            if(as_isNumber(tokens[i], 1)) {
                if(command_type[commandIndex] == TYP_INT)
                    currentCell |= 0xFF & as_valueOf(tokens[i],16);
                
                if(command_type[commandIndex] == TYP_DATA)
                    currentCell |= 0xFFFF & as_valueOf(tokens[i],16);
                    
                if(command_type[commandIndex] == TYP_IMMONLY)    
                    memory[*address + words++] = 0xFFFF & as_valueOf(tokens[i],16);
            }else{
                if(command_type[commandIndex] == TYP_INT) {
                    printf("Error: Attempt to use a symbol as an interrupt number at line %d.\n", *lineNumber);
                    return -1;
                }
                
                if(command_type[commandIndex] == TYP_IMMONLY) {
                    addInstance(tokens[i], *address + 1);
                    memory[*address + words++] = 0;
                }
                
                if(command_type[commandIndex] == TYP_DATA) {
                    addInstance(tokens[i], *address);
                }
                
            }
        }    
    
    }

    if(++i != tokenCount) {
        printf("Error: Expected end of line at line %d.\n", *lineNumber);
    }

    memory[*address] = currentCell;
    *address += words;
    
    for(j = 0; j < 20; j++)
        free(tokens[j]);
    free(tokens);
    
    if(eof)
        return 0;
    else
        return 1;

}

int main(int argc, char *argv[]) {
  
    const char usageMessage[] = "Usage:\tas source.asm [-o output.sf]\n";
    int address = 0;
    int lineNumber = 0;
    int returnCode;
    char* outName;
    FILE *sourceFile;
    unsigned short* memory;
  
    if(argc != 2 && argc != 4 ) {
        printf(usageMessage);
        return 0;
    }
    
    if(argc == 4) {
        if(!strcmp(argv[2], "-o")) {
            outName = argv[3];
        }else{
            printf(usageMessage);
            return 0;
        }
    }else{
          
          if(!(outName = malloc(strlen(argv[1])+1))){
              printf("Error: Could not allocate memory for the output filename buffer.\n");
              return 0;
          }
          
          strcpy(outName, argv[1]);
          outName[strlen(outName)-4] = '.';
          outName[strlen(outName)-3] = 's';
          outName[strlen(outName)-2] = 'f';
          outName[strlen(outName)-1] = 0;
          
    }
        
    sourceFile = fopen(argv[1], "r");
    if(sourceFile == NULL) {
        printf("Error: Could not open source file %s.\n", argv[1]);
        return 0;
    }
    
    memory = (unsigned short*)malloc(sizeof(unsigned short)*0x10000);
    if(memory == NULL) {
        printf("Error: Could not allocate working memory in which to build the binary.\n");
        return 0;
    }
    
    while(1) {
              
        returnCode = as_translateLine(sourceFile, &address, memory, &lineNumber);
        
        if(returnCode < 0)
            break;
            
        if(returnCode == 0) {
            as_writeObject(memory, address, outName);
            break;
        }
    
    }
  
    fclose(sourceFile);
    free(memory);
    return 0;
  
}
