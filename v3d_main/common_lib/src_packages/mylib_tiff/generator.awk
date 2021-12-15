###########################################################################################
#                                                                                         #
#  Generator macro awk translator                                                         #
#                                                                                         #
#  Author:  Gene Myers                                                                    #
#  Date  :  June 2009                                                                     #
#                                                                                         #
#  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   #
#      Copyrighted as per the full copy in the associated 'README' file                   #
#                                                                                         #
###########################################################################################
#
#   Expands #GENERATE statements to ease the burden of programming for each of a set of
#     instances in a list, say the types of an array.  #GENERATE statements, and all other
#     #-statments (save for #WHEN) must occur on a line by themselves.  The statements are:
#
#   #LISTDEF @<lvar> = <item_1> [ ... <item_n> ]
#
#     A #LISTDEF statement defines a list-variable, @<lvar>, that denotes the list of items
#     after the =-sign, and that can be used in subsequent #GENERATE statements to specify
#     what they iterate over.  White space separates the items that may be any string that
#     does not contain whitespace.  The lvar name can be any string that starts with an
#     @-sign and does not contain white space.
#
#   #GENERATE <uvar_1> [ , ... , <uvar_k> ] = <list_1>  [ , ... , <list_k> ]
#     <code>
#   #END
#
#         where   <list_j>  <- <item_1> [ ... <item_n> ] | @<lvar> 
#
#     The <code> between the #GENERATE and its bracketing #END statement is repeated once
#     for each ordered k-tuple of items from the k lists in the header.  For the i'th iteration,
#     <uvar_j> will designate the i'th value in the list designated by <list_j>.  Obviously all
#     k lists must have the same length, say n, and the <code> is repeated n times in sequence.
#     Within the i'th copy of the code, occurences of '<'uvar_j'>' are replaced by the i'th item
#     in <list_j>, say it is item_ji.  Occurences of '<'lower_case(uvar_j)'>' are replaced by 
#     lower_case(item_ji) and occurences of '<'upper_case(uvar_j)'>' are replaced by
#     upper_case(item_ji).  If the upper_case(uvar) = uvar and/or lower_case(uvar) = uvar,
#     then item_ji is substituted.  Obviously when @<lvar> is designated as a list in the
#     header it is denoting the list that it was last defined to be assigned to in a #LISTDEF
#     statement.
#
#   #IF <predicate>
#      <code>
#   #ELSEIF <predicate>
#      <code>
#   ...
#   #ELSE
#      <code>
#   #END
#
#      The code for the first true predicate in the IF-chain is retained.  The predicate can
#      be any expression built up from &&, ||, <=, >=, <, >, !=, ==, +, -, *, /, % and uvars
#      from enclosing #GENERATE statements.  Note carefully that parenthesis are not permitted,
#      nor is the negation operator !.  (Sorry, I didn't want to have to work too hard on the
#      interpretor for evaluating predicates and made this simplification as any desired expression
#      is expressible in this restricted from by deMorgan's rule and the distributive law for +/*.
#      The value of a uvar is its ordinal position of its current item in the list generating it,
#      unless the list is of integer constants in which case its value is that of the constant.
#
#  <code>  #WHEN <predicate>
#
#      A convenience when only one line of code is conditional.  If the predicate is true
#      then <code> is retained, otherwise it is not.

BEGIN { FS = " +";
        nest = 0;
        nblks = 0;

        WHITE = "[ \t]+";

        DEBUG = 0;

        Gen_Line    = 0;
        If_Line     = 1;
        ElseIf_Line = 2;
        Else_Line   = 3;
        End_Line    = 4;
        When_Line   = 5;
      }

