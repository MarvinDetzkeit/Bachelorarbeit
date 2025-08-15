#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "parser.h"
#include "translate.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Expected name of code file as argument.\n");
        return 0;
    }

    scanLines(argv[1]);
    splitLines();

    initParser();
    //parse expressions
    parseExpressions();
    
    //translate into busy beaver machine
    translate();

    char nameBuf[100];
    strcpy(nameBuf, argv[1]);
    nameBuf[strlen(nameBuf)-2] = '\0';
    strcat(nameBuf, "json");
    writeMachineIntoFile(nameBuf);

    exitAndFree(EXIT_SUCCESS);
}