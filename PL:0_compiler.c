#define _CRT_SECURE_NO_WARNINGS        
#pragma warning(disable : 6031)        
#define MAX_TABLE_SIZE 500

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char text[MAX_TABLE_SIZE]; // Stores files inputs
char formattedArray[MAX_TABLE_SIZE]; // processes the text to be seperated so we can differentiate

/*
typedef enum {
    oddsym = 1,
    identsym,
    numbersym,
    plussym,
    minussym,
    multsym,
    slashsym,
    ifelsym,
    eqlsym,
    neqsym,
    lessym,
    leqsym,
    gtrsym,
    geqsym,
    lparentsym,
    rparentsym,
    commasym,
    semicolonsym,
    periodsym,
    becomessym,
    beginsym,
    endsym,
    ifsym,
    thensym,
    whilesym,
    dosym,
    callsym,
    constsym,
    varsym,
    procsym,
    writesym,
    readsym,
    elsesym
} token;
*/                  
//Structure for tokens
typedef struct {
    int token_number;
    char identifier[11];
}  token_number;

//Structure for symbol table from pdf
typedef struct {
    int kind;   //const = 1; var = 2;
    char identifier[11];  //name up to 11 chars
    int val;    //number value
    int level; //Lexical level
    int address;    //m address
    int mark;   //to indicate unavailable or deleted
} symbol;

typedef struct {
    char identifier[11];
    int level;
    int address;
} assembly;

token_number tokenList[MAX_TABLE_SIZE]; //Stores tokens
symbol symbolTable[MAX_TABLE_SIZE]; //symbol table
assembly outputArray[MAX_TABLE_SIZE];   //assembly table for outputs
int tokenLength = 0, currentToken = 0, tableLength = 0, lexical_level = -1; //index variable so we always know where we are at
int symIndex, outputLength = 0;
FILE* inputFilePointer;

//Token declaration so we can access it inputFilePointer all functions
token_number current_token;

//function prototypes because of errors
void EXPRESSION();
void STATEMENT();
int isNum(char* digit);
void BLOCK();

// Function to mark everything as done at the end of the program
void markDone() {
    for (int i = 0; i < tableLength; i++) {
        symbolTable[i].mark = 1;
    }
}


//Function for EMIT
void addEMIT(char* identifier, int level, int address) {
    strcpy(outputArray[outputLength].identifier, identifier);
    outputArray[outputLength].level = level;
    outputArray[outputLength].address = address;
    outputLength++;
}

//Function for determining token types          
char* getToken(char* token_number) {

    if (token_number == NULL) return "-1";

    if (strcmp(token_number, "+") == 0)
        return "4";
    else if (isNum(token_number) && (strlen(token_number) < 6)) 
        return "3";
    else if (isNum(token_number) && (strlen(token_number) > 5))
    {
        printf("\nNumber '%s' too long\n\n", token_number);
        return "-1";
    }
    else if (strcmp(token_number, "null") == 0)
        return "1";
    else if (strcmp(token_number, "-") == 0)
        return "5";
    else if (strcmp(token_number, "*") == 0)
        return "6";                                                                                             
    else if (strcmp(token_number, "/") == 0)
        return "7";
    else if (strcmp(token_number, "odd") == 0)
        return "8";
    else if (strcmp(token_number, "=") == 0)
        return "9";
    else if (strcmp(token_number, "!=") == 0)
        return "10";
    else if (strcmp(token_number, "<") == 0)
        return "11";
    else if (strcmp(token_number, "<=") == 0)
        return "12";
    else if (strcmp(token_number, ">") == 0)
        return "13";
    else if (strcmp(token_number, ">=") == 0)
        return "14";
    else if (strcmp(token_number, "(") == 0)
        return "15";
    else if (strcmp(token_number, ")") == 0)
        return "16";
    else if (strcmp(token_number, ",") == 0)
        return "17";
    else if (strcmp(token_number, ";") == 0)
        return "18";
    else if (strcmp(token_number, ".") == 0)
        return "19";
    else if (strcmp(token_number, ":=") == 0)
        return "20";
    else if (strcmp(token_number, "begin") == 0)
        return "21";
    else if (strcmp(token_number, "end") == 0)
        return "22";
    else if (strcmp(token_number, "if") == 0)
        return "23";
    else if (strcmp(token_number, "then") == 0)
        return "24";
    else if (strcmp(token_number, "while") == 0)
        return "25";
    else if (strcmp(token_number, "do") == 0)
        return "26";
    else if (strcmp(token_number, "call") == 0)
        return "27";
    else if (strcmp(token_number, "const") == 0)
        return "28";
    else if (strcmp(token_number, "var") == 0)
        return "29";
    else if (strcmp(token_number, "procedure") == 0)
        return "30";
    else if (strcmp(token_number, "write") == 0)
        return "31";
    else if (strcmp(token_number, "read") == 0)
        return "32";
    else if (strcmp(token_number, "else") == 0)
        return "33";
    else if (strlen(token_number) < 12 && isalpha(token_number[0]))  //identifier not starting with number and is less than 12 
        return "2";
    else if (strlen(token_number) > 12)
    {
        printf("\n'%s' Error number 26, Identifier too long\n\n", token_number);
        return "-1";
    }
    else if (strlen(token_number) < 12 && !isalpha(token_number[0]))
    {
        printf("\nIdentifier/Unrecognized item '%s' cannot start with a number\n\n", token_number);
        return "-1";
    }
    else if (strlen(token_number) < 12)

        return "-2";
}

