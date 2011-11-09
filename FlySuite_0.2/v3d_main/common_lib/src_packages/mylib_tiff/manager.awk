###########################################################################################
#                                                                                         #
#  Object manager macro awk translator                                                    #
#                                                                                         #
#  Author:  Gene Myers                                                                    #
#  Date  :  June 2009                                                                     #
#                                                                                         #
#  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   #
#      Copyrighted as per the full copy in the associated 'README' file                   #
#                                                                                         #
###########################################################################################
#
#   Expand MANAGER declaration lines.  These are required to have the syntax:
#
#    'MANAGER' [-cprfk[iI][oO]] <class_names> <field_descriptor> *
#
#       <class_names> <- <visible_class_name>('('<hidden_class_name>')')?
#
#       <field_desriptor> <- <block_name>':'<wrapper_size_name>
#                          | <block_name>'!'<64bit_wrapper_size_name>
#                          | <field_name>'*'<sub_class>
#                          | <field_name>'@'<referenced_class>

BEGIN { FS = " +"; }

/^MANAGER/ {

# determine flag settings

  copy = pack = reset = free = kill = read = write = 0;
  input = output = 0;
  if (index($2,"-") == 1)
    { if (index($2,"c") != 0)
        copy = 1;
      if (index($2,"p") != 0)
        pack = 1;
      if (index($2,"r") != 0)
        reset = 1;
      if (index($2,"f") != 0)
        free = 1;
      if (index($2,"k") != 0)
        kill = 1;
      if (index($2,"i") != 0)
        input = read = 1;
      if (index($2,"o") != 0)
        output = write = 1;
      if (index($2,"I") != 0)
        input = 1;
      if (index($2,"O") != 0)
        output = 1;
      base = 3;
    }
  else
    base = 2;

# setup class names

  if ((j = index($base,"(")) != 0)
    { hidden = 1;
      Y = substr($base,1,j-1);
      X = substr($base,j+1,length($base)-(j+1));
    } 
  else
    { hidden = 0;
      Y = $base;
      X = Y;
    }
  y = tolower(Y);
  x = tolower(X);
  if (hidden)
    z = "((" X " *) " y ")";
  else
    z = y;

# interpret field information

  WRAPPER   = 0;
  WRAPPER64 = 1;
  SUBOBJECT = 2;
  REFERENCE = 3;
  if (NF > base)
    { for (i = base+1; i <= NF; i++)
        { if ((j = index($i,":")) != 0)
            type[i] = WRAPPER;
          else if ((j = index($i,"!")) != 0)
            type[i] = WRAPPER64;
          else if ((j = index($i,"*")) != 0)
            type[i] = SUBOBJECT;
          else if ((j = index($i,"@")) != 0)
            type[i] = REFERENCE;
          else
            { print "ERROR: illegal macro argument \"" $i "\"";
              exit;
            }
          field[i] = substr($i,1,j-1);
          value[i] = substr($i,j+1,length($i)-j);
          fname[i] = field[i];
          gsub("[^a-zA-Z0-9_]+","_",fname[i]);
        }
    }

# generate container and free list declarations

  print  "";
  print  "typedef struct __" X;
  print  "  { struct __" X " *next;";
  print  "    struct __" X " *prev;";
  printf "    int%*srefcnt;\n", length(X)+8, "";
  for (i = base+1; i <= NF; i++)
    if (type[i] == WRAPPER) 
      printf "    int%*s%s;\n", length(X)+8, "", value[i];
    else if (type[i] == WRAPPER64) 
      printf "    uint64%*s%s;\n", length(X)+5, "", value[i];
  print "    " X "           " x ";";
  print "  } _" X ";";
  print "";
  print "static _" X " *Free_" X "_List = NULL;";
  print "static _" X " *Use_" X "_List  = NULL;";
  print "static _" X "  " X "_Proto;";
  print "";
  print "static int " X "_Offset = ((char *) &(" X "_Proto." x ")) - ((char *) &" X "_Proto);";
  print "static int " X "_Inuse  = 0;";

# generate <X>_Refcount

  print  ""
  print  "int " Y "_Refcount(" Y " *" y ")"; 
  print  "{ _" X " *object = (_" X " *) (((char *) " y ") - " X "_Offset);";
  print  "  return (object->refcnt);";
  print  "}"

# generate allocate_<x>_<field[i]> and sizeof_<x>_<field[i]>

  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { print ""
        if (type[i] == WRAPPER)
          printf "static inline void allocate_%s_%s(%s *%s, int %s, char *routine)\n",
                 x, fname[i], X, x, value[i]; 
        else # type[i] == WRAPPER64
          printf "static inline void allocate_%s_%s(%s *%s, uint64 %s, char *routine)\n",
                 x, fname[i], X, x, value[i]; 
        print  "{ _" X " *object = (_" X " *) (((char *) " x ") - " X "_Offset);";
        print  "  if (object->" value[i] " < " value[i] ")";
        printf "    { %s->%s = Guarded_Realloc(%s->%s,%s,routine);\n",
               x, field[i], x, field[i], value[i];
        print  "      object->" value[i] " = " value[i] ";";
        print  "    }";
        print  "}";

        print ""
        if (type[i] == WRAPPER)
          printf "static inline int sizeof_%s_%s(%s *%s)\n",
                 x, fname[i], X, x; 
        else # type[i] == WRAPPER64
          printf "static inline uint64 sizeof_%s_%s(%s *%s)\n",
                 x, fname[i], X, x; 
        print  "{ _" X " *object = (_" X " *) (((char *) " x ") - " X "_Offset);";
        print  "  return (object->" value[i] ");";
        print  "}";
      }

# generate new_<x>

  print "";
  printf "static inline %s *new_%s(", X, x;
  for (i = base+1; i <= NF; i++)
    if (type[i] == WRAPPER)
      printf "int %s, ", value[i];
    else if (type[i] == WRAPPER64)
      printf "uint64 %s, ", value[i];
  printf "char *routine)\n";
  print "{ _" X " *object;";
  print "  " X "  *" x ";";
  print "";
  print "  if (Free_" X "_List == NULL)";
  print "    { object = (_" X " *) Guarded_Realloc(NULL,sizeof(_" X "),routine);";
  print "      " x " = &(object->" x ");"
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { print "      object->" value[i] " = 0;";
        print "      " x "->" field[i] " = NULL;";
      }
  print "    }";
  print "  else";
  print "    { object = Free_" X "_List;";
  print "      Free_" X "_List = object->next;";
  print "      " x " = &(object->" x ");"
  print "    }";
  print "  " X "_Inuse += 1;";
  print "  object->refcnt = 1;";
  print "  if (Use_" X "_List != NULL)";
  print "    Use_" X "_List->prev = object;";
  print "  object->next = Use_" X "_List;";
  print "  object->prev = NULL;"
  print "  Use_" X "_List = object;";
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      print "  allocate_" x "_" fname[i] "(" x "," value[i] ",routine);";
    else
      print "  " x "->" field[i] " = NULL;";
  print "  return (" x ");";
  print "}";

# generate copy_<x> and Copy_<Y> if -c is not set

  print "";
  print "static inline " X " *copy_" x "(" X " *" x ")";
  printf "{ %s *copy = new_%s(", X, x;
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      printf "%s_%s(%s),", x, value[i], x; 
  printf "\"Copy_%s\");\n", Y;
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      print "  void *_" field[i] " = copy->" field[i] ";";
  print "  *copy = *" x ";";
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { print "  copy->" field[i] " = _" field[i] ";";
        print "  if (" x "->" field[i] " != NULL)";
        print "    memcpy(copy->" field[i] "," x "->" field[i] "," x "_" value[i] "(" x "));";
      }
    else
      { print "  if (" x "->" field[i] " != NULL)";
        if (type[i] == REFERENCE)
          print "    Inc_" value[i] "(" x "->" field[i] ");";
        else
          print "    copy->" field[i] " = Copy_" value[i] "(" x "->" field[i] ");";
      }
  print "  return (copy);"
  print "}";

  if (!copy)
    { print "";
      print Y " *Copy_" Y "(" Y " *" y ")";
      print "{ return ((" Y " *) copy_" x "(" z ")); }";
    }

# generate pack_<x> and Pack_<Y> if the -p flag is not set

  print "";
  print "static inline void pack_" x "(" X " *" x ")";
  print "{ _" X " *object  = (_" X " *) (((char *) " x ") - " X "_Offset);";
  for (i = base+1; i <= NF; i++)
    if (type[i] <= WRAPPER64)
      { print "  if (object->" value[i] " > " x "_" value[i] "(" x "))";
        print "    { object->" value[i] " = " x "_" value[i] "(" x ");";
        print "      if (object->" value[i] " != 0)";
        print "        " x "->" field[i] " = Guarded_Realloc(" x "->" field[i] ",";
        printf "%*s  object->%s,\"Pack_%s\");\n", 27 + length(field[i]) + length(x), "",
                                                  value[i], X;
        print "      else";
        print "        { free(" x "->" field[i] ");";
        print "          " x "->" field[i] " = NULL;"
        print "        }"
        print "    }";
      }
    else if (type[i] == SUBOBJECT)
      { print "  if (" x "->" field[i] " != NULL)";
        print "    Pack_" value[i] "(" x "->" field[i] ");";
      }
  print "}";

  if (!pack)
    { print "";
      print Y " *Pack_" Y "(" Y " *" y ")";
      print "{ pack_" x "(" z ");";
      print "  return (" y ");";
      print "}";
    }

# generate <Y> *Inc_<Y>(<Y> *<y>)

  print ""
  print Y " *Inc_" Y "(" Y " *" y ")";
  print "{ _" X " *object  = (_" X " *) (((char *) " y ") - " X "_Offset);";
  print "  object->refcnt += 1;";
  print "  return (" y ");";
  print "}";

# generate free_<x> and Free_<Y> if the -f flag is not set

  print "";
  print "static inline void free_" x "(" X " *" x ")";
  print "{ _" X " *object  = (_" X " *) (((char *) " x ") - " X "_Offset);";
  print "  if (--object->refcnt > 0) return;";
  print "  if (object->refcnt < 0)";
  print "    fprintf(stderr,\"Warning: Freeing previously released " Y "\\n\");";
  print "  if (object->prev != NULL)";
  print "    object->prev->next = object->next;";
  print "  else";
  print "    Use_" X "_List = object->next;";
  print "  if (object->next != NULL)";
  print "    object->next->prev = object->prev;";
  print "  object->next = Free_" X "_List;";
  print "  Free_" X "_List = object;";
  for (i = NF; i > base; i--)
    if (type[i] > WRAPPER64)
      { print "  if (" x "->" field[i] " != NULL)";
        print "    Free_" value[i] "(" x "->" field[i] ");"
      }
  print "  " X "_Inuse -= 1;";
  print "}";

  if (!free)
    { print "";
      print "void Free_" Y "(" Y " *" y ")";
      print "{ free_" x "(" z "); }";
    }

# generate kill_<x> and Kill_<Y> if the -k flag is not set

  print "";
  print "static inline void kill_" x "(" X " *" x ")";
  print "{ _" X " *object  = (_" X " *) (((char *) " x ") - " X "_Offset);";
  print "  if (--object->refcnt > 0) return;";
  print "  if (object->refcnt < 0)";
  print "    fprintf(stderr,\"Warning: Killing previously released " Y "\\n\");";
  print "  if (object->prev != NULL)";
  print "    object->prev->next = object->next;";
  print "  else";
  print "    Use_" X "_List = object->next;";
  print "  if (object->next != NULL)";
  print "    object->next->prev = object->prev;";
  for (i = NF; i > base; i--)
    if (type[i] > WRAPPER64)
      { print "  if (" x "->" field[i] " != NULL)"
        print "    Kill_" value[i] "(" x "->" field[i] ");"
      }
    else
      { print "  if (object->" value[i] " != 0)"
        print "    free(" x "->" field[i] ");"
      }
  print "  free(((char *) " x ") - " X "_Offset);";
  print "  " X "_Inuse -= 1;";
  print "}";

  if (!kill)
    { print "";
      print "void Kill_" Y "(" Y " *" y ")";
      print "{ kill_" x "(" z "); }";
    }

# generate reset_<x> and Reset_<Y> if the -r flag is not set

  print "";
  print "static inline void reset_" x "()";
  print "{ _" X " *object;";
  print "  " X "  *" x ";";
  print "  while (Free_" X "_List != NULL)";
  print "    { object = Free_" X "_List;";
  print "      Free_" X "_List = object->next;";
  if (base < NF)
    { print "      " x " = &(object->" x ");";
      for (i = NF; i > base; i--)
        if (type[i] > WRAPPER64)
          { print "      if (" x "->" field[i] " != NULL)"
            print "        Kill_" value[i] "(" x "->" field[i] ");"
          }
        else
          { print "      if (object->" value[i] " != 0)"
            print "        free(" x "->" field[i] ");"
          }
    }
  print "      free(object);";
  print "    }";
  print "}";

  if (!reset)
    { print "";
      print "void Reset_" Y "()";
      print "{ reset_" x "(); }";
    }

# generate <Y>_Usage

  print "";
  print "int " Y "_Usage()";
  print "{ return (" X "_Inuse); }";

#generate Used_<Y>_List

  print "";
  print "void " Y "_List(void (*handler)(" Y " *))";
  print "{ _" X " *a, *b;"; 
  print "  for (a = Use_" X "_List; a != NULL; a = b)";
  print "    { b = a->next;";
  print "      handler((" Y " *) &(a->" x "));";
  print "    }";
  print "}";

# if -i or -I is set then generate read_<x> and if -i is not set then generate Read_<Y>

  if (input)
    { print "";
      print "static inline " X " *read_" x "(FILE *input)";
      print "{ char name[" length(Y) "];";
      print "  fread(name," length(Y) ",1,input);";
      print "  if (strncmp(name,\"" Y "\"," length(Y) ") != 0)";
      print "    return (NULL);";
      printf "  %s *obj = new_%s(", X, x;
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64)
          printf "0,"; 
      printf "\"Read_%s\");\n", Y;
      print "  fread(obj,sizeof(" X "),1,input);";
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64)
          { print "  obj->" field[i] " = NULL;";
            print "  if (" x "_" value[i] "(obj) != 0)";
            print "    { allocate_" x "_" field[i] "(obj," x "_" value[i] "(obj),\"Read_" Y "\");";
            print "      fread(obj->" field[i] "," x "_" value[i] "(obj),1,input);";
            print "    }";
          }
        else if (type[i] == SUBOBJECT)
          { print "  if (obj->" field[i] " != NULL)";
            print "    obj->" field[i] " = Read_" value[i] "(input);";
          }
        else
          print "    obj->" field[i] " = NULL;";
      print "  return (obj);"
      print "}";
    
      if (!read)
        { print "";
          print Y " *Read_" Y "(FILE *input)";
          print "{ return ((" Y " *) read_" x "(input)); }";
        }
    }

# if -o or -O is set then generate write_<x> and if -o is not set then generate Write_<Y>

  if (output)
    { print "";
      print "static inline void write_" x "(" X " *" x ", FILE *output)";
      print "{ fwrite(\"" Y "\"," length(Y) ",1,output);";
      print "  fwrite(" x ",sizeof(" X "),1,output);";
      for (i = base+1; i <= NF; i++)
        if (type[i] <= WRAPPER64)
          { print "  if (" x "_" value[i] "(" x ") != 0)";
            print "    fwrite(" x "->" field[i] "," x "_" value[i] "(" x "),1,output);";
          }
        else if (type[i] == SUBOBJECT)
          { print "  if (" x "->" field[i] " != NULL)";
            print "    Write_" value[i] "(" x "->" field[i] ",output);";
          }
      print "}";

      if (!write)
        { print "";
          print "void Write_" Y "(" Y " *" y ", FILE *output)";
          print "{ write_" x "(" z ",output); }";
        }
    }
}

! /^MANAGER/ { print $0; }
