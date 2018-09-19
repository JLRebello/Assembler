#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1];
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

typedef struct {
    int iter;
    const char *array[8];
    int hexop; //hey
} OpCodes;
OpCodes allCodes[14];

int main () {
    allCodes[7].iter = 1;
    allCodes[7].array[0] = "LEA";
    allCodes[7].hexop = 0xE;

    allCodes[8].iter = 1;
    allCodes[8].array[0] = "RTI";
    allCodes[8].hexop = 0x8;

    allCodes[9].iter = 3;               //***remember to do iter-1 when cycling
    allCodes[9].array[0] = "LSHF";
    allCodes[9].array[1] = "RSHFL";
    allCodes[9].array[2] = "RSHFA";
    allCodes[9].hexop =  0xD;

    allCodes[10].iter = 1;
    allCodes[10].array[0] = "STB";
    allCodes[10].hexop = 0x3;

    allCodes[11].iter = 1;
    allCodes[11].array[0] = "STW";
    allCodes[11].hexop = 0x7;

    allCodes[12].iter = 1;
    allCodes[12].array[0] = "TRAP";
    allCodes[12].hexop = 0xF;

    allCodes[13].iter = 1;
    allCodes[13].array[0] = "XOR";
    allCodes[13].hexop = 0x9;

    //why though
    char str1[20]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    FILE * fptr;
    fptr = fopen("testing.txt", "r");
    if(fptr==NULL){
        printf("ERROR");
    }
    fscanf(fptr, "%s", str1);
    printf("%s", str1);
    fclose(fptr);
}

