//TODO: put the right header comments
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

//HELPER FUNCTIONS
int getAddress(char str[]);     //gets address of a label
int getIndex(char str[20]);     //returns position in allCodes array
bool isAlphaNum(char str[]);    //checks if alphanumerical
bool isLabel(char str[20]);     //checks if its a label
bool isValid(char str[20]);
FILE* moveOn(FILE* fp);         //moves to next line of document
int numToBits(int num, int res, int startBit, int endBit);      //amazing
int powpow(int a, int b);       //calculates exponential
void printSymbolTable(void);    //prints symbol table //TODO: we should probably remove this one before we turn it in since it won't be necessary anymore

//OPCODE FUNCTIONS
void ADD(FILE* fp);
void AND(FILE* fp);
void BR(FILE* fp, char str[20]);
void JMP(FILE* fp, char str[20]);
void JSR(FILE* fp, char str[20]);
void LDB(FILE* fp);
void LDW(FILE* fp);
void LEA(FILE* fp);
void RTI(FILE* fp);
void SHF(FILE* fp, char str[20]);
void STB(FILE* fp);
void STW(FILE* fp);
void TRAP(FILE* fp, char str[20]);
void XOR(FILE* fp, char str[20]);

//TODO: look at the "helpful functions" document on Patt's website when we change all the stuff to do the command line things and everything else
int main () {
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


    char word1[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char word2[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    FILE *fptr;
    //TODO: read the name of the file from the command line arguments
    fptr = fopen("errorTests.txt", "r");
    if (fptr == NULL) {
        printf("ERROR READING FILE\n");
    }

//***********************************************************************************************************************************
//************************************ FIRST PASS TO BUILD SYMBOL TABLE *************************************************************
//***********************************************************************************************************************************

    while (strcmp(word1, ".END")) {
        fscanf(fptr, "%s", word1);
        if ((word1[0] == ';')||(!strcmp(word1,"NOP"))) {
            fptr = moveOn(fptr);                                //move on to the next line of the document
        } else if (!strcmp(word1, ".ORIG")) {                   //SURPRISE! strcmp returns 0 if they're the same!
            fscanf(fptr, "%s", word1);
            if ((word1[0] == 'x') || (word1[0] == '#')) {
                if (word1[0] == 'x') {
                    BAdd = (int) strtol(word1 + 1, NULL, 16);   //grab address of prog word+1 b/c you want to avoid x
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                }
            } else {
                //TODO: exit instead - exit(4)
                printf("ERROR CODE 4: OTHER ERROR\n");          //if hex is not in correct format
            }
            if((BAdd%2)==1){
                //TODO: exit instead - exit(3)
                printf("ERROR CODE 3: INVALID CONSTANT\n");          //prog cant start at odd address
            }
        } else if (!strcmp(word1, ".FILL")) {
            fptr = moveOn(fptr);                                //move on to the next line of the document
            BAdd += 2;                                          //increment address counter
        } else if (isValid(word1)) {                            //if word1 is VALID
            fptr = moveOn(fptr);                                //move on to the next line of the document
            BAdd += 2;                                          //increment address counter
        } else if ((!isValid(word1)) && (strcmp(word1, ".END"))) {         //if word1 is NOT VALID and NOT .END
            fscanf(fptr, "%s", word2);                          //if its not, check if word2 is a valid opcode
            //if word1 and word2 are both invalid opcodes, ERROR, but word2 can be .END or .FILL
            if ((!isValid(word2)) && (strcmp(word2, ".END")) && (strcmp(word2, ".FILL"))) {
                //TODO: exit instead - exit(2)
                printf("ERROR CODE 2: INVALID OPCODE\n");
                //TODO: we won't need these two lines anymore after we start exiting right?
                fptr = moveOn(fptr);                            //move on to the next line of the document
                BAdd += 2;                                      //increment address counter
            }
                //below, we check if the word1 is alphanumeric, doesnt begin with x, and isn't IN, OUT, GETC, or PUTS
            else if ((isAlphaNum(word1)) && (word1[0] != 'x') && (word1[0] != 'X') && (strcmp(word1, "IN")) && (strcmp(word1, "OUT")) &&
                     (strcmp(word1, "GETC")) && (strcmp(word1, "PUTS")) && (!isValid(word1))) {
                //TODO: check if the label only starts with an alphabet character
                //if word1 is invalid opcode and meets standards for label, but word2 is valid opcode, word1 is label
                symbolTable[STCtr].address = BAdd;
                //TODO: word1 might actually need to be 21 long since the longest a label can be is 20 and then you need another place for the null terminator
                strncpy(symbolTable[STCtr].label, word1, 20);
                STCtr++;
                fptr = moveOn(fptr);        //move on to the next line of the document
                BAdd += 2;                  //increment address counter
            } else {
                //TODO: exit instead - exit(4)
                printf("ERROR CODE 4: OTHER ERROR\n");
            }
        }

    }
    //TODO: get rid of this next line when we're ready
    printSymbolTable();

    rewind(fptr);
    for (int i = 0; i < 20; i++) {  //clear word1
        word1[i] = 0;
    }
    for (int i = 0; i < 20; i++) {  //clear word2
        word2[i] = 0;
    }

    //TODO: get rid of this next line when we're ready
    printf("\nSTARTING SECOND PASS\n\n");

//***********************************************************************************************************************************
//************************************* SECOND PASS TO ASSEMBLE *********************************************************************
//***********************************************************************************************************************************

    while (strcmp(word1, ".END")) {
        fscanf(fptr, "%s", word1);
        if (word1[0]== ';') {
            fptr = moveOn(fptr);            //move on to the next line of the document
        } else if (!strcmp(word1, ".ORIG")) {  //SURPRISE! strcmp returns 0 if they're the same!
            fscanf(fptr, "%s", word1);
            word1[0]= word1[0]; //TODO: does this line actually do anything?
            if ((word1[0]== 'x') || (word1[0]== '#')) {
                if (word1[0] == 'x') {
                    BAdd = (int) strtol(word1+1, NULL, 16); //grab address where the program begins. word+1 to avoid x
                    if((BAdd%2)==1){
                        //TODO: exit instead - exit(3)
                        //TODO: also I think we already caught this error when building the symbol table, so we might not need this if
                        printf("ERROR CODE 3: INVALID CONSTANT\n");          //prog cant start at odd address
                    }
                    else {
                        //TODO: change this so it goes into the .obj file from the command line instead
                        printf("0x");
                        printf("%04X\n", BAdd); //want this to print in hex
                    }
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                    if((BAdd%2)==1){
                        //TODO: exit instead - exit(3)
                        printf("ERROR CODE 3: INVALID CONSTANT\n");          //prog cant start at odd address
                    }
                    else {
                        //TODO: change this so it goes into the .obj file from the command line instead
                        printf("0x");
                        printf("%04X\n", BAdd); //want this to print in hex
                    }
                }
            } else {
                //TODO: exit instead - exit(4)
                printf("ERROR CODE 4: OTHER ERROR\n");//if hex is not in correct format
            }
            fptr = moveOn(fptr);            //move on to the next line of the document
        } else if (!strcmp(word1, ".FILL")) {
            fscanf(fptr, "%s", word1);
            if ((word1[0] == 'x') || (word1[0]== '#')) {
                if (word1[0] == 'x') {
                    BAdd = (int) strtol(word1 + 1, NULL, 16); //grab the constant that .FILL places at that address
                    //TODO: put this in the .obj file instead
                    printf("0x");
                    printf("%04X\n", BAdd); //want this to print in hex
                } else {
                    BAdd = (int) strtol(word1 + 1, NULL, 10);
                    //TODO: put this in the .obj file instead
                    printf("0x");
                    printf("%04X\n", BAdd); //want this to print in hex
                }
            } else {
                //TODO: exit instead - exit(4)
                printf("ERROR CODE 4: OTHER ERROR\n");
            }
            fptr = moveOn(fptr);            //move on to the next line of the document
        } else if(isValid(word1)){                 //if the word is a valid opcode
            int index = getIndex(word1);
            //TODO: technically we could take the moveOn call in all of these and put it at the end since each if() uses it and then it would only take one line
            if(index == 0){
                ADD(fptr);
                moveOn(fptr);
            }
            else if(index == 1){
                AND(fptr);
                moveOn(fptr);
            }
            else if(index == 2){
                BR(fptr, word1);
                moveOn(fptr);
            }
            else if(index == 3){
                JMP(fptr, word1);
                moveOn(fptr);
            }
            else if(index == 4){
                JSR(fptr, word1);
                moveOn(fptr);
            }
            else if(index == 5){
                LDB(fptr);
                moveOn(fptr);
            }
            else if(index == 6){
                LDW(fptr);
                moveOn(fptr);
            }
            else if(index == 7){
                LEA(fptr);
                moveOn(fptr);
            }
            else if(index == 8){
                RTI(fptr);
                moveOn(fptr);
            }
            else if(index == 9){
                SHF(fptr, word1);
                moveOn(fptr);
            }
            else if(index == 10){
                STB(fptr);
                moveOn(fptr);
            }
            else if(index == 11){
                STW(fptr);
                moveOn(fptr);
            }
            else if(index == 12){
                TRAP(fptr, word1);
                moveOn(fptr);
            }
            else if(index == 13){
                XOR(fptr, word1);
                moveOn(fptr);
            }
        }
        //TODO: do all these if/else ifs check if the first word it grabbed is a label so that then it can still assemble the instruction after it?
        else if((!isLabel(word1))&&(strcmp(word1,".END"))){
            //TODO: exit instead - exit(2)
            printf("ERROR CODE 2: INVALID OPCODE\n");
            //TODO: once we change to exiting I don't think we'll need the next two lines
            moveOn(fptr);
            BAdd+=2;
        }
    }
    fclose(fptr); //this goes at the end of program
    //TODO: I think we might have to add exit(0) here (at the end of the program) since it says we have to return an error code of 0 if everything assembled correctly
}
//***********************************************************************************************************************************
//************************************************ HELPER FUNCTIONS *****************************************************************
//***********************************************************************************************************************************

int getAddress(char str[]){
    int addr = 0;
    for(int i = 0; symbolTable[i].address != NULL; i++){
        if(!strcmp(str, symbolTable[i].label)){
            addr = symbolTable[i].address;
            break;
        }
    }
    return addr;
}

int getIndex(char str[20]){ //TODO: do we need to specify that the character array is 20 long?
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

//TODO: we'll want to at least comment this out once we start putting everything in the .obj file instead
void printSymbolTable(void){
    for(int i = 0; symbolTable[i].address!=NULL;i++) {  //to reach entire symbol table
        printf("%s ", symbolTable[i].label);
        printf("x");
        printf("%04X\n",symbolTable[i].address); //want this to print in hex
    }
}

//***********************************************************************************************************************************
//************************************************ OPCODE FUNCTIONS *****************************************************************
//***********************************************************************************************************************************

void ADD(FILE* fp){
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of SR1
    int num3 = 0;                                       //this is the decimal rep of binary of SR2 or imm5
    fscanf(fp, "%s", word);
    //TODO: should these and's be or's?
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with SR1
    }
    fscanf(fp, "%s", word);
    if((word[0] == 'x') || (word[0] == '#')){          //if immediate operand
        if(word[0] == 'x') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3= (int)strtol(word+1, NULL, 10);
        }
        //TODO: the range of an immediate should be -16 to 15
        if(num3 > 31){                                //if immediate is too large
            //TODO: exit instead - exit(3)
            printf("ERROR CODE 3: INVALID CONSTANT\n");
        }
        else {
            if (num3 < 0) {num3 += 32;}        //if negative immediate //TODO: what is this doing?
            num3 += 32;                          //this is bit 5 //TODO: what is this doing?
            //TODO: output this to the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[0].hexop); //want this to print in hex//bit 5 is 1
        }
    }
    else if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else if (word[0] == 'R'){                         //if second source operand
        num3 = num3 + numToBits((word[1] - '0'), 0, 2, 0);        //set bits 2-0 with SR2
        //TODO: output this to the .obj file instead
        printf("0x");
        printf("%04X\n", num1 + num2 + num3 + allCodes[0].hexop); //want this to print in hex
    }
    BAdd += 2;
}

void AND(FILE* fp){
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of SR1
    int num3 = 0;                                       //this is the decimal rep of binary of SR2 or imm5
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
        return;
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with SR1
    }
    fscanf(fp, "%s", word);
    if((word[0] == 'x') || (word[0] == '#')){          //if immediate operand
        if(word[0] == 'x') {
            num3 = (int)strtol(word + 1, NULL, 16);
        }
        else{
            num3 = (int)strtol(word + 1, NULL, 10);
        }
        //TODO: the range of an immediate is -16 to 15
        if(num3 > 31){                                //if immediate is too large
            //TODO: exit instead - exit(3)
            printf("ERROR CODE 3: INVALID CONSTANT\n");
        }
        else {
            if (num3 < 0) {num3 += 32;}        //if negative immediate
            num3 += 32;                          //this is bit 5
            //TODO: put this in the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[1].hexop); //want this to print in hex//bit 5 is 1
        }
    }
    else if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else if (word[0] == 'R'){                         //if second source operand
        num3 = num3 + numToBits((word[1] - '0'), 0, 2, 0);        //set bits 2-0 with SR2
        //TODO: put this in the .obj file instead
        printf("0x");
        printf("%04X\n", num1 + num2 + num3 + allCodes[1].hexop); //want this to print in hex
    }
    BAdd += 2;
}

