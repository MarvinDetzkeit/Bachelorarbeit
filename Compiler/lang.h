#ifndef LANG_H
#define LANG_H

typedef enum {
    IS_ZERO,
    NOT_ZERO
} Comparison;

typedef enum {
    DECLARATION,
    CONDITIONAL,
    INSTRUCTION,
    LABEL
} ExpressionType;

typedef enum {
    INCREMENT,
    DECREMENT,
    GOTO,
    HALT,
    IF_ZERO,
    IF_NOT_ZERO
} InstructionType;

//Instruction IDs
#define ID_INC 0
#define ID_DEC 1
#define ID_GOTO 2
#define ID_HALT 3

//Operator IDs
#define ID_EQUAL 4
#define ID_UNEQUAL 5
#define ID_ASSIGN 6


typedef union {
    struct {
        int numReg;
        int initVal;
    } declaration;
    struct {
        InstructionType iType;
        int address;
        union {
            int regID;
            int jumpAdress;
            char* labelName;
        } param;
    } instruction;
    struct {
        int address;
    } label;
} ExpressionValue;

typedef struct {
    ExpressionType eType;
    ExpressionValue eVal;
} Expression;

#endif