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

#include "air.h"

static void
_airLenSet(airArray *a, unsigned int len) {

  a->len = len;
  /* printf("    HEY: len = %d\n", a->len); */
  if (a->lenP) {
    *(a->lenP) = len;
    /* printf("    HEY: *(a->lenP) = *(%lu) = %d\n",
       (unsigned long)a->lenP, *(a->lenP)); */
  }
}

static void
_airSetData(airArray *a, void *data) {

  a->data = data;
  if (a->dataP) {
    *(a->dataP) = data;
  }
}

/*
******** airArrayNew()
**
** creates a new airArray struct and returns a pointer to it.
** dataP is a pointer to the user's data pointer
** lenP is a pointer to the user's array length variable (optional)
** unit is the size (in bytes) of one element in the array
** incr is the number of units by which the array will grow or shrink
**
** returns NULL on error, or the new airArray pointer if okay
** errors: bogus arguments, or couldn't alloc airArray struct
**
** --> The user CAN NOT change the pointer variable (of which *dataP
** is the address) after this is called, or else everything will
** get all bolloxed up.  The same goes for the array length
** variable, if its address is passed- though in that case the
** correct value will over-write any other.
*/
airArray *
airArrayNew(void **dataP, unsigned int *lenP, size_t unit, unsigned int incr) {
  airArray *a;

  if (unit<=0 || incr<=0) {
    return NULL;
  }

  a = AIR_CALLOC(1, airArray);
  if (!a) {
    return NULL;
  }

  a->dataP = dataP;
  _airSetData(a, NULL);
  a->lenP = lenP;
  _airLenSet(a, 0);
  a->incr = incr;
  a->unit = unit;
  a->noReallocWhenSmaller = AIR_FALSE;

  a->allocCB = NULL;
  a->freeCB = NULL;
  a->initCB = NULL;
  a->doneCB = NULL;

  return a;
}

/*
******** airArrayStructCB()
**
** set callbacks to maintain array of structs
*/
void
airArrayStructCB(airArray *a,
                 void (*initCB)(void *), void (*doneCB)(void *)) {

  if (a) {
    a->initCB = initCB;
    a->doneCB = doneCB;
    a->allocCB = NULL;
    a->freeCB = NULL;
  }
}

/*
******** airArrayPointerCB()
**
** set callbacks to maintain array of pointers
*/
void
airArrayPointerCB(airArray *a,
                  void *(*allocCB)(void), void *(*freeCB)(void *)) {

  if (a) {
    a->initCB = NULL;
    a->doneCB = NULL;
    a->allocCB = allocCB;
    a->freeCB = freeCB;
  }
}

/* ---- BEGIN non-NrrdIO */
/*
******** airArrayLenPreSet()
**
** allocates the array to hold up to given length, without
** actually changing the length.  In order for this to be
** useful, this also turns on noReallocWhenSmaller
**
** NB: this used to have a "boolean" return to indicate allocation
** error, but nothing in Teem actually did the error checking.  Now
** conscientious users can look at NULL-ity of a->data to detect such
** an error.
*/
void
airArrayLenPreSet(airArray *a, unsigned int newlen) {
  /* char me[]="airArrayLenPreSet"; */
  unsigned int newsize;
  void *newdata;

  if (!a) {
    return;
  }

  if (newlen == 0) {
    /* there is no pre-set length, turn off noReallocWhenSmaller */
    a->noReallocWhenSmaller = AIR_FALSE;
  } else {
    newsize = (newlen-1)/a->incr + 1;
    /*
    fprintf(stderr, "!%s: newlen = %u, incr = %u -> newsize = %u\n", me,
            newlen, a->incr, newsize);
    fprintf(stderr, "!%s: a->size = %u, a->len = %u, a->unit = %u\n", me,
            a->size, a->len, a->unit);
    */
    if (newsize > a->size) {
      newdata = calloc(newsize*a->incr, a->unit);
      /*
      fprintf(stderr, "!%s: a->data = %p, newdata = %p\n", me,
              a->data, newdata);
      */
      if (!newdata) {
        free(a->data);
        _airSetData(a, NULL);
        return;
      }
      if (a->data) {
        memcpy(newdata, a->data, AIR_MIN(a->len*a->unit,
                                         newsize*a->incr*a->unit));
        free(a->data);
      }
      _airSetData(a, newdata);
      a->size = newsize;
    }
    a->noReallocWhenSmaller = AIR_TRUE;
  }

  /* fprintf(stderr, "!%s: returning data %p\n", me, a->data); */
  return;
}
/* ---- END non-NrrdIO */

