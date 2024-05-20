This assembler was built as the final project for a course in C in the Open University of Israel, (course no' 20465).

The program receives any number of '.as' files and returns three files per one '.as' file. The most important of them is the '.obj' files that has each line of the original files codes as base 64 bits. The other two files are created as needed.  A '.ext' file stores all the addresses of the places that call for an external label and a '.ent' file that stores all the labels defined in the document.

The format of the output files is as follows:

  '.obj' file:
      At the top of the file will be printed the value of the IC (instruction counter) and the value of the DC (data counter).
      After that, the data will appear encoded in 64bits.
      
  '.ent' file:
    The file will print two colomns: The left one would be the address of the label defined in the file and the right on would print the name of said label.


  '.ext file:'
   The file will print two colomns: The left one would be the addresses of the times the label was called and the right on would print the name of said label.

If, in the original file, there are no labels with a '.ext' prefix or no labels are defined in the file, then the '.ext' and '.ent files will not be configured, respectively.

The assembler is built of three main parts:
1. The preprocesser - this expands macros and removes comment lines in the original file. At the end of this stage the original '.as' file will be converted into a '.am' file.
2. The parser - this parses the file a line at a time and updates the instruction counter, data counter and respective arrays accordingly (this will later allow us to create the output files correctly).
3. The third and last part is when the output files are generated as explained above.

The code files are as following:
'main.c' - this file runs the program and all the sub-methods
'preprocessor.h' (and matching code file) - this is the preprocessor
'parser.h' (and matching code file) - this is the parser and it uses the following files:
   'directives.h' (and matching code file) - saves and parses the directives
   'instructions.h' (and matching code file) - saves and parses the instructions 
   'labels.h' (and matching code file) - saves the labels according to how they are defined in the file
'utils.h' - defines all the variables used 
'print.h' (and matching code file) - handles all the different print options
'makefile' - the project's makefile
   
