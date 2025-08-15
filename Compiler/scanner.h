#ifndef SCANNER_H
#define SCANNER_H

extern int numLines;
extern char** lines;
extern char** expressions;
extern int numExpressions;

void scanLines(char* filename);
void freeLines();
void splitLines();


#endif