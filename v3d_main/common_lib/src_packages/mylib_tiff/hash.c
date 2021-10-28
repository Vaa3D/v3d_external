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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "hash.h"

/* Hash Table cell or entry, index is implicitly given
   by position in table->cells array.                     */

typedef struct
{ int      next;  /* hash bucket link */
  int      text;  /* offset of string corresponding to entry in string array */
  void    *user;  /* user hook */
} Hash_Entry;

typedef struct
{ int         size;    /* size of hash vector */
  int         count;   /* number of entries in hash table */
  int         strmax;  /* current max of string array */
  int         strtop;  /* current top of string array */
  int        *vector;  /* hash vector */
  Hash_Entry *cells;   /* array where hash cells are allocated */
  char       *strings; /* array of entry strings */
} Table;

#define CELL_RATIO    .4  // maximum ratio of cells to hash vector length
#define STRING_RATIO   6  // expected average entry length (including terminating 0-byte)

#define T(x) ((Table *) x)


/*  Hash Table memory management  */

static inline int table_vsize(Table *table)
{ return (sizeof(int)*table->size); }

static inline int table_csize(Table *table)
{ return (sizeof(Hash_Entry)*((int) (table->size*CELL_RATIO))); }

static inline int table_ssize(Table *table)
{ return (table->strmax); }


typedef struct __Table
  { struct __Table *next;
    struct __Table *prev;
    int             refcnt;
    int             vsize;
    int             csize;
    int             ssize;
    Table           table;
  } _Table;

static _Table *Free_Table_List = NULL;
static _Table *Use_Table_List  = NULL;
static _Table  Table_Proto;

static int Table_Offset = ((char *) &(Table_Proto.table)) - ((char *) &Table_Proto);
static int Table_Inuse  = 0;

int Hash_Table_Refcount(Hash_Table *hash_table)
{ _Table *object = (_Table *) (((char *) hash_table) - Table_Offset);
  return (object->refcnt);
}

static inline void allocate_table_vector(Table *table, int vsize, char *routine)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  if (object->vsize < vsize)
    { table->vector = Guarded_Realloc(table->vector,vsize,routine);
      object->vsize = vsize;
    }
}

static inline int sizeof_table_vector(Table *table)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  return (object->vsize);
}

static inline void allocate_table_cells(Table *table, int csize, char *routine)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  if (object->csize < csize)
    { table->cells = Guarded_Realloc(table->cells,csize,routine);
      object->csize = csize;
    }
}

static inline int sizeof_table_cells(Table *table)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  return (object->csize);
}

static inline void allocate_table_strings(Table *table, int ssize, char *routine)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  if (object->ssize < ssize)
    { table->strings = Guarded_Realloc(table->strings,ssize,routine);
      object->ssize = ssize;
    }
}

static inline int sizeof_table_strings(Table *table)
{ _Table *object = (_Table *) (((char *) table) - Table_Offset);
  return (object->ssize);
}

static inline Table *new_table(int vsize, int csize, int ssize, char *routine)
{ _Table *object;
  Table  *table;

  if (Free_Table_List == NULL)
    { object = (_Table *) Guarded_Realloc(NULL,sizeof(_Table),routine);
      table = &(object->table);
      object->vsize = 0;
      table->vector = NULL;
      object->csize = 0;
      table->cells = NULL;
      object->ssize = 0;
      table->strings = NULL;
    }
  else
    { object = Free_Table_List;
      Free_Table_List = object->next;
      table = &(object->table);
    }
  Table_Inuse += 1;
  object->refcnt = 1;
  if (Use_Table_List != NULL)
    Use_Table_List->prev = object;
  object->next = Use_Table_List;
  object->prev = NULL;
  Use_Table_List = object;
  allocate_table_vector(table,vsize,routine);
  allocate_table_cells(table,csize,routine);
  allocate_table_strings(table,ssize,routine);
  return (table);
}

