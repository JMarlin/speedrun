#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE* sfFile;
    unsigned short codeSize, symbolCount, relocationCount;
    char* stringTable;
    int i;
    
    typedef struct symbol {
        char* name;
        int resolved;
        int nameOffset;
        int address;
    } symbol;
    
    typedef struct relocation {
        int symbolNumber;
        int address;
    } relocation;
    
    symbol* symbolList;
    relocation* relocationList;
    
    stringTable = (char*)malloc(2000);
    
    if(argc != 2) {
        printf("Usage:\tsfdump object.sf\n");
        return 0;
    }
    
    if(!(sfFile = fopen(argv[1], "rb"))) {
        printf("Error: Could not open object '%s'.\n", argv[1]);
        return 0;
    }
    
    if(fgetc(sfFile) != 'S') {
        printf("Error: Not an sf format file.\n");
        return 0;
    }
    
    if(fgetc(sfFile) != 'F') {
        printf("Error: Not an sf format file.\n");
        return 0;
    }
    
    codeSize = fgetc(sfFile) << 8;
    codeSize += fgetc(sfFile);
    
    symbolCount = fgetc(sfFile) << 8;
    symbolCount += fgetc(sfFile);
    symbolList = (symbol*)malloc(sizeof(symbol) * symbolCount);
    
    relocationCount = fgetc(sfFile) << 8;
    relocationCount += fgetc(sfFile);
    relocationList = (relocation*)malloc(sizeof(relocation) * relocationCount); 
    
    for(i = 0; i < codeSize; i++) {
        fgetc(sfFile);
        fgetc(sfFile);
    }
    
    for(i = 0; i < symbolCount; i++) {
        symbolList[i].resolved = (fgetc(sfFile) == 0xFF);
        symbolList[i].address = fgetc(sfFile) << 8;
        symbolList[i].address += fgetc(sfFile);
        symbolList[i].nameOffset = fgetc(sfFile) << 8;
        symbolList[i].nameOffset += fgetc(sfFile);
    }
    
    for(i = 0; i < relocationCount; i++) {
        relocationList[i].symbolNumber = fgetc(sfFile) << 8;
        relocationList[i].symbolNumber += fgetc(sfFile);
        relocationList[i].address = fgetc(sfFile) << 8;
        relocationList[i].address += fgetc(sfFile);
    }
    
    for( i = 0; (stringTable[i] = fgetc(sfFile)) != EOF ; i++);
    
    for(i = 0; i < symbolCount; i++)
        symbolList[i].name = stringTable + symbolList[i].nameOffset;
        
    printf("Code size: %d\n", codeSize);
    printf("Symbol count: %d\n", symbolCount);
    printf("Relocation count: %d\n", relocationCount);
    
    printf("\nSymbols:\n");
    for(i = 0; i < symbolCount; i++)
        if(symbolList[i].resolved)
            printf("\t%s = 0x%04X\n", symbolList[i].name, symbolList[i].address);
        else
            printf("\t%s = Unresolved\n", symbolList[i].name);
        
    printf("\nRelocations:\n");
    for(i = 0; i < relocationCount; i++)
        printf("\t#%d: %s @ 0x%04X\n", i+1, symbolList[relocationList[i].symbolNumber].name, relocationList[i].address);
    
}
