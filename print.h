#ifndef PRINT_H
#define PRINT_H

/**
 * Prints a debug message to a specified file along with the line number and additional formatted arguments.
 * @param fileName The name of the file to print to.
 * @param lineNumber The line number associated with the message.
 * @param format The format string of the message.
 * @param ... Additional formatted arguments.
 */
void printDebug(char *fileName, int lineNumber, const char *format, ...);

/**
 * Prints a general warning message.
 * @param str The warning message.
 */
void printWarningGeneral(char *str);

/**
 * Prints a general error message.
 * @param str The error message.
 */
void printErrorGeneral(char *str);

/**
 * Prints a warning message along with the line number.
 * @param str The warning message.
 * @param lineNumber The line number associated with the message.
 */
void printWarning(char *str, int lineNumber);

/**
 * Prints an error message along with the line number.
 * @param str The error message.
 * @param lineNumber The line number associated with the message.
 */
void printError(char *str, int lineNumber);

#endif /* PRINT_H */