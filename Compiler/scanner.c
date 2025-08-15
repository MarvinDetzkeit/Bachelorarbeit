#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "stringTree.h"
#include "parser.h"

char** lines = NULL;
int numLines = 0;
char** expressions = NULL;
int numExpressions = 0;
int sizeExpressions;


/*
* Returns a pointer to the next line of the file
* Returns NULL if the end of the file is reached
*/

char* readLine(FILE* file) {
    int size = 100;
    int count = 0;
    char* line = malloc(size * sizeof(char));
    char ch;
    int iterations = 0;
    while ((ch = fgetc(file)) != EOF) {
        iterations++;
        if (ch == '\n') break;
        line[count] = ch;
        if (line[count] == '/') line[count] = '\0';
        count++;
        if (count == size) {
            size *= 2;
            line = realloc(line, size * sizeof(char));
        }
    }
    if (iterations == 0) {
        free(line);
        return NULL;
    }
    line[count] = '\0';
    return line;
}

/*
* Stores the file line by line in the array lines
*/
void scanLines(char* filename) {
    char buf[200];
    strcpy(buf, "_BusyCCode/");
    strcat(buf, filename);
    FILE* code = fopen(buf, "r");
    if (!code) {
        printf("Failed to open %s\n", filename);
        exitAndFree(EXIT_FAILURE);
    }

    int size = 500;
    lines = malloc(size * sizeof(char*));

    int line = 0;
    while ((lines[line] = readLine(code))) {
        line++;
        if (line == size) {
            size *= 2;
            lines = realloc(lines, size * sizeof(char*));
        }
    }
    numLines = line;
}

/*
* Frees the lines
*/
void freeLines() {
    if (lines != NULL) for (int i = 0; i < numLines; i++) free(lines[i]);
    free(lines);
    lines = NULL;
    free(expressions);
    expressions = NULL;
}

/*
* Splits a line into expressions
*/
void splitLine(char* line) {
    line = skipSpaces(line);
    char* buf = line;
    char splitsymbol;
    while (*line != '\0') {
        if (*line >= 'A' && *line <= 'Z') splitsymbol = ':';
        else splitsymbol = ';';
        while (*line != splitsymbol) {
            if (*line == '\0') {
                printf("Expected '%c' in: %s\n", splitsymbol, buf);
                exitAndFree(EXIT_FAILURE);
            }
            line++;
        }
        *line = '\0';
        line++;
        expressions[numExpressions] = buf;
        numExpressions++;
        if (numExpressions == sizeExpressions) {
            sizeExpressions *= 2;
            expressions = realloc(expressions, sizeExpressions * sizeof(char*));
        }
        line = skipSpaces(line);
        buf = line;
    }
    return;
}

/*
* Splits all lines into seperate expressions and stores them inside expressions
*/
void splitLines() {
    sizeExpressions = numLines;
    expressions = malloc(sizeExpressions * sizeof(char*));
    for (int i = 0; i < numLines; i++) {
        splitLine(lines[i]);
    }
}