static inline Table *copy_table(Table *table)
{ Table *copy = new_table(table_vsize(table),table_csize(table),table_ssize(table),"Copy_Hash_Table");
  void *_vector = copy->vector;
  void *_cells = copy->cells;
  void *_strings = copy->strings;
  *copy = *table;
  copy->vector = _vector;
  if (table->vector != NULL)
    memcpy(copy->vector,table->vector,table_vsize(table));
  copy->cells = _cells;
  if (table->cells != NULL)
    memcpy(copy->cells,table->cells,table_csize(table));
  copy->strings = _strings;
  if (table->strings != NULL)
    memcpy(copy->strings,table->strings,table_ssize(table));
  return (copy);
}

Hash_Table *Copy_Hash_Table(Hash_Table *hash_table)
{ return ((Hash_Table *) copy_table(((Table *) hash_table))); }

static inline void pack_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (object->vsize > table_vsize(table))
    { object->vsize = table_vsize(table);
      if (object->vsize != 0)
        table->vector = Guarded_Realloc(table->vector,
                                        object->vsize,"Pack_Table");
      else
        { free(table->vector);
          table->vector = NULL;
        }
    }
  if (object->csize > table_csize(table))
    { object->csize = table_csize(table);
      if (object->csize != 0)
        table->cells = Guarded_Realloc(table->cells,
                                       object->csize,"Pack_Table");
      else
        { free(table->cells);
          table->cells = NULL;
        }
    }
  if (object->ssize > table_ssize(table))
    { object->ssize = table_ssize(table);
      if (object->ssize != 0)
        table->strings = Guarded_Realloc(table->strings,
                                         object->ssize,"Pack_Table");
      else
        { free(table->strings);
          table->strings = NULL;
        }
    }
}

Hash_Table *Pack_Hash_Table(Hash_Table *hash_table)
{ pack_table(((Table *) hash_table));
  return (hash_table);
}

Hash_Table *Inc_Hash_Table(Hash_Table *hash_table)
{ _Table *object  = (_Table *) (((char *) hash_table) - Table_Offset);
  object->refcnt += 1;
  return (hash_table);
}

static inline void free_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Hash_Table\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Table_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Table_List;
  Free_Table_List = object;
  Table_Inuse -= 1;
}

void Free_Hash_Table(Hash_Table *hash_table)
{ free_table(((Table *) hash_table)); }

