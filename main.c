#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocessor.h"
#include "parser.h"
#include "directives.h"
#include "instructions.h"
#include "labels.h"
#include "utils.h"
#include "generateOutput.h"
#include "print.h"

int main(int argc, char * argv[]) {
    int i, IC, DC, lineNumber;
    boolean ERROR_FOUND;
    char *fileName;
    FILE *fileAs, *fileAm;
    long int file_offset;
    machine_word codeImage[MAX_MEMORY_SPACE], dataImage[MAX_MEMORY_SPACE];
    char line[MAX_LINE_LENGTH+1]; /* adding one extra space for NULL ending */

    labels_tables labels;
    labels.internal = NULL;
    labels.external = NULL;
    labels.exportal = NULL;

    if (argc <= 1) {
        printErrorGeneral("No files in command line\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        fileName = argv[i];
        lineNumber = 1;
        IC = 0;
        DC = 0;
        ERROR_FOUND = FALSE;
        
        fileAs = openFile(fileName, ".as", "r");
        if (fileAs == NULL) {
            printWarningGeneral("Skipping file ");
            printf("'%s.as'.\n", fileName);
            continue;
        }
        
        fileAm = openFile(fileName, ".am", "w+");
        if (fileAm == NULL) {
            printWarningGeneral("Skipping file ");
            printf("'%s.as'.\n", fileName);
            fclose(fileAs);
            continue;
        }
        
        /*preproccess files*/
        fseek(fileAs, 0, SEEK_END);
        file_offset = ftell(fileAs); 
        rewind(fileAs);

        printf("Preprocessing file: '%s'\n", fileName);
        if (preprocessFile(fileAs, fileAm, 0, file_offset) == TRUE) { /*preprocessor error occured */ 
            printWarningGeneral("Skipping file ");
            printf("'%s.as'.\n", fileName);
            fclose(fileAs);
            fclose(fileAm);
            continue;
        }
                
		printf("Finished preprocessing file: '%s'\n", fileName);
       /*done with .as file. Rewinding .am file to assemble it: */
        fclose(fileAs);
        rewind(fileAm);
		
        printf("Processing file: '%s'\n", fileName);
        /*process the file line by line*/
        while (fgets(line, sizeof(line), fileAm) != NULL) {
            ERROR_FOUND |= (parseLine(line, codeImage, dataImage, &labels, &IC, &DC, lineNumber) == FALSE);
            lineNumber++;
        }
        /*close file*/
        fclose(fileAm);

        if (checkValidLabelsTables(labels) == FALSE) {
            ERROR_FOUND = TRUE;
        } else if (checkAllLabelsDefined(labels, codeImage, IC) == FALSE) {
            ERROR_FOUND = TRUE;
        }
        if (ERROR_FOUND == TRUE) {
            printErrorGeneral("Skipping file ");
            printf("%s because it has at least one error in it! \n", fileName);
            freeTables(labels);
            labels.internal = NULL;
            labels.external = NULL;
            labels.exportal = NULL;
            continue;
        }
        
        /*if no errors were found then creates the files */
        if (updateAdressesAndWriteExtFile(fileName, labels, codeImage, IC) == FALSE) {
            printErrorGeneral("Updating addresses and writing .ext file failed\n");
        } else if (writeObjFile(fileName, codeImage, dataImage, IC, DC) == FALSE) {
            printErrorGeneral("Writing .obj file failed\n");
        } else if (writeEntFile(fileName, labels) == FALSE) {
            printErrorGeneral("Writing .ent file failed\n");
        }
       
        freeTables(labels);
        labels.internal = NULL;
        labels.external = NULL;
        labels.exportal = NULL;
        printf("Finished processing file: '%s'\n", fileName);

    }
    return 0;
}
