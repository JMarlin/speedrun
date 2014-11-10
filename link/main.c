#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFMT_NULL -1
#define OFMT_BIN 1
#define OFMT_SF 2

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
    symbol* newSymbol;

    //If a symbol is found that was already defined, either use it to
    //resolve the previous declaration or ignore the dupe
    if((newSymbol = findSymbol(symbolName)) != NULL) {
        if(resolved) {
            if(newSymbol->resolved) {
                printf("Multiple declaration of symbol %s.\n", symbolName);
                return NULL;
            }else{
                newSymbol->resolved = resolved;
                newSymbol->address = address;
            }
        }

        return newSymbol;
    }

    newSymbol = (symbol*)malloc(sizeof(symbol));
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

int link_writeBin(unsigned short* memory, int binSize, char* objectName) {

    FILE* outFile;
    unsigned short i, j;

    if(!(outFile = fopen(objectName, "wb"))) {
        printf("Error: Could not open output file %s.\n", objectName);
        return 0;
    }

    for(i = 0; i < symbolCount; i++)
        if(!symbolList[i]->resolved) {
            printf("Error: Unresolved symbol '%s'.\n", symbolList[i]->name);
            fclose(outFile);
            return 0;
        }

    //Write the code
    for(i = 0; i < binSize; i++) {
        for(j = 0; j < instanceCount; j++)
            if(instanceList[j]->address == i) break;

        if(j == instanceCount) {
            //printf("%04X\n", memory[i]);
            fputc((memory[i] & 0xFF00) >> 8, outFile);
            fputc((memory[i] & 0xFF), outFile);
        } else {
            //printf("%04X\n", instanceList[j]->sym->address);
            fputc((instanceList[j]->sym->address & 0xFF00) >> 8, outFile);
            fputc((instanceList[j]->sym->address & 0xFF), outFile);
        }
    }

    fclose(outFile);

}

