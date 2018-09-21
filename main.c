#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1];
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

typedef struct {
    int iter;
    char* array[8];
    int hexop;
} OpCodes;
OpCodes allCodes[14];

int BAdd;                   //address counter starting with addr specified by .ORIG (oops I needed a Global Variable)

//HELPER FUNCTIONS
bool isValid(char str[20]);
FILE* moveOn(FILE* fp);         //moves to next line of document
void printSymbolTable(void);
bool isAlphaNum(char str[]);    //checks if alphanumerical
int getIndex(char str[20]);
bool isLabel(char str[20]);
int getAddress(char str[]);
int numToBits(int num, int res, int startBit);
//OPCODE HELPER FUNCTIONS
int ADD(FILE* fp);
int AND(FILE* fp);
int BR(FILE* fp);
int JMP(FILE* fp);
int JSR(FILE* fp);
int LDB(FILE* fp);
int LDW(FILE* fp);
int LEA(FILE* fp);
int RTI(FILE* fp);
int SHF(FILE* fp);
int STB(FILE* fp);
int STW(FILE* fp);
int TRAP(FILE* fp);
int XOR(FILE* fp);


int main () {
    int STCtr = 0;                  //SYMBOL TABLE COUNTER
    char test = 0;                  //this lil guy will test for semicolons
    BAdd = 0;                       //address counter starting with add specified by .ORIG

    allCodes[0].iter = 1;
    allCodes[0].array[0] = "ADD";
    allCodes[0].hexop = 0x1000;

    allCodes[1].iter = 1;
    allCodes[1].array[0] = "AND";
    allCodes[1].hexop = 0x5000;

    allCodes[2].iter = 8;
    allCodes[2].array[0] = "BRN";
    allCodes[2].array[1] = "BRZ";
    allCodes[2].array[2] = "BRP";
    allCodes[2].array[3] = "BR";
    allCodes[2].array[4] = "BRZP";
    allCodes[2].array[5] = "BRNP";
    allCodes[2].array[6] = "BRNZ";
    allCodes[2].array[7] = "BRNZP";
    allCodes[2].hexop = 0x0000;

    allCodes[3].iter = 2;
    allCodes[3].array[0] = "JMP";
    allCodes[3].array[1] = "RET";
    allCodes[3].hexop = 0xC000;

    allCodes[4].iter = 2;
    allCodes[4].array[0] = "JSR";
    allCodes[4].array[1] = "JSRR";
    allCodes[4].hexop = 0x4000;

    allCodes[5].iter = 1;
    allCodes[5].array[0] = "LDB";
    allCodes[5].hexop = 0x2000;

    allCodes[6].iter = 1;
    allCodes[6].array[0] = "LDW";
    allCodes[6].hexop = 0x6000;

    allCodes[7].iter = 1;
    allCodes[7].array[0] = "LEA";
    allCodes[7].hexop = 0xE000;

    allCodes[8].iter = 1;
    allCodes[8].array[0] = "RTI";
    allCodes[8].hexop = 0x8000;

    allCodes[9].iter = 3;
    allCodes[9].array[0] = "LSHF";
    allCodes[9].array[1] = "RSHFL";
    allCodes[9].array[2] = "RSHFA";
    allCodes[9].hexop = 0xD000;

    allCodes[10].iter = 1;
    allCodes[10].array[0] = "STB";
    allCodes[10].hexop = 0x3000;

    allCodes[11].iter = 1;
    allCodes[11].array[0] = "STW";
    allCodes[11].hexop = 0x7000;

    allCodes[12].iter = 2;
    allCodes[12].array[0] = "TRAP";
    allCodes[12].array[1] = "HALT"; //added this because it needed to be recognized as TRAP x25
    allCodes[12].hexop = 0xF000;

    allCodes[13].iter = 1;
    allCodes[13].array[0] = "XOR";
    allCodes[13].hexop = 0x9000;


    char word1[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char word2[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    FILE *fptr;
    fptr = fopen("source.asm", "r");
    if (fptr == NULL) {
        printf("ERROR READING FILE");
    }

//***********************************************************************************************************************************
//************************************ FIRST PASS TO BUILD SYMBOL TABLE *************************************************************
//***********************************************************************************************************************************

    while (strcmp(word1, ".END")) {
        fscanf(fptr, "%s", word1);
        test = word1[0];                    //this is to catch semicolons
        if (test == ';') {
            fptr = moveOn(fptr);            //move on to the next line of the document
        } else if (!strcmp(word1, ".ORIG")) {  //SURPRISE! strcmp returns 0 if they're the same!
            fscanf(fptr, "%s", word1);
            test = word1[0];
            if ((test == 'x') || (test == '#')) {
                if (test == 'x') {
                    BAdd = (int) strtol(word1 + 1, NULL,
                                        16); //grab address where the program begins. word+1 b/c you want to avoid x
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                }
            } else {
                printf("ERROR CODE 4: INVALID ADDRESS\n");//if hex is not in correct format
            }
        } else if (!strcmp(word1, ".FILL")) {
            fptr = moveOn(fptr);            //move on to the next line of the document
            BAdd += 2;                      //increment address counter
        } else if (isValid(word1)) {           //if word1 is VALID
            fptr = moveOn(fptr);            //move on to the next line of the document
            BAdd += 2;                  //increment address counter
        } else if ((!isValid(word1)) && (strcmp(word1, ".END"))) {         //if word1 is NOT VALID and NOT .END
            fscanf(fptr, "%s", word2);      //if its not, check if word2 is a valid opcode
            //if word1 and word2 are both invalid opcodes, ERROR, but word2 can be .END or .FILL
            if ((!isValid(word2)) && (strcmp(word2, ".END")) && (strcmp(word2, ".FILL"))) {
                printf("ERROR CODE 2: INVALID OPCODE\n");
                fptr = moveOn(fptr);        //move on to the next line of the document
                BAdd += 2;                  //increment address counter
            }
                //below, we check if the word1 is alphanumeric, doesnt begin with x, and isn't IN, OUT, GETC, or PUTS
            else if ((isAlphaNum(word1)) && (test != 'x') && (strcmp(word1, "IN")) && (strcmp(word1, "OUT")) &&
                     (strcmp(word1, "GETC")) && (strcmp(word1, "PUTS"))) {
                //if word1 is invalid opcode and meets standards for label, but word2 is valid opcode, word1 is label
                symbolTable[STCtr].address = BAdd;
                strncpy(symbolTable[STCtr].label, word1, 20);
                STCtr++;
                fptr = moveOn(fptr);        //move on to the next line of the document
                BAdd += 2;                  //increment address counter
            } else {
                printf("ERROR CODE 4: OTHER ERROR: INVALID LABEL");
            }
        }

    }
    printSymbolTable();
    rewind(fptr);
    for (int i = 0; i < 20; i++) {  //clear word1
        word1[i] = 0;
    }
    for (int i = 0; i < 20; i++) {  //clear word2
        word2[i] = 0;
    }

//***********************************************************************************************************************************
//************************************* SECOND PASS TO ASSEMBLE *********************************************************************
//***********************************************************************************************************************************

    while (strcmp(word1, ".END")) {
        fscanf(fptr, "%s", word1);
        test = word1[0];                    //this is to catch semicolons
        if (test == ';') {
            fptr = moveOn(fptr);            //move on to the next line of the document
        } else if (!strcmp(word1, ".ORIG")) {  //SURPRISE! strcmp returns 0 if they're the same!
            fscanf(fptr, "%s", word1);
            test = word1[0];
            if ((test == 'x') || (test == '#')) {
                if (test == 'x') {
                    BAdd = (int) strtol(word1+1, NULL, 16); //grab address where the program begins. word+1 to avoid x
                    printf("x");
                    printf("%04X\n", BAdd); //want this to print in hex
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                    printf("x");
                    printf("%04X\n", BAdd); //want this to print in hex
                }
            } else {
                printf("ERROR CODE 4: OTHER ERROR: INVALID ADDRESS\n");//if hex is not in correct format
            }
            fptr = moveOn(fptr);            //move on to the next line of the document
        } else if (!strcmp(word1, ".FILL")) {
            fscanf(fptr, "%s", word1);
            test = word1[0];
            if ((test == 'x') || (test == '#')) {
                if (test == 'x') {
                    BAdd = (int) strtol(word1 + 1, NULL,
                                        16); //grab address where the program begins. word+1 b/c you want to avoid x
                    printf("x");
                    printf("%04X\n", BAdd); //want this to print in hex
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                    printf("x");
                    printf("%04X\n", BAdd); //want this to print in hex
                }
            } else {
                printf("ERROR CODE 4: OTHER ERROR: INVALID ADDRESS\n");//if hex is not in correct format
            }
            fptr = moveOn(fptr);            //move on to the next line of the document
        }

        else if(isValid(word1)){                 //if the word is a valid opcode
            int index = getIndex(word1);
            if(index==0){}
            if(index==1){}
            if(index==2){}
            if(index==3){}
            if(index==4){}
            if(index==5){}
            if(index==6){}
            if(index==7){ LEA(fptr);}
            if(index==8){}
            if(index==9){}
            if(index==10){}
            if(index==11){}
            if(index==12){}
            if(index==13){}
        }
        /*else if(!isLabel(word1)){
            printf("ERROR CODE 2: INVALID OPCODE\n");
        }*/

    }

    fclose(fptr); //this goes at the end of program
}
//***********************************************************************************************************************************
//************************************************ HELPER FUNCTIONS *****************************************************************
//***********************************************************************************************************************************

int numToBits(int num, int res, int startBit){
    if(num==0){return res;}
    if(num>pow(2,startBit)){
        res=res+pow(2,startBit);
        return numToBits(num-pow(2,startBit),res, startBit-1);
    }
}

bool isValid(char str[]){
    bool res = false;
    for(int i = 0;i<14 ; i++){                            //because 14 unique opcodes
        for(int j = 0;j<allCodes[i].iter ; j++){          //this will test for each version of opcode
            if(!strcmp(str,allCodes[i].array[j])){
                res = true;
                break;
            }
        }
        if(res==true){break;}
    }
    return res;
}

int getIndex(char str[20]){
    int index = 0;
    bool flag = false;
    for(int i = 0;i<14 ; i++){                            //because 14 unique opcodes
        for(int j = 0;j<allCodes[i].iter ; j++){          //this will test for each version of opcode
            if(!strcmp(str,allCodes[i].array[j])){
                index=i;
                flag = true;
                break;
            }
        }
        if(flag==true){break;}
    }
    return index;                                       //return which opcode it is
}

bool isLabel(char str[20]){                             //this will see if an invalid opcode is a label
    bool res = false;
    for(int i = 0; symbolTable[i].address != NULL; i++){
        if(!strcmp(str,symbolTable[i].label)){
            res=true;
            break;
        }
    }
    return res;
}

int getAddress(char str[]){
    int addr = 0;
    for(int i = 0; symbolTable[i].address != NULL; i++){
        if(!strcmp(str,symbolTable[i].label)){
            addr=symbolTable[i].address;
            break;
        }
    }
    return addr;
}

FILE* moveOn(FILE* fp){
    FILE *f = fp;
    char moveIt;
    fscanf(f, "%c", &moveIt);
    while(moveIt != '\n'){
        fscanf(f, "%c", &moveIt);
    }
    return f;
}

void printSymbolTable(void){
    for(int i = 0; symbolTable[i].address!=NULL;i++) {  //to reach entire symbol table
        printf("%s ", symbolTable[i].label);
        printf("x");
        printf("%04X\n",symbolTable[i].address); //want this to print in hex
    }
}

bool isAlphaNum(char str[]){
    bool res = true;
    for(int i=0; str[i] != 0;i++){
        if(!(isalnum(str[i]))){
            res=false;
            break;
        }
    }
    return res;
}

int LEA(FILE* fp){
    char bin[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char word[5] = {0, 0, 0, 0, 0};
    int num = 0;                                        //this is the decimal rep of binary
    int offset = 0;                                     //this is to calculate offset of LEA
    char t = 0;
    fscanf(fp, "%s", word);
    if((word[0] != 'R')&&(!(word[1]<8))&&(!(word[1]<=0))&&(word[2] != 0)){ //needs to be a valid register
        printf("ERROR CODE 4: OTHER ERROR: UNEXPECTED OPERAND");
    }
    else{
        num=num+numToBits(word[1],0,11);
    }
    fscanf(fp, "%s", word);
    t = word[0];
    if(isLabel(word)){
        offset = getAddress(word);
        offset = BAdd-num;
    }



    BAdd+=2;
}