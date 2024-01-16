#ifndef GENERATE_OUTPUT_H
#define GENERATE_OUTPUT_H

#include "utils.h"
#include "stdio.h"

/**
 * Opens a file with the given file name, extension, and mode.
 * @param fileName The base name of the file.
 * @param fileExtension The file extension to append to the base name.
 * @param mode The mode in which to open the file (e.g., "r", "w", "a").
 * @return A pointer to the opened file, or NULL on failure.
 */
FILE *openFile(const char* fileName, const char* fileExtension, const char *mode);

/**
 * Updates internal label addresses and writes the external labels into the '.ext' file.
 * @param fileName The base name of the file.
 * @param labels Pointer to the various label tabels.
 * @param codeImage Array that stores the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @return TRUE if successful, FALSE otherwise.
 */
boolean updateAdressesAndWriteExtFile(char *fileName, labels_tables labels, machine_word codeImage[], int IC);

/**
 * Writes the machine code and data segments into the '.obj' file.
 * @param fileName The base name of the file.
 * @param codeImage Array that stores the machine words for instructions.
 * @param dataImage Array that stores the machine words for data.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @return TRUE if successful, FALSE otherwise.
 */
boolean writeObjFile(char *fileName, machine_word codeImage[], machine_word dataImage[], int IC, int DC);

/**
 * Writes the labels that were marked as '.entry' into the '.ent' file.
 * @param fileName The base name of the file.
 * @param labels Pointer to the various label tabels.
 * @return TRUE if successful, FALSE otherwise.
 */
boolean writeEntFile(char *fileName, labels_tables labels);

#endif /*GENERATE_OUTPUT_H*/