int link_writeObject(unsigned short* memory, int binSize, char* objectName) {

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
        //printf("%04X\n", memory[i]);
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

int link_collectArgs(int* argc, char **argv, int* outFormat, int* inCount, int* outIndex) {

    int i, inFiles;

    *outFormat = OFMT_SF;
    *inCount = 0;
    *outIndex = 0;

    inFiles = 1;

    if(*argc < 4) return 0;

    for(i = 1; i < *argc; i++) {

        if(!strcmp(argv[i], "-o")) {
            if(i == 1) {
                printf("Output switch must follow input files.\n");
                return 0;
            }
            if(*argc == i + 1 || !strcmp(argv[i + 1], "-f")) {
                printf("Output filename must follow '-o' switch.\n");
                return 0;
            }
            if(*argc > i + 2)
                if(strcmp(argv[i + 2], "-f") != 0) {
                    printf("Unknown argument following output filename.\n");
                    return 0;
                }
            *outIndex = i + 1;
            inFiles = 0;
            i++;
        }

        if(!strcmp(argv[i], "-f")) {
            if(i < 3) {
                printf("Format switch must be at the end of the arguments.\n");
                return 0;
            }
            if(*argc == i + 1) {
                printf("An output format must be specified after the '-f' switch.\nValid formats are:\n\tbin:\tA flat binary image.\n\tsf:\tAn SF format relocatable object.\n");
                return 0;
            }
            if(*argc > i + 2) {
                printf("Unknown arguments at end of input A.\n");
                return 0;
            }
            if(!strcmp(argv[i + 1], "bin")) {
                  *outFormat = OFMT_BIN;
                  break;
            }else if(!strcmp(argv[i + 1], "sf")) {
                  *outFormat = OFMT_SF;
                  break;
            }else{
                printf("Unrecognized output format.\n");
                *outFormat = OFMT_NULL;
                return 0;
            }
        }

        if(inFiles) {
            *inCount = i;
        }

    }

    if(!(*inCount)) {
        printf("No input files specified.\n");
        return 0;
    }

    if(outIndex == 0) {
        printf("No output file specified.\n");
        return 0;
    }

    return 1;

}

int main(int argc, char *argv[])
{
    FILE* sfFile;
    unsigned short codeSize, codeOffset;
    char* stringTable;
    unsigned short* binBlock;
    char** inList;
    int outIndex;
    int i, binOffset, outFormat, inCount, fileIndex, symCnt, relocCnt;

    typedef struct symentry {
        char* name;
        int resolved;
        int nameOffset;
        int address;
    } symentry;

    typedef struct relocation {
        int symbolNumber;
        int address;
    } relocation;

    relocation* lclRelocs;
    symentry* lclSyms;
    stringTable = (char*)malloc(2000);
    binBlock = (unsigned short*)malloc(4000);

    if(!link_collectArgs(&argc, argv, &outFormat, &inCount, &outIndex)) {
        printf("Usage:\tlink object1.sf [object2.sf [object3.sf [...]]] -o output.bin [-f (bin|sf)]\n");
        return 0;
    }

    codeOffset = 0;
    for(fileIndex = 1; fileIndex < inCount + 1; fileIndex++) {

        if(!(sfFile = fopen(argv[fileIndex], "rb"))) {
            printf("Error: Could not open object #%d '%s'.\n", fileIndex, argv[fileIndex]);
            free(lclRelocs);
            free(lclSyms);
            return 0;
        }

        if(fgetc(sfFile) != 'S') {
            printf("Error: %s is not an sf format file.\n", argv[fileIndex]);
            free(lclRelocs);
            free(lclSyms);
            return 0;
        }

        if(fgetc(sfFile) != 'F') {
            printf("Error: %s is not an sf format file.\n", argv[fileIndex]);
            free(lclRelocs);
            free(lclSyms);
            return 0;
        }

        codeSize = fgetc(sfFile) << 8;
        codeSize += fgetc(sfFile);

        symCnt = fgetc(sfFile) << 8;
        symCnt += fgetc(sfFile);
        lclSyms = (symentry*)malloc(sizeof(symentry) * symCnt);

        relocCnt = fgetc(sfFile) << 8;
        relocCnt += fgetc(sfFile);
        lclRelocs = (relocation*)malloc(sizeof(relocation) * relocCnt);

        for(i = 0; i < codeSize; i++) {
            binBlock[i + codeOffset] = fgetc(sfFile) << 8;
            binBlock[i + codeOffset] += fgetc(sfFile);
        }

        for(i = 0; i < symCnt; i++) {
            lclSyms[i].resolved = (fgetc(sfFile) == 0xFF);
            lclSyms[i].address = fgetc(sfFile) << 8;
            lclSyms[i].address += fgetc(sfFile);
            lclSyms[i].nameOffset = fgetc(sfFile) << 8;
            lclSyms[i].nameOffset += fgetc(sfFile);
        }

        for(i = 0; i < relocCnt; i++) {
            lclRelocs[i].symbolNumber = fgetc(sfFile) << 8;
            lclRelocs[i].symbolNumber += fgetc(sfFile);
            lclRelocs[i].address = fgetc(sfFile) << 8;
            lclRelocs[i].address += fgetc(sfFile);
        }

        for( i = 0; (stringTable[i] = fgetc(sfFile)) != EOF ; i++);

        for(i = 0; i < symCnt; i++) {
            lclSyms[i].name = stringTable + lclSyms[i].nameOffset;
            addSymbol(lclSyms[i].name, lclSyms[i].resolved, lclSyms[i].address + codeOffset);
        }

        for(i = 0; i < relocCnt; i++)
            addInstance(lclSyms[lclRelocs[i].symbolNumber].name, lclRelocs[i].address + codeOffset);

        fclose(sfFile);
        free(lclRelocs);
        free(lclSyms);

        codeOffset += codeSize;

    }

    printf("Code size: %d\n", codeOffset);
    printf("Symbol count: %d\n", symbolCount);
    printf("Relocation count: %d\n", instanceCount);

    printf("\nSymbols:\n");
    for(i = 0; i < symbolCount; i++)
        if(symbolList[i]->resolved)
            printf("\t%s = 0x%04X\n", symbolList[i]->name, symbolList[i]->address);
        else
            printf("\t%s = Unresolved\n", symbolList[i]->name);

    printf("\nRelocations:\n");
    for(i = 0; i < instanceCount; i++)
        printf("\t#%d: %s @ 0x%04X\n", i+1, instanceList[i]->sym->name, instanceList[i]->address);

    if(outFormat == OFMT_SF)
        link_writeObject(binBlock, codeOffset, argv[outIndex]);
    else
        link_writeBin(binBlock, codeOffset, argv[outIndex]);

    free(stringTable);
    free(binBlock);

}
