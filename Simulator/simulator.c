#include "simulator.h"

tapeType* tape = NULL; //Holds the tape
tapeType* pointer = NULL; //Points to current cell
size_t tapeSize = 50001;
int numStates;
State* machine = NULL; //Holds all states
State* currentState = NULL; //Points to current state

MachineCache machineCache; //Lazy Cache
int cacheSize;

//For looking up cacheIndex faster than the previous method with if else
int lookUpArray[3];
int* lookUpTable;

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Expected name of machine file as argument.\n");
        return 0;
    }

    init(argv[1], tapeSize);
    simulateMachine();
    cleanUp();
    return 0;
}

int init(char* machineName, int tapesize) {
    char machineFileName[100];
    strcpy(machineFileName, machineName);
    strcat(machineFileName, ".json");
    parseMachine(machineFileName);
    tape = calloc(tapesize, sizeof(tapeType));
    pointer = tape + (tapesize / 2);
    initCache();
    return 0;
}

void initCache() {
    cacheSize = (1 << COMPRESSIONSIZE) * numStates * 2;
    lookUpArray[0] = 0;
    lookUpArray[2] = cacheSize / 2;
    lookUpTable = lookUpArray+1;
    machineCache.cellCache = malloc(cacheSize * sizeof(tapeType));
    machineCache.stateCache = calloc(cacheSize, sizeof(State*));
    machineCache.shiftDirectionCache = malloc(cacheSize * sizeof(int));
    machineCache.simulatedStepsCache = malloc(cacheSize * sizeof(uint64_t));
    machineCache.diffWrittenOnesCache = malloc(cacheSize * sizeof(int));
}

void parseMachine(char* filename) {
    char buf[200];
    strcpy(buf, "python3 Lib/parse.py _machines/");
    strcat(buf, filename);
    system(buf);
    
    FILE* mfile = fopen("machine.txt", "r");
    char buffer[20];

    //get number of states
    fgets(buffer, 9, mfile);
    numStates = atoi(buffer); //atoi ignores '\n'

    machine = malloc(numStates * sizeof(State));
    currentState = machine;
    int* trickster = (int*) machine;

    //Copy file data into machine array
    for (int i = 0; i < numStates * (sizeof(State) / sizeof(int)); i++) {
        fgets(buffer, 9, mfile);
        *trickster = atoi(buffer);
        trickster++;
    }

    fclose(mfile);
    system("rm machine.txt");
}

