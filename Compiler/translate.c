#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Lib/lib.h"
#include "parser.h"
#include "translate.h"

State* machine = NULL;
int machineSize = 100;
int offset = 0;

char* negativeValueNames = NULL;
char* stateNames;
int maxNameLen = 30;

//Submachine offsets

//Dynamic submachines
int initTapeOffset; //must be state 0
int fetchInstructionOffset; //must be known before parsing subMachines
int addressRegisterIncrOffset;
int addressRegisterDecrOffset;
int addressRegisterIfZeroOffset;
int addressRegisterNotZeroOffset;
int positiveJumpSequenceOffset;
int negativeJumpSequenceOffset;
int negativeJumpAdressingOffset;

//Static submachines
int returnMachineOffset;
int sizeReturnMachine;

int incrementIP_MachineOffset;
int sizeIncrementIP_Machine;

int incrementMachineOffset;
int sizeIncrementMachine;

int decrementMachineOffset;
int sizeDecrementMachine;

int ifZeroMachineOffset;
int sizeIfZeroMachine;

int ifNotZeroMachineOffset;
int sizeIfNotZeroMachine;

//Holds reachability of states
bool* reachable = NULL;
int numOfStates = 0;

/*
* Parses a machine and stores it at position
* Replaces the negative states in the submachine with corresponding submachine states
* Returns the number of states of that machine
*/
int parseSubmachine(char* name, int subMachine) {
    State* position = machine + subMachine;
    char buf[200];
    strcpy(buf, "python3 Lib/parse.py _machines/compiler/");
    strcat(buf, name);
    system(buf);

    strcpy(buf, name);
    buf[strlen(buf) - 5] = '\0'; //remove ".json" from name

    FILE* mfile = fopen("machine.txt", "r");
    char buffer[20];

    //get number of states
    fgets(buffer, 9, mfile);
    int num = atoi(buffer); //atoi ignores '\n'

    int* trickster = (int*) position;

    //Copy file data into machine array
    for (int i = 0; i < num * (sizeof(State) / sizeof(int)); i++) {
        fgets(buffer, 9, mfile);
        *trickster = atoi(buffer);
        trickster++;
    }
    //Add state offsets and replace negative state IDs with correct submachine
    int subMachineOffset = offset;
    for (int i = 0; i < num; i++) {
        switch (position[i].blankState)
        {
        case MACHINE_FETCH_ID:
            position[i].blankState = 1;
            break;
        case MACHINE_RET_ID:
            position[i].blankState = 2;
            break;
        case MACHINE_DECREMENT_IP_ID:
            position[i].blankState = 2 + sizeReturnMachine;
            break;
        case ERROR_STATE:
            break;
        default:
            position[i].blankState += subMachineOffset;
            break;
        }
        switch (position[i].oneState)
        {
        case MACHINE_FETCH_ID:
            position[i].oneState = 1;
            break;
        case MACHINE_RET_ID:
            position[i].oneState = 2;
            break;
        case MACHINE_DECREMENT_IP_ID:
            position[i].oneState = 2 + sizeReturnMachine;
            break;
        case ERROR_STATE:
            break;
        default:
            position[i].oneState += subMachineOffset;
            break;
        }
        position[i].id += subMachineOffset;

        char* posInStateNames = copyString(stateNames + (offset * maxNameLen), buf);
        sprintf(posInStateNames, "%d", i);
        offset++;
    }

    fclose(mfile);
    system("rm machine.txt");

    return num;
}

/*
* Reallocs the machine and the state names if the buffer is full.
*/
void reallocMachine() {
    if (offset < machineSize) return;

    machineSize *= 2;
    machine = realloc(machine, machineSize * sizeof(State));
    negativeValueNames = realloc(negativeValueNames, (-ERROR_STATE * maxNameLen ) + (machineSize * maxNameLen * sizeof(char)));
    stateNames = negativeValueNames + (-ERROR_STATE * maxNameLen );

}

