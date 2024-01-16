#include <stdio.h>
#include <stdarg.h>

#include "print.h"
int counter = 0;

 /*prints a message to a specified file along with the line number and additional formatted arguments*/
void printDebug (char *fileName, int lineNumber, const char *format, ...) {
    va_list args;
    printf("%02d: \033[1;35mDEBUG  \033[0m - \033[1;36m%s:%d\033[0m: ", counter, fileName, lineNumber); 
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    counter++;
}

/*prints a general warning message*/
void printWarningGeneral (char *str) {
    printf("\033[1;33mWARNING\033[0m - %s", str);
}

/*prints a general error message*/
void printErrorGeneral(char *str) {
    printf("\033[1;31mERROR  \033[0m - %s", str);
}

/*prints a warning message with a line number*/
void printWarning (char *str, int lineNumber) {
    printf("\033[1;33mWARNING\033[0m - \033[1;32mline #%d\033[0m: %s\n", lineNumber, str); 
}

/*prints an error message with a line number*/
void printError (char *str, int lineNumber) {
    printf("\033[1;31mERROR  \033[0m - \033[1;34mline #%d\033[0m: %s\n", lineNumber, str);
}


