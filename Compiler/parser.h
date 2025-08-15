#ifndef PARSER_H
#define PARSER_H

#include "lang.h"

extern Expression* expressionList;
extern int expressionCount; //number of parsed expressions

extern Expression** declarations;
extern int numRegisters;
extern Expression** conditionals;
extern int numConditionals;
extern Expression** instructions;
extern int numInstructions;
extern Expression** labels;
extern int numLabels;

void initParser();
void parseExpression(char* e);
void parseExpressions();
void printExpression(Expression* e);
void exitAndFree(int exitSignal);

#endif