void initTranslation() {
    machine = malloc(machineSize * sizeof(State));
    negativeValueNames = malloc((-ERROR_STATE * maxNameLen ) + (machineSize * maxNameLen * sizeof(char)));
    strcpy(negativeValueNames, "ERROR_STATE");
    strcpy(negativeValueNames + ((-ERROR_STATE + HALTING_STATE) * maxNameLen), "HALT");
    stateNames = negativeValueNames + (-ERROR_STATE * maxNameLen );

    initTapeOffset = 0;
    strcpy(stateNames, "initIP");
    fetchInstructionOffset = initTapeOffset+1;
    strcpy(stateNames + maxNameLen, "fetch0");

    //Parse sub machines
    returnMachineOffset = fetchInstructionOffset+1;
    offset = 2;
    sizeReturnMachine = parseSubmachine("return.json", returnMachineOffset);

    incrementIP_MachineOffset = returnMachineOffset + sizeReturnMachine;
    sizeIncrementIP_Machine = parseSubmachine("incrementIP.json", incrementIP_MachineOffset);

    incrementMachineOffset = incrementIP_MachineOffset + sizeIncrementIP_Machine;
    sizeIncrementMachine = parseSubmachine("increment.json", incrementMachineOffset);

    decrementMachineOffset = incrementMachineOffset + sizeIncrementMachine;
    sizeDecrementMachine = parseSubmachine("decrement.json", decrementMachineOffset);

    ifZeroMachineOffset = decrementMachineOffset + sizeDecrementMachine;
    sizeIfZeroMachine = parseSubmachine("ifZero.json", ifZeroMachineOffset);

    ifNotZeroMachineOffset = ifZeroMachineOffset + sizeIfZeroMachine;
    sizeIfNotZeroMachine = parseSubmachine("ifNotZero.json", ifNotZeroMachineOffset);

}

/*
* Creates the submachine that initialises the tape: Writes IP and registers with initialized values on tape
*/
void createInitTape() {
    int numInitStates = 1;
    machine[0].id = 0;
    machine[0].blankWrite = 1;
    machine[0].blankShift = R;
    machine[0].blankState = offset;
    //Set to error state if tape is not zeroed out
    machine[0].oneWrite = 1;
    machine[0].oneShift = L;
    machine[0].oneState = ERROR_STATE;

    //Write IP on tape
    for (int i = 0; FALSE; i++) {
        machine[offset].id = offset;
        machine[offset].blankWrite = 1;
        machine[offset].blankShift = R;
        machine[offset].blankState = offset+1;
        //Set to error state if tape is not zeroed out
        machine[offset].oneWrite = 1;
        machine[offset].oneShift = L;
        machine[offset].oneState = ERROR_STATE;

        char* posInStateNames = copyString(stateNames + (offset * maxNameLen), "InitIP");
        sprintf(posInStateNames, "%d", numInitStates);
        numInitStates++;
        offset++;
        reallocMachine();
    }

    //Write registers on tape
    for (int i = 0; i < numRegisters; i++) {
        //Splitting Zero
        machine[offset].id = offset;
        machine[offset].blankWrite = 0;
        machine[offset].blankShift = R;
        machine[offset].blankState = offset+1;
        //Set to error state if tape is not zeroed out
        machine[offset].oneWrite = 0;
        machine[offset].oneShift = L;
        machine[offset].oneState = ERROR_STATE;

        char* posInStateNames = copyString(stateNames + (offset * maxNameLen), "SplittingZero");
        sprintf(posInStateNames, "%d", i);
        numInitStates++;
        offset++;
        reallocMachine();

        for (int j = 0; j <= declarations[i]->eVal.declaration.initVal; j++) {
            machine[offset].id = offset;
            //Write initval+1 Ones
            machine[offset].blankWrite = 1;
            machine[offset].blankShift = R;
            machine[offset].blankState = offset+1;
            //Set to error state if tape is not zeroed out
            machine[offset].oneWrite = 1;
            machine[offset].oneShift = L;
            machine[offset].oneState = ERROR_STATE;

            posInStateNames = copyString(stateNames + (offset * maxNameLen), "InitReg");
            sprintf(posInStateNames, "%d-%d", i, j);
            numInitStates++;
            offset++;
            reallocMachine();
        }
    }
    //After initialisation, go to return
    if (numRegisters > 0) machine[offset-1].blankState = 2;
    else machine[0].blankState = 2;
    
}

