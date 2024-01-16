#ifndef UTILS_H
#define UTILS_H
#define BASE_ADDRESS 100
#define MAX_MEMORY_SPACE 924
#define MAX_LINE_LENGTH 81
#define MAX_LABEL_LENGTH 31
#define NUM_OF_DIRECTIVES 4
#define NUM_OF_INSTRUCTIONS 16

/* Boolean variable */
typedef enum {
    FALSE = 0,
    TRUE = 1
} boolean;

/* Addressing modes */
typedef enum {
    ADDRESSING_MODE_IMMEDIATE = 1,
    ADDRESSING_MODE_DIRECT = 3,
    ADDRESSING_MODE_REGISTER = 5
} AdressingMode;

/* Addressing modes */
typedef enum {
    ARE_ABSOLUTE = 0,
    ARE_EXTERNAL = 1,
    ARE_RELOCATABLE = 2,
    ARE_NOT_DETERMINED = 3
} ARE_BITS;

/* Variable to indicate label type */
typedef enum {
    INTERNAL,
    EXTERNAL,
    EXPORTAL
} labelType;

/* Labels */
typedef struct label_t {
    char name[MAX_LABEL_LENGTH+1]; /* adding one extra space for NULL ending */
    int address;
    boolean isData;
} label_t;

/* Label table entry */
typedef struct table_entry {
    label_t label;
    struct table_entry *next;
} table_entry;

/* Label table */
typedef struct labels_tables{
    table_entry *internal;
    table_entry *external;
    table_entry *exportal;
} labels_tables;

/* Token type */
typedef enum {
    END,
    NUMBER,
    STRING,
    LABEL_DECLARATION,
    LABEL,
    REGISTER,
    COMMA,
    DIRECTIVE,
    INSTRUCTION_NO_OPERANDS,
    INSTRUCTION_ONE_OPERAND,
    INSTRUCTION_TWO_OPERANDS,
    INVALID
} TokenType;

/* Token */
typedef struct {
    TokenType type;
    union value {
        char string[MAX_LINE_LENGTH + 1];  /* adding one extra space for NULL ending */
        int integer;
    } value;
} Token;

/* Define the machine word that always comes first */
typedef struct first_word {
    unsigned int src_am: 3;
    unsigned int op_code: 4;
    unsigned int dst_am: 3;
    unsigned int ARE: 2;
} first_word;

/* Define a machine word with direct or immediate addressing */
typedef struct immdt_drct_word {
    unsigned int ARE: 2;
    unsigned int operand: 10;
} immdt_drct_word;

/* Define a machine word that is just data */
typedef struct data_word {
    unsigned int data: 12;
} data_word;

/* Define a machine word that holds the value and addresses of between one to two registers */
typedef struct rgstr_word {
    unsigned int ARE: 2;
    unsigned int dest: 5;
    unsigned int src: 5;
} rgstr_word;

/* Variable to indicate machine word type */
typedef enum {
    WORD_TYPE_FIRST,
    WORD_TYPE_IMMDT_DRCT,
    WORD_TYPE_DATA,
    WORD_TYPE_REGISTER
} WordType;

/* Define a general machine word - one that is either an instruction, data word, register word or immediate or direct addressing word */
typedef struct machine_word {
    boolean isLabel; /* Here we will place the labels in the first pass */
    char labelName[MAX_LABEL_LENGTH+1]; 
    WordType type; /* This indicates the type of word */
    union word {
        first_word first_word;
        immdt_drct_word immdt_drct_word;
        rgstr_word register_word;
        data_word data_word;
    } word;
} machine_word;

#endif /* UTILS_H */
