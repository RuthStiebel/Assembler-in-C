This assembler was built as the final project for a course in C in the Open University of Israel, (course number 20465).

The program receives any number of '.as' files and returns three files. The most important of them is the '.obj' files that has each line of the original files codes as base 64 bits. The other two files are created as needed.  A '.ext' file stores all the addresses of the places that call for an external label and a '.ent' file that stores all the labels defined in the document.

The format of the output files is as follows:

  '.obj' file:
      At the top of the file will be printed the value of the IC (instruction counter) and the value of the DC (data counter).
      After that, the data will appear encoded in 64bits.
      
  '.ent' file:


  '.ext file:'
