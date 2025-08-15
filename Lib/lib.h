#ifndef LIB_H
#define LIB_H

typedef struct{
    int id;
    int blankWrite;
    int blankShift;
    int blankState;
    int oneWrite;
    int oneShift;
    int oneState;
} State;

#define bool int
#define TRUE 1
#define FALSE 0

#define HALTING_STATE -1
#define ERROR_STATE -10

void printState(State* s);
char* copyString(char* dest, const char* s);

#endif