{ if ((k = index($0,"#GENERATE")) != 0)
    { if (substr($0,1,k-1) !~ "^[ \t]*$")
        { print "ERROR Line ", NR, ": #GENERATE must be on a line by itself" > "/dev/stderr";
          exit;
        }

      type[nest] = Gen_Line;
      line[nest] = NR;
      srce[nest] = nblks;
      nest += 1;

      op[nblks]  = Gen_Line;
      len[nblks] = 0;

      control = substr($0,k+9,length($0)-(k+8));
      if ((j = index(control,"=")) == 0)
        { print "ERROR Line ", NR, ": #GENERATE does not contain an =-sign" > "/dev/stderr";
          exit;
        }

      nvars[nblks] = n = split(substr(control,1,j-1),varlist,",");
      for (i = 1; i <= n; i++)
        { gsub(WHITE,"",varlist[i]);
          vars[nblks,i] = varlist[i];
        }

      n = split(substr(control,j+1,length(control)-j),thelists,",");
      if (n != nvars[nblks])
        { print "ERROR Line ", NR, ": #GENERATE vars and lists don't correspond" > "/dev/stderr";
          exit;
        }
      for (i = 1; i <= n; i++)
        { m = k = split(thelists[i],aglist,WHITE);
          if (aglist[m] == "")
            k = k-1;
          firstvar = aglist[1];
          if (firstvar == "")
            { k = k-1;
              firstvar = aglist[2];
            }
          if (k == 0)
            { print "ERROR Line ", NR, ": #GENERATE list cannot be empty" > "/dev/stderr";
              exit;
            }

          isanlvar = 0;
          if (k == 1 && index(firstvar,"@") == 1)
            { isanlvar = 1;
              if ( ! (firstvar in listlen))
                { print "ERROR Line ", NR, ": List var " firstvar " is not defined" > "/dev/stderr";
                  exit;
                }
              k = listlen[firstvar];
            }

          if (i > 1)
            { if (k != nopts[nblks])
                { print "ERROR Line ", NR,
                        ": #GENERATE list lengths are not all the same" > "/dev/stderr";
                  exit;
                }
            }
          else
            nopts[nblks] = k;

          if (isanlvar)
            for (j = 1; j <= k; j++)
              glist[nblks,i,j] = listitems[firstvar,j];
          else
            { k = 0;
              for (j = 1; j <= m; j++)
                if (aglist[j] != "")
                  { k += 1;
                    glist[nblks,i,k] = aglist[j];
                  }
            }

          number = 1;
          for (j = 1; j <= k; j++)
            if (glist[nblks,i,j] !~ /^-?[0-9]+$/)
              { number = 0;
                break;
              }
          ordn[nblks,i] = number;

          if (DEBUG)
            print "  var '" vars[nblks,i] "' has ord " number;
        }

      nblks += 1;
      blen = 0;
    }

  else if ((k = index($0,"#IF")) != 0)
    { if (substr($0,1,k-1) !~ "^[ \t]*$")
        { print "ERROR Line ", NR, ": #IF must be on a line by itself" > "/dev/stderr";
          exit;
        }
      if (nest <= 0)
        { print "ERROR Line ", NR, ": #IF is not nested withing a #GENERATE block" > "/dev/stderr";
          exit;
        }
      type[nest] = If_Line;
      line[nest] = NR;
      srce[nest] = nblks;
      nest += 1;

      op[nblks]  = If_Line;
      len[nblks] = 0;
      expr[nblks] = substr($0,k+3,length($0)-(k+2));
      nblks += 1;
      blen = 0;
    }

  else if ((k = index($0,"#ELSEIF")) != 0)
    { if (substr($0,1,k-1) !~ "^[ \t]*$")
        { print "ERROR Line ", NR, ": #ELSEIF must be on a line by itself" > "/dev/stderr";
          exit;
        }
      if (type[nest-1] != If_Line && type[nest-1] != ElseIf_Line)
        { print "ERROR Line ", NR,
                ": #ELSEIF not proceeded by #IF or another #ELSEIF" > "/dev/stderr";
          exit;
        }
      link[srce[nest-1]] = nblks;
      type[nest-1] = ElseIf_Line;
      line[nest-1] = NR;
      srce[nest-1] = nblks;

      op[nblks]   = ElseIf_Line;
      len[nblks]  = 0;
      expr[nblks] = substr($0,k+7,length($0)-(k+6));
      nblks += 1;
      blen = 0;
    }

  else if ((k = index($0,"#ELSE")) != 0)
    { if (substr($0,1,k-1) !~ "^[ \t]*$")
        { print "ERROR Line ", NR, ": #ELSE must be on a line by itself" > "/dev/stderr";
          exit;
        }
      if (type[nest-1] != If_Line && type[nest-1] != ElseIf_Line)
        { print "ERROR Line ", NR,
                ": ELSE_TYPE not proceeded by #IF or another #ELSEIF" > "/dev/stderr";
          exit;
        }
      link[srce[nest-1]] = nblks;
      type[nest-1] = Else_Line;
      line[nest-1] = NR;
      srce[nest-1] = nblks;

      op[nblks]   = Else_Line;
      len[nblks]  = 0;
      nblks += 1;
      blen = 0;
    }

  else if ((k = index($0,"#END")) != 0)
    { if (substr($0,1,k-1) !~ "^[ \t]*$")
        { print "ERROR Line ", NR, ": #END must be on a line by itself" > "/dev/stderr";
          exit;
        }
      if (nest == 0)
        { print "ERROR Line ", NR,
                ": #END has no starting #GENERATE or cascading #IF" > "/dev/stderr";
          exit;
        }
      link[srce[nest-1]] = nblks;
      nest -= 1;

      op[nblks]   = End_Line;
      len[nblks]  = 0;
      nblks += 1;
      blen = 0;

      if (nest == 0)
        { generate_code_block();
          nblks = 0;
        }
    }

  else if ((k = index($0,"#LISTDEF")) != 0)
    { if (substr($0,1,k-1) !~ "^[ \t]*$")
        { print "ERROR Line ", NR, ": #LISTDEF must be on a line by itself" > "/dev/stderr";
          exit;
        }
      if (nest != 0)
        { print "ERROR Line ", NR, ": #LISTDEF is nested in a #GENERATE block" > "/dev/stderr";
          exit;
        }

      control = substr($0,k+8,length($0)-(k+7));
      if ((j = index(control,"=")) == 0)
        { print "ERROR Line ", NR, ": #LISTDEF does not contain an =-sign" > "/dev/stderr";
          exit;
        }

      lvar = substr(control,1,j-1);
      gsub(WHITE,"",lvar);

      if (index(lvar,"@") != 1)
        { print "ERROR Line ", NR, ": list var must start with an @-sign" > "/dev/stderr";
          exit;
        }

      n = split(substr(control,j+1,length(control)-j),thelists,WHITE);
      t = 0;
      for (i = 1; i <= n; i++)
        if (thelists[i] != "")
          { t += 1;
            listitems[lvar,t] = thelists[i];
          }
      listlen[lvar] = t;
    }

  else if ((k = index($0,"#WHEN")) != 0)
    { if (nest <= 0)
        { print "ERROR Line ", NR,
                ": #WHEN is not nested withing a #GENERATE block" > "/dev/stderr";
          exit;
        }
      op[nblks]     = If_Line;
      expr[nblks]   = substr($0,k+5,length($0)-(k+4));
      list[nblks,0] = substr($0,1,k-1);
      len[nblks]    = 1;
      link[nblks]   = nblks+1;
      nblks += 1;
      op[nblks]   = End_Line;
      len[nblks]  = 0;
      nblks += 1;
      blen = 0;
    }

  else if (nest > 0)
    { list[nblks-1,blen] = $0;
      blen += 1;
      len[nblks-1] = blen;
    }

  else
    print $0;
}

