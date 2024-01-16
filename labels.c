#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"
#include "directives.h"
#include "instructions.h"
#include "labels.h"
#include "utils.h"
#include "print.h"

/**
 * Checks if a label name is a valid label's name, an instruction's name, or a directive's name.
 * @param name The name of the label to check.
 * @return 1 if it's a valid label name, 2 if it's an instruction's name, 3 if it's a directive's name.
 */
static int isKeyword (char * name) {
    int currentIn = 0;
    int currentDi = 0;
    
    while (currentIn < NUM_OF_INSTRUCTIONS) {
        /* check if the name is an instruction's name */
        if (strcmp(name, instructions[currentIn]) == 0)
            return 2;
        currentIn++;
    }

    while (currentDi < NUM_OF_DIRECTIVES) {
        /* check if the name is a directive's name */
        if (strcmp(name, directives[currentDi]) == 0)
            return 3;
        currentDi++;
    }
    return 1;
}

/**
 * Retrieves the appropriate table based on the label type.
 * @param labels Pointer to the various label tabels.
 * @param type The label type.
 * @return A pointer to the appropriate table.
 */
static table_entry *getTable (labels_tables *labels, labelType type) {
    switch (type) {
        case INTERNAL: return labels->internal;
        case EXTERNAL: return labels->external;
        case EXPORTAL: return labels->exportal;
        default: return NULL;
    }
}

/**
 * Checks if a label name is valid.
 * @param str The name of the label to check.
 * @param type The type of the label.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if the label name is valid, FALSE otherwise.
 */
boolean isValidLabel (char * str, TokenType type, int lineNumber) {
    int i = 1;
    /* check if the label name is valid */
    if (!isalpha(str[0])) {
        printError("Label should start with a letter.", lineNumber);
        return FALSE;
    }

    /* check if the label is the right length */
    if (strlen(str) > MAX_LABEL_LENGTH) {
        printError("Label name too long.", lineNumber);
        return FALSE;
    }

    /* check if the label has only letters and numbers */
    for (i = 1; i < strlen(str)-1; i++) {
        if (!isalpha(str[i]) && !isdigit(str[i])) {
            printError("Label name should only contain letters or numbers.", lineNumber);
            return FALSE;
        }
    }

    /* check if label definition ends with colon */
    if (str[i] != ':' && type == LABEL_DECLARATION) {
        printError("Label definition should end with a colon.", lineNumber);
        return FALSE;
    } 

    /* check if label name is a keyword */
    if (isKeyword(str) == 2) {
        printError("Illegal label name - cannot be an instruction's name.", lineNumber);
        return FALSE;
    } else if (isKeyword(str) == 3) {
        printError("Illegal label name - cannot be a directive's name.", lineNumber);
        return FALSE;
    }
    return TRUE;
}

/**
 * Finds a label in the label table.
 * @param name The label name to find.
 * @param labels Pointer to the various label tabels.
 * @param type The label type.
 * @return A pointer to the found label, or NULL if not found.
 */
table_entry* findLabel(char * name, labels_tables *labels, labelType type) {
    table_entry *entry = getTable(labels, type);
    while (entry != NULL) {
        /* check if the label exists in the table */
        if (strcmp(entry->label.name, name) == 0)
            break;
        entry = entry->next;
    }
    return entry;
}

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
boolean addLabel (char * name, labels_tables *labels, labelType type, boolean isData, int *IC, int *DC, int lineNumber) {
    table_entry *new_entry;
    table_entry *table = getTable(labels, type);
    
    /* check if label is already in the table: If yes, send an error message. If not, add the new label to the table */
    if (labels != NULL && findLabel(name, labels, type) != NULL) {
        printError("Label is already defined.", lineNumber);
        return FALSE;
    }

    /* define a new label and allocate space */
    new_entry = (table_entry *) calloc(MAX_LABEL_LENGTH, sizeof(table_entry));
    if (new_entry == NULL) {
        printError("Could not allocate space for label.", lineNumber);
        return FALSE;
    }
    
    /* copy information from file */
    strcpy(new_entry->label.name, name);
    new_entry->label.isData = isData;

    /* update the IC or DC accordingly */
    if (type == INTERNAL) {
        new_entry->label.address = isData ? (*DC+*IC) : *IC;
    }
    
    /* add label to table */
    new_entry->next = table;
    switch (type) {
        case INTERNAL: labels->internal = new_entry; break; 
        case EXTERNAL: labels->external = new_entry; break;
        case EXPORTAL: labels->exportal = new_entry; break;
    }
    return TRUE;
}

/**
 * Frees a linked list of table_entry nodes (which represent a label table).
 * @param head The head of the linked list.
 */
static void freeTable (table_entry *head) {
    table_entry *next;
    while(head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}

/**
 * Frees all memory allocated for the various label tabels.
 * @param labels Pointer to the various label tabels to be freed.
 */
void freeTables(labels_tables labels) {
    freeTable(labels.internal);
    freeTable(labels.external);
    freeTable(labels.exportal);
    labels.internal = NULL;
    labels.external = NULL;
    labels.exportal = NULL;
}

/**
 * Checks the validity of labels in the label tables.
 * @param labels Pointer to the various label tabels.
 * @return TRUE if all labels are valid, FALSE otherwise.
 */
boolean checkValidLabelsTables (labels_tables labels) {
    table_entry *external = labels.external;
    table_entry *exportal = labels.exportal;

    /* check that every external entry is not internal and not exportal */
    while (external != NULL) {
        if (findLabel(external->label.name, &labels, INTERNAL) != NULL) {
            return FALSE;
        }
        if (findLabel(external->label.name, &labels, EXPORTAL) != NULL) {
            printErrorGeneral("Label ");
            printf("'%s' cannot be defined as both '.entry' and '.extern'.\n", external->label.name);
        }
        external = external->next;
    }

    /* check that every exportal label is also an internal one*/
    while (exportal != NULL) {
        if (findLabel(exportal->label.name, &labels, INTERNAL) == NULL) {
            printErrorGeneral("Label ");
            printf("'%s' marked as '.entry' but not defined in file.\n", exportal->label.name);
            return FALSE;
        }
        exportal = exportal->next;
    }
    return TRUE;
}

/**
 * Checks if all labels used in the code are defined in the label tables.
 * @param labels Pointer to the various label tabels.
 * @param codeImage Array to store the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @return TRUE if all labels are defined, FALSE otherwise.
 */
boolean checkAllLabelsDefined (labels_tables labels, machine_word codeImage[], int IC) {
    int i;
    for (i=0; i<IC; i++) {
        if (codeImage[i].isLabel != TRUE)
            continue;
        if (findLabel(codeImage[i].labelName, &labels, EXTERNAL) == NULL && 
            findLabel(codeImage[i].labelName, &labels, INTERNAL) == NULL) {
                printErrorGeneral("Label ");
                printf("'%s' could not be found.\n", codeImage[i].labelName);
                return FALSE;
            }            
        }
    return TRUE;
}


