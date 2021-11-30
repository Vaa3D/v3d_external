/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef UNRRDU_PRIVATE_HAS_BEEN_INCLUDED
#define UNRRDU_PRIVATE_HAS_BEEN_INCLUDED

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* handling of "quiet quit", to avoid having a string of piped unu
   commands generate multiple pages of unwelcome usage info */
/* the environment variable to look for */
#define UNRRDU_QUIET_QUIT_ENV "UNRRDU_QUIET_QUIT"
/* the string to search for in the error message that signifies that
   we should die quietly; this is clearly a hack because it depends on
   the text of error messages set by a different library.  With more
   work it could be made into less of a hack.  At worst the hack's
   breakage will lead again to the many error messages that inspired
   the hack in the first place , and will inspire fixing it again */
#define UNRRDU_QUIET_QUIT_STR "[nrrd] _nrrdRead: immediately hit EOF"

/*
** OPT_ADD_XXX
**
** These macros are used for setting up command-line options for the various
** unu commands.  They define options which are common across many different
** commands, so that the unu interface is as consistent as possible.  They
** all assume a hestOpt *opt variable, but they take the option variable
** and option description as arguments.  The expected type of the variable
** is given before each macro.
*/
/* Nrrd *var */
#define OPT_ADD_NIN(var, desc) \
  hestOptAdd(&opt, "i,input", "nin", airTypeOther, 1, 1, &(var), "-", desc, \
             NULL, NULL, nrrdHestNrrd)

/* char *var */
#define OPT_ADD_NOUT(var, desc) \
  hestOptAdd(&opt, "o,output", "nout", airTypeString, 1, 1, &(var), "-", desc)

/* unsigned int var */
#define OPT_ADD_AXIS(var, desc) \
  hestOptAdd(&opt, "a,axis", "axis", airTypeUInt, 1, 1, &(var), NULL, desc)

/* int *var; int saw */
#define OPT_ADD_BOUND(name, needmin, var, deflt, desc, saw)             \
  hestOptAdd(&opt, name, "pos0", airTypeOther, needmin, -1, &(var),     \
             deflt, desc, &(saw), NULL, &unrrduHestPosCB)

/* int var */
#define OPT_ADD_TYPE(var, desc, dflt) \
  hestOptAdd(&opt, "t,type", "type", airTypeEnum, 1, 1, &(var), dflt, desc, \
             NULL, nrrdType)

/*
** USAGE, PARSE, SAVE
**
** These are macros at their worst and most fragile, because of how
** many local variables are assumed.  This code is basically the same,
** verbatim, across all the different unrrdu functions, and having
** them as macros just shortens (without necessarily clarifying) their
** code.
**
** The return value for generating usage information was changed from
** 1 to 0 with the thought that merely asking for usage info shouldn't
** be treated as an erroneous invocation; unu about and unu env (which
** don't use this macro) had already been this way.
*/
#define USAGE(info) \
  if (!argc) { \
    hestInfo(stdout, me, (info), hparm); \
    hestUsage(stdout, opt, me, hparm); \
    hestGlossary(stdout, opt, hparm); \
    airMopError(mop); \
    return 0; \
  }

  /*

I nixed this because it meant unu invocations with only a
few args (less than hestMinNumArgs()), which were botched
because they were missing options, were not being described
in the error messages.

**
** NB: below is an unidiomatic use of hestMinNumArgs(), because of
** how unu's main invokes the "main" function of the different
** commands.  Normally the comparison is with argc-1, or argc-2
** the case of cvs/svn-like commands.


  if ( (hparm->respFileEnable && !argc) || \
       (!hparm->respFileEnable && argc < hestMinNumArgs(opt)) ) { \
  */

/*
** NOTE: of all places it is inside the PARSE() macro that the
** "quiet-quit" functionality is implemented; this is defensible
** because all unu commands use PARSE
*/
#define PARSE()                                                         \
  if ((pret=hestParse(opt, argc, argv, &err, hparm))) {                 \
    if (1 == pret || 2 == pret) {                                       \
      if (!(getenv(UNRRDU_QUIET_QUIT_ENV)                               \
            && airEndsWith(err, UNRRDU_QUIET_QUIT_STR "\n"))) {         \
        fprintf(stderr, "%s: %s\n", me, err); free(err);                \
        hestUsage(stderr, opt, me, hparm);                              \
        hestGlossary(stderr, opt, hparm);                               \
      }                                                                 \
      airMopError(mop);                                                 \
      return 1;                                                         \
    } else {                                                            \
      /* . . . like tears . . . in rain. Time . . . to die. */          \
      exit(1);                                                          \
    }                                                                   \
  }

#define SAVE(outS, nout, io) \
  if (nrrdSave((outS), (nout), (io))) { \
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways); \
    fprintf(stderr, "%s: error saving nrrd to \"%s\":\n%s\n", me, (outS), err); \
    airMopError(mop); \
    return 1; \
  }

#ifdef __cplusplus
}
#endif

#endif /* UNRRDU_PRIVATE_HAS_BEEN_INCLUDED */
