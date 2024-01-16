#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "directives.h"
#include "instructions.h"
#include "labels.h"
#include "utils.h"
#include "print.h"

/* instructions: location in array is also the instruction's opcode */
char * instructions[NUM_OF_INSTRUCTIONS] = {
        "mov",
        "cmp",
        "add",
        "sub",
        "not",
        "clr",
        "lea",
        "inc",
        "dec",
        "jmp",
        "bne",
        "red",
        "prn",
        "jsr",
        "rts",
        "stop"
};


/**
 * Finds the opcode corresponding to the token's value.
 * @param token The token containing the instruction name.
 * @return The opcode found, or -1 if not found.
 */
static int findOpcode (Token token) {
    int opcode;
    for (opcode = 0; opcode < NUM_OF_INSTRUCTIONS; opcode++) {
        if (strcmp(token.value.string, instructions[opcode]) == 0)
            return opcode;
    }
    return -1;
}

/**
 * Parses instructions with two operands and generates machine words accordingly.
 * @param line A pointer to the current line being parsed.
 * @param token The token representing the instruction.
 * @param codeImage Array to store the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if is successful, FALSE otherwise.
 */
boolean parseTwoOperands (char ** line, Token token, machine_word codeImage[], int *IC, int *DC, int lineNumber) {
    int i; 
    int instructionSize = 3; /* instruction size is 3 12-bit words, or 2 12-bit words if both src and dest are registers */
    machine_word mw[3];
    int opCode = findOpcode(token);

    /* getting the next four tokens */
    Token tokenSrc = getNextToken(line, lineNumber);
    Token tokenComma = getNextToken(line, lineNumber);
    Token tokenDest = getNextToken(line, lineNumber);
    Token tokenEnd = getNextToken(line, lineNumber);

    /* check if there are exactly two opernads */
    if (tokenSrc.type == END || tokenComma.type != COMMA || tokenDest.type == END || tokenEnd.type != END) {
        printError("Invalid number of operands. Expecting 2 opernads.", lineNumber);
        return FALSE;
    }

    /* check src addressing mode */
    if (opCode == 6) { /* instruction 'lea' */
        if (tokenSrc.type != LABEL) {
            printError("Invalid src addressing mode.", lineNumber);
            return FALSE;
        }
    }
    
    /* check dest addressing mode */
    if (opCode != 1) { /* instruction not 'cmp' */
        if (!(tokenDest.type == LABEL || tokenDest.type == REGISTER)) {
            printError("Invalid dest addressing mode.", lineNumber);
            return FALSE;
        }
    }

    /* check that numbers are only 10 bits since this is the maximum size the opcode can hold */
    if (tokenSrc.type == NUMBER && (tokenSrc.value.integer < -1024 || tokenSrc.value.integer > 1023)) {
        printError("Invalid immediate number in source. only 10-bit numbers are allowed (-1024 - 1023).", lineNumber);
        return FALSE;
    }
    if (tokenDest.type == NUMBER && (tokenDest.value.integer < -1024 || tokenDest.value.integer > 1023)) {
        printError("Invalid immediate number in destination. only 10-bit numbers are allowed (-1024 - 1023).", lineNumber);
        return FALSE;
    }

    /* check if there is enough memory to write the instruction */
    if (tokenSrc.type == REGISTER && tokenDest.type == REGISTER) {
        instructionSize--;
    }
    if ((*IC + *DC + instructionSize) > MAX_MEMORY_SPACE) {
        printError("Maximum number of machine words (1024) reached. Not enough space.", lineNumber);
        return FALSE;
    }

    /* write machine words - src_am and dst_am are yet to be filled*/
    mw[0].isLabel = FALSE;
    mw[0].type = WORD_TYPE_FIRST;
    mw[0].word.first_word.ARE = ARE_ABSOLUTE;
    mw[0].word.first_word.op_code = opCode;

    switch (tokenSrc.type) {
        case NUMBER:
            mw[0].word.first_word.src_am = ADDRESSING_MODE_IMMEDIATE;
            mw[1].type = WORD_TYPE_IMMDT_DRCT;
            mw[1].isLabel = FALSE;
            mw[1].word.immdt_drct_word.operand = tokenSrc.value.integer;
            mw[1].word.immdt_drct_word.ARE = ARE_ABSOLUTE;
            break;
        case REGISTER:
            mw[0].word.first_word.src_am = ADDRESSING_MODE_REGISTER;
            mw[1].type = WORD_TYPE_REGISTER;
            mw[1].isLabel = FALSE;
            mw[1].word.register_word.ARE = ARE_ABSOLUTE;
            mw[1].word.register_word.src = tokenSrc.value.integer;
            mw[1].word.register_word.dest = 0;
            break;
        case LABEL:
            mw[0].word.first_word.src_am = ADDRESSING_MODE_DIRECT;
            mw[1].type = WORD_TYPE_IMMDT_DRCT;
            mw[1].isLabel = TRUE; /* flag this as a label */
            mw[1].word.immdt_drct_word.operand = tokenSrc.value.integer;
            mw[1].word.immdt_drct_word.ARE = ARE_NOT_DETERMINED;
            strcpy(mw[1].labelName, tokenSrc.value.string); /* copy label name */
            break;
        default:
            return FALSE;
    }

    switch (tokenDest.type) {
        case NUMBER:
            mw[0].word.first_word.dst_am = ADDRESSING_MODE_IMMEDIATE;
            mw[2].type = WORD_TYPE_IMMDT_DRCT;
            mw[2].isLabel = FALSE;
            mw[2].word.immdt_drct_word.operand = tokenDest.value.integer;
            mw[2].word.immdt_drct_word.ARE = ARE_ABSOLUTE;
            break;
        case REGISTER:
            mw[0].word.first_word.dst_am = ADDRESSING_MODE_REGISTER;
            if (tokenSrc.type == REGISTER) { /* if both are registers, use just the second machine word */
                mw[1].word.register_word.dest = tokenDest.value.integer;
            } else {
                mw[2].type = WORD_TYPE_REGISTER;
                mw[2].isLabel = FALSE;
                mw[2].word.register_word.ARE = ARE_ABSOLUTE;
                mw[2].word.register_word.src = 0;
                mw[2].word.register_word.dest = tokenDest.value.integer;
            }
            break;
        case LABEL:
            mw[0].word.first_word.dst_am = ADDRESSING_MODE_DIRECT;
            mw[2].type = WORD_TYPE_IMMDT_DRCT;
            mw[2].isLabel = TRUE; /* flag this as a label */
            mw[2].word.immdt_drct_word.operand = tokenDest.value.integer;
            mw[2].word.immdt_drct_word.ARE = ARE_NOT_DETERMINED;
            strcpy(mw[2].labelName, tokenDest.value.string); /* copy label name */
            break;
        default:
            return FALSE;
    }

    for (i = 0; i < instructionSize; i++)
        codeImage[(*IC)++] = mw[i];
    return TRUE;
}