//Function for checking if current is a number
int isNum(char* digit)
{
    int length = strlen(digit);

    for (int i = 0; i < length; i++) {
        if (isdigit(digit[i])) continue;
        else return 0;
    }
    return 1;
}

//Function for adding tokens to the tokenlist
void addToTokens(char* identifier, int token_number) {
    if (tokenLength >= 500) {
        printf("token overload");
        exit(-1);
    }
    strcpy(tokenList[tokenLength].identifier, identifier);
    tokenList[tokenLength].token_number = token_number;
    tokenLength++;
}

//Function for looking up symbols inputFilePointer the symboltable
int symbolTableLookup(char* identifier) {
    for (int i = 0; i < MAX_TABLE_SIZE; i++) {
        if (strcmp(symbolTable[i].identifier, identifier) == 0) return i; //returns index of symbol
    }
    return -1; //returns -1 if name is not inside table
}

//Function for adding symbols to the symboltable
void addToSymbolTable(int kind, char* identifier, int value, int level, int address, int mark) {
    symbolTable[tableLength].kind = kind;
    strcpy(symbolTable[tableLength].identifier, identifier);
    symbolTable[tableLength].val = value;
    symbolTable[tableLength].level = level;
    symbolTable[tableLength].address = address;
    symbolTable[tableLength].mark = mark;
    tableLength++; //Length of table incremented
}

//Function for grabbing the token for parser
void getNextToken() {
    current_token = tokenList[currentToken];
    //printf("%d %s\n", current_token.token_number, current_token.identifier);
    currentToken++;
    //printf("%d and %s\n", currToken.token, currToken.name);
}

//For const
void CONST_DECLARATION() {
    if (current_token.token_number == 28 /*constsym*/) {
        do {
            getNextToken();
            if (current_token.token_number != 2 /*identsym*/) {
                 printf("Error number 4, const, var, procedure must be followed by identifier\n");
                exit(-1);
            }
            if (symbolTableLookup(current_token.identifier) != -1 && (symbolTable[symbolTableLookup(current_token.identifier)].level == lexical_level)) {
                printf("\nError number 31, symbol name has already been declared\n");
                exit(-1);
            }
            char temp[11]; strcpy(temp, current_token.identifier);    //STORING NAME SO WE CAN ADD TO TABLE
            getNextToken();
            if (current_token.token_number != 9 /*eqlsym*/) {
                printf("\nError number 3, Identifier must be followed by =\n");
                exit(-1);
            }
            getNextToken();
            if (current_token.token_number != 3 /*numbersym*/) {
                 printf("Error number 4, const, var, procedure must be followed by identifier\n");
                exit(-1);
            }
            addToSymbolTable(1, temp, atoi(current_token.identifier), lexical_level, 0, 0);
            getNextToken();
        } while (current_token.token_number == 17 /*commasym*/);
        if (current_token.token_number != 18 /*semicolonsym*/) {
            printf("\nError number 17, Semicolon or end expected\n");
            exit(-1);
        }
        getNextToken();
    }
}



