#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "preprocessor.h"
#include "print.h"
#include "utils.h"
#include "instructions.h"
#include "directives.h"

#define MACRO_START "mcro "
#define MACRO_END "endmcro"

/* macro has name 'name', and it's contents lie in the file from 'offset_start' to 'offset_end' including */
typedef struct macro_t {
    char name[MAX_LINE_LENGTH+1]; /*adding one extra space for NULL ending*/
    long int offset_start;
    long int offset_end;
} macro_t;

/* macro table entry */
typedef struct macro_entry {
    macro_t macro;
    struct macro_entry *next;
} macro_entry;

/*macro table*/
typedef struct macro_table {
    macro_entry *macros;
} macro_table;

/**
 * Frees a linked list of macro_entry nodes (which represent a macro table).
 * @param head The head of the linked list.
 */
static void freeTable (macro_entry *head) {
    macro_entry *next;
    while (head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}

/**
 * Trims leading and trailing whitespace characters from a string.
 * @param str The string to trim.
 * @return A pointer to the trimmed string.
 */
static char* strtrim (char* str) {
    size_t start = 0, end, i, len = strlen(str);

    if (len == 0)
      return str;

    end = len - 1;

    while (isspace(str[start])) start++;
    while (isspace(str[end])) end--;

    for (i = 0; i <= end - start; i++)
      str[i] = str[start + i];

    str[i] = '\0';
    return str;
}

/**
 * Checks if a given string is a valid macro name.
 * @param name The macro name to check.
 * @return TRUE if the string is a valid macro name, FALSE otherwise.
 */
static boolean isValidMacroName (char * name) {
    int i = 0;
    char *c = name;
    boolean isSpaceFlag = FALSE;
    
    /* no spaces are allowed in macro name */
    while (*c != '\0') {
        if (isspace(*c))
            isSpaceFlag = TRUE;
        if (!isspace(*c) && isSpaceFlag == TRUE)
            return FALSE;
        c++;
    }  

    /* check if the macro name is an instruction's name */
    for (i=0; i<NUM_OF_INSTRUCTIONS; i++) {
        if (strcmp(name, instructions[i]) == 0)
            return FALSE;
    }
    
    /* check if the macro name is a directive's name */
    for (i=0; i<NUM_OF_DIRECTIVES; i++) {
        if (strcmp(name, directives[i]) == 0)
            return FALSE;
    }
    return TRUE;
}

/**
 * Checks if a given string is a valid 'endmacro' statement.
 * @param name The string to check.
 * @return TRUE if the string is a valid 'endmacro' statement, FALSE otherwise.
 */
static boolean isValidMacroEnd (char* name) {
    char* c = name + strlen(MACRO_END);  
    /* check that there are no characters after 'endmcro' */
    while (*c != '\0') {
        if (!isspace(*c))
            return FALSE;
        c++;
    }
    return TRUE;
}

/**
 * Finds a macro in the macro table.
 * @param macroTable Pointer to the macro table.
 * @param name The macro name to find.
 * @return A pointer to the found label, or NULL if not found.
 */
static macro_entry *findMacro (macro_table *macroTable, char *name) {
    macro_entry *entry = macroTable->macros;

    while (entry != NULL) {
        /* check if the macro exists in the table */
        if (strcmp(entry->macro.name, name) == 0) {
            break;
        }
        entry = entry->next;
    }
    return entry;
}

/**
 * Adds a new macro entry to the macro_table.
 * @param macroTable Pointer to the macro table.
 * @param name The macro name.
 * @param offset_start The start offset of the macro in the source file.
 * @param offset_end The end offset of the macro in the source file.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if the macro is added successfully, FALSE otherwise.
 */
static boolean addMacro (macro_table *macroTable, char* name, long int offset_start, long int offset_end, int lineNumber) {
    macro_entry *new_entry;

    /* allocate space for new macro */
    new_entry = (macro_entry *) calloc(MAX_LINE_LENGTH, sizeof(macro_entry));
    if (new_entry == NULL) {
        printError("Could not allocate space for macro.", lineNumber);
        return FALSE;
    }
    strcpy(new_entry->macro.name, name);
    new_entry->macro.offset_start = offset_start;
    new_entry->macro.offset_end = offset_end;

    /* add macro to table*/
    new_entry->next = macroTable->macros;
    macroTable->macros = new_entry;
    return TRUE;
}

/**
 * Preprocesses a source file, expanding macros and removing comment lines.
 * @param fileAs Pointer to the source file.
 * @param fileAm Pointer to the output file.
 * @param as_offset_start The start offset in the source file.
 * @param as_offset_end The end offset in the source file.
 * @return TRUE if preprocessing is successful, FALSE otherwise.
 */
boolean preprocessFile(FILE* fileAs, FILE* fileAm, long int as_offset_start, long int as_offset_end) {
    char buffer[MAX_LINE_LENGTH+1]; /* add one extra space for NULL ending */
    char *current; /* index of current char in buffer */
    int lineLength, lineNumber = 0;
    boolean insideMacro = FALSE;
    boolean errorFlag = FALSE;
    char macroName[MAX_LINE_LENGTH+1];
    char macroToSearch[MAX_LINE_LENGTH+1];
    long int offset_start, offset_end, asOffsetOriginal;
    macro_table macros_table;
    macro_entry *entry;
    macros_table.macros = NULL;
    
    asOffsetOriginal = ftell(fileAs);
    fseek(fileAs, as_offset_start, SEEK_SET);
    /* loop over every line in source file */
    while (fgets(buffer, sizeof(buffer), fileAs) != NULL && ftell(fileAs) <= as_offset_end) {
        lineNumber++;
        
        /* skip all spaces */
        current = buffer;
        while (isspace(*current)) {
            current++;
        }

        lineLength = strlen(current); /* length with \n */
        
        if ((lineLength == 1) && (*current == '\n')) { /* check if is an empty line */
            continue;
        }

        if ((lineLength >= 2) && (*current == ';')) { /* comment line, or at least ';\n' */
            continue;
        }       

        /* remove trailing \n and whitespaces */
        strcpy(macroToSearch, current); 
        strtrim(macroToSearch);

        /* check if line is a macro */
        entry = findMacro(&macros_table, macroToSearch);

        if (entry != NULL) { /* if macro is found */
            /* write macro contents into file instead of continuing to read the line */
            preprocessFile(fileAs, fileAm, entry->macro.offset_start, entry->macro.offset_end);
        } else if (insideMacro == TRUE) { /* if is inside macro */
            /* if found 'endmcro' */
            if (strncmp(current, MACRO_END, strlen(MACRO_END)) == 0)  { 
                if (isValidMacroEnd(current) == FALSE) { /* check if there are other characters on the line */
                    printError("No characters allowed on line after 'endmcro' flag.", lineNumber);
                    errorFlag = TRUE;
                    break;
                }
                insideMacro = FALSE;
                if (addMacro(&macros_table, macroName, offset_start, offset_end, lineNumber) == FALSE) { /* error trying to add macro */
                    errorFlag = TRUE;
                    break;
                }

            }
        } else { /* if is outside macro */
            /* if line starts with MACRO_START flag */
            if ((lineLength - 1 >= strlen(MACRO_START)) && (strncmp(current, MACRO_START, strlen(MACRO_START)) == 0)) {
                insideMacro = TRUE;
                strcpy(macroName, strtrim(current + strlen(MACRO_START)));

                /* check if macro name is valid, and if its not already defined */
                if (isValidMacroName(macroName) == FALSE) {
                    printError ("Macro name is invalid (keywords are not allowed).", lineNumber);
                    errorFlag = TRUE;
                    break;
                }
     
                /* check if macro is already defiend */
                if (findMacro(&macros_table, macroName) != NULL) {
                    printError("Macro is already defined.", lineNumber);
                    errorFlag = TRUE;
                    break;
                }
                
                offset_start = ftell(fileAs);
            } else { /* if is a regular line */
                fputs(buffer, fileAm); /* copy line as it is to .am */
            }
        }
        offset_end = ftell(fileAs);
    }
    
    fseek(fileAs, asOffsetOriginal, SEEK_SET);
    freeTable(macros_table.macros);
    return errorFlag;
}