/**
 * Parses instructions with one operand and generates machine words accordingly.
 * @param line A pointer to the current line being parsed.
 * @param token The token representing the instruction.
 * @param codeImage Array to store the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if is successful, FALSE otherwise.
 */
boolean parseOneOperand (char ** line, Token token, machine_word codeImage[], int *IC, int *DC, int lineNumber) {
    int i; 
    int instructionSize = 2; /* instruction size is 2 12-bit words */
    machine_word mw[2];
    int opCode = findOpcode(token);

    /* getting the next tokens */
    Token tokenDest = getNextToken(line, lineNumber);
    Token tokenEnd = getNextToken(line, lineNumber);

    /* check if there is exactly one operand */
    if (tokenDest.type == END || tokenEnd.type != END) {
        printError("Invalid number of operands. Expecting 1 opernad.", lineNumber);
        return FALSE;
    }

    /* check destination addressing mode */
    if (opCode != 12) { /* instruction not 'prn' */
        if (tokenDest.type == NUMBER) {
            printError("Invalid addressing mode.", lineNumber);
            return FALSE;
        }
    }

    /* check that numbers are only 10 bits since this is the maximum size the opcode can hold */
    if (tokenDest.type == NUMBER && (tokenDest.value.integer < -1024 || tokenDest.value.integer > 1023)) {
        printError("Invalid immediate number in destination. only 10-bit numbers are allowed (-1024 - 1023).", lineNumber);
        return FALSE;
    }

    /* check if there is enough memory to write the instruction */
    if ((*IC + *DC + instructionSize) > MAX_MEMORY_SPACE) {
        printError("Maximum number of machine words (1024) reached. Not enough space.", lineNumber);
        return FALSE;
    }

    /* write machine words - dst_am is yet to be filled */
    mw[0].isLabel = FALSE;
    mw[0].type = WORD_TYPE_FIRST;
    mw[0].word.first_word.ARE = ARE_ABSOLUTE;
    mw[0].word.first_word.op_code = opCode;
    mw[0].word.first_word.src_am = 0;
    
    switch (tokenDest.type) {
        case NUMBER:
            mw[0].word.first_word.dst_am = ADDRESSING_MODE_IMMEDIATE;
            mw[1].type = WORD_TYPE_IMMDT_DRCT;
            mw[1].isLabel = FALSE;
            mw[1].word.immdt_drct_word.ARE = ARE_ABSOLUTE;
            mw[1].word.immdt_drct_word.operand = tokenDest.value.integer;
            break;
        case REGISTER:
            mw[0].word.first_word.dst_am = ADDRESSING_MODE_REGISTER;
            mw[1].type = WORD_TYPE_REGISTER;
            mw[1].isLabel = FALSE;
            mw[1].word.register_word.ARE = ARE_ABSOLUTE;
            mw[1].word.register_word.dest = tokenDest.value.integer;
            break;
        case LABEL:
            mw[0].word.first_word.dst_am = ADDRESSING_MODE_DIRECT;
            mw[1].type = WORD_TYPE_IMMDT_DRCT;
            mw[1].isLabel = TRUE; /* flag this as a label */
            mw[1].word.immdt_drct_word.operand = tokenDest.value.integer;
            mw[1].word.immdt_drct_word.ARE = ARE_NOT_DETERMINED;
            strcpy(mw[1].labelName, tokenDest.value.string); /* copy label name */
            break;
        default:
            return FALSE;
    }

    for (i = 0; i<instructionSize; i++)
        codeImage[(*IC)++] = mw[i];
    return TRUE;
}