// Function for handling variable declarations
int VAR_DECLARATION() {
    int numVars = 0;
    if (current_token.token_number == 29 /*varsym*/) { // Check if the current token is 'varsym' (which indicates a variable declaration)
        do {
            numVars++;
            getNextToken();
             // Check if the next token is 'identsym' (identifier)
            if (current_token.token_number != 2 /*identsym*/) {
                 printf("Error number 4, const, var, procedure must be followed by identifier\n");
                exit(-1);
            }
             // Check if the identifier has already been declared in the symbol table
            if (symbolTableLookup(current_token.identifier) != -1 && (symbolTable[symbolTableLookup(current_token.identifier)].level == lexical_level)) {
                printf("\nError number 31, symbol name has already been declared\n");
                exit(-1);
            }
             // Add the identifier to the symbol table with appropriate details
            addToSymbolTable(2, current_token.identifier, 0, lexical_level, numVars + 2, 0); // ADDRESS AND NAME POSSIBLE SCREW UPS
            getNextToken();
        } while (current_token.token_number == 17 /*commasym*/);
        if (current_token.token_number != 18 /*semicolonsym*/) {
            printf("%d\n", current_token.token_number);
            printf("\nError number 10, missing comma or semicolon\n");
            exit(-1);
        }
        getNextToken();
    }
    return numVars;
}

void PROC_DECLARATION(int numberOfVariables) {
    if(current_token.token_number == 30 /*procsym*/) {
        getNextToken();
        char name[11];
        if(current_token.token_number != 2 /*identsym*/) {
            printf("Error number 4, const, var, procedure must be followed by identifier\n");
            exit(-1);
        }
        if(symbolTableLookup(current_token.identifier) != -1 ) {
            printf("\nError number 31, symbol name has already been declared\n");
            exit(-1);
        }
        strcpy(name, current_token.identifier); //for table
        getNextToken();
        if(current_token.token_number != 18 /*semicolonsym*/) {
            printf("Error number 10, semicolon or comma missing\n");
            exit(-1);
        }
        addToSymbolTable(3, name, 0, lexical_level, outputLength*3, 0);
        getNextToken();
        BLOCK();
        if(current_token.token_number != 18) {
            printf("Error number 10, missing comma or semicolon\n");
            exit(-1);
        }
        getNextToken();
    }
}


//Block function
void BLOCK() {
    lexical_level++;

    int indexPlaceholder = outputLength;
    addEMIT("JMP", 0, 0);

    CONST_DECLARATION();
    int numVars = VAR_DECLARATION();
    PROC_DECLARATION(numVars);

    outputArray[indexPlaceholder].address = (outputLength) *3;
    
    //EMIT INC M -> m = 3 + numVars; //M inputFilePointer record
    addEMIT("INC", 0, 3 + numVars);
    STATEMENT();
    if(lexical_level != 0) addEMIT("OPR", 0, 0);
    lexical_level--;
}

//Program function
void PROGRAM() {
    //addToSymbolTable(3, "main", 0, 0, 3, 0);
    BLOCK();
    if (current_token.token_number != 19 /*periodsym*/) {
        printf("\nError number 9, Period expected\n");
        exit(-1);
    }
    //EMIT HALT
    addEMIT("SYS", 0, 3);
    markDone(); //FUNCTION FOR SETTING MARK TO 1 AT END OF PROGRAM
}

