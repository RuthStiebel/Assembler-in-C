#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"
#include "directives.h"
#include "instructions.h"
#include "labels.h"
#include "utils.h"
#include "print.h"

/**
 * Checks if a line of assembly code exceeds the maximum length allowed.
 * @param line The line of assembly code to be checked.
 * @param lineNumber The current line number being processed.
 * @return TRUE if the line is within the length limit and parsing was successful, FALSE otherwise.
 */
static boolean isLineTooLong (char * line, int lineNumber) {
    int charCount = 0;
    boolean ERROR_FLAG = FALSE;
    while (line[charCount] != '\0') {
        charCount++;
        /* check if line length exceeds 80 characters */
        if (charCount > MAX_LINE_LENGTH) {
            printError("Line is longer than 80 characters.", lineNumber);
            ERROR_FLAG = TRUE;
        }
    }
    return ERROR_FLAG;
}

/** Checks if the token value is a valid number. 
 * @param token The token to be checked.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if the token is a valid number, FALSE otherwise.
*/
static boolean isNumber (Token *token, int lineNumber) {
    int i = 0;
    boolean hasDigits = FALSE;

    if (token->value.string[0] == '-' || token->value.string[0] == '+') {
        i++;
    }
    while (isdigit(token->value.string[i])) {
        i++;
        hasDigits = TRUE;
    }
    
    return !(token->value.string[i] != '\0' || hasDigits == FALSE);
}

/**
 * Checks if the token value is a valid register.
 * @param token The token to be checked.
 * @param lineNumber The current line number being processed.
 * @return TRUE if the token is a valid register, FALSE otherwise.
 */
static boolean isRegister (Token *token, int lineNumber) {
    /* check if the number of the register is within range */
    if ( token->value.string[1] == 'r' &&  token->value.string[2] >= '0' && token->value.string[2] <= '7' && token->value.string[3] == '\0')
        return TRUE;
    else
        printError("Invalid register.", lineNumber);
    return FALSE;
}

/**
 * Parses a command token and updates the instruction counter, data counter and respective arrays accordingly.
 * @param token The current command token.
 * @param tokenLabel The label token (if exists).
 * @param line_index Pointer to the current position in the line.
 * @param codeImage Array to store the machine words for instructions.
 * @param dataImage Array to store the machine words for data commands.
 * @param labels Pointer to the various label tabels.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @param lineNumber The number of the current line being processed.
 * @return TRUE if the parsing is successful, FALSE otherwise.
 */
static boolean parseCommand (Token token, Token tokenLabel, char **line_index, machine_word codeImage[], machine_word dataImage[], labels_tables* labels, int *IC, int *DC, int lineNumber) {    
    if (token.type == DIRECTIVE) {
        /* mark token as a data word */
        if (tokenLabel.type == LABEL_DECLARATION) {
            if ((!strcmp( ".data", token.value.string)==0) && !strcmp(".string", token.value.string)==0) {
                printError("Invalid input after label name.", lineNumber);
                return FALSE;
            }
            if (!addLabel(tokenLabel.value.string, labels, INTERNAL, TRUE, IC, DC, lineNumber))
                return FALSE;
        }
        return parseDirective(token, line_index, dataImage, labels, IC, DC, lineNumber);          
    }
    
    if (tokenLabel.type == LABEL_DECLARATION) {
         if (!addLabel(tokenLabel.value.string, labels, INTERNAL, FALSE, IC, DC, lineNumber))
                return FALSE;
    }
    
    /* from here, token is ONE_OPERAND, TWO_OPERAND, or NO_OPERAND */
    if (token.type == INSTRUCTION_ONE_OPERAND) {
        return parseOneOperand(line_index, token, codeImage, IC, DC, lineNumber);
    } else if (token.type == INSTRUCTION_TWO_OPERANDS) {
        return parseTwoOperands(line_index, token, codeImage, IC, DC, lineNumber);
    } else if (token.type == INSTRUCTION_NO_OPERANDS) {
        return parseNoOperands(line_index, token, codeImage, IC, DC, lineNumber);
    }
    return FALSE;
    
}

/**
 * Retrieves the next token from the line and processes it.
 * @param line Pointer to the current line.
 * @param lineNumber The current line number being processed.
 * @return The next token in the line.
 */
