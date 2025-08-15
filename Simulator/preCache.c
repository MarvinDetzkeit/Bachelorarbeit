#include "simulator.h"

MachineCache preCache8Bit; //Precomputed 8 Bit Cache
int preCacheSizeHalf;

void initPreCache() {
    int preCacheSize = 256 * numStates * 2;
    preCacheSizeHalf = preCacheSize / 2;
    preCache8Bit.cellCache = malloc(preCacheSize * sizeof(tapeType));
    preCache8Bit.stateCache = malloc(preCacheSize * sizeof(State*));
    preCache8Bit.shiftDirectionCache = malloc(preCacheSize * sizeof(int));
    preCache8Bit.simulatedStepsCache = malloc(preCacheSize * sizeof(int64_t));
    preCache8Bit.diffWrittenOnesCache = malloc(preCacheSize * sizeof(int));
}

void cleanPreCache() {
    free(preCache8Bit.cellCache);
    free(preCache8Bit.stateCache);
    free(preCache8Bit.shiftDirectionCache);
    free(preCache8Bit.simulatedStepsCache);
    free(preCache8Bit.diffWrittenOnesCache);
}

int getPreCacheIndex(tapeType cell, int lastShiftDirection, int stateID) {
    int cacheIndex;
    if (lastShiftDirection == 1) cacheIndex = 0;
    else cacheIndex = preCacheSizeHalf;
    cacheIndex += stateID * 256;
    cacheIndex += cell;
    return cacheIndex;
}

bool calculateCellWithPreCache(tapeType cell, int headPosition, int cacheIndex) {
    bool infinite = FALSE;
    register uint64_t steps = 0;
    int diffWrittenOnes = 0;
    int lastShiftDirection;
    if (headPosition == 0) lastShiftDirection = 1;
    else lastShiftDirection = -1;
    headPosition >>= 3;
    int miniTape[2];
    miniTape[0] = (0xff00 & cell) >> 8;
    miniTape[1] = 0x00ff & cell;
    int preCacheIndex;

    while ((headPosition >= 0) && (headPosition < 2) && (currentState != (machine - 1))) {
        preCacheIndex = getPreCacheIndex(miniTape[headPosition], lastShiftDirection, currentState->id);
        miniTape[headPosition] = preCache8Bit.cellCache[preCacheIndex];
        steps += preCache8Bit.simulatedStepsCache[preCacheIndex];
        diffWrittenOnes += preCache8Bit.diffWrittenOnesCache[preCacheIndex];
        currentState = preCache8Bit.stateCache[preCacheIndex];
        lastShiftDirection = preCache8Bit.shiftDirectionCache[preCacheIndex];
        headPosition += lastShiftDirection;
        
        if ((currentState == (machine - 2)) || (steps > (numStates * 16 * (1 << 16)))) {
            infinite = TRUE;
            break;
        }
        
    }

    cell = (miniTape[0] << 8) | miniTape[1];
    
    machineCache.cellCache[cacheIndex] = cell;
    machineCache.stateCache[cacheIndex] = currentState;
    machineCache.shiftDirectionCache[cacheIndex] = lastShiftDirection;
    machineCache.simulatedStepsCache[cacheIndex] = steps;
    machineCache.diffWrittenOnesCache[cacheIndex] = diffWrittenOnes;
    return infinite;
    
}

void fillPreCache() {
    for (int cell = 0; cell < 256; cell++) {
        for (int state = 0; state < numStates; state++) {
            calculateCell8Bit(cell, 0, getPreCacheIndex(cell, 1, state), machine + state);
            calculateCell8Bit(cell, 7, getPreCacheIndex(cell, -1, state), machine + state);
        }
    }
}

bool calculateCell8Bit(tapeType cell, int headPosition, int cacheIndex, State* currentState) {
    bool infinite = FALSE;
    register uint64_t steps = 0;
    int diffWrittenOnes = 0;
    int size = 8;
    int cellArray[size];
    // put cell into array for simulation
    for (int i = 0; i < size; i++) {
        cellArray[i] = (cell >> (size - i - 1)) & 1;
        diffWrittenOnes -= cellArray[i];
    }
    // simulation on cellArray
    while ((headPosition >= 0) && (headPosition < size)) {
        if (cellArray[headPosition]) {
            cellArray[headPosition] = currentState->oneWrite;
            headPosition += currentState->oneShift;
            currentState = machine + currentState->oneState;
        }
        else {
            cellArray[headPosition] = currentState->blankWrite;
            headPosition += currentState->blankShift;
            currentState = machine + currentState->blankState;
        }
        steps++;
        if (currentState == (machine - 1)) break; // machine switched to halting state
        if (steps > (numStates * 8 * 256) ) {
            preCache8Bit.stateCache[cacheIndex] = machine-2;
            break;
        }
    }
    //calculate shift direction (-1 for left; 1 for right)
    int shiftDirection;
    if (headPosition < 0) shiftDirection = -1;
    else shiftDirection = 1;
    // write value of array back into cell
    cell = 0;
    for (int i = 0; i < size; i++) {
        cell = cell | (cellArray[i] << (size - i - 1));
        diffWrittenOnes += cellArray[i];
    }
    // write result into cache
    preCache8Bit.cellCache[cacheIndex] = cell;
    preCache8Bit.stateCache[cacheIndex] = currentState;
    preCache8Bit.shiftDirectionCache[cacheIndex] = shiftDirection;
    preCache8Bit.simulatedStepsCache[cacheIndex] = steps;
    preCache8Bit.diffWrittenOnesCache[cacheIndex] = diffWrittenOnes;
    return infinite;
}