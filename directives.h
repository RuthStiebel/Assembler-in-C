#ifndef DIRECTIVES_H
#define DIRECTIVES_H

#include "parser.h"
#include "instructions.h"
#include "labels.h"
#include "utils.h"


/* Directives */
extern char *directives[NUM_OF_DIRECTIVES];

/**
 * Processes a directive token and generates machines words accordingly.
 * @param token The directive token.
 * @param index_in_line The current line of assembly code.
 * @param dataImage The array to store the machine words for data.
 * @param labelTable The label table.
 * @param IC The instruction counter.
 * @param DC The data counter.
 * @param lineNumber The number of the line currently being processed.
 * @return TRUE if processing was successful, FALSE otherwise.
 */
boolean parseDirective(Token token, char ** index_in_line, machine_word dataImage[], labels_tables *labels, int *IC, int *DC, int lineNumber);

#endif /* DIRECTIVES_H */