//Condition func
void CONDITION() {
    if (current_token.token_number == 8 /*oddsym*/) {
        getNextToken();
        EXPRESSION();
        //EMIT ODD
        addEMIT("OPR", 0, 11);
    }
    else {
        EXPRESSION();
        if (current_token.token_number == 9 /*eqlsym*/) {
            getNextToken();
            EXPRESSION();
            //EMIT EQL
            addEMIT("OPR", 0, 5);
        }
        else if (current_token.token_number == 10 /*neqsym*/) {
            getNextToken();
            EXPRESSION();
            //EMIT NEQ
            addEMIT("OPR", 0, 6);
        }
        else if (current_token.token_number == 11 /*lessym*/) {
            getNextToken();
            EXPRESSION();
            //EMIT LSS
            addEMIT("OPR", 0, 7);
        }
        else if (current_token.token_number == 12 /*leqsym*/) {
            getNextToken();
            EXPRESSION();
            //EMIT LEQ
            addEMIT("OPR", 0, 8);
        }
        else if (current_token.token_number == 13 /*gtrsym*/) {
            getNextToken();
            EXPRESSION();
            //EMIT GTR
            addEMIT("OPR", 0, 9);
        }
        else if (current_token.token_number == 14 /*geqsym*/) {
            getNextToken();
            EXPRESSION();
            //EMIT GEQ
            addEMIT("OPR", 0, 10);
        }
        else {
            printf("\nError number 11, Undeclared identofier\n");
            exit(-1);
        }
    }
}


//factor function
void FACTOR() {
    if (current_token.token_number == 2 /*identsym*/) {
        /*int*/ symIndex = symbolTableLookup(current_token.identifier);
        if (symIndex == -1) {
            printf("\nError number 27, Invalid symbol\n");
            exit(-1);
        }
        if (symbolTable[symIndex].kind == 1)
            //EMIT LIT (M=table[symIndex].value
            addEMIT("LIT", 0, symbolTable[symIndex].val);

        else {
            //EMIT LOD (M=table[symIndex].address
            addEMIT("LOD", symbolTable[symIndex].level, symbolTable[symIndex].address);

        }
        getNextToken();
    }
    else if (current_token.token_number == 3 /*numbersym*/) {
        //EMIT LIT
        addEMIT("LIT", 0, atoi(current_token.identifier));

        getNextToken();
    }
    else if (current_token.token_number == 15 /*lparentsym*/) {
        getNextToken();
        EXPRESSION();
        if (current_token.token_number != 16 /*rparentsym*/) {
            printf("\nError number 22,left parenthesis is missing\n");
            exit(-1);
        }
        getNextToken();
    }
    else {
        printf("\neerror number 13, Assigmnet operator expected\n");
        exit(-1);
    }
}


void TERM() {
    FACTOR();
    while (current_token.token_number == 6 /*multsym*/ || current_token.token_number == 7 /*slashsym*/) {
        if (current_token.token_number == 6 /*multsym*/) {
            getNextToken();
            FACTOR();
            //EMIT MUL
            addEMIT("OPR", 0, 3);

        }
        else if (current_token.token_number == 7 /*slashsym*/) {
            getNextToken();
            FACTOR();
            //EMIT DIV
            addEMIT("OPR", 0, 4);

        }
    }
}

//Expression func
void EXPRESSION() {
    if (current_token.token_number == 5 /*minussym*/) {
        getNextToken();
        TERM();

        //EMIT NEG?


        while (current_token.token_number == 4 /*plussym*/ || current_token.token_number == 5 /*minussym*/) {
            if (current_token.token_number == 4 /*plussym*/) {
                getNextToken();
                TERM();
                //EMIT ADD
                addEMIT("OPR", 0, 1);
            }
            else {
                getNextToken();
                TERM();
                //EMIT SUB
                addEMIT("OPR", 0, 2);
            }
        }
    }
    else {
        if (current_token.token_number == 4 /*plussym*/) {
            getNextToken();
        }
        TERM();
        while (current_token.token_number == 4 /*plussym*/ || current_token.token_number == 5 /*minussym*/) {
            if (current_token.token_number == 4 /*plussym*/) {
                getNextToken();
                TERM();
                //EMIT ADD
                addEMIT("OPR", 0, 1);
            }
            else {
                getNextToken();
                TERM();
                //EMIT SUB
                addEMIT("OPR", 0, 2);
            }
        }
    }
}