void BR(FILE* fp, char word[20]){
    char wordr[5] = {0, 0, 0, 0, 0};                    //this is to read file into
    int num1 = 0;                                       //for iteration of br
    int num2 = 0;                                       //for offset
    int offset = 0;                                     //offset
    if(!strcmp(word, "BRN")){num1 += 2048;}
    else if(!strcmp(word, "BRZ")){num1 += 1024;}
    else if(!strcmp(word, "BRP")){num1 += 512;}
    else if(!strcmp(word, "BR")){num1 += 3584;}
    else if(!strcmp(word, "BRZP")){num1 += 1536;}
    else if(!strcmp(word, "BRNP")){num1 += 2560;}
    else if(!strcmp(word, "BRNZ")){num1 += 3072;}
    else if(!strcmp(word, "BRNZP")){num1 += 3584;}

    fscanf(fp, "%s", wordr);                            //get the label
    if(isLabel(wordr)){                                 //if label
        offset = getAddress(wordr);                     //get address
        offset = (((offset - 2) - BAdd) / 2);                 //get offset from incremented PC
        num2 = num2 + numToBits(offset, 0, 8, 0);              //set bits 8-0 with DR
        //TODO: put this in the .obj file instead
        printf("0x");
        printf("%04X\n", num1 + num2 + allCodes[2].hexop);  //want this to print in hex
    }
    else {
        //TODO: exit instead - exit(1)
        printf("ERROR CODE 1: UNDEFINED LABEL\n");
    }
    BAdd += 2;
}

