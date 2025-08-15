#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stringTree.h"


StringTree* tree = NULL;
StringTree** leafArrays = NULL;
int numLeaves = 1;
int cap = 100;

// tables for looking up characteristics of chars
int lookUpIndex[256];
int isNumerical[256];
int isAlphanumerical[256];
int isALPHAnumerical[256];
int isMathSymbol[256];


char* skipSpaces(char* s) {
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    return s;
}

/*
* Returns the index for char c in the leaves array
* Returns -1 if the char is not allowed
*/
int getIndex(char c) {
    return lookUpIndex[(int) c];
}

/*
* Initialises the lookup tables
*/
void initTables() {
    for (int i = 0; i < 256; i++) lookUpIndex[i] = -1;
    for (int i = '0'; i <= '9'; i++) {
        lookUpIndex[i] = i - '0';
        isNumerical[i] = 1;
        isAlphanumerical[i] = 1;
        isALPHAnumerical[i] = 1;
    }
    for (int i = 'a'; i <= 'z'; i++) {
        lookUpIndex[i] = (i - 'a') + 10;
        isAlphanumerical[i] = 1;
    }
    for (int i = 'A'; i <= 'Z'; i++) {
        lookUpIndex[i] = (i - 'A') + 36;
        isAlphanumerical[i] = 1;
        isALPHAnumerical[i] = 1;
    }
    isMathSymbol['!'] = 1;
    isMathSymbol['='] = 1;
    isMathSymbol['+'] = 1;
    isMathSymbol['-'] = 1;
    isALPHAnumerical['_'] = 1;
    lookUpIndex['_'] = 62;
    lookUpIndex['+'] = 63;
    lookUpIndex['-'] = 64;
    lookUpIndex['='] = 65;
    lookUpIndex['!'] = 66;
}

void initTree() {
    tree = malloc(cap * sizeof(StringTree));
    leafArrays = calloc(cap * NUMCHARS, sizeof(StringTree*));
    tree->leaves =  leafArrays;
    initTables();
    addToTree("uint", DECLARATION_T, -1);
    addToTree("if", CONDITIONAL_T, -1);
    addToTree("goto", INSTRUCTION_T, ID_GOTO);
    addToTree("halt", INSTRUCTION_T, ID_HALT);
    addToTree("++", OPERATOR_T, ID_INC);
    addToTree("--", OPERATOR_T, ID_DEC);
    addToTree("==", OPERATOR_T, ID_EQUAL);
    addToTree("!=", OPERATOR_T, ID_UNEQUAL);
    addToTree("=", OPERATOR_T, ID_ASSIGN);
}

void cleanTree() {
    free(tree);
    tree = NULL;
    free(leafArrays);
    leafArrays = NULL;
}

void reallocLeaves() {
    cap *= 16;
    StringTree* oldTree = tree;
    tree = realloc(tree, cap * sizeof(StringTree));
    StringTree** oldLeaves = leafArrays;
    leafArrays = realloc(leafArrays, cap * NUMCHARS * sizeof(StringTree*));
    memset(leafArrays+(numLeaves*NUMCHARS), 0, (cap - numLeaves) * NUMCHARS * sizeof(StringTree*));

    if ((leafArrays - oldLeaves) != 0) {
        for (int i = 0; i < numLeaves; i++) {
            tree[i].leaves += leafArrays - oldLeaves;
        }
    }

    if ((tree - oldTree) != 0) {
        for (int i = 0; i < numLeaves * NUMCHARS / 2; i++) {
            if(leafArrays[i]) {
                leafArrays[i] += tree - oldTree;
            }
        }
    }
}

/*
* Adds string s to the tree and assigns t and id
* Returns 0 if successful, -1 otherwise
*/
int addToTree(char* s, Type t, int id) {
    char* buf = s;
    int len = 0;
    StringTree* currentLeaf = tree;
    int index;
    while (*s != '\0') {
        index = getIndex(*s);
        if (index == -1) {
            printf("Invalid Character: %c\n", *s);
            return -1;
        }
        if (currentLeaf->leaves[index] == NULL) {
            currentLeaf->leaves[index] = tree + numLeaves;
            currentLeaf->leaves[index]->leaves = leafArrays + (NUMCHARS * numLeaves);
            currentLeaf->leaves[index]->t = NOTHING_T;
            numLeaves++;
            if (numLeaves == cap) reallocLeaves();
        }
        currentLeaf = currentLeaf->leaves[index];
        s++;
        len++;
    }
    if (currentLeaf->t != NOTHING_T) {
        printf("Identifier already in Use: %s\n", buf);
        return -1;
    }
    currentLeaf->t = t;
    currentLeaf->id = id;
    currentLeaf->strLength = len;
    return 0;
}