//Statement function
void STATEMENT() {
    int loopIndex, jpcIndex, symIndex;

    if (current_token.token_number == 2 /*identsym*/) {
        int symbolIndex = symbolTableLookup(current_token.identifier);
        if (symbolIndex == -1) {
            printf("\nError numer 27, Invalid symbol\n");
            exit(-1);
        }
        if (symbolTable[symbolIndex].kind != 2) {
            printf("\nonly variable values may be altered\n");
            exit(-1);
        }
        getNextToken();
        if (current_token.token_number != 20 /*becomesym*/) {
            printf("\nError number 1, Use = instead of :=\n");
            exit(-1);
        }
        getNextToken();
        EXPRESSION();
        //EMIT STO (M = table[symIndex.addr]) lexical level also stored inputFilePointer sym table
        addEMIT("STO", symbolTable[symbolIndex].val, symbolTable[symbolIndex].address);
        return;
    }

    if(current_token.token_number == 27 /*callsym*/) {
        getNextToken();
        if(current_token.token_number != 2 /*identsym*/) {
            printf("Error number 4, const, var, procedure must be followed by identifier\n");
            exit(-1);
        }
        int symbolIndex = symbolTableLookup(current_token.identifier);
        if(symbolIndex == 0) {
            printf("Error number 28, recursively calling the program (endless loop)\n");
            exit(-1);
        }
        if(symbolTable[symbolIndex].kind != 3) {
            printf("Error number 29, procedure expected\n");
            exit(-1);
        }
        addEMIT("CAL", symbolTable[symbolIndex].level, symbolTable[symbolIndex].address);
        getNextToken();
        return;
    }

    if (current_token.token_number == 21 /*beginsym*/) {
        do {
            getNextToken();
            STATEMENT();
        } while (current_token.token_number == 18 /*semicolonsym*/);
        if (current_token.token_number != 22 /*endsym*/) {
            printf("\n Error number 30, begin must be followed by end\n");
            exit(-1);
        }
        getNextToken();
        return;
    }

    if (current_token.token_number == 23 /*ifsym*/) {
        getNextToken();
        CONDITION();
        jpcIndex = outputLength;
        //EMIT JPC
        addEMIT("JPC", 0, jpcIndex);
        if (current_token.token_number != 24 /*thensym*/) {
            printf("\nError number 16, then  expected\n");
            exit(-1);
        }
        getNextToken();
        STATEMENT();
        outputArray[jpcIndex].address = outputLength * 3;
        return;
    }

    if (current_token.token_number == 25 /*whilesym*/) {
        getNextToken();
        loopIndex = outputLength;
        CONDITION();
        if (current_token.token_number != 26 /*dosym*/) {
            printf("\nError number 18, do  epected\n");
            exit(-1);
        }
        getNextToken();
        jpcIndex = outputLength;
        //EMIT JPC
        addEMIT("JPC", 0, jpcIndex);
        STATEMENT();
        //EMIT JMP (M = loopIdx)
        addEMIT("JMP", 0, loopIndex);
        outputArray[jpcIndex].address = outputLength * 3;  //THIS NEEDS REVIEW FS
        return;
    }

    if (current_token.token_number == 32 /*readsym*/) {
        getNextToken();
        if (current_token.token_number != 2 /*identsym*/) {
            printf("Error number 4, const, var, procedure must be followed by identifier\n");
            exit(-1);
        }
        symIndex = symbolTableLookup(current_token.identifier);
        if (symIndex == -1) {
            printf("\nError number 27, Invalid Symbol\n");
            exit(-1);
        }
        if (symbolTable[symIndex].kind != 2) {
            printf("\nonly variable values may be altered\n");
            exit(-1);
        }
        getNextToken();
        //EMIT READ
        addEMIT("SYS", 0, 2);   //COULD BE WRONG (READ)
        //EMIT STO (M = table[symindex].addr)
        addEMIT("STO", symbolTable[symIndex].val, symbolTable[symIndex].address);
        return;
    }

    if (current_token.token_number == 31 /*writesym*/) {
        getNextToken();
        EXPRESSION();
        //EMIT WRITE
        addEMIT("SYS", 0, 1);    //COULD BE WRONG (WRITE)
        return;
    }
}


