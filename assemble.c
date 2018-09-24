/*
    Name 1: Julia Rebello
    Name 2: Clara Johnson
    UTEID 1: jlr3755
    UTEID 2: caj2773
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
typedef struct {            //this is where we will keep the symbol table
    int address;
    char label[MAX_LABEL_LEN + 1];
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

typedef struct {            //this is where we keep the iterations and opcode of each instruction
    int iter;               //number of iterations
    char* array[8];         //each iteration
    int hexop;              //opcode in hex form
} OpCodes;
OpCodes allCodes[14];

int BAdd;                   //address counter starting with addr specified by .ORIG (oops I needed a Global Variable)
FILE * pOutfile;

//HELPER FUNCTIONS
int getAddress(char str[]);     //gets address of a label
int getIndex(char str[]);       //returns position in allCodes array
bool isAlphaNum(char str[]);    //checks if alphanumerical
bool isLabel(char str[]);       //checks if its a label
bool isValid(char str[]);
FILE* moveOn(FILE* fp);         //moves to next line of document
int numToBits(int num, int res, int startBit, int endBit);      //amazing
int powpow(int a, int b);       //calculates exponential
void printSymbolTable(void);    //prints symbol table// TODO: we should probably remove this one before we turn it in since it won't be necessary anymore

//OPCODE FUNCTIONS
void ADD(FILE* fp);
void AND(FILE* fp);
void BR(FILE* fp, char str[]);
void JMP(FILE* fp, char str[]);
void JSR(FILE* fp, char str[]);
void LDB(FILE* fp);
void LDW(FILE* fp);
void LEA(FILE* fp);
void RTI(FILE* fp);
void SHF(FILE* fp, char str[]);
void STB(FILE* fp);
void STW(FILE* fp);
void TRAP(FILE* fp, char str[]);
void XOR(FILE* fp, char str[]);


int main (int argc, char* argv[]) {

    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    int STCtr = 0;                  //SYMBOL TABLE COUNTER
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
    allCodes[12].array[1] = "HALT";
    allCodes[12].hexop = 0xF000;

    allCodes[13].iter = 2;
    allCodes[13].array[0] = "XOR";
    allCodes[13].array[1] = "NOT";
    allCodes[13].hexop = 0x9000;

    char word1[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char word2[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    FILE *fptr;
    //fptr = fopen("source.asm", "r");
    fptr = fopen(iFileName, "r");
    if (fptr == NULL) {
        printf("ERROR READING FILE 1\n");
        exit(4);
    }
    pOutfile = fopen(oFileName, "w" );
    if (pOutfile == NULL) {
        printf("ERROR READING FILE 2\n");
        exit(4);
    }

//***********************************************************************************************************************************
//************************************ FIRST PASS TO BUILD SYMBOL TABLE *************************************************************
//***********************************************************************************************************************************

    while (strcmp(word1, ".END")) {
        fscanf(fptr, "%s", word1);
        for(int i = 0; i < strlen(word1); i++){
            word1[i] = toupper(word1[i]);
        }
        if ((word1[0] == ';')||(!strcmp(word1,"NOP"))) {
            fptr = moveOn(fptr);                                //move on to the next line of the document
        } else if (!strcmp(word1, ".ORIG")) {                   //SURPRISE! strcmp returns 0 if they're the same!
            fscanf(fptr, "%s", word1);
            for(int i = 0; i < strlen(word1); i++){
                word1[i] = toupper(word1[i]);
            }
            if ((word1[0] == 'X') || (word1[0] == '#')) {
                if (word1[0] == 'X') {
                    BAdd = (int) strtol(word1 + 1, NULL, 16);       //grab address of prog word+1 b/c you want to avoid x
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                }
            } else {
                //printf("ERROR CODE 4: OTHER ERROR\n");              //if hex is not in correct format
                exit(4);
            }
            if((BAdd%2)==1){
                //printf("ERROR CODE 3: INVALID CONSTANT\n");          //prog cant start at odd address
                exit(3);
            }
            if((BAdd > 0xFFFF) || (BAdd < 0)){
                exit(3);
            }
        } else if (!strcmp(word1, ".FILL")) {
            fptr = moveOn(fptr);                                //move on to the next line of the document
            BAdd += 2;                                          //increment address counter
        } else if (isValid(word1)) {                            //if word1 is VALID
            if((strcmp(word1,"RET")) && (strcmp(word1,"RTI")) && (strcmp(word1,"HALT"))){
                fscanf(fptr, "%s", word1);
                for(int i = 0; i < strlen(word1); i++){
                    word1[i] = toupper(word1[i]);
                }
                if(isValid(word1)){
                    //printf("ERROR CODE 4: OTHER ERROR\n");
                    //fptr = moveOn(fptr);//move on to the next line of the document
                    //BAdd += 2;
                    exit(4);
                }
                else {
                    fptr = moveOn(fptr);//move on to the next line of the document
                    BAdd += 2;
                }
            }
            else {
                fptr = moveOn(fptr);                                //move on to the next line of the document
                BAdd += 2;                                          //increment address counter
            }
        } else if ((!isValid(word1)) && (strcmp(word1, ".END"))) {         //if word1 is NOT VALID and NOT .END
            fscanf(fptr, "%s", word2);
            for(int i = 0; i < strlen(word2); i++){
                word2[i] = toupper(word2[i]);
            }
            //if its not, check if word2 is a valid opcode
            //if word1 and word2 are both invalid opcodes, ERROR, but word2 can be .FILL
            if ((!isValid(word2)) && (strcmp(word2, ".FILL"))) {
                //printf("ERROR CODE 2: INVALID OPCODE\n");
                //fptr = moveOn(fptr);                            //move on to the next line of the document
                //BAdd += 2;                                      //increment address counter
                exit(2);
            }
                //below, we check if the word1 is alphanumeric, doesnt begin with x, and isn't IN, OUT, GETC, or PUTS
            else if ((isAlphaNum(word1)) && ((word1[0] < 48) || (word1[0] > 57)) && (word1[0] != 'X') && (strcmp(word1, "IN")) && (strcmp(word1, "OUT")) && (strcmp(word1, "GETC")) && (strcmp(word1, "PUTS")) && (!isValid(word1)) && (!(isLabel(word1)))) {
                //if word1 is invalid opcode and meets standards for label, but word2 is valid opcode, word1 is label
                symbolTable[STCtr].address = BAdd;
                strncpy(symbolTable[STCtr].label, word1, 21);
                STCtr++;
                fptr = moveOn(fptr);        //move on to the next line of the document
                BAdd += 2;                  //increment address counter
            } else {
                //printf("ERROR CODE 4: OTHER ERROR\n");
                exit(4);
            }
        }
    }
    //TODO: get rid of this next line when we're ready
    printSymbolTable();

    rewind(fptr);
    for (int i = 0; i < 21; i++) {  //clear word1
        word1[i] = 0;
    }
    for (int i = 0; i < 21; i++) {  //clear word2
        word2[i] = 0;
    }

    //TODO: get rid of this next line when we're ready
    printf("\nSTARTING SECOND PASS\n\n");

//***********************************************************************************************************************************
//************************************* SECOND PASS TO ASSEMBLE *********************************************************************
//***********************************************************************************************************************************

    while (strcmp(word1, ".END")) {
        fscanf(fptr, "%s", word1);
        for(int i = 0; i < strlen(word1); i++){
            word1[i] = toupper(word1[i]);
        }
        if (word1[0]== ';') {
            fptr = moveOn(fptr);                //move on to the next line of the document
        } else if (!strcmp(word1, ".ORIG")) {   //SURPRISE! strcmp returns 0 if they're the same!
            fscanf(fptr, "%s", word1);
            for(int i = 0; i < strlen(word1); i++){
                word1[i] = toupper(word1[i]);
            }
            if ((word1[0]== 'X') || (word1[0]== '#')) {
                if (word1[0] == 'X') {
                    BAdd = (int) strtol(word1+1, NULL, 16); //grab address where the program begins. word+1 to avoid x
                    if((BAdd%2)==1){
                        //printf("ERROR CODE 3: INVALID CONSTANT\n");          //program cant start at odd address
                        exit(3);
                    }
                    else {
                        printf("0x");
                        printf("%04X\n", BAdd);                             //want this to print in hex
                        fprintf(pOutfile, "0x");
                        fprintf(pOutfile, "%04X\n", BAdd);
                    }
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                    if((BAdd%2)==1){
                        //printf("ERROR CODE 3: INVALID CONSTANT\n");          //prog cant start at odd address
                        exit(3);
                    }
                    else {
                        printf("0x");
                        printf("%04X\n", BAdd);                             //want this to print in hex
                        fprintf(pOutfile, "0x");
                        fprintf(pOutfile, "%04X\n", BAdd);
                    }
                }
            } else {
                //printf("ERROR CODE 4: OTHER ERROR\n");//if hex is not in correct format
                exit(4);
            }
            fptr = moveOn(fptr);                                //move on to the next line of the document
        } else if (!strcmp(word1, ".FILL")) {
            fscanf(fptr, "%s", word1);
            for(int i = 0; i < strlen(word1); i++){
                word1[i] = toupper(word1[i]);
            }
            if ((word1[0] == 'X') || (word1[0]== '#')) {
                if (word1[0] == 'X') {
                    BAdd = (int) strtol(word1 + 1, NULL, 16);   //grab the constant that .FILL places at that address
                    printf("0x");
                    printf("%04X\n", BAdd);                     //want this to print in hex
                    fprintf(pOutfile, "0x");
                    fprintf(pOutfile, "%04X\n", BAdd);
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                    printf("0x");
                    printf("%04X\n", BAdd);                     //want this to print in hex
                    fprintf(pOutfile, "0x");
                    fprintf(pOutfile, "%04X\n", BAdd);
                }
            } else {
                //printf("ERROR CODE 4: OTHER ERROR\n");
                exit(4);
            }
            fptr = moveOn(fptr);                    //move on to the next line of the document
        } else if(isValid(word1)){                  //if the word is a valid opcode
            int index = getIndex(word1);
            if(index == 0){
                ADD(fptr);
            }
            else if(index == 1){
                AND(fptr);
            }
            else if(index == 2){
                BR(fptr, word1);
            }
            else if(index == 3){
                JMP(fptr, word1);
            }
            else if(index == 4){
                JSR(fptr, word1);
            }
            else if(index == 5){
                LDB(fptr);
            }
            else if(index == 6){
                LDW(fptr);
            }
            else if(index == 7){
                LEA(fptr);
            }
            else if(index == 8){
                RTI(fptr);
            }
            else if(index == 9){
                SHF(fptr, word1);
            }
            else if(index == 10){
                STB(fptr);
            }
            else if(index == 11){
                STW(fptr);
            }
            else if(index == 12){
                TRAP(fptr, word1);
            }
            else if(index == 13){
                XOR(fptr, word1);
            }
            moveOn(fptr);
        }
        else if((!isLabel(word1))&&(strcmp(word1,".END"))){
            //printf("ERROR CODE 2: INVALID OPCODE\n");
           // moveOn(fptr);
            //BAdd+=2;
            exit(2);
        }
    }
    fclose(fptr); //this goes at the end of program
    exit(0);
    //TODO: I think we might have to add exit(0) here (at the end of the program) since it says we have to return an error code of 0 if everything assembled correctly
}
//***********************************************************************************************************************************
//************************************************ HELPER FUNCTIONS *****************************************************************
//***********************************************************************************************************************************

int getAddress(char str[]){
    int addr = 0;
    for(int i = 0; symbolTable[i].address != 0; i++){
        if(!strcmp(str, symbolTable[i].label)){
            addr = symbolTable[i].address;
            break;
        }
    }
    return addr;
}

int getIndex(char str[]){
    int index = 0;
    bool flag = false;
    for(int i = 0; i < 14; i++){                            //because 14 unique opcodes
        for(int j = 0; j < allCodes[i].iter; j++){          //this will test for each version of opcode
            if(!strcmp(str, allCodes[i].array[j])){
                index = i;
                flag = true;
                break;
            }
        }
        if(flag == true){break;}
    }
    return index;                                       //return which opcode it is
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

bool isLabel(char str[]){                             //this will see if an invalid opcode is a label
    bool res = false;
    for(int i = 0; symbolTable[i].address != 0; i++){
        if(!strcmp(str,symbolTable[i].label)){
            res=true;
            break;
        }
    }
    return res;
}

bool isValid(char str[]){
    bool res = false;
    for(int i = 0; i < 14; i++){                            //because 14 unique opcodes
        for(int j = 0; j < allCodes[i].iter; j++){          //this will test for each version of opcode
            if(!strcmp(str, allCodes[i].array[j])){
                res = true;
                break;
            }
        }
        if(res == true){break;}
    }
    return res;
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

int numToBits(int num, int res, int startBit, int endBit){
    if((num < 0) && (endBit == 0)){           //if negative number
        res = num + powpow(2, startBit + 1);
        return res;
    }
    int index = startBit - endBit;
    int p = powpow(2, startBit);
    int comp = powpow(2, index);
    if(num == 0){return res;}
    else if(num >= comp){
        res += p;
        return numToBits(num - comp, res, startBit - 1, endBit);
    }else{
        return numToBits(num, res, startBit - 1, endBit);
    }
}

int powpow(int a, int b){
    if(b == 0){return 1;}
    int res = a;
    for(int i = 0; i < b - 1; i++){
        res = res * a;
    }
    return res;
}

//TODO: we'll want to at least comment this out once we start putting everything in the .obj file instead*******************
void printSymbolTable(void){
    for(int i = 0; symbolTable[i].address != 0 ; i++) {  //to reach entire symbol table
        printf("%s ", symbolTable[i].label);
        printf("x");
        printf("%04X\n",symbolTable[i].address); //want this to print in hex
    }
}

//***********************************************************************************************************************************
//************************************************ OPCODE FUNCTIONS *****************************************************************
//***********************************************************************************************************************************
void ADD(FILE* fp){
    char word[12] = {0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0};           //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of SR1
    int num3 = 0;                                       //this is the decimal rep of binary of SR2 or imm5
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************


    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);          //set bits 8-6 with SR1
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] == 'X') || (word[0] == '#')){                       //if immediate operand
        if(word[0] == 'X') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3= (int)strtol(word+1, NULL, 10);
        }
        if((num3 > 15)||(num3 < -16)){                             //if immediate is out of range (-16,15)
            //printf("ERROR CODE 3: INVALID CONSTANT\n");
            exit(3);
        }
        else {
            if (num3 < 0) {num3 += 32;}        //if negative immediate
            num3 += 32;                          //this is bit 5
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[0].hexop); //want this to print in hex//bit 5 is 1
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[0].hexop);
        }
    }
    else if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else if (word[0] == 'R'){                                       //if second source operand
        num3 = num3 + numToBits((word[1] - '0'), 0, 2, 0);          //set bits 2-0 with SR2
        printf("0x");
        printf("%04X\n", num1 + num2 + num3 + allCodes[0].hexop);   //want this to print in hex
        fprintf(pOutfile, "0x");
        fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[0].hexop);
    }
    BAdd += 2;
}

void AND(FILE* fp){
    char word[12] = {0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0};           //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of SR1
    int num3 = 0;                                       //this is the decimal rep of binary of SR2 or imm5
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************


    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);          //set bits 8-6 with SR1
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] == 'X') || (word[0] == '#')){                       //if immediate operand
        if(word[0] == 'X') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3= (int)strtol(word+1, NULL, 10);
        }
        if((num3 > 15)||(num3 < -16)){                             //if immediate is out of range (-16,15)
            //printf("ERROR CODE 3: INVALID CONSTANT\n");
            exit(3);
        }
        else {
            if (num3 < 0) {num3 += 32;}        //if negative immediate
            num3 += 32;                          //this is bit 5
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[1].hexop); //want this to print in hex//bit 5 is 1
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[1].hexop);
        }
    }
    else if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else if (word[0] == 'R'){                                       //if second source operand
        num3 = num3 + numToBits((word[1] - '0'), 0, 2, 0);          //set bits 2-0 with SR2
        printf("0x");
        printf("%04X\n", num1 + num2 + num3 + allCodes[1].hexop);   //want this to print in hex
        fprintf(pOutfile, "0x");
        fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[1].hexop);
    }
    BAdd += 2;
}


void BR(FILE* fp, char word1[]){
    char word[5] = {0, 0, 0, 0, 0};                             //this is to read file into
    int num1 = 0;                                               //for iteration of br
    int num2 = 0;                                               //for offset
    int offset = 0;                                             //offset
    if(!strcmp(word1, "BRN")){num1 += 2048;}
    else if(!strcmp(word1, "BRZ")){num1 += 1024;}
    else if(!strcmp(word1, "BRP")){num1 += 512;}
    else if(!strcmp(word1, "BR")){num1 += 3584;}
    else if(!strcmp(word1, "BRZP")){num1 += 1536;}
    else if(!strcmp(word1, "BRNP")){num1 += 2560;}
    else if(!strcmp(word1, "BRNZ")){num1 += 3072;}
    else if(!strcmp(word1, "BRNZP")){num1 += 3584;}

    fscanf(fp, "%s", word);                                     //get the label
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if(isLabel(word)){                                          //if label
        offset = getAddress(word);                              //get address
        offset = (((offset - 2) - BAdd) / 2);                   //get offset from incremented PC
        if((offset < -256) || (offset > 255)){
            //printf("ERROR CODE 4: OTHER ERROR\n");
            exit(4);
        }
        else {
            num2 = num2 + numToBits(offset, 0, 8, 0);//set bits 8-0 with DR
            printf("0x");
            printf("%04X\n", num1 + num2 + allCodes[2].hexop);          //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + allCodes[2].hexop);
        }
    }
    else {
        //printf("ERROR CODE 1: UNDEFINED LABEL\n");
        exit(1);
    }
    BAdd += 2;
}

void JMP(FILE* fp, char word1[]){
    int num = 0;
    char word[5] = {0, 0, 0, 0, 0};             //this is to read file into
    if(!strcmp(word1, "JMP")){
        fscanf(fp, "%s", word);                     //get the next word
        for(int i = 0; i < strlen(word); i++){
            word[i] = toupper(word[i]);
        }
        if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
            //printf("ERROR CODE 4: OTHER ERROR\n");
            exit(4);
        }
        else{
            num = num + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with BR
            printf("0x");
            printf("%04X\n", num + allCodes[3].hexop);              //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num + allCodes[3].hexop);
        }
    }
    else if(!strcmp(word1, "RET")){
        printf("0xC1C0\n");
        fprintf(pOutfile, "0xC1C0\n");
    }
    BAdd += 2;
}

void JSR(FILE* fp, char word1[]){
    int num = 0;
    char word[5] = {0, 0, 0, 0, 0};                             //this is to read file into
    int offset = 0;
    fscanf(fp, "%s", word);                                     //get the next word
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if(!strcmp(word1, "JSR")){                                  //JSR
        if(isLabel(word)){                                      //if label
            offset = getAddress(word);                          //get address
            offset = (((offset - 2) - BAdd) / 2);               //get offset from incremented PC
            if((offset < -1024) || (offset > 1023)){
                //printf("ERROR CODE 4: OTHER ERROR\n");
                exit(4);
            }
            else {
                num = num + numToBits(offset, 0, 10, 0);            //set bits 10-0 with DR
                printf("0x");
                printf("%04X\n", num + 2048 + allCodes[4].hexop);   //want this to print in hex//adding 2048 because bit 11 is set
                fprintf(pOutfile, "0x");
                fprintf(pOutfile, "%04X\n", num + 2048 + allCodes[4].hexop);
            }
        }
        else{
            //printf("ERROR CODE 1: UNDEFINED LABEL\n");
            exit(1);
        }
    }

    else if(!strcmp(word1, "JSRR")){                      //JSRR
        if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
            //printf("ERROR CODE 4: OTHER ERROR\n");
            exit(4);
        }
        else{
            num = num + numToBits((word[1] - '0'), 0, 8, 6);
            printf("0x");
            printf("%04X\n", num + allCodes[4].hexop); //want this to print in hex //adding 2048 because bit 11 is set
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num + allCodes[4].hexop);
        }
    }
    BAdd += 2;
}

void LDB(FILE* fp){
    char word[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};       //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);             //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);              //set bits 8-6 with BR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] == 'X') || (word[0] == '#')){                           //if immediate operand
        if(word[0] == 'X') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3 = (int)strtol(word + 1, NULL, 10);
        }
        if((num3 > 31)||(num3 < -32)){                              //if immediate is out of range (-32,31)
            //printf("ERROR CODE 3: INVALID CONSTANT\n");
            exit(3);
        }
        else {
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[5].hexop); //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[5].hexop);
        }
    }
    else{
        exit(4);
    }
    BAdd += 2;
}

void LDW(FILE* fp){
    char word[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};       //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);             //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);              //set bits 8-6 with BR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] == 'X') || (word[0] == '#')){                           //if immediate operand
        if(word[0] == 'X') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3 = (int)strtol(word + 1, NULL, 10);
        }
        if((num3 > 31)||(num3 < -32)){                              //if immediate is out of range (-32,31)
            //printf("ERROR CODE 3: INVALID CONSTANT\n");
            exit(3);
        }
        else {
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[6].hexop); //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[6].hexop);
        }
    }
    else{
        exit(4);
    }
    BAdd += 2;
}

void LEA(FILE* fp){      //we need an error here for when the offset is too far away************************************************
    char word[26] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of offset
    int offset = 0;                                     //this is to calculate offset of LEA
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 23; i++){
            word[i]=word[i+3];
        }
        word[23]=0;
        word[24]=0;
        word[25]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if(isLabel(word)){                                              //if label
        offset = getAddress(word);                                  //get address
        offset = (((offset - 2) - BAdd) / 2);                       //get offset from incremented PC
        if((offset < -256) || (offset > 255)){
            //printf("ERROR CODE 4: OTHER ERROR\n");
            exit(4);
        }
        else {
            num2 = num2 + numToBits(offset, 0, 8, 0);               //set bits 8-0 with DR
            printf("0x");
            printf("%04X\n", num1 + num2 + allCodes[7].hexop);      //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + allCodes[7].hexop);
        }
    }
    else{
        //printf("ERROR CODE 1: UNDEFINED LABEL\n");
        if((word[0] == 'X') || (word[0] == '#')){
            exit(4);
        }
        else{
            exit(1);
        }
    }
    BAdd += 2;
}

void RTI(FILE* fp){
    printf("0x8000\n");
    fprintf(pOutfile, "ox8000\n");
    BAdd += 2;
}

void SHF(FILE* fp, char word1[]){
    char word[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};             //this is to read file into
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    if(!strcmp(word1, "RSHF")){num1 = 16;}
    if(!strcmp(word1, "RSHFA")){num1 = 48;}

    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 SR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if(word[0] != '#') {
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    } else{
        num3 = (int)strtol(word + 1, NULL, 10);
        if((num3 > 15) || (num3 < 0)){                      //this can't be negative
            //printf("ERROR CODE 3: INVALID CONSTANT\n");
            exit(3);
        }
        else {
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[9].hexop); //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[9].hexop);
        }
    }
    BAdd += 2;
}


void STB(FILE* fp){
    char word[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);             //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);              //set bits 8-6 with BR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] == 'X') || (word[0] == '#')){                           //if immediate operand
        if(word[0] == 'X') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3 = (int)strtol(word + 1, NULL, 10);
        }
        if((num3 > 31)||(num3 < -32)){                              //if immediate is out of range (-32,31)
            //printf("ERROR CODE 3: INVALID CONSTANT\n");
            exit(3);
        }
        else {
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[10].hexop);   //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[10].hexop);
        }
    }
    else{
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    BAdd += 2;
}

void STW(FILE* fp){
    char word[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);             //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);              //set bits 8-6 with BR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] == 'X') || (word[0] == '#')){                           //if immediate operand
        if(word[0] == 'X') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3 = (int)strtol(word + 1, NULL, 10);
        }
        if((num3 > 31)||(num3 < -32)){                              //if immediate is out of range (-32,31)
            //printf("ERROR CODE 3: INVALID CONSTANT\n");
            exit(3);
        }
        else {
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[11].hexop);   //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[11].hexop);
        }
    }
    else{
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    BAdd += 2;
}

void TRAP(FILE* fp, char word1[]){
    char word[5] = {0, 0, 0, 0, 0};                 //this is to read file into
    if(!strcmp(word1, "TRAP")){
        fscanf(fp, "%s", word);                     //get the next word
        for(int i = 0; i < strlen(word); i++){
            word[i] = toupper(word[i]);
        }
        if(!strcmp(word, "X25")){
            printf("0xF025\n");
            fprintf(pOutfile, "0xF025\n");
        }
        else{
            //printf("ERROR CODE 4: OTHER ERROR\n");
            exit(4);
        }
    }
    else if(!strcmp(word1, "HALT")){
        printf("0xF025\n");
        fprintf(pOutfile, "0xF025\n");
    }
    BAdd += 2;
}

void XOR(FILE* fp, char word1[]){
    char word[12] = {0, 0, 0, 0, 0, 0, 0 ,0 ,0, 0, 0, 0};           //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of SR1
    int num3 = 0;                                       //this is the decimal rep of binary of SR2 or imm5
    fscanf(fp, "%s", word);
    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);                  //set bits 11-9 with DR
    }

    if((word[2]==',')&&(word[3]!=0)){//**********************************
        for(int i = 0; i < 9; i++){
            word[i]=word[i+3];
        }
        word[9]=0;
        word[10]=0;
        word[11]=0;
    }

    else {
        fscanf(fp, "%s", word);
    } //******************************************************************

    for(int i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
    if((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
        //printf("ERROR CODE 4: OTHER ERROR\n");
        exit(4);
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);               //set bits 8-6 with DR
    }

    if(!strcmp(word1, "XOR")) {

        if((word[2]==',')&&(word[3]!=0)){//**********************************
            for(int i = 0; i < 9; i++){
                word[i]=word[i+3];
            }
            word[9]=0;
            word[10]=0;
            word[11]=0;
        }

        else {
            fscanf(fp, "%s", word);
        } //******************************************************************

        for(int i = 0; i < strlen(word); i++){
            word[i] = toupper(word[i]);
        }
        if ((word[0] == 'X') || (word[0] == '#')) {                          //if immediate operand
            if (word[0] == 'X') {
                num3 = (int) strtol(word + 1, NULL, 16);
            } else {
                num3 = (int) strtol(word + 1, NULL, 10);
            }
            if((num3 > 15)||(num3 < -16)){                                   //if immediate is out of range (-16,15)
                //printf("ERROR CODE 3: INVALID CONSTANT\n");
                exit(3);
            } else {
                if (num3 < 0) {num3 += 32;}        //if negative immediate
                num3 += 32;                          //this is bit 5
                printf("0x");
                printf("%04X\n", num1 + num2 + num3 + allCodes[13].hexop);    //want this to print in hex//bit 5 is 1
                fprintf(pOutfile, "0x");
                fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[13].hexop);
            }
        } else if ((word[0] != 'R') || (!((word[1]-'0') >= 0)) || (!((word[1]-'0') < 8)) || (!((word[2] == ' ')||(word[2] == 0)||(word[2] == ',')||(word[2] == ';')))){ //needs to be a valid register
            //printf("ERROR CODE 4: OTHER ERROR\n");
            exit(4);
        } else if (word[0] == 'R') {                                        //if second source operand
            num3 = num3 + numToBits((word[1] - '0'), 0, 2, 0);              //set bits 2-0 with SR2
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[13].hexop);      //want this to print in hex
            fprintf(pOutfile, "0x");
            fprintf(pOutfile, "%04X\n", num1 + num2 + num3 + allCodes[13].hexop);
        }
    }
    else{                                                    //NOT
        printf("0x");
        printf("%04X\n", num1 + num2 + 63 + allCodes[13].hexop);            //want this to print in hex
        fprintf(pOutfile, "0x");
        fprintf(pOutfile, "%04X\n", num1 + num2 + 63 + allCodes[13].hexop);
    }
    BAdd += 2;
}