/*
* Calculates the machine's behavior on a cell of size COMPRESSIONSIZE
* writes the result into the cache
*
* Returns TRUE if infinite loop is detected
*/
bool calculateCell(tapeType cell, int headPosition, int cacheIndex) {
    bool infinite = FALSE;
    register uint64_t steps = 0;
    int diffWrittenOnes = 0;
    int size = COMPRESSIONSIZE;
    int cellArray[COMPRESSIONSIZE];
    // put cell into array for simulation
    for (int i = 0; i < size; i++) {
        cellArray[i] = (cell >> (size - i - 1)) & 1;
        diffWrittenOnes -= cellArray[i];
    }
    // simulation on cellArray
    while ((headPosition >= 0) && (headPosition < size) && (currentState >= machine )) {
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
        //infinite Loop detection might break for a big COMPRESSIONSIZE because of runtime and/or integer overflow
        if (steps > (numStates * COMPRESSIONSIZE * (1 << COMPRESSIONSIZE))) {
            infinite = TRUE;
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
    machineCache.cellCache[cacheIndex] = cell;
    machineCache.stateCache[cacheIndex] = currentState;
    machineCache.shiftDirectionCache[cacheIndex] = shiftDirection;
    machineCache.simulatedStepsCache[cacheIndex] = steps;
    machineCache.diffWrittenOnesCache[cacheIndex] = diffWrittenOnes;
    return infinite;
}

int getCacheIndexForCell(tapeType cell, int lastShiftDirection, int stateID) {
    int cacheIndex;
    cacheIndex = lookUpTable[lastShiftDirection];
    cacheIndex += stateID * (1 << COMPRESSIONSIZE);
    cacheIndex += cell;
    return cacheIndex;
}

void logExecutionTime(double exTime) {
    int tapeTypeSize = sizeof(tapeType) * 8;
    int blockSize = COMPRESSIONSIZE;
    char filename[100];

    snprintf(filename, sizeof(filename), "Benchmarks/benchmarkSimLog%d-%d.txt", blockSize, tapeTypeSize);
    FILE* f = fopen(filename, "a");

    if (!f) {
        perror("Error opening file");
        exit(1);
    }
    
    fprintf(f, "%lf\n", exTime);
    fclose(f);
}

/*
* Simulates the machine using Tape Compression
* Uses Lazy caching for the compressed cells
*/
void simulateMachine() {
    int numLookups = 0;
    int numCalculations = 0;
    register uint64_t steps = 0;
    int writtenOnes = 0;
    int lastShift = 1; //start as if you shifted right in the previos step
    struct timespec start, end;
    register State* newState;
    int cacheIndex;
    unsigned int i = 0;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    // simulate machine
    while (TRUE) {
        i++;
        tapeType cell = *pointer;
        cacheIndex = getCacheIndexForCell(cell, lastShift, currentState->id);
        // simulate on cell if not cached
        if (!(newState = machineCache.stateCache[cacheIndex])) {
            numCalculations++;
            int headposition;
            if (lastShift == 1) headposition = 0;
            else headposition = COMPRESSIONSIZE - 1;
            if (calculateCell(cell, headposition, cacheIndex)) {
                printf("Infinite loop detected!\n");
                break;
            }
            newState = machineCache.stateCache[cacheIndex];
            if ((uintptr_t)newState < (uintptr_t)machine) break;
        }
        // read result from cache
        numLookups++;
        *pointer = machineCache.cellCache[cacheIndex];
        currentState = newState;
        lastShift = machineCache.shiftDirectionCache[cacheIndex];
        pointer += lastShift;
        steps += machineCache.simulatedStepsCache[cacheIndex];
        writtenOnes += machineCache.diffWrittenOnesCache[cacheIndex];
        
        if ((i & 1) && (((uintptr_t)pointer >= ((uintptr_t)(tape + tapeSize))) || ((uintptr_t)pointer < (uintptr_t)tape))) growTape();
    }

    numLookups++;
    *pointer = machineCache.cellCache[cacheIndex];
    currentState = newState;
    lastShift = machineCache.shiftDirectionCache[cacheIndex];
    pointer += lastShift;
    steps += machineCache.simulatedStepsCache[cacheIndex];
    writtenOnes += machineCache.diffWrittenOnesCache[cacheIndex];

    numLookups -= numCalculations;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    double exeutionTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Simulation took %lf seconds.\n", exeutionTime);
    printf("Machine ran for %llu steps.\n", steps);
    printf("Machine wrote %d 1s on the tape.\n", writtenOnes);
    printf("%d cache lookups, %d individual calculations, Lookup Ratio: %lf%%\n",
        numLookups,
        numCalculations,
        (double) (numLookups * 100.0) / ((double) numLookups + (double) numCalculations));
    printf("Fill Rate of cache: %lf%%\n", (double) (numCalculations * 100.0) / (double) cacheSize);
    if (currentState == machine + ERROR_STATE) printf("Machine halted because of an error.\n");
    //logExecutionTime(exeutionTime * 1000.0);
}

void growTape() {
    tapeType* buf = tape;
    if ((uintptr_t)pointer > ((uintptr_t)tape + tapeSize)) { //machine went too far right
        tape = realloc(tape, (2 * tapeSize - 1) * sizeof(tapeType));
        if (!tape) {
            printf("Error during growing of tape!\n");
            cleanUp();
            exit(1);
        }
        bzero(tape + tapeSize, (tapeSize - 1) * sizeof(tapeType));
        pointer = tape + tapeSize;
    }
    else if ((uintptr_t)pointer < (uintptr_t)tape) { //machine went too far left
        tape = malloc((2 * tapeSize - 1) * sizeof(tapeType));
        if (!tape) {
            printf("Error during growing of tape!\n");
            cleanUp();
            exit(1);
        }
        bzero(tape, (tapeSize - 1) * sizeof(tapeType));
        memcpy(tape + tapeSize - 1, buf, tapeSize * sizeof(tapeType));
        free(buf);
        pointer = tape + tapeSize - 2;
    }
    else {
        printf("Function growTape shouldn't have been called!\n");
        cleanUp();
        exit(1);
    }
    tapeSize = (2 * tapeSize) - 1;
}

void cleanCache() {
    free(machineCache.cellCache);
    free(machineCache.stateCache);
    free(machineCache.shiftDirectionCache);
    free(machineCache.simulatedStepsCache);
    free(machineCache.diffWrittenOnesCache);
}

void cleanUp() {
    free(tape);
    free(machine);
    cleanCache();
}