END {
  if (nest > 0)
    { print "ERROR: End of file without closing #END, unclosed at line ",
             line[nest-1] > "/dev/stderr";
      exit;
    }

  if (DEBUG)
    for (v in listlen)
      { printf "  '%s' =", v;
        for (i = 1; i <= listlen[v]; i++)
          printf " '%s'", listitems[v,i];
        printf "\n";
      }
}

function generate_code_block()
{ 
  if (DEBUG)
    for (i = 0; i < nblks; i++)
      { if (op[i] == Gen_Line)
          { print i ": " op[i] " " len[i] " (" link[i] ") " nvars[i] "+" nopts[i];
            for (j = 1; j <= nvars[i]; j++)
              { printf "  '%s' =", vars[i,j];
                for (k = 1; k <= nopts[i]; k++)
                  printf " '%s'", glist[i,j,k];
                printf("\n");
              }
          }
        else if (op[i] == If_Line || op[i] == ElseIf_Line)
          print i ": " op[i] " " len[i] " (" link[i] ") {" expr[i] "}";
        else
          print i ": " op[i] " " len[i] " (" link[i] ")";
        for (j = 0; j < len[i]; j++)
          print list[i,j];
      }

  code_gen(0);
}

function code_gen(beg,    end,no,i,k)
{ if (op[beg] == Generate_Line)
    { end = link[beg];

      for (i = 1; i <= nvars[beg]; i++)
        { no = vars[beg,i];
          if (no in depth)
            depth[no] += 1;
          else
            depth[no] = 1;
          if ( ! ordn[beg,i])
            for (k = 1; k <= nopts[beg]; k++)
              { no = glist[beg,i,k];
                if (no in depth)
                  depth[no] += 1;
                else
                  depth[no] = 1;
                no = no depth[no];
                varvals[no] = k;
              }
        }
      
      for (k = 1; k <= nopts[beg]; k++)
        { for (i = 1; i <= nvars[beg]; i++)
            { no = vars[beg,i];
              no = no depth[no];
              if (ordn[beg,i])
                varvals[no] = varstrs[no] = glist[beg,i,k];
              else
                { varvals[no] = k;
                  varstrs[no] = glist[beg,i,k];
                }
            }

          if (DEBUG)
            { print "  GENING CONTEXT:";
              for (no in depth)
                print "    v('" no "." depth[no] "') = '" varvals[no depth[no]] "'";
              for (no in depth)
                if (no depth[no] in varstrs)
                  print "    s('" no "." depth[no] "') = '" varstrs[no depth[no]] "'";
            }

          code_block(beg,end);
        }

      for (i = 1; i <= nvars[beg]; i++)
        { no = vars[beg,i];
          if (depth[no] <= 1)
            delete depth[no];
          else
            depth[no] -= 1;
          if ( ! ordn[beg,i])
            for (k = 1; k <= nopts[beg]; k++)
              { no = glist[beg,i,k];
                if (depth[no] <= 1)
                  delete depth[no];
                else
                  depth[no] -= 1;
              }
        }
    }

  else   # op[beg] == If_Line
    { end = beg;
      no  = 1;
      while (op[end] != End_Line)
        { if (no && (op[end] == Else_Line || evaluate_predicate(expr[end])))
            { no = 0;
              code_block(end,link[end]);
            }
          end = link[end];
        }
    }
  return (end);
}

