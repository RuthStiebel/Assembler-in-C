#ifndef PARSER_H
#define PARSER_H

#include "directives.h"
#include "instructions.h"
#include "labels.h"
#include "utils.h"

/**
 * Parses a line and updates the instruction counter, data counter and respective arrays accordingly.
 * @param line The current line to parse.
 * @param codeImage Array to store the machine words for instructions.
 * @param dataImage Array to store the machine words for data commands.
 * @param labels Pointer to the various label tabels.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if the parsing is successful, FALSE otherwise.
 */
boolean parseLine(char *line, machine_word codeImage[], machine_word dataImage[], labels_tables *label, int *IC, int *DC, int lineNumber);

/**
 * Retrieves the next token from the line and processes it.
 * @param line Pointer to the current line.
 * @param lineNumber The current line number being processed.
 * @return The next token in the line.
 */
Token getNextToken(char ** line, int lineNumber);

#endif /* PARSER_H */