/*
* Creates a submachine that adresses a register for an instruction
* To adress register i-1 for an instruction, use adressInstruction...[i] (zero indexed)
* register 0 is automatically addressed after fetch
*/
void createAddressSubmachine(int instructionSubmachineOffset, int numRegistersToAddress, int* adressMachinePosition, char* name) {
    *adressMachinePosition = offset;
    //[instructionMachine] <- address1 <- address2 <- ...
    machine[offset].id = offset;
    machine[offset].blankWrite = 0;
    machine[offset].blankShift = R;
    machine[offset].blankState = instructionSubmachineOffset;
    machine[offset].oneWrite = 1;
    machine[offset].oneShift = R;
    machine[offset].oneState = offset;

    char* posInStateNames = copyString(stateNames + (offset * maxNameLen), name);
    sprintf(posInStateNames, "%d", 1);
    offset++;
    reallocMachine();

    for (int i = 2; i < numRegistersToAddress; i++) {
        machine[offset].id = offset;
        machine[offset].blankWrite = 0;
        machine[offset].blankShift = R;
        machine[offset].blankState = offset-1;
        machine[offset].oneWrite = 1;
        machine[offset].oneShift = R;
        machine[offset].oneState = offset;

        posInStateNames = copyString(stateNames + (offset * maxNameLen), name);
        sprintf(posInStateNames, "%d", i);
        offset++;
        reallocMachine();
    }
}

int max(int a, int b) {
    if (a > b) return a;
    else return b;
}

/*
* Creates the sequences to modify IP for realizing goto
*/
void createGotoSequences() {
    int maxNegativeJump = -1;
    int maxPositiveJump = -1;
    int addressGoto;
    int currentAddress;
    int jumpAdress;
    int relativeJump;
    //determine max jump lengths
    for (int i = 0; i < numInstructions; i++) {
        if (instructions[i]->eVal.instruction.iType != GOTO) continue;
        addressGoto = instructions[i]->eVal.instruction.address;
        currentAddress = addressGoto + 1; //fetch increments IP
        jumpAdress = instructions[i]->eVal.instruction.param.jumpAdress;
        relativeJump = jumpAdress - currentAddress;
        if (relativeJump > 0) maxPositiveJump = max(maxPositiveJump, relativeJump);
        else maxNegativeJump = max(maxNegativeJump, - relativeJump);
    }

    //machine[positiveJumpSequenceOffset + x] increases IP by x+1
    if (maxPositiveJump > 0) {
        positiveJumpSequenceOffset = offset;
        for (int i = 0; i < maxPositiveJump; i++) {
            machine[offset].id = offset;
            machine[offset].blankWrite = 1;
            machine[offset].blankShift = L;
            machine[offset].blankState = offset-1;
            machine[offset].oneWrite = 1;
            machine[offset].oneShift = L;
            machine[offset].oneState = offset;

            char* posInStateNames = copyString(stateNames + (offset * maxNameLen), "posGotoSeq");
            sprintf(posInStateNames, "%d", i+1);
            offset++;
            reallocMachine();
        }
        machine[positiveJumpSequenceOffset].blankState = fetchInstructionOffset;
    }

    //machine[negativeJumpSequenceOffset + x] decreases IP by x and goes into fetch without fetch increasing IP
    if (maxNegativeJump > 0) {
        negativeJumpSequenceOffset = offset;
        for (int i = 0; i < maxNegativeJump; i++) {
            machine[offset].id = offset;
            machine[offset].blankWrite = 0;
            machine[offset].blankShift = L;
            machine[offset].blankState = ERROR_STATE;
            machine[offset].oneWrite = 0;
            machine[offset].oneShift = R;
            machine[offset].oneState = offset-1;

            char* posInStateNames = copyString(stateNames + (offset * maxNameLen), "negGotoSeq");
            sprintf(posInStateNames, "%d", i+1);
            offset++;
            reallocMachine();
        }
        machine[negativeJumpSequenceOffset].oneWrite = 1;
        machine[negativeJumpSequenceOffset].oneState = fetchInstructionOffset;

        //machine[negativeJumpAdressingOffset + x] is a negative jump by x+1
        negativeJumpAdressingOffset = offset;
        for (int i = 0; i < maxNegativeJump; i++) {
            machine[offset].id = offset;
            machine[offset].blankWrite = 0;
            machine[offset].blankShift = R;
            machine[offset].blankState = negativeJumpSequenceOffset + i;
            machine[offset].oneWrite = 1;
            machine[offset].oneShift = L;
            machine[offset].oneState = offset;

            char* posInStateNames = copyString(stateNames + (offset * maxNameLen), "negGotoAddr");
            sprintf(posInStateNames, "%d", i+1);
            offset++;
            reallocMachine();
        }
    }

}