/*
******** airArrayLenSet()
**
** Set the length of the array, allocating or freeing as needed
**
** returns 1 on error, otherwise 0 if okay
** possible errors: bogus arguments, or couldn't allocate new memory segment
**
** In case we can't allocate the new space, the old space is left untouched,
** however if the new length is smaller, the free/done callbacks will
** have been called on invalidated elements
**
** NB: this used to have a "boolean" return to indicate allocation
** error, but almost nothing in Teem actually did the error checking.
** Now conscientious users can look at NULL-ity of a->data to detect
** such an error.
*/
void
airArrayLenSet(airArray *a, unsigned int newlen) {
  /* char me[]="airArrayLenSet"; */
  unsigned int ii, newsize;
  void *addr, *newdata;

  if (!a) {
    /* user is a moron, what can you do */
    return;
  }

  if (newlen == a->len) {
    /* nothing to do */
    return;
  }

  /* call freeCB/doneCB on all the elements which are going bye-bye */
  /* Wed Sep 12 14:40:45 EDT 2007: the order in which these called is
     now ascending, instead of descending (as was the way before) */
  if (newlen < a->len && (a->freeCB || a->doneCB)) {
    for (ii=newlen; ii<a->len; ii++) {
      addr = (char*)(a->data) + ii*a->unit;
      if (a->freeCB) {
        (a->freeCB)(*((void**)addr));
      } else {
        (a->doneCB)(addr);
      }
    }
  }

  newsize = newlen ? (newlen-1)/a->incr + 1 : 0;
  if (newsize != a->size) {
    /* we have to change the size of the array */
    if (newsize) {
      /* array should be bigger or smaller, but not zero-length */
      if (newsize > a->size
          || (newsize < a->size && !(a->noReallocWhenSmaller)) ) {
        newdata = calloc(newsize*a->incr, a->unit);
        if (!newdata) {
          free(a->data);
          _airSetData(a, NULL);
          return;
        }
        memcpy(newdata, a->data, AIR_MIN(a->len*a->unit,
                                         newsize*a->incr*a->unit));
        free(a->data);
        _airSetData(a, newdata);
        a->size = newsize;
      }
    } else {
      /* array should be zero-length */
      free(a->data);
      _airSetData(a, NULL);
      a->size = newsize;
    }
  }
  /* else new size is still within current allocated length,
     and neither "size" nor "data" need to change */

  /* call allocCB/initCB on newly created elements */
  if (newlen > a->len && (a->allocCB || a->initCB)) {
    for (ii=a->len; ii<newlen; ii++) {
      addr = (char*)(a->data) + ii*a->unit;
      if (a->allocCB) {
        *((void**)addr) = (a->allocCB)();
      } else {
        (a->initCB)(addr);
      }
    }
  }
  _airLenSet(a, newlen);

  return;
}

/*
******** airArrayLenIncr()
**
** Like airArrayLenSet, but works with an increment instead of an
** absolute length.  Return value is different:
**             got NULL: return 0
**     allocation error: return 0, and a->data set to NULL
**  no error, delta > 0: return index of 1st element in newly allocated
**                       segment (a->len before length was increased)
** no error, delta <= 0: return 0, and a->data unchanged
**
** HEY: it is apparently not clear how to do error checking (aside from
** looking at a->data) when there was NO data previously allocated, and the
** first index of the newly allocated data is zero.
*/
unsigned int
airArrayLenIncr(airArray *a, int delta) {
  /* char me[]="airArrayLenIncr"; */
  unsigned int oldlen, ret, negdel;

  if (!a) {
    return 0;
  }
  negdel = (delta < 0
            ? AIR_UINT(-delta)
            : 0);
  if (delta < 0 && negdel > a->len) {
    /* error: asked for newlength to be negative */
    airArrayLenSet(a, 0);
    return 0;
  }
  oldlen = a->len;
  airArrayLenSet(a, (delta >= 0
                     ? oldlen + AIR_UINT(delta)
                     : oldlen - negdel));
  if (!a->data) {
    /* allocation error */
    ret = 0;
  } else {
    ret = (delta <= 0 ? 0 : oldlen);
  }

  return ret;
}

/*
******** airArrayNuke()
**
** free both the memory pointed to by the struct and the struct itself
*/
airArray *
airArrayNuke(airArray *a) {

  if (a) {
    airArrayLenSet(a, 0);
    free(a);
  }
  return NULL;
}

/*
******** airArrayNix()
**
** frees just the struct, leaving the memory it points to untouched
*/
airArray *
airArrayNix(airArray *a) {

  if (a) {
    free(a);
  }
  return NULL;
}
