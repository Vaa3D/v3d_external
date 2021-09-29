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

#ifndef HEST_PRIVATE_HAS_BEEN_INCLUDED
#define HEST_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* methodsHest.c */
extern char *_hestIdent(char *ident, hestOpt *opt, hestParm *parm, int brief);
extern int _hestKind(hestOpt *opt);
extern void _hestPrintArgv(int argc, char **argv);
extern int _hestWhichFlag(hestOpt *opt, char *flag, hestParm *parm);
extern int _hestCase(hestOpt *opt, int *udflt, unsigned int *nprm, int *appr, int op);
extern char *_hestExtract(int *argcP, char **argv,
                          unsigned int base, unsigned int pnum);
extern int _hestNumOpts(hestOpt *opt);
extern int _hestMax(int max);

/* parseHest.c */
extern int _hestPanic(hestOpt *opt, char *err, hestParm *parm);
extern int _hestErrStrlen(hestOpt *opt, int argc, const char **argv);

#ifdef __cplusplus
}
#endif

#endif /* HEST_HAS_BEEN_INCLUDED */