/* For printing our assembly code */
void printAssemble() {
    int OPnum = 0;
    printf("\n%4s %4s %4s\n","OP", "L", "M");
    for (int i = 0; i < outputLength; i++) {
        if (strcmp(outputArray[i].identifier, "JMP")==0){
            OPnum = 7;
        } else if (strcmp(outputArray[i].identifier, "INC")==0){
            OPnum = 6;
        } else if (strcmp(outputArray[i].identifier, "CAL")==0){
            OPnum = 5;
        } else if (strcmp(outputArray[i].identifier, "STO")==0){
            OPnum = 4;
        } else if (strcmp(outputArray[i].identifier, "LOD")==0){
            OPnum = 3;
        } else if (strcmp(outputArray[i].identifier, "OPR")==0){
            OPnum = 2;
        } else if (strcmp(outputArray[i].identifier, "LIT")==0){
            OPnum = 1;
        } else if (strcmp(outputArray[i].identifier, "SYS")==0){
            OPnum = 9;
        }
        printf("  %d  %4d %4d\n", OPnum, outputArray[i].level, outputArray[i].address);
        //printf("%4d %4s %4d %4d\n", i, outputArray[i].identifier, outputArray[i].level, outputArray[i].address);
    }
}

/* four printing our symbol table */
void printSymbol() {
    printf("\nSymbol Table:\n\n%-4s | %-12s | %-5s | %-5s | %-7s | %-5s\n", "Kind", "Name", "Value", "Level", "Address", "Mark");
    printf("-------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < tableLength; i++) {
        printf("%4d | %12s | %5d | %5d | %7d | %5d\n", symbolTable[i].kind, symbolTable[i].identifier, symbolTable[i].val, symbolTable[i].level, symbolTable[i].address, symbolTable[i].mark);
    }
}

