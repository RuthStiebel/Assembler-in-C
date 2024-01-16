#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "utils.h"
#include <stdio.h>

/**
 * Preprocesses a source file, expanding macros and removing comment lines.
 * @param fileAs Pointer to the source file.
 * @param fileAm Pointer to the output file.
 * @param as_offset_start The start offset in the source file.
 * @param as_offset_end The end offset in the source file.
 * @return TRUE if preprocessing is successful, FALSE otherwise.
 */
boolean preprocessFile(FILE* fileAs, FILE* fileAm, long int as_offset_start, long int as_offset_end);

#endif /* PREPROCESSOR_H */