static inline void kill_table(Table *table)
{ _Table *object  = (_Table *) (((char *) table) - Table_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Hash_Table\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Table_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  if (object->ssize != 0)
    free(table->strings);
  if (object->csize != 0)
    free(table->cells);
  if (object->vsize != 0)
    free(table->vector);
  free(((char *) table) - Table_Offset);
  Table_Inuse -= 1;
}

void Kill_Hash_Table(Hash_Table *hash_table)
{ kill_table(((Table *) hash_table)); }

static inline void reset_table()
{ _Table *object;
  Table  *table;
  while (Free_Table_List != NULL)
    { object = Free_Table_List;
      Free_Table_List = object->next;
      table = &(object->table);
      if (object->ssize != 0)
        free(table->strings);
      if (object->csize != 0)
        free(table->cells);
      if (object->vsize != 0)
        free(table->vector);
      free(object);
    }
}

void Reset_Hash_Table()
{ reset_table(); }

int Hash_Table_Usage()
{ return (Table_Inuse); }

void Hash_Table_List(void (*handler)(Hash_Table *))
{ _Table *a, *b;
  for (a = Use_Table_List; a != NULL; a = b)
    { b = a->next;
      handler((Hash_Table *) &(a->table));
    }
}

static inline Table *read_table(FILE *input)
{ char name[10];
  fread(name,10,1,input);
  if (strncmp(name,"Hash_Table",10) != 0)
    return (NULL);
  Table *obj = new_table(0,0,0,"Read_Hash_Table");
  fread(obj,sizeof(Table),1,input);
  obj->vector = NULL;
  if (table_vsize(obj) != 0)
    { allocate_table_vector(obj,table_vsize(obj),"Read_Hash_Table");
      fread(obj->vector,table_vsize(obj),1,input);
    }
  obj->cells = NULL;
  if (table_csize(obj) != 0)
    { allocate_table_cells(obj,table_csize(obj),"Read_Hash_Table");
      fread(obj->cells,table_csize(obj),1,input);
    }
  obj->strings = NULL;
  if (table_ssize(obj) != 0)
    { allocate_table_strings(obj,table_ssize(obj),"Read_Hash_Table");
      fread(obj->strings,table_ssize(obj),1,input);
    }
  return (obj);
}

Hash_Table *Read_Hash_Table(FILE *input)
{ return ((Hash_Table *) read_table(input)); }

static inline void write_table(Table *table, FILE *output)
{ fwrite("Hash_Table",10,1,output);
  fwrite(table,sizeof(Table),1,output);
  if (table_vsize(table) != 0)
    fwrite(table->vector,table_vsize(table),1,output);
  if (table_csize(table) != 0)
    fwrite(table->cells,table_csize(table),1,output);
  if (table_ssize(table) != 0)
    fwrite(table->strings,table_ssize(table),1,output);
}

void Write_Hash_Table(Hash_Table *hash_table, FILE *output)
{ write_table(((Table *) hash_table),output); }

/* Hash key for a string is xor of each consecutive 3 bytes. */

static int hash_key(char *entry)
{ int i, key, glob;

  key = 0;
  glob = 0;
  for (i = 0; entry[i] != '\0'; i++)
    { glob = (glob << 8) | entry[i];
      if (i % 3 == 2)
        { key = key ^ glob;
          glob = 0;
        }
    }
  if (i % 3 != 0)
    key = key ^ glob;
  return (key);
}

/*  Find the next prime larger than size.  Variant of Sieve of Arosthenes:  First
      time its called computes all primes between 2 and 0xFFFF using the basic
      sieve algorithm.  With these one builds a sieve for the 0xFFFF number from
      size upwards, using the primes to x-out sieve elements as being non-prime.
      This will work up to 0xFFFFFFF, beyond the largest integer, because it suffices
      to sieve agains the square root of the large number in the sieve.              */

static int next_prime(int size)
{ static int           firstime = 1;
  static int           Prime[0x4000], Ptop;
  static unsigned char Sieve[0x10000];
  int p, q, n;

  if (firstime)
    { firstime = 0;

      Ptop = 0;
      for (p = 2; p < 0x10000; p++)
        Sieve[p] = 1;
      for (p = 2; p < 0x10000; p++)
        if (Sieve[p])
          { for (q = 2*p; q < 0x10000; q += p)
              Sieve[q] = 0;
            Prime[Ptop++] = p;
          }
    }

  while (size < 0x8FFEFFFF)
    { for (q = 0; q < 0x10000; q++)
        Sieve[q] = 1;
      for (p = 0; p < Ptop; p++)
        { n = Prime[p];
          if (n >= size) break;
          for (q = ((size-1)/n+1)*n - size; q < 0x10000; q += n)
            Sieve[q] = 0;
        }
      for (q = 0; q < 0x10000; q++)
        if (Sieve[q])
          return (size+q);
      size += 0x10000;
    }
  return (size);
}


/* Diagnostic output of hash table contents. */

void Print_Hash_Table(FILE *file, Hash_Table *hash_table)
{ Table      *table  = T(hash_table);
  int        *vector = table->vector;
  Hash_Entry *cells  = table->cells;
  int         i, c;

  fprintf(file,"\nHASH TABLE (%d,%d):\n",table->count,table->size);
  for (i = 0; i < table->size; i++)
    if ((c = vector[i]) >= 0)
      { fprintf(file,"  Vector %4d:\n",i);
        for (; c >= 0; c = cells[c].next)
          fprintf(file,"    %4d: '%s'\n",c,table->strings + cells[c].text);
      }
}

Hash_Table *G(New_Hash_Table)(int size)
{ Table *table;
  int    i, smax;

  size  = next_prime((int) (size/CELL_RATIO));
  smax  = size*STRING_RATIO;
  table = new_table(sizeof(int)*size,sizeof(Hash_Entry)*((int) (size*CELL_RATIO)),smax,
                    "New_Hash_Table");
  table->count  = 0;
  table->size   = size;
  table->strmax = smax;
  table->strtop = 0;
  for (i = 0; i < size; i++)
    table->vector[i] = -1;

  return ((Hash_Table *) table);
}

/* Double the size of a hash table
   while preserving its contents.    */

static Table *double_hash_table(Table *table)
{ int size, smax;

  size = next_prime(2*table->size);
  smax = 2.1 * table->strtop + 1000;
 
  allocate_table_vector(table,sizeof(int)*size,"Hash_Add");
  allocate_table_cells(table,sizeof(Hash_Entry)*((int) (size*CELL_RATIO)),"Hash_Add");
  allocate_table_strings(table,smax,"Hash_Add");

  { int        *vector = table->vector;
    Hash_Entry *cells  = table->cells;
    int         c;

    table->size   = size;
    table->strmax = smax;
    for (c = 0; c < size; c++)
      vector[c] = -1;
    for (c = 0; c < table->count; c++)
      { int key = hash_key(table->strings + cells[c].text) % size;
        cells[c].next = vector[key];
        vector[key] = c;
      }
  }

  return (table);
}

/* Lookup string 'entry' in table 'table' and return its
   unique nonnegative id, or -1 if it is not in the table. */

int Hash_Lookup(Hash_Table *hash_table, char *entry)
{ Table *table = T(hash_table);
  int    key, chain;

  key   = hash_key(entry) % table->size;
  chain = table->vector[key];
  while (chain >= 0)
    { if (strcmp(table->strings + table->cells[chain].text,entry) == 0)
        return (chain);
      chain = table->cells[chain].next;
    }
  return (-1);
}

/* Add string 'entry' in table 'table' and return its assigned
   uniqe nonnegative id, or -1 if it is already in the table.  */

int Hash_Add(Hash_Table *M(hash_table), char *entry)
{ Table *table = T(hash_table);
  int key, chain, len;

  key   = hash_key(entry) % table->size;
  chain = table->vector[key];
  while (chain >= 0)
    { if (strcmp(table->strings + table->cells[chain].text,entry) == 0)
        return (-1);
      chain = table->cells[chain].next;
    }

  if (table->count+1 > table->size*CELL_RATIO)
    { table = double_hash_table(table);
      key   = hash_key(entry) % table->size;
    }

  chain = table->count;
  table->cells[chain].next = table->vector[key];
  table->vector[key] = chain;

  len = strlen(entry) + 1;
  if (table->strtop + len > table->strmax)
    { int size = table->size;
      int smax = (table->strtop + len) * (CELL_RATIO * table->size / table->count) * 1.1 + 1000;
      allocate_table_strings(table,smax,"Hash_Add");
      table->strmax = smax;
    }
  strcpy(table->strings + table->strtop, entry);
  table->cells[chain].text = table->strtop;
  table->strtop += len;
  return (table->count++);
}

/* Return the size of the hash table. */

int Get_Hash_Table_Size(Hash_Table *hash_table)
{ return (T(hash_table)->count); }

/* Return the string with unique id i in table. */

char *Get_Hash_String(Hash_Table *hash_table, int i)
{ return (T(hash_table)->strings + T(hash_table)->cells[i].text); }

/* Return a pointer to the user data field for unique id i in table. */

void **Get_Hash_User_Hook(Hash_Table *hash_table, int i)
{ return (& (T(hash_table)->cells[i].user)); }

/* Clear the contents of hash table, reseting it to be empty. */

void Clear_Hash_Table(Hash_Table *M(hash_table))
{ Table *table = T(hash_table);
  int    i;

  table->count  = 0;
  table->strtop = 0;
  for (i = 0; i < table->size; i++)
    table->vector[i] = -1;
}