void createFetchInstruction() {
    //First fetch state increments IP and will not jump to any instruction
    machine[fetchInstructionOffset].id = fetchInstructionOffset;
    machine[fetchInstructionOffset].blankWrite = 1;
    machine[fetchInstructionOffset].blankShift = R;
    machine[fetchInstructionOffset].blankState = fetchInstructionOffset;
    machine[fetchInstructionOffset].oneWrite = 1;
    machine[fetchInstructionOffset].oneShift = R;
    machine[fetchInstructionOffset].oneState = offset;

    char* posInStateNames;
    for (int i = 0; i < numInstructions; i++) {
        machine[offset].id = offset;
        machine[offset].blankWrite = 0;
        machine[offset].blankShift = R;
        int reg;
        int currentAddress;
        int jumpAddress;
        int relativeJump;
        switch (instructions[i]->eVal.instruction.iType) {
            case INCREMENT:
                reg = instructions[i]->eVal.instruction.param.regID;
                if (reg == 0) machine[offset].blankState = incrementMachineOffset;
                else machine[offset].blankState = addressRegisterIncrOffset + reg - 1;
                break;
            case DECREMENT:
                reg = instructions[i]->eVal.instruction.param.regID;
                if (reg == 0) machine[offset].blankState = decrementMachineOffset;
                else machine[offset].blankState = addressRegisterDecrOffset + reg - 1;
                break;
            case GOTO:
                machine[offset].blankShift = L;
                currentAddress = instructions[i]->eVal.instruction.address + 1;
                jumpAddress = instructions[i]->eVal.instruction.param.jumpAdress;
                relativeJump = jumpAddress - currentAddress;
                if (relativeJump > 0) machine[offset].blankState = positiveJumpSequenceOffset + relativeJump-1;
                if (relativeJump < 0) machine[offset].blankState = negativeJumpAdressingOffset - relativeJump - 1;
                if (relativeJump == 0) machine[offset].blankState = returnMachineOffset;
                break;
            case HALT:
                machine[offset].blankState = HALTING_STATE;
                break;
            case IF_ZERO:
                reg = instructions[i]->eVal.instruction.param.regID;
                if (reg == 0) machine[offset].blankState = ifZeroMachineOffset;
                else machine[offset].blankState = addressRegisterIfZeroOffset + reg - 1;
                break;
            case IF_NOT_ZERO:
                reg = instructions[i]->eVal.instruction.param.regID;
                if (reg == 0) machine[offset].blankState = ifNotZeroMachineOffset;
                else machine[offset].blankState = addressRegisterNotZeroOffset + reg - 1;
                break;
        }
        machine[offset].oneWrite = 1;
        machine[offset].oneShift = R;
        machine[offset].oneState = offset+1;

        posInStateNames = copyString(stateNames + (offset * maxNameLen), "fetch");
        sprintf(posInStateNames, "%d", i+1);
        offset++;
        reallocMachine();
    }
    if (numInstructions > 0) machine[offset-1].oneState = HALTING_STATE;
    else machine[fetchInstructionOffset].oneState = HALTING_STATE;
}

