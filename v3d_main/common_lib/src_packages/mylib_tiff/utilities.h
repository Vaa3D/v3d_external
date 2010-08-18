/*****************************************************************************************\
*                                                                                         *
*  Utilities for allocating memory, opening files, and processing command line arguments  *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  October 2005                                                                  *
*                                                                                         *
\*****************************************************************************************/

#ifndef SR_UTILITIES

#define SR_UTILITIES

#include <stdio.h>

#define ASCII 128

/* The usual protected allocation and file opening routines.   */

void *Guarded_Malloc(size_t size, char *routine);
void *Guarded_Realloc(void *array, size_t size, char *routine);
char *Guarded_Strdup(char *string, char *routine);
FILE *Guarded_Fopen(char *name, char *options, char *routine);

/* Process the command line according to 'spec' (See DOCUMENT/utilities.pdf for details).  Any
   failure results in an error message and the end of execution.  One tricky thing
   here is that 'spec' itself is interpreted, so an invalid spec will also produce
   an error message.  Be sure to debug the spec!  If no_escapes is non-zero then any
   escaping quotes in the specification will not be seen in a usage statement, should
   one be printed out with an error.                                                  */ 

void Process_Arguments(int argc, char *argv[], char *spec[], int no_escapes);

/* Once the command line has been parsed you can get the value of any argument by calling
   the appropriately typed 'Get' routine with the name of the argument.  You presumably
   know the type since you wrote the spec.  If an argument is an iterator then you must
   further suppply the 'Get' call with the index of the instance you want, where the numbering
   starts at 1.  To know how many instances there are call Get_Repeat_Count. If an argument
   is a multi-value option then you must also specify which value you want.  Is_Arg_Matched
   will tell you if any particular argument has been matched on the command line or not.
   Finally, you can get the program name with Program_Name.                             */

char  *Program_Name();

int    Get_Repeat_Count(char *name);
int    Is_Arg_Matched(char *name, ... /* [int no] */ );

int    Get_Int_Arg   (char *name, ... /* [int no [int an]] */ );
double Get_Double_Arg(char *name, ... /* [int no [int an]] */ );
char  *Get_String_Arg(char *name, ... /* [int no [int an]] */ );

/* There may be constraints among the arguments that are not captured by the spec that
   you explictly check after the call to Process_Args.  If you detect an error and wish
   to print out a usage message, a call to Print_Argument_Usage will do so on the file
   'file'.  As for Processs_Arguments, passing in a non-zero no_escapes value suppresses
   the printing of escape chars in the statement.                                       */

void Print_Argument_Usage(FILE *file, int no_escapes);

#endif