void JMP(FILE* fp, char word[20]){
    int num = 0;
    char wordr[5] = {0, 0, 0, 0, 0};             //this is to read file into
    fscanf(fp, "%s", wordr);                     //get the next word
    if(!strcmp(word, "JMP")){
        if((wordr[0] != 'R') && (!(wordr[1] <= 0)) && (!(wordr[1] < 8)) && (!((wordr[2] == 0) || (wordr[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
            //TODO: exit instead - exit(4)
            printf("ERROR CODE 4: OTHER ERROR\n");
        }
        else{
            num = num + numToBits((wordr[1] - '0'), 0, 8, 6);        //set bits 8-6 with BR
            //TODO: put this in the .obj file instead
            printf("0x");
            printf("%04X\n", num + allCodes[3].hexop);        //want this to print in hex
        }
    }
    else if(!strcmp(word, "RET")){
        //TODO: put this in the .obj file instead
        printf("0xC1C0\n");
    }
    BAdd += 2;
}

void JSR(FILE* fp, char word[20]){
    int num = 0;
    char wordr[5] = {0, 0, 0, 0, 0};                    //this is to read file into
    int offset = 0;
    fscanf(fp, "%s", wordr);                            //get the next word
    if(!strcmp(word, "JSR")){                            //JSR
        if(isLabel(wordr)){                             //if label
            offset = getAddress(wordr);                 //get address
            offset = (((offset - 2) - BAdd) / 2);             //get offset from incremented PC
            num = num + numToBits(offset, 0, 10, 0);          //set bits 10-0
            //TODO: put this in the .obj file instead
            printf("0x");
            printf("%04X\n", 2048 + num + allCodes[4].hexop); //want this to print in hex //adding 2048 because bit 11 is set
        }
 /*       else if ((wordr[0] == 'x') || (wordr[0] == '#')) {
            if (wordr[0] == 'x') {
                num = (int) strtol(wordr + 1, NULL, 16); //grab address where the program begins. word+1 b/c you want to avoid x
                if(num > 1023){
                printf("ERROR CODE 3: INVALID CONSTANT");
                }
                else{
                printf("0x");
                printf("%04X\n", 2048 + num + allCodes[4].hexop); //want this to print in hex
                }
            } else {
                num = (int) strtol(wordr + 1, NULL, 10);
                if(num > 1023){
                printf("ERROR CODE 3: INVALID CONSTANT");
                }
                else{
                printf("0x");
                printf("%04X\n", 2048 + num + allCodes[4].hexop); //want this to print in hex
                }
            }
        }*/                                                     // can JSR take a direct offset??? has to be a label right??
        else{
            //TODO: exit instead - exit(1)
            printf("ERROR CODE 1: UNDEFINED LABEL\n");
        }
    }

    else if(!strcmp(word, "JSRR")){                      //JSRR
        if((wordr[0] != 'R') && (!(wordr[1] <= 0)) && (!(wordr[1] < 8)) && (!((wordr[2] == 0) || (wordr[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
            //TODO: exit intead - exit(4)
            printf("ERROR CODE 4: OTHER ERROR\n");
        }
        else{
            num = num + numToBits((word[1] - '0'), 0, 5, 0);        //set bits 5-0
            //TODO: put this in the .obj file instead
            printf("0x");
            printf("%04X\n", num + allCodes[4].hexop); //want this to print in hex //adding 2048 because bit 11 is set
        }
    }
    BAdd += 2;
}

void LDB(FILE* fp){
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }

    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with BR
    }

    fscanf(fp, "%s", word);                                             //get the offset
    //TODO: check if we need to check for a hex number too
    if(word[0] != '#') {                                                //can the offset be in hex???
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    } else{
        num3 = (int)strtol(word + 1, NULL, 10);
        //TODO: make sure 63 is the right number
        if(num3 > 63){
            //TODO: exit instead - exit(3)
            printf("ERROR CODE 3: INVALID CONSTANT\n");
        }
        else {
            //TODO: put this in the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[5].hexop); //want this to print in hex
        }
    }
    BAdd += 2;
}

void LDW(FILE* fp){
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }

    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with BR
    }

    fscanf(fp, "%s", word);                                             //get the offset
    //TODO: check if we need to check for a hex number too
    if(word[0] != '#') {                                                //can the offset be in hex???
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    } else{
        num3 = (int)strtol(word + 1, NULL, 10);
        //TODO: check if 63 is the right number
        if(num3 > 63){
            //TODO: exit instead - exit(3)
            printf("ERROR CODE 3: INVALID CONSTANT\n");
        }
        else {
            //TODO: put this in the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[6].hexop); //want this to print in hex
        }
    }
    BAdd += 2;
}

void LEA(FILE* fp){      //we need an error here for when the offset is too far away************************************************
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of offset
    int offset = 0;                                     //this is to calculate offset of LEA
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }
    //TODO: check if LEA can only take a label and not a direct offset
    fscanf(fp, "%s", word);                     //get the label or direct offset
    if(isLabel(word)){                          //if label
        offset = getAddress(word);              //get address
        offset = (((offset - 2) - BAdd) / 2);         //get offset from incremented PC
        num2 = num2 + numToBits(offset, 0, 8, 0);          //set bits 8-0 with DR
        //TODO: check if the offset is too large
        //TODO: put this in the .obj file instead
        printf("0x");
        printf("%04X\n", num1 + num2 + allCodes[7].hexop); //want this to print in hex
    }
        /*       else if ((wordr[0] == 'x') || (wordr[0] == '#')) {
                   if (wordr[0] == 'x') {
                       num = (int) strtol(wordr + 1, NULL, 16); //grab address where the program begins. word+1 b/c you want to avoid x
                       if(num > 511){
                       printf("ERROR CODE 3: INVALID CONSTANT");
                       }
                       else{
                       printf("0x");
                       printf("%04X\n", 2048 + num + allCodes[4].hexop); //want this to print in hex
                       }
                   } else {
                       num = (int) strtol(wordr + 1, NULL, 10);
                       if(num > 511){
                       printf("ERROR CODE 3: INVALID CONSTANT");
                       }
                       else{
                       printf("0x");
                       printf("%04X\n", 2048 + num + allCodes[4].hexop); //want this to print in hex
                       }
                   }
               }*/                                                     // can LEA take a direct offset??? has to be a label right??
    else{
        //TODO: exit instead - exit(1)
        printf("ERROR CODE 1: UNDEFINED LABEL\n");
    }
    BAdd += 2;
}

void RTI(FILE* fp){
    //TODO: put this in the .obj file instead
    printf("0x8000\n");
    BAdd += 2;
}

void SHF(FILE* fp, char word[20]){
    char wordr[5] = {0, 0, 0, 0, 0};             //this is to read file into
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    if(!strcmp(word, "RSHF")){num1 = 16;}
    if(!strcmp(word, "RSHFA")){num1 = 48;}

    fscanf(fp, "%s", wordr);
    //TODO: also should the 0's and 8's and stuff in all these be characters instead since wordr is a character array and not an int array?
    if((wordr[0] != 'R') && (!(wordr[1] <= 0)) && (!(wordr[1] < 8)) && (!((wordr[2] == 0) || (wordr[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((wordr[1] - '0'), 0, 11, 9);        //set bits 11-9 DR
    }

    fscanf(fp, "%s", wordr);
    if((wordr[0] != 'R') && (!(wordr[1] <= 0)) && (!(wordr[1] < 8)) && (!((wordr[2] == 0) || (wordr[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num2 = num2 + numToBits((wordr[1] - '0'), 0, 8, 6);        //set bits 8-6 SR
    }

    fscanf(fp, "%s", wordr);
    if(wordr[0] != '#') {
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    } else{
        num3 = (int)strtol(wordr + 1, NULL, 10);
        //TODO: make sure this checks against the correct number
        if(num3 > 15){
            //TODO: exit instead - exit(3)
            printf("ERROR CODE 3: INVALID CONSTANT\n");
        }
        else {
            //TODO: put this in the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[9].hexop); //want this to print in hex
        }
    }
    BAdd += 2;
}
void STB(FILE* fp){
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }

    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with BR
    }

    fscanf(fp, "%s", word);                                             //get the offset
    //TODO: check if the offset can also be in hex
    if(word[0] != '#') {                                                //can the offset be in hex???
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    } else{
        num3 = (int)strtol(word + 1, NULL, 10);
        //TODO: make sure 63 is the right number to check against
        if(num3 > 63){
            //TODO: exit instead - exit(3)
            printf("ERROR CODE 3: INVALID CONSTANT\n");
        }
        else {
            //TODO: put this into the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[10].hexop); //want this to print in hex
        }
    }
    BAdd += 2;
}
void STW(FILE* fp){
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of BR
    int num3 = 0;                                       //this is the decimal rep of offset
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }

    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with BR
    }

    fscanf(fp, "%s", word);                                             //get the offset
    //TODO: check if the offset can also be given in hex
    if(word[0] != '#') {                                                //can the offset be in hex???
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    } else{
        num3 = (int)strtol(word + 1, NULL, 10);
        //TODO: make sure 63 is the right number to check against
        if(num3 > 63){
            //TODO: exit instead - exit(3)
            printf("ERROR CODE 3: INVALID CONSTANT\n");
        }
        else {
            //TODO: put this into the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[11].hexop); //want this to print in hex
        }
    }
    BAdd += 2;
}

void TRAP(FILE* fp, char word[20]){
    char wordr[5] = {0, 0, 0, 0, 0};             //this is to read file into
    if(!strcmp(word, "TRAP")){
        fscanf(fp, "%s", wordr);                     //get the next word
        if(!strcmp(wordr, "x25")){
            //TODO: put this into the .obj file instead
            printf("0xF025\n");
        }
        //TODO: make sure there aren't any other TRAP codes we should be watching for
        else{
            //TODO: exit instead - exit(4)
            printf("ERROR CODE 4: OTHER ERROR\n");
        }
    }
    else if(!strcmp(word, "HALT")){
        //TODO: put this into the .obj file instead
        printf("0xF025\n");
    }
    BAdd += 2;
}

void XOR(FILE* fp, char word1[20]){
    char word[5] = {0, 0, 0, 0, 0};                     //this is to read file into
    int num1 = 0;                                       //this is the decimal rep of binary of DR
    int num2 = 0;                                       //this is the decimal rep of binary of SR1
    int num3 = 0;                                       //this is the decimal rep of binary of SR2 or imm5
    fscanf(fp, "%s", word);
    if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
        //TODO: exit instead - exit(4)
        printf("ERROR CODE 4: OTHER ERROR\n");
    }
    else{
        num1 = num1 + numToBits((word[1] - '0'), 0, 11, 9);        //set bits 11-9 with DR
    }
    fscanf(fp, "%s", word);
    if(!strcmp(word1, "XOR")) {
        if((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',') || (word[2] == ';')))){ //needs to be a valid register
            //TODO: exit instead - exit(4)
            printf("ERROR CODE 4: OTHER ERROR\n");
        }
        else{
            num2 = num2 + numToBits((word[1] - '0'), 0, 8, 6);        //set bits 8-6 with DR
        }
        fscanf(fp, "%s", word);
        if ((word[0] == 'x') || (word[0] == '#')) {          //if immediate operand
            if (word[0] == 'x') {
                num3 = (int) strtol(word + 1, NULL, 16);
            } else {
                num3 = (int) strtol(word + 1, NULL, 10);
            }
            //TODO: the immediate can have a range of -16 to 15
            if (num3 > 31) {                                //if immediate is too large
                //TODO: exit instead - exit(3)
                printf("ERROR CODE 3: INVALID CONSTANT\n");
            } else {
                if (num3 < 0) {num3 += 32;}        //if negative immediate
                num3 += 32;                          //this is bit 5
                //TODO: put this into the .obj file instead
                printf("0x");
                printf("%04X\n", num1 + num2 + num3 + allCodes[0].hexop); //want this to print in hex//bit 5 is 1
            }
        } else if ((word[0] != 'R') && (!(word[1] <= 0)) && (!(word[1] < 8)) && (!((word[2] == 0) || (word[2] == ',')))) { //needs to be a valid register
            //TODO: exit instead - exit(4)
            printf("ERROR CODE 4: OTHER ERROR\n");
        } else if (word[0] == 'R') {                         //if second source operand
            num3 = num3 + numToBits((word[1] - '0'), 0, 2, 0);        //set bits 2-0 with SR2
            //TODO: put this into the .obj file instead
            printf("0x");
            printf("%04X\n", num1 + num2 + num3 + allCodes[13].hexop); //want this to print in hex
        }
    }
    //TODO: are we sure num1 and num2 are set to the right thing here? (since num2 at least still seems to be 0)
    else{
        //TODO: put this into the .obj file instead
        printf("0x");
        printf("%04X\n", num1 + num2 + 63 + allCodes[13].hexop); //want this to print in hex
    }
    BAdd += 2;
}