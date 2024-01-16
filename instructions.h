#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "parser.h"
#include "directives.h"
#include "labels.h"
#include "utils.h"

/*Instructions*/
extern char *instructions[NUM_OF_INSTRUCTIONS];

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
boolean parseTwoOperands(char **line, Token token, machine_word codeImage[], int *IC, int *DC, int lineNumber);

/**
 * Parses an instruction with one operand and generates machine words accordingly.
 * @param line A pointer to the current line being parsed.
 * @param token The token representing the instruction.
 * @param codeImage Array to store the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if is successful, FALSE otherwise.
 */
boolean parseOneOperand(char **line, Token token, machine_word codeImage[], int *IC, int *DC, int lineNumber);

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
boolean parseNoOperands(char ** line, Token token, machine_word codeImage[], int *IC, int *DC, int lineNumber);

#endif /* INSTRUCTIONS_H */
