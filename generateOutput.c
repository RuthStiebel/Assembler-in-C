#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "generateOutput.h"
#include "parser.h"
#include "directives.h"
#include "instructions.h"
#include "labels.h"
#include "utils.h"
#include "print.h"

/* define a base 64 word */
typedef struct base_64_word {
    char data [2];
} base_64_word;


/**
 * Converts a machine word to a base64 representation.
 * @param machineWord The machine word to convert.
 * @return The base 64 representation.
  */
static base_64_word machineWordToBase64 (machine_word * machineWord) {

    char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    base_64_word result;
    unsigned short binary;
    switch (machineWord -> type) {
        case WORD_TYPE_FIRST:
            binary =    ((machineWord -> word.first_word.src_am    & 0x007) << 9 ) |
                        ((machineWord -> word.first_word.op_code        & 0x00F) << 5 ) |
                        ((machineWord -> word.first_word.dst_am    & 0x007) << 2 ) |
                        ((machineWord -> word.first_word.ARE            & 0x003) << 0 );
            break;
        case WORD_TYPE_IMMDT_DRCT:
            binary =    ((machineWord -> word.immdt_drct_word.operand   & 0x3FF) << 2 ) |
                        ((machineWord -> word.immdt_drct_word.ARE       & 0x003) << 0 );
            break;
        case WORD_TYPE_DATA:
            binary =    ((machineWord -> word.data_word.data            & 0xFFF) << 0 );
            break;
        case WORD_TYPE_REGISTER:
            binary =    ((machineWord -> word.register_word.src & 0x01F) << 7 ) |
                        ((machineWord -> word.register_word.dest & 0x01F) << 2 ) |
                        ((machineWord -> word.register_word.ARE         & 0x003) << 0 );
            break;
    }
    /* extract each 6-bit group and map it to a base 64 character */
    result.data[0] = base64Chars[(binary >> 6) & 0x3F];
    result.data[1] = base64Chars[binary & 0x3F];
    return result;
}

/**
 * Opens a file with the given file name, extension, and mode.
 * @param fileName The base name of the file.
 * @param fileExtension The file extension to append to the base name.
 * @param mode The mode in which to open the file (e.g., "r", "w", "a").
 * @return A pointer to the opened file, or NULL on failure.
 */
FILE *openFile (const char* fileName, const char* fileExtension, const char *mode) {
    FILE *file;
    int nameLength = strlen(fileName);
    int extensionLength = strlen(fileExtension);
    char *name = malloc(nameLength + extensionLength + 1);
    if (name == NULL) {
        printErrorGeneral ("Not enough memory");
        printf(" - Could not create filename %s with extension %s\n", fileName, fileExtension);
        return NULL;
    }

    /* copy file name and extension, including trailing '\0' */
    strncpy(name, fileName, nameLength);
    strncpy(name + nameLength, fileExtension, extensionLength + 1); 

    file = fopen(name, mode);
    free(name);
    if (file == NULL) {
        printErrorGeneral("File error");
        printf(" - cant open '%s%s'\n", fileName, fileExtension);
        return FALSE;
    }
    return file;
}

/**
 * Updates internal label addresses and writes the external labels into the '.ext' file.
 * @param fileName The base name of the file.
 * @param labels Pointer to the various label tabels.
 * @param codeImage Array that stores the machine words for instructions.
 * @param IC Pointer to the instruction counter.
 * @return TRUE if successful, FALSE otherwise.
 */
boolean updateAdressesAndWriteExtFile (char* fileName, labels_tables labels, machine_word codeImage[], int IC) {
    int i;
    table_entry *tableEntry = labels.internal;
    FILE* fileExt = NULL;
    /* at this point, all lables used by code were checked, and therefore all labels are either EXTERNAL or INTERNAL */
    
    /* increment all internal lables by BASE_ADDRESS */
    while (tableEntry != NULL) {
        tableEntry->label.address += BASE_ADDRESS;
        tableEntry = tableEntry->next;
    }

    for (i = 0; i < IC; i++) {
        if (codeImage[i].isLabel != TRUE){
                    continue;
        }

        /* if label is defined as '.extern' then write into '.ext' file the address of the word that calls it */
        if (findLabel(codeImage[i].labelName, &labels, EXTERNAL) != NULL) {
            /* open file if this is the first label - this prevents creating the file if there are no external labels used */
            if (fileExt == NULL) {
                fileExt = openFile(fileName, ".ext", "w");
                if (fileExt == NULL) {
                    printWarningGeneral("Skipping updating addresses and writing .ext file\n");
                    return FALSE;
                }
            }
            codeImage[i].word.immdt_drct_word.operand = 0;
            codeImage[i].word.immdt_drct_word.ARE = ARE_EXTERNAL;
            fprintf(fileExt, "%s\t %d\n", codeImage[i].labelName, i + BASE_ADDRESS); /* write IC where external label is used by code */
        } else { /* label is internal (must be at this point) */
            codeImage[i].word.immdt_drct_word.ARE = ARE_RELOCATABLE;
            tableEntry = findLabel(codeImage[i].labelName, &labels, INTERNAL);
            if (tableEntry != NULL) {
                codeImage[i].word.immdt_drct_word.operand = tableEntry->label.address;
            }
        }
    }
    if (fileExt != NULL)
    	fclose(fileExt);
    return TRUE;
}

/**
 * Writes the machine code and data segments into the '.obj' file.
 * @param fileName The base name of the file.
 * @param codeImage Array that stores the machine words for instructions.
 * @param dataImage Array that stores the machine words for data.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter.
 * @return TRUE if successful, FALSE otherwise.
 */
boolean writeObjFile (char *fileName, machine_word codeImage[], machine_word dataImage[], int IC, int DC) {
    int i;
    FILE * fileObj;
    base_64_word word;

    fileObj = openFile(fileName, ".obj", "w");
    if (fileObj == NULL) {
        printWarningGeneral("Skipping writing .obj file\n");
        return FALSE;
    }

    fprintf(fileObj, "%d %d\n", IC, DC);

    /* write IC array into '.obj' file */
    for (i = 0; i < IC; i++) {
       word = machineWordToBase64(&codeImage[i]);
       fprintf(fileObj, "%c%c\n", word.data[0], word.data[1]);
    }
    
    /* write DC array into '.obj' file */
     for (i = 0; i < DC; i++) {
        word = machineWordToBase64(&dataImage[i]);
        fprintf(fileObj, "%c%c\n", word.data[0], word.data[1]);
    }
    
    fclose(fileObj);
    
    return TRUE;
}

/**
 * Writes the labels that were marked as '.entry' into the '.ent' file.
 * @param fileName The base name of the file.
 * @param labels Pointer to the various label tabels.
 * @return TRUE if successful, FALSE otherwise.
 */
boolean writeEntFile (char *fileName, labels_tables labels) {

    FILE *fileEnt;
    table_entry *exportal = labels.exportal;
    table_entry *internal;
    /* check if there are no '.entry' labels at all */
    if (exportal == NULL) {
        return TRUE;
    }

    fileEnt = openFile(fileName, ".ent", "w");
    if (fileEnt == NULL) {
        printWarningGeneral("Skipping writing .ent file\n");
        return FALSE;
    }

    /* check that all the labels declared as '.entry' are defined in the file */
    while (exportal != NULL) {
        internal = findLabel(exportal->label.name, &labels, INTERNAL);
        fprintf(fileEnt, "%s\t%d\n", internal->label.name, internal->label.address);
        exportal = exportal->next;
    }

    fclose(fileEnt);
    return TRUE;
}