function code_block(beg,end,   i,j)
{ i = beg;
  while (i < end)
    { for (j = 0; j < len[i]; j++)
        expand_line(list[i,j]);
      i += 1;
      if (i < end)
        i = code_gen(i);
    }
}

function expand_line(line,    name,pat,str)
{ for (name in depth)
    { str = name depth[name];
      if (str in varstrs)
        { pat = "<" name ">";
          str = varstrs[str];
          gsub(pat,str,line);
          pat = "<" tolower(name) ">";
          gsub(pat,tolower(str),line);
          pat = "<" toupper(name) ">";
          gsub(pat,toupper(str),line);
        }
    }
  printf "%s\n", line;
}

function evaluate_predicate(expr, i,n)
{ n = split(expr,clauses,/\|\|/)
  for (i = 1; i <= n; i++)
    if (evaluate_clause(clauses[i]))
      return (1);
  return (0);
}

function evaluate_clause(expr, i,n)
{ n = split(expr,terms,/\&\&/)
  for (i = 1; i <= n; i++)
    if ( ! evaluate_comparison(terms[i]))
      return (0);
  return (1);
}

function evaluate_comparison(expr,  i,n)
{ n = length(expr);
  if ((i = index(expr,"<=")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) <= evaluate_term(substr(expr,i+2,n-(i+1))));  
  else if ((i = index(expr,">=")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) >= evaluate_term(substr(expr,i+2,n-(i+1))));  
  else if ((i = index(expr,"==")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) == evaluate_term(substr(expr,i+2,n-(i+1))));  
  else if ((i = index(expr,"!=")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) != evaluate_term(substr(expr,i+2,n-(i+1))));  
  else if ((i = index(expr,"<")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) < evaluate_term(substr(expr,i+1,n-i)));  
  else if ((i = index(expr,">")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) > evaluate_term(substr(expr,i+1,n-i)));  
  else
    return (evaluate_term(expr));
}

function evaluate_term(expr, i,n)
{ n = length(expr);
  if ((i = index(expr,"+")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) + evaluate_term(substr(expr,i+1,n-i)));
  else if ((i = index(expr,"-")) != 0)
    return (evaluate_term(substr(expr,1,i-1)) - evaluate_term(substr(expr,i+1,n-i)));
  else
    return (evaluate_factor(expr));
}

function evaluate_factor(expr, i,n)
{ n = length(expr);
  if ((i = index(expr,"*")) != 0)
    return (evaluate_factor(substr(expr,1,i-1)) * evaluate_factor(substr(expr,i+1,n-i)));
  else if ((i = index(expr,"/")) != 0)
    return (evaluate_factor(substr(expr,1,i-1)) / evaluate_factor(substr(expr,i+1,n-i)));
  else if ((i = index(expr,"%")) != 0)
    return (evaluate_factor(substr(expr,1,i-1)) % evaluate_factor(substr(expr,i+1,n-i)));
  else
    return (evaluate_atom(expr));
}

function evaluate_atom(expr, name)
{ gsub(WHITE,"",expr);

  if (expr in depth)
    return (varvals[expr depth[expr]]);
  return (expr);
}
