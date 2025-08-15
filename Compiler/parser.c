#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "scanner.h"
#include "stringTree.h"
#include "parser.h"
#include "translate.h"

Expression* expressionList = NULL;
int expressionCount = 0; //number of parsed expressions

Expression** declarations = NULL;
int numRegisters = 0;
Expression** conditionals = NULL;
int numConditionals = 0;
Expression** instructions = NULL;
int numInstructions = 0;
Expression** labels = NULL;
int numLabels = 0;

void initParser() {
    expressionList = malloc(2 * numExpressions * sizeof(Expression));
    declarations = malloc(numExpressions * sizeof(Expression*));
    conditionals = malloc(numExpressions * sizeof(Expression*));
    instructions = malloc(numExpressions * sizeof(Expression*));
    labels = malloc(numExpressions * sizeof(Expression*));
    initTree();
}

/*
* Declares a label
*/
void declareLabel(char* l) {
    char* buf = l;
    l = addLabelToTree(l);
    if (l == NULL) exitAndFree(EXIT_FAILURE);
    l = skipSpaces(l);
    if (*l != '\0') {
        printf("Expected ':' in: %s", buf);
        exitAndFree(EXIT_FAILURE);
    }
    
    expressionList[expressionCount].eType = LABEL;
    expressionList[expressionCount].eVal.label.address = numInstructions;
    labels[numLabels] = &(expressionList[expressionCount]);
    expressionCount++;
    numLabels++;
}

/*
* Reads the label and returns the adress of the next instruction after the label
*/
int readLabel(char* l) {
    StringTree* leaf = getLabel(l);
    if (leaf == NULL) exitAndFree(EXIT_FAILURE);
    l += leaf->strLength;
    l = skipSpaces(l);
    if (*l != '\0') {
        printf("Expected ';'\n");
        exitAndFree(EXIT_FAILURE);
    }
    return labels[leaf->id]->eVal.label.address;
}

/*
* Parses Expressions that start with a register identifier: increments or decrements
*/
char* parseInDecrement(char* c, int regID) {
    c = skipSpaces(c);
    char* buf = c;
    StringTree* leaf = getOperator(c);
    c += leaf->strLength;
    if (leaf == NULL) {
        printf("Not an operand: %s\n", buf);
        exitAndFree(EXIT_FAILURE);
    }
    expressionList[expressionCount].eType = INSTRUCTION;
    switch (leaf->id) {
        case ID_INC:
            expressionList[expressionCount].eVal.instruction.iType = INCREMENT;
            break;
        case ID_DEC:
            expressionList[expressionCount].eVal.instruction.iType = DECREMENT;
            break;
        default:
            printf("Error: Invalid Instruction ID!\n");
            exitAndFree(EXIT_FAILURE);
            break;
    }
    expressionList[expressionCount].eVal.instruction.address = numInstructions;
    expressionList[expressionCount].eVal.instruction.param.regID = regID;
    instructions[numInstructions] = &(expressionList[expressionCount]);
    expressionCount++;
    numInstructions++;
    return c;

}

char* parseGoto(char* g) {
    g = skipSpaces(g);
    
    expressionList[expressionCount].eType = INSTRUCTION;
    expressionList[expressionCount].eVal.instruction.iType = GOTO;
    expressionList[expressionCount].eVal.instruction.address = numInstructions;
    expressionList[expressionCount].eVal.instruction.param.labelName = g;
    instructions[numInstructions] = &(expressionList[expressionCount]);
    expressionCount++;
    numInstructions++;
    while (isALPHAnumerical[(int) *g]) g++;
    return g;
}

void parseHalt() {
    expressionList[expressionCount].eType = INSTRUCTION;
    expressionList[expressionCount].eVal.instruction.iType = HALT;
    expressionList[expressionCount].eVal.instruction.address = numInstructions;
    instructions[numInstructions] = &(expressionList[expressionCount]);
    expressionCount++;
    numInstructions++;
}

char* parseDeclaration(char* d) {
    d = skipSpaces(d);
    char* buf = d;
    //Register
    d = addRegisterToTree(d);
    if (d == NULL) exitAndFree(EXIT_FAILURE);
    int regID = getRegisterOrInstruction(buf)->id;
    //Assignment operator
    d = skipSpaces(d);
    StringTree* leaf = getOperator(d);
    if (leaf == NULL || leaf->id != ID_ASSIGN) {
        printf("Expected '=' in: uint %s", buf);
        exitAndFree(EXIT_FAILURE);
    }
    //get number
    d += leaf->strLength;
    d = skipSpaces(d);
    if (!isNumerical[(int) *d]) {
        printf("Exprected a number in: %s", buf);
        exitAndFree(EXIT_FAILURE);
    }
    buf = d;
    while (isNumerical[(int) *d]) {
        d++;
    }
    char b = *d;
    *d = '\0';
    int number = atoi(buf);
    *d = b;

    expressionList[expressionCount].eType = DECLARATION;
    expressionList[expressionCount].eVal.declaration.numReg = regID;
    expressionList[expressionCount].eVal.declaration.initVal = number;
    declarations[numRegisters] = &(expressionList[expressionCount]);
    expressionCount++;
    numRegisters++;
    return d;
    
}