/*
* Uses breadth search to mark all reachable states
*/
void markUnusedStates() {
    reachable = calloc(offset, sizeof(bool));
    int* stack = malloc(offset * sizeof(int));
    int stackPointer = 0;

    stack[0] = 0;
    reachable[0] = TRUE;

    while (stackPointer >= 0) {
        int stateID = stack[stackPointer];
        stackPointer--;

        if ((machine[stateID].blankState >= 0) && !reachable[machine[stateID].blankState]) {
            reachable[machine[stateID].blankState] = TRUE;
            stackPointer++;
            stack[stackPointer] = machine[stateID].blankState;
        }

        if ((machine[stateID].oneState >= 0) && !reachable[machine[stateID].oneState]) {
            reachable[machine[stateID].oneState] = TRUE;
            stackPointer++;
            stack[stackPointer] = machine[stateID].oneState;
        }
        numOfStates++;

    }
    free(stack);
}

void translate() {
    initTranslation();
    createInitTape();

    createAddressSubmachine(incrementMachineOffset, numRegisters, &addressRegisterIncrOffset, "AddressIncr");
    createAddressSubmachine(decrementMachineOffset, numRegisters, &addressRegisterDecrOffset, "AddressDecr");
    createAddressSubmachine(ifZeroMachineOffset, numRegisters, &addressRegisterIfZeroOffset, "AddressIfZero");
    createAddressSubmachine(ifNotZeroMachineOffset, numRegisters, &addressRegisterNotZeroOffset, "AdressIfNotZero");

    createGotoSequences();

    createFetchInstruction();
    markUnusedStates();
    
    printf("Compilation was successful!\nNumber of States: %d\n", numOfStates);

}


void writeMachineIntoFile(char* outputFileName) {
    char* machineTxt = malloc(offset * 300 * sizeof(char));
    char* currentPos = copyString(machineTxt, "{\n");

    char *(arr[]) = {"L", "InvalidShift", "R"};
    char **shiftDirection = arr+1;

    for (int i = 0; i < offset; i++) {
        if (!reachable[i]) continue;
        currentPos = copyString(currentPos, "    \"");
        char numBuf[30];
        //snprintf(numBuf, sizeof(numBuf), "%d", i);
        currentPos = copyString(currentPos, stateNames + (i * maxNameLen));
        currentPos = copyString(currentPos, "\":{\n");

        currentPos = copyString(currentPos, "        \"blankWrite\":");
        snprintf(numBuf, sizeof(numBuf), "%d", machine[i].blankWrite);
        currentPos = copyString(currentPos, numBuf);

        currentPos = copyString(currentPos, ",\n        \"blankShift\":");
        snprintf(numBuf, sizeof(numBuf), "\"%s\"", shiftDirection[machine[i].blankShift]);
        currentPos = copyString(currentPos, numBuf);

        currentPos = copyString(currentPos, ",\n        \"blankState\":");
        snprintf(numBuf, sizeof(numBuf), "\"%s\"", stateNames + (machine[i].blankState * maxNameLen));
        currentPos = copyString(currentPos, numBuf);

        currentPos = copyString(currentPos, ",\n        \"oneWrite\":");
        snprintf(numBuf, sizeof(numBuf), "%d", machine[i].oneWrite);
        currentPos = copyString(currentPos, numBuf);

        currentPos = copyString(currentPos, ",\n        \"oneShift\":");
        snprintf(numBuf, sizeof(numBuf), "\"%s\"", shiftDirection[machine[i].oneShift]);
        currentPos = copyString(currentPos, numBuf);

        currentPos = copyString(currentPos, ",\n        \"oneState\":");
        snprintf(numBuf, sizeof(numBuf), "\"%s\"", stateNames + (machine[i].oneState * maxNameLen));
        currentPos = copyString(currentPos, numBuf);

        currentPos = copyString(currentPos, "\n    },\n");
    }

    machineTxt[strlen(machineTxt)-2] = '\0'; //remove ",\n"" after last state
    strcat(machineTxt, "\n}");

    //write into file
    char buf[200];
    strcpy(buf, "_machines/");
    strcat(buf, outputFileName);
    FILE* machineFile = fopen(buf, "w");
    fprintf(machineFile, "%s", machineTxt);
    fclose(machineFile);
    free(machineTxt);

}

void cleanTranslation() {
    free(machine);
    machine = NULL;
    free(negativeValueNames);
    negativeValueNames = NULL;
    free(reachable);
    reachable = NULL;
}
