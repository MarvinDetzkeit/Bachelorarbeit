#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "../Lib/lib.h"


#define tapeType uint32_t
#define COMPRESSIONSIZE 20


typedef struct {
    tapeType* cellCache;
    State** stateCache;
    int* shiftDirectionCache;
    uint64_t* simulatedStepsCache;
    int* diffWrittenOnesCache;
} MachineCache;

/*
* Variable Declarations
*/
extern tapeType* tape; //Holds the tape
extern tapeType* pointer; //Points to current cell
extern int numStates;
extern State* machine; //Holds all states
extern State* currentState; //Points to current state

extern MachineCache machineCache; //Lazy 16 Bit Cache
extern int cacheSizeHalf; // cacheSize / 2 cached ;)

extern MachineCache preCache8Bit; //Precomputed 8 Bit Cache
extern int preCacheSizeHalf;

void printState(State* s);
int init(char* machineName, int tapesize);
void parseMachine(char* filename);

bool calculateCell(tapeType cell, int headPosition, int cacheIndex);
int getCacheIndexForCell(tapeType cell, int lastShiftDirection, int stateID);
void simulateMachine();
void growTape();
void initCache();
void cleanCache();
void cleanUp();

/*
* Functions for 8 Bit Cache
*/
bool calculateCell8Bit(tapeType cell, int headPosition, int cacheIndex, State* currentState);
void fillPreCache();
int getPreCacheIndex(tapeType cell, int lastShiftDirection, int stateID);
void initPreCache();
void cleanPreCache();
bool calculateCellWithPreCache(tapeType cell, int headPosition, int cacheIndex);

#endif