/**
 * Parses an instruction with no operands and generates machine words accordingly.
 * @param line A pointer to the current line being parsed.
 * @param token The token representing the instruction.
 * @param codeImage Array to store the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if is successful, FALSE otherwise.
 */
boolean parseNoOperands (char ** line, Token token, machine_word codeImage[], int *IC, int *DC, int lineNumber) {
    int instructionSize = 1; /* instruction size is 2 12-bit words */
    machine_word mw[1];
    int opCode = findOpcode(token);
    
    /* getting the next token */
    Token tokenEnd = getNextToken(line, lineNumber);
    
    /* check if there is exactly one operand */
    if (tokenEnd.type != END) {
        printError("Invalid number of operands. Expecting none.", lineNumber);
        return FALSE;
    }

    /* check if there is enough memory to write the instruction */
    if ((*IC + *DC + instructionSize) > MAX_MEMORY_SPACE) {
        printError("Maximum number of machine words (1024) reached. Not enough space.", lineNumber);
        return FALSE;
    }

    /* write machine word */
    mw[0].type = WORD_TYPE_FIRST;
    mw[0].word.first_word.ARE = ARE_ABSOLUTE;
    mw[0].word.first_word.op_code = opCode;
    mw[0].word.first_word.src_am = 0;
    mw[0].word.first_word.dst_am = 0;
    
    codeImage[(*IC)++] = mw[0];
    return TRUE;
}