char* parseConditional(char* c) {
    c = skipSpaces(c);
    char* buf = c;
    if (*c != '(') {
        printf("Expected '(' in: if %s\n", buf);
        exitAndFree(EXIT_FAILURE);
    }
    //Register
    c++;
    StringTree* leaf = getRegisterOrInstruction(c);
    if (leaf == NULL || leaf->t != REGISTER_T) {
        printf("Not a Register: if %s\n", buf);
        exitAndFree(EXIT_FAILURE);
    }
    int regID = leaf->id;
    //Operator
    c += leaf->strLength;
    c = skipSpaces(c);
    leaf = getOperator(c);
    if (leaf->t != OPERATOR_T) {
        printf("Expected an operator in %s\n", buf);
        exitAndFree(EXIT_FAILURE);
    }
    int comparisonID = leaf->id;
    //0
    c += leaf->strLength;
    c = skipSpaces(c);
    if (*c != '0') {
        printf("Expected '0' in: if %s\n", buf);
        exitAndFree(EXIT_FAILURE);
    }
    c++;
    c = skipSpaces(c);
    if (*c != ')') {
        printf("Expected ')' in: if %s\n", buf);
        exitAndFree(EXIT_FAILURE);
    }

    expressionList[expressionCount].eType = INSTRUCTION;
    switch (comparisonID) {
        case ID_EQUAL:
            expressionList[expressionCount].eVal.instruction.iType = IF_ZERO;
            break;
        case ID_UNEQUAL:
            expressionList[expressionCount].eVal.instruction.iType = IF_NOT_ZERO;
            break;
        default:
            printf("Invalid operator in: if %s\n", buf);
            exitAndFree(EXIT_FAILURE);
            break;
    }
    expressionList[expressionCount].eVal.instruction.param.regID = regID;
    expressionList[expressionCount].eVal.instruction.address = numInstructions;
    instructions[numInstructions] = &(expressionList[expressionCount]);
    expressionCount++;
    numInstructions++;

    c++;
    c = skipSpaces(c);
    leaf = getRegisterOrInstruction(c);
    switch (leaf->t)
    {
    case REGISTER_T:
    case INSTRUCTION_T:
        parseExpression(c);
        break;
    default:
        printf("Invalid Instruction after Conditional: %s\n", c);
        exitAndFree(EXIT_FAILURE);
        break;
    }
    return "";

}

void parseExpression(char* e) {
    char* buf = e;
    if (isNumerical[(int) *e]) {
        printf("Invalid expression: %s\n", buf);
        exitAndFree(EXIT_FAILURE);

    }
    StringTree* leaf;
    //Label
    if (isALPHAnumerical[(int) *e] && *e != '_') {
        declareLabel(e);
        return;
    }
    
    leaf = getRegisterOrInstruction(e);
    if (leaf == NULL) exitAndFree(EXIT_FAILURE);
    e += leaf->strLength;

    switch (leaf->t) {
        case REGISTER_T:
            e = parseInDecrement(e, leaf->id);
            break;
        case INSTRUCTION_T:
            switch (leaf->id) {
                case ID_GOTO:
                    e = parseGoto(e);
                    break;
                case ID_HALT:
                    parseHalt();
                    break;
                default:
                    printf("Invalid Instruction ID\n");
                    exitAndFree(EXIT_FAILURE);
                    break;
            }
            break;
        case DECLARATION_T:
            e = parseDeclaration(e);
            break;
        case CONDITIONAL_T:
            e = parseConditional(e);
            break;
        default:
            printf("Invalid expression: %s\n", buf);
            break;
    }

    //make sure rest of expression is empty
    e = skipSpaces(e);
    if (*e != '\0') {
        printf("Expected ';' in: %s", buf);
        exitAndFree(EXIT_FAILURE);
    }

}

void printExpression(Expression* e) {
    switch (e->eType) {
        case LABEL:
            printf("Label points to Instruction %d\n", e->eVal.label.address);
            break;
        case DECLARATION:
            printf("Declaration declares register %d as %d\n", e->eVal.declaration.numReg, e->eVal.declaration.initVal);
            break;
        case CONDITIONAL:
        case INSTRUCTION:
            printf("%d| ", e->eVal.instruction.address);
            switch (e->eVal.instruction.iType) {
                case INCREMENT:
                    printf("increment register %d\n", e->eVal.instruction.param.regID);
                    break;
                case DECREMENT:
                    printf("decrement register %d\n", e->eVal.instruction.param.regID);
                    break;
                case GOTO:
                    printf("goto instruction with adress %d\n", e->eVal.instruction.param.jumpAdress);
                    break;
                case HALT:
                    printf("Halt\n");
                    break;
                case IF_ZERO:
                    printf("if (register %d == 0)\n", e->eVal.instruction.param.regID);
                    break;
                case IF_NOT_ZERO:
                    printf("if (register %d != 0)\n", e->eVal.instruction.param.regID);
                    break;
            }
            break;
    }
}

void getGotoAddresses() {
    for (int i = 0; i < numInstructions; i++) {
        if (instructions[i]->eVal.instruction.iType == GOTO) {
            instructions[i]->eVal.instruction.param.jumpAdress = readLabel(instructions[i]->eVal.instruction.param.labelName);
        }
    }
}

void parseExpressions() {
    for (int i = 0; i < numExpressions; i++) {
        parseExpression(expressions[i]);
    }
    getGotoAddresses();
}

void cleanParser() {
    cleanTree();
    free(instructions);
    instructions = NULL;
    free(declarations);
    declarations = NULL;
    free(conditionals);
    conditionals = NULL;
    free(expressionList);
    expressionList = NULL;
    free(labels);
    labels = NULL;
}

void exitAndFree(int exitSignal) {
    freeLines();
    cleanParser();
    cleanTranslation();
    exit(exitSignal);
}