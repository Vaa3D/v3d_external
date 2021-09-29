/*
  sanity.c: stand-alone demo of nrrdSanity() from Teem
  Copyright (C) 2008
 
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.
 
  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:
 
  1. The origin of this software must not be misrepresented; you must
     not claim that you wrote the original software. If you use this
     software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
 
  2. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.
 
  3. This notice may not be removed or altered from any source distribution.
*/

#include <teem/nrrd.h>

int
main(int argc, char *argv[]) {
  int ret;
  char *me, *err;

  AIR_UNUSED(argc);
  me = argv[0];
  if (!nrrdSanity()) {
    err = biffGetDone(NRRD);
    fprintf(stderr, "%s: nrrdSanity failed:\n%s", me, err);
    free(err);
    ret = 1;
  } else {
    fprintf(stderr, "%s: nrrdSanity passed\n", me);
    ret = 0;
  }

  return ret;
}

/* EOF */
