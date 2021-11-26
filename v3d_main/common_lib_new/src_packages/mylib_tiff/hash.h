/*****************************************************************************************\
*                                                                                         *
*  Hash Table data abstraction.                                                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  March 2006                                                                    *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

/* Hash strings to the integers from 0 ... N-1 where N is the number of
       strings in the table.   For each string or entry, the table also
       maintains a (void *) data field that the user may set as desired.
       One may create an initially empty table  with New_Hash_Table.  Size
       is the maximum number of entries you expect to put into the hash
       table whose vector size will be the first prime larger than
       size / CELL_RATIO(=.4).
*/

#ifndef _SR_HASH
#define _SR_HASH

#include "parameters.h"

typedef void Hash_Table;

Hash_Table *G(New_Hash_Table)(int size);

//  As per space management convention

Hash_Table *G(Copy_Hash_Table)(Hash_Table *table);
Hash_Table *Pack_Hash_Table(Hash_Table *R(M(table)));
Hash_Table *Inc_Hash_Table(Hash_Table *R(I(table)));
void        Free_Hash_Table(Hash_Table *F(table));
void        Kill_Hash_Table(Hash_Table *K(table));
void        Reset_Hash_Table();
int         Hash_Table_Usage();
void        Hash_Table_List(void (*handler)(Hash_Table *));
int         Hash_Table_Refcount(Hash_Table *table);
Hash_Table *G(Read_Hash_Table)(FILE *input);
void        Write_Hash_Table(Hash_Table *table, FILE *output);

/*   Clear_Hash_Table resets the hash table so that it is empty freeing
       all stored strings that were in it.
     Print_Hash_Table prints a representation of the hash table.
*/

void Clear_Hash_Table(Hash_Table *M(table));
void Print_Hash_Table(FILE *file, Hash_Table *table);

/*  Hash_Lookup returns the integer for string "entry" if it is in the table,
      otherwise it returns -1.
    Hash_Add adds the string "entry" to the table and returns its integer mapping
      if it is not yet in the table.  Otherwise it returns -1.
*/

int Hash_Lookup(Hash_Table *table, char *entry);
int Hash_Add(Hash_Table *M(table), char *entry);

/*  Get_Hash_Table_Size returns the number of entries currently in the table.
    Get_Hash_String returns the string corresponding to integer i.  The range of
      i is not checked, it is assumed to be in [0,Get_Hash_Table_Size()-1].
    Get_Hash_User_Hook returns a pointer to the void pointer associated with
      the given entry.
*/

int    Get_Hash_Size(Hash_Table *table);
char  *Get_Hash_String(Hash_Table *table, int i);
void **Get_Hash_User_Hook(Hash_Table *table, int i);

#endif