Token getNextToken (char **line, int lineNumber) {
    Token token;
    int length, i;
    char *colonIndex;
    token.type = INVALID; /* default token type */

    /* move to the first whitespace character */
    while (**line != '\0' && isspace(**line)) {
        (*line)++;
    }

    colonIndex = strchr(*line, ':');
    
    /* check if the line ended */
    if (**line == '\n' || **line == '\0') {
        token.type = END;
        strcpy(token.value.string, "end of line");
        return token;
    }
    if (**line == ',') {
        token.type = COMMA;
        strcpy(token.value.string, "comma");
        (*line)++;
        return token;
    }

    /* copy the token value */
    length = 0;
    while (!isspace(**line) && **line != '\0' && **line != ',') {
        token.value.string[length] = **line;
        length++;
        (*line)++;
    }
    token.value.string[length] = '\0';

    if ((token.value.string[0] == '+' || token.value.string[0] == '-' || isdigit(token.value.string[0])) && isNumber(&token, lineNumber)) {
        token.type = NUMBER;
        token.value.integer = atoi(token.value.string);
        
        /* check if the number is within 12 bits */
        if (!(token.value.integer < 2048 && token.value.integer >= -2048)) {
            printError("Number exceeds 12 bits.", lineNumber);
            token.type = INVALID;
        }
        return token;
    }

    /* check for labels */
    if (colonIndex != NULL) {
        /* if there is a colon somewhere in the line, there has to be a label definition there */
        token.type = LABEL_DECLARATION;
        if (isValidLabel(token.value.string, LABEL_DECLARATION, lineNumber) == FALSE) {
            printError("A colon must appear right after the label definition.", lineNumber);
            token.type = INVALID;
        }

        token.value.string[length-1] = '\0';
        return token;
    }

    if (length >= MAX_LABEL_LENGTH) {
        printError("Line too long.", lineNumber);
        token.type = INVALID;
        return token;
    }

    /* determine the token type based on the token value */
    if (strcmp(token.value.string, "mov") == 0 || strcmp(token.value.string, "cmp") == 0 ||
        strcmp(token.value.string, "add") == 0 || strcmp(token.value.string, "sub") == 0 ||
        strcmp(token.value.string, "lea") == 0) {
        token.type = INSTRUCTION_TWO_OPERANDS;
    } else if (strcmp(token.value.string, "not") == 0 || strcmp(token.value.string, "clr") == 0 || strcmp(token.value.string, "inc") == 0 ||
                strcmp(token.value.string, "dec") == 0 || strcmp(token.value.string, "jmp") == 0 || strcmp(token.value.string, "bne") == 0 || strcmp(token.value.string, "red") == 0 ||
                strcmp(token.value.string, "prn") == 0 || strcmp(token.value.string, "jsr") == 0) {
        token.type = INSTRUCTION_ONE_OPERAND;
    } else if (strcmp(token.value.string, "rts") == 0 || strcmp(token.value.string, "stop") == 0) {
        token.type = INSTRUCTION_NO_OPERANDS;
    } else if (token.value.string[0] == '.') {
        token.type = DIRECTIVE;
    } else if (isalpha(token.value.string[0]) && isValidLabel(token.value.string, LABEL, lineNumber) == TRUE) {
        token.type = LABEL;
    } else if (token.value.string[0] == '@') {
        if (isRegister(&token, lineNumber) == TRUE) {
            token.type = REGISTER;
            token.value.integer = atoi(token.value.string + 2); /* skip "@r" */
        }
    } else if (token.value.string[0] == '"' && token.value.string[length-1] == '"') {
        token.type = STRING;
        for(i=0; i<length-1; i++) {
            token.value.string[i] = token.value.string[i+1];
        }
        token.value.string[length - 2] = '\0';
    }
    return token;
}

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
boolean parseLine (char * line, machine_word codeImage[], machine_word dataImage[], labels_tables* labels, int *IC, int *DC, int lineNumber) {
    Token token, tokenLabel;
    char *line_index = line;
    tokenLabel.type = INVALID;


    if (isLineTooLong(line, lineNumber)) {
        printError("Could not process file because line exceeded the maximum length limit.", lineNumber);
        return FALSE;
    }

    /* if line is within legal limit then parse it */
    token = getNextToken(&line_index, lineNumber);
    
    if (token.type == END) { /* skip empty lines */
        return TRUE;
    }

    /* check the first token - the rest of the tokens in the line will be checked in their respective functions */
    if (token.type == LABEL_DECLARATION) {
        tokenLabel = token;
        token = getNextToken(&line_index, lineNumber);
    }
    
    if (!(token.type == DIRECTIVE || token.type == INSTRUCTION_TWO_OPERANDS || token.type == INSTRUCTION_ONE_OPERAND || token.type == INSTRUCTION_NO_OPERANDS)) {
        printError("Invalid token.", lineNumber);
        printf("\t '%s'.\n",token.value.string);
        return FALSE;
    }
    
    return parseCommand (token, tokenLabel, &line_index, codeImage, dataImage, labels, IC, DC, lineNumber);
}
