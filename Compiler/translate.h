#ifndef TRANSLATE_H
#define TRANSLATE_H

#define MACHINE_FETCH_ID -1
#define MACHINE_RET_ID -2
#define MACHINE_DECREMENT_IP_ID -3

#define R 1
#define L -1

void translate();
void writeMachineIntoFile(char* outputFileName);
void cleanTranslation();

#endif