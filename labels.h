#ifndef LABELS_H
#define LABELS_H

#include "parser.h"
#include "directives.h"
#include "instructions.h"
#include "utils.h"

/**
 * Checks if a label name is valid.
 * @param str The name of the label to check.
 * @param type The type of the label.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if the label name is valid, FALSE otherwise.
 */
boolean isValidLabel(char * str, TokenType type, int lineNumber);

/**
 * Finds a label in the label table.
 * @param name The label name to find.
 * @param labels Pointer to the various label tabels.
 * @param type The label type.
 * @return A pointer to the found label, or NULL if not found.
 */
table_entry *findLabel(char * name, labels_tables* labels, labelType type);

/**
 * Creates a new label and adds it to the label table.
 * @param name The label name.
 * @param labels Pointer to the various label tabels.
 * @param type The label type.
 * @param isData Indicates whether the label is associated with data.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if the label is added successfully, FALSE otherwise.
 */
boolean addLabel(char * name, labels_tables *labels, labelType type, boolean isData, int *IC, int *DC, int lineNumber);

/**
 * Frees all memory allocated for the various label tabels.
 * @param labels Pointer to the various label tabels to be freed.
 */
void freeTables(labels_tables labels);

/**
 * Checks the validity of labels in the label tables.
 * @param labels Pointer to the various label tabels.
 * @return TRUE if all labels are valid, FALSE otherwise.
 */
boolean checkValidLabelsTables (labels_tables labels);

/**
 * Checks if all labels used in the code are defined in the label tables.
 * @param labels Pointer to the various label tabels.
 * @param codeImage Array to store the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @return TRUE if all labels are defined, FALSE otherwise.
 */
boolean checkAllLabelsDefined (labels_tables labels, machine_word codeImage[], int IC);


#endif /* LABELS_H */