/*
* Adds a register identifier to the tree
* Returns a pointer to the next char after the identfier if successful, NULL otherwise
*/
char* addRegisterToTree(char* r) {
    static int numRegisters = 0;
    int len = 0;
    r = skipSpaces(r);
    char* buf = r;
    int index = getIndex(*r);
    if (index < 10 || index > 35) {
        printf("Identifier must start with small alphabetical letter: %s\n", buf);
        return NULL;
    }

    StringTree* currentLeaf = tree;
    while (isAlphanumerical[(int) *r]) {
        index = getIndex(*r);
        if (currentLeaf->leaves[index] == NULL) {
            currentLeaf->leaves[index] = tree + numLeaves;
            currentLeaf->leaves[index]->leaves = leafArrays + (NUMCHARS * numLeaves);
            currentLeaf->leaves[index]->t = NOTHING_T;
            numLeaves++;
            if (numLeaves == cap) reallocLeaves();
        }
        currentLeaf = currentLeaf->leaves[index];
        r++;
        len++;
    }
    if (currentLeaf->t != NOTHING_T) {
        printf("Identifier already in Use: %s\n", buf);
        return NULL;
    }
    currentLeaf->t = REGISTER_T;
    currentLeaf->id = numRegisters;
    currentLeaf->strLength = len;
    numRegisters++;
    return r;

}

/*
* Adds a label identifier to the tree
* Returns a pointer to the next char after the identfier if successful, NULL otherwise
*/
char* addLabelToTree(char* l) {
    static int numLabels = 0;
    int len = 0;
    l = skipSpaces(l);
    char* buf = l;
    int index = getIndex(*l);
    if (index < 36 || index > 61) {
        printf("Labels must start with an upper case letter: %s\n", buf);
        return NULL;
    }

    StringTree* currentLeaf = tree;
    while (isALPHAnumerical[(int) *l]) {
        index = getIndex(*l);
        if (currentLeaf->leaves[index] == NULL) {
            currentLeaf->leaves[index] = tree + numLeaves;
            currentLeaf->leaves[index]->leaves = leafArrays + (NUMCHARS * numLeaves);
            currentLeaf->leaves[index]->t = NOTHING_T;
            numLeaves++;
            if (numLeaves == cap) reallocLeaves();
        }
        currentLeaf = currentLeaf->leaves[index];
        l++;
        len++;
    }
    if (currentLeaf->t != NOTHING_T) {
        printf("Label already in Use: %s\n", buf);
        return NULL;
    }
    currentLeaf->t = LABEL_T;
    currentLeaf->id = numLabels;
    currentLeaf->strLength = len;
    numLabels++;
    return l;

}

StringTree* getLeaf(char* s) {
    char* buf = s;
    StringTree* currentLeaf = tree;
    int index;
    while (*s != '\0') {
        index = getIndex(*s);
        if (index == -1) {
            printf("Invalid Character: %c\n", *s);
            return NULL;
        }
        if ((currentLeaf = currentLeaf->leaves[index]) == NULL) {
            printf("Unknown Identifier: %s\n", buf);
            return NULL;
        }
        s++;
    }
    return currentLeaf;
}

StringTree* getRegisterOrInstruction(char* ri) {
    char* buf = ri;
    StringTree* currentLeaf = tree;
    int index;
    while (isAlphanumerical[(int) *ri]) {
        index = getIndex(*ri);
        if ((currentLeaf = currentLeaf->leaves[index]) == NULL) {
            printf("Unknown identifier: %s\n", buf);
            return NULL;
        }
        ri++;
    }
    return currentLeaf;
}

StringTree* getLabel(char* l) {
    char* buf = l;
    StringTree* currentLeaf = tree;
    int index;
    while (isALPHAnumerical[(int) *l]) {
        index = getIndex(*l);
        if ((currentLeaf = currentLeaf->leaves[index]) == NULL) {
            printf("Unknown Label: %s\n", buf);
            return NULL;
        }
        l++;
    }
    return currentLeaf;
}

StringTree* getOperator(char* o) {
    char* buf = o;
    StringTree* currentLeaf = tree;
    int index;
    while (isMathSymbol[(int) *o]) {
        index = getIndex(*o);
        if ((currentLeaf = currentLeaf->leaves[index]) == NULL) {
            printf("Unknown Operator: %s\n", buf);
            
            return NULL;
        }
        o++;
    }
    return currentLeaf;
}
