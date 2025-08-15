#ifndef STRINGTREE_H
#define STRINGTREE_H

#include "lang.h"

#define NUMCHARS 67

extern int lookUpIndex[256];
extern int isNumerical[256];
extern int isAlphanumerical[256];
extern int isALPHAnumerical[256];
extern int isMathSymbol[256];

typedef enum {
    NOTHING_T = 0,
    REGISTER_T,
    LABEL_T,
    INSTRUCTION_T,
    DECLARATION_T,
    CONDITIONAL_T,
    OPERATOR_T,
} Type;


typedef struct StringTree {
    struct StringTree** leaves;
    Type t;
    int id;
    int strLength;
} StringTree;

char* skipSpaces(char* s);
int getIndex(char c);
void initTables();
void initTree();
void cleanTree();
void reallocLeaves();
int addToTree(char* s, Type t, int id);
char* addRegisterToTree(char* r);
char* addLabelToTree(char* l);
StringTree* getLeaf(char* s);
StringTree* getRegisterOrInstruction(char* ri);
StringTree* getLabel(char* l);
StringTree* getOperator(char* o);

#endif