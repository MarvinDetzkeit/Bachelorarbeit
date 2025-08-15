#include <stdio.h>
#include "lib.h"

void printState(State* s) {
    printf("id: %d\tblankWrite: %d\tblankShift: %d\tblankState: %d\toneWrite: %d\toneShift: %d\toneState: %d\n",
    s->id,
    s->blankWrite,
    s->blankShift,
    s->blankState,
    s->oneWrite,
    s->oneShift,
    s->oneState);
}

/*
* Copies the string s to dest.
* Returns a pointer null byte ending the string at dest.
*/
char* copyString(char* dest, const char* s) {
    while ((*dest = *s)) {
        dest++;
        s++;
    }
    return dest;
}