int main(int argc, char* argv[]) {

    // Reads inputFilePointer the file input at the command line and into global array
    //inputFilePointer = fopen(argv[1], "r");               // USED FOR RUNNING ON EUSTICE
    inputFilePointer = fopen("input.txt", "r");           // USED FOR TESTING SO WE DONT HAVEE TO KEEP GIVING IT THE FILE
    int x = 0, running = 1;
    while (running) {
        if (feof(inputFilePointer))
            //when we reach end of file break the loop
            break;
        fscanf(inputFilePointer, "%c", &text[x]);
        x++;
    }
    
    
    fclose(inputFilePointer);
    
    printf("Source Program:\n%s\n", text);
    

    //Formatting the text into format array that we can tokenize with easier
    int j = 0;
    int comment = 0;    //SET === 1 if we are in the middle of reading a comment

    /*
        Make sure whole alphabet is covered --check
        every possible symbol that could be used is here so should be okay!
        added seperators as well -=-- check   
    */


    for (int i = 0; i < strlen(text); i++) {
        if (text[i] == ' ') continue;
        if (text[i] == '\t') continue;
        if (text[i] == '\n') continue;
        if ((comment == 1) && (text[i] == '*' && text[i + 1] == '/')) { //ending of a comment
            comment = 0;
            i++;
            continue;
        }
        if ((text[i] == '/' && text[i + 1] == '*') || (comment == 1)) { // opening of a comment
            comment = 1;    //we are in a comment
            continue;
        }
        if ((isalpha(text[i]) && text[i + 1] == ' ') || (isalpha(text[i]) && text[i + 1] == ',') || (isalpha(text[i]) && text[i + 1] == ';'))   //probably variables
        {
            formattedArray[j] = text[i];
            //adding to our formatted array with a seperator - is supposed to help???
            formattedArray[j + 1] = '~';    // squiggly line lets us differentiate between tokens
            j += 2; //moving two indices forward cause we just added 2 things
            continue;
        }
        if (text[i] == ':' && text[i + 1] == '=')
        {
            formattedArray[j] = text[i];    //defining
            j++;
            continue;
        }
        if (text[i] == '<' && text[i + 1] == '=') {
            formattedArray[j] = text[i];
            j++;        //comparison op
            continue;
        }
        if (text[i] == '>' && text[i + 1] == '=') {
            formattedArray[j] = text[i];
            j++;    //another comparison opp
            continue;
        }
        if (text[i] == '!' && text[i + 1] == '=') {
            formattedArray[j] = text[i];
            j++;    //another comparison
            continue;
        }
        if (text[i] == ',' )
        {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2;// another comparison
            continue;

        }
        if (text[i] == ':') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '.') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == ']') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '[') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == ')') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '(') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '>') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '<') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '/') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '*') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '-') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '+') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == '=') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (text[i] == ';') {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2; // another comparison
            continue;
        }
        if (isdigit(text[i]) && isdigit(text[i + 1]))
        {
            formattedArray[j] = text[i];
            j++;
            continue;
        }
        if (isdigit(text[i]) && isalpha(text[i + 1]))
        {
            formattedArray[j] = text[i];
            j++;
            continue;
        }
        if (isdigit(text[i]) && (!isdigit(text[i + 1] && !isalpha(text[i+1]))))
        {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2;
            continue;
        }
        if (isalpha(text[i]) && (isdigit(text[i+1])))
        {
            formattedArray[j] = text[i];
            j++;
            continue;
        }
        if (isalpha(text[i]) && isalpha(text[i + 1]))
        {
            formattedArray[j] = text[i];
            j++;
            continue;
        }
         if (isalpha(text[i]) && (!isdigit(text[i + 1] && !isalpha(text[i+1]))))
        {
            formattedArray[j] = text[i];
            formattedArray[j + 1] = '~';
            j += 2;
            continue;
        }
        else
        {
            formattedArray[j] = text[i];
            j++;
        }
    }

    //Function for printing the array to see if it is correct
    //printf("%s", formattedArray);
    //Creating sub-arrays and token values (adding tokens to array of tokens)
    //using strtok to cut at the ~'s to make our sub arrays
    char* token_number = strtok(formattedArray, "~");
    while (token_number != NULL) {

        //Gets token associated with item
        char* tokenNum = getToken(token_number);

        if (atoi(tokenNum) == -1)
        {
            //no need
        }
        else if (atoi(tokenNum) == -2)
        {
            printf("\n'%s' Error number 8, Incorrect symbol after statement\n\n", token_number);
        }
        else
        {
            //printf("Adding `%s` with `%d`\n", token_number, atoi(tokenNum));
            addToTokens(token_number, atoi(tokenNum));
        }

        token_number = strtok(NULL, "~");
    }

    //calling the recursion functions
    getNextToken();
    PROGRAM();

    //printf("No errors, program is syntactically correct\n");
    //outputting
    printAssemble();


    FILE *fptr;

    // Create a file
    fptr = fopen("elf.txt", "w");
    for (int i = 0; i < outputLength; i++) {
        int OPnum = 0;
        if (strcmp(outputArray[i].identifier, "JMP")==0){
            OPnum = 7;
        } else if (strcmp(outputArray[i].identifier, "INC")==0){
            OPnum = 6;
        } else if (strcmp(outputArray[i].identifier, "CAL")==0){
            OPnum = 5;
        } else if (strcmp(outputArray[i].identifier, "STO")==0){
            OPnum = 4;
        } else if (strcmp(outputArray[i].identifier, "LOD")==0){
            OPnum = 3;
        } else if (strcmp(outputArray[i].identifier, "OPR")==0){
            OPnum = 2;
        } else if (strcmp(outputArray[i].identifier, "LIT")==0){
            OPnum = 1;
        } else if (strcmp(outputArray[i].identifier, "SYS")==0){
            OPnum = 9;
        }
        fprintf(fptr,"  %d  %4d %4d\n", OPnum, outputArray[i].level, outputArray[i].address);
        //printf("%4d %4s %4d %4d\n", i, outputArray[i].identifier, outputArray[i].level, outputArray[i].address);
    }

    fclose(fptr);


    return 0;
}