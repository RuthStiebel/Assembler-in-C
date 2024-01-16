#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "directives.h"
#include "labels.h"
#include "utils.h"
#include "print.h"

/* directives */
char * directives[NUM_OF_DIRECTIVES] = {
        ".data",
        ".string",
        ".entry",
        ".extern"
};

/**
 * Processes a '.data' directive and generates machine words accordingly.
 * @param index_in_line The current line of assembly code.
 * @param dataImage The array to store the machine words for data.
 * @param IC The instruction counter.
 * @param DC The data counter.
 * @param lineNumber The number of the line currently being processed.
 * @return TRUE if processing was successful, FALSE otherwise.
*/
boolean parseDirectiveData(char ** index_in_line, machine_word dataImage[], int *IC, int *DC, int lineNumber) {
    int numberCounter = 0, commaCounter = 0;
    Token token = getNextToken(index_in_line, lineNumber);
    
    if (token.type != NUMBER)
    {
        printError("After '.data' at least one number must appear.", lineNumber);
        return FALSE;
    }
    
    while (token.type != END) {
        if (token.type == NUMBER) {
            /* check if we have reached the maximum number of machine words */
            if ((*IC + *DC + 1) >= MAX_MEMORY_SPACE) {
                printError("Maximum number of machine words (1024) reached.", lineNumber);
                return FALSE;
            }
            numberCounter++;
            /* save number */
            dataImage[*DC].type = WORD_TYPE_DATA;
            dataImage[*DC].word.data_word.data = token.value.integer & 0xFFF; /* convert to a 12-bit word */
            (*DC)++;
        } else if (token.type == COMMA) {
            commaCounter++;
        } else {
            printError("Invalid character.", lineNumber);
            return FALSE;
        }
        token = getNextToken(index_in_line, lineNumber);
    }
    /* check that there was the right ratio of commas to numbers */
    if (!(numberCounter == (commaCounter+1))) {
        printError("Invalid number of commas.", lineNumber);
        return FALSE;
    }

    return TRUE;
}

/**
 * Processes a '.string' directive and generates machine words accordingly.
 * @param index_in_line The current line of assembly code.
 * @param dataImage The array to store the machine words for data.
 * @param IC The instruction counter.
 * @param DC The data counter.
 * @param lineNumber The number of the line currently being processed.
 * @return TRUE if processing was successful, FALSE otherwise.
 */
static boolean parseDirectiveString(char **index_in_line, machine_word dataImage[], int *IC, int *DC, int lineNumber) {
    int i, asciiValue, length;
    /* move to the token after the ".string" directive */
    Token tokenString = getNextToken(index_in_line, lineNumber);
    
    if (tokenString.type != STRING) {
        printError("Directive .string must be followed by a string.", lineNumber);
        return FALSE;
    }

    /* check that the next token is the end of the line */
    if (getNextToken(index_in_line, lineNumber).type != END) {
        printError("Invalid character after string.", lineNumber);
        return FALSE;
    }
    
    /* check if there is enough memory left for the word */
    length = strlen(tokenString.value.string);
    if ((*DC + length + 1 + *IC) >= MAX_MEMORY_SPACE) {
        printError("Maximum number of machine words (1024) reached.", lineNumber);
        return FALSE;
    }

    /* save the string as data words */
    for (i=0; i <= length ; i++) {
        asciiValue = tokenString.value.string[i];
        dataImage[*DC + i].word.data_word.data = asciiValue & 0xFFF; /* convert to a 12-bit word */
        dataImage[*DC + i].type = WORD_TYPE_DATA;           
    }
    
    /* update the DC counter */
    *DC += length + 1;

    return TRUE;
}

/**
 * Processes a '.extern' directive and updates the label table accordingly.
 * @param index_in_line The current line of assembly code.
 * @param labelTable The label table.
 * @param lineNumber The number of the line currently being processed.
 * @return TRUE if processing was successful, FALSE otherwise.
 */
static boolean parseDirectiveExternal(char ** index_in_line, labels_tables *labels, int lineNumber) {
    Token token = getNextToken(index_in_line, lineNumber);
    if(token.type != LABEL) {
        printError("After '.extern' only a label name should appear.", lineNumber);
        return FALSE;
    }
    if(getNextToken(index_in_line, lineNumber).type != END) {
        printError("Line has an invalid token.", lineNumber);
        return FALSE;
    }
    addLabel(token.value.string, labels, EXTERNAL, FALSE, 0, 0, lineNumber);
    return TRUE;
}


/**
 * Processes a '.entry' directive and updates the label table accordingly.
 * @param index_in_line The current line of assembly code.
 * @param labelTable The label table.
 * @param lineNumber The number of the line currently being processed.
 * @return TRUE if processing was successful, FALSE otherwise.
 */
static boolean parseDirectiveEntry(char ** index_in_line, labels_tables *labels, int lineNumber) {
    Token token = getNextToken(index_in_line, lineNumber);
    if(token.type != LABEL) {
        printError("After '.entry' only a label name should appear.", lineNumber);
        return FALSE;
    }
    if(getNextToken(index_in_line, lineNumber).type != END) {
        printError("Line has an invalid token.", lineNumber);
        return FALSE;
    }
    addLabel(token.value.string, labels, EXPORTAL, FALSE, 0, 0, lineNumber);
    return TRUE;
}

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
boolean parseDirective(Token token, char ** index_in_line, machine_word dataImage[], labels_tables *labels, int *IC, int *DC, int lineNumber) {
    if (strcmp(token.value.string, ".data") == 0) {
        return parseDirectiveData(index_in_line, dataImage, IC, DC, lineNumber);
    } else if (strcmp(token.value.string, ".string") == 0) {
        return parseDirectiveString(index_in_line, dataImage, IC, DC, lineNumber);
    } else if (strcmp(token.value.string, ".entry") == 0) {
        return parseDirectiveEntry(index_in_line, labels, lineNumber);
    } else if (strcmp(token.value.string, ".extern") == 0) {
        return parseDirectiveExternal(index_in_line, labels, lineNumber);
    } else {
        printError("If a word starts with a dot it must be an directive name.", lineNumber);
        return FALSE;
    }
}
