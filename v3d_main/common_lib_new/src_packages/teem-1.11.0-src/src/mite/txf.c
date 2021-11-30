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

/* learned: don't confuse allocate an array of structs with an array
   of pointers to structs.  Don't be surprised when you bus error
   because of the difference
*/

#include "mite.h"
#include "privateMite.h"

char
miteRangeChar[MITE_RANGE_NUM+1] = "ARGBEadsp";

const char *
_miteStageOpStr[] = {
  "(unknown miteStageOp)",
  "min",
  "max",
  "add",
  "multiply"
};

const int
_miteStageOpVal[] = {
  miteStageOpUnknown,
  miteStageOpMin,
  miteStageOpMax,
  miteStageOpAdd,
  miteStageOpMultiply
};

const char *
_miteStageOpStrEqv[] = {
  "min",
  "max",
  "add", "+",
  "multiply", "*", "x",
  ""
};

const int
_miteStageOpValEqv[] = {
  miteStageOpMin,
  miteStageOpMax,
  miteStageOpAdd, miteStageOpAdd,
  miteStageOpMultiply, miteStageOpMultiply, miteStageOpMultiply
};

const airEnum
_miteStageOp = {
  "miteStageOp",
  MITE_STAGE_OP_MAX,
  _miteStageOpStr, _miteStageOpVal,
  NULL,
  _miteStageOpStrEqv, _miteStageOpValEqv,
  AIR_FALSE
};
const airEnum *const
miteStageOp = &_miteStageOp;

/*
******** miteVariableParse()
**
** takes a string (usually the label from a nrrd axis) and parses it
** to determine the gageItemSpec from it (which means finding the
** kind and item).  The valid formats are:
**
**   ""                  : NULL kind, 0 item
**   <item>              : miteValGageKind (DEPRECATED)
**   mite(<item>)        : miteValGageKind
**   gage(<item>)        : gageKindScl (DEPRECATED)
**   gage(scalar:<item>) : gageKindScl
**   gage(vector:<item>) : gageKindVec
**   gage(tensor:<item>) : tenGageKind
**
** Notice that "scalar", "vector", and "tensor" do NOT refer to the type
** of the quantity being measured, but rather to the type of volume in
** which quantity is measured (i.e., the gageKind used)
*/
int
miteVariableParse(gageItemSpec *isp, const char *label) {
  static const char me[]="miteVariableParse";

  char *buff, *endparen, *kqstr, *col, *kstr, *qstr;
  airArray *mop;

  if (!( isp && label )) {
    biffAddf(MITE, "%s: got NULL pointer", me);
    return 1;
  }
  if (0 == strlen(label)) {
    /* nothing was specified; we try to indicate that by mimicking
       the return of gageItemSpecNew() */
    isp->item = 0;
    isp->kind = NULL;
    return 0;
  }
  /* else given string was non-empty */
  mop = airMopNew();
  buff = airStrdup(label);
  if (!buff) {
    biffAddf(MITE, "%s: couldn't strdup label!", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, buff, airFree, airMopAlways);
  if (strstr(buff, "gage(") == buff) {
    /* txf domain variable is to be measured directly by gage */
    if (!(endparen = strstr(buff, ")"))) {
      biffAddf(MITE, "%s: didn't see close paren after \"gage(\"", me);
      airMopError(mop); return 1;
    }
    *endparen = 0;
    kqstr = buff + strlen("gage(");
    /* first see if its a (deprecated) gageKindScl specification */
    isp->item = airEnumVal(gageScl, kqstr);
    if (0 != isp->item) {
      isp->kind = gageKindScl;
      fprintf(stderr, "\n%s: WARNING: deprecated use of txf domain "
              "\"gage(%s)\" without explicit gage kind specification; "
              "should use \"gage(%s:%s)\" instead\n\n",
              me, kqstr, gageKindScl->name, kqstr);
    } else {
      /* should be of form "<kind>:<item>" */
      col = strstr(kqstr, ":");
      if (!col) {
        biffAddf(MITE, "%s: didn't see \":\" separator between gage "
                 "kind and item", me);
        airMopError(mop); return 1;
      }
      *col = 0;
      kstr = kqstr;
      qstr = col+1;
      if (!strcmp(gageKindScl->name, kstr)) {
        isp->kind = gageKindScl;
      } else if (!strcmp(gageKindVec->name, kstr)) {
        isp->kind = gageKindVec;
      } else if (!strcmp(tenGageKind->name, kstr)) {
        isp->kind = tenGageKind;
      } else {
        biffAddf(MITE, "%s: don't recognized \"%s\" gage kind", me, kstr);
        airMopError(mop); return 1;
      }
      isp->item = airEnumVal(isp->kind->enm, qstr);
      if (0 == isp->item) {
        biffAddf(MITE, "%s: couldn't parse \"%s\" as a %s variable",
                 me, qstr, isp->kind->name);
        airMopError(mop); return 1;
      }
    }
  } else if (strstr(buff, "mite(") == buff) {
    /* txf domain variable is *not* directly measured by gage */
    if (!(endparen = strstr(buff, ")"))) {
      biffAddf(MITE, "%s: didn't see close paren after \"mite(\"", me);
      airMopError(mop); return 1;
    }
    *endparen = 0;
    qstr = buff + strlen("mite(");
    isp->item = airEnumVal(miteVal, qstr);
    if (0 == isp->item) {
      biffAddf(MITE, "%s: couldn't parse \"%s\" as a miteVal variable",
               me, qstr);
      airMopError(mop); return 1;
    }
    isp->kind = miteValGageKind;
  } else {
    /* didn't start with "gage(" or "mite(" */
    isp->item = airEnumVal(miteVal, label);
    if (0 != isp->item) {
      /* its measured by mite */
      isp->kind = miteValGageKind;
      fprintf(stderr, "\n%s: WARNING: deprecated use of txf domain "
              "\"%s\"; should use \"mite(%s)\" instead\n\n",
              me, label, label);
    } else {
      biffAddf(MITE, "%s: \"%s\" not a recognized variable", me, label);
      airMopError(mop); return 1;
    }
  }
  airMopOkay(mop);
  return 0;
}

void
miteVariablePrint(char *buff, const gageItemSpec *isp) {
  static const char me[]="miteVariablePrint";

  if (!(isp->kind)) {
    strcpy(buff, "");
  } else if (gageKindScl == isp->kind
             || gageKindVec == isp->kind
             || tenGageKind == isp->kind) {
    sprintf(buff, "gage(%s:%s)", isp->kind->name,
            airEnumStr(isp->kind->enm, isp->item));
  } else if (miteValGageKind == isp->kind) {
    sprintf(buff, "%s(%s)", isp->kind->name,
            airEnumStr(isp->kind->enm, isp->item));
  } else {
    sprintf(buff, "(%s: unknown gageKind!)", me);
  }
  return;
}

int
miteNtxfCheck(const Nrrd *ntxf) {
  static const char me[]="miteNtxfCheck";
  char *rangeStr, *domStr;
  gageItemSpec isp;
  unsigned int rii, axi;
  int ilog2;

  if (nrrdCheck(ntxf)) {
    biffMovef(MITE, NRRD, "%s: basic nrrd validity check failed", me);
    return 1;
  }
  if (!( nrrdTypeFloat == ntxf->type ||
         nrrdTypeDouble == ntxf->type ||
         nrrdTypeUChar == ntxf->type )) {
    biffAddf(MITE, "%s: need a type %s, %s or %s nrrd (not %s)", me,
             airEnumStr(nrrdType, nrrdTypeFloat),
             airEnumStr(nrrdType, nrrdTypeDouble),
             airEnumStr(nrrdType, nrrdTypeUChar),
             airEnumStr(nrrdType, ntxf->type));
    return 1;
  }
  if (!( 2 <= ntxf->dim )) {
    biffAddf(MITE, "%s: nrrd dim (%d) isn't at least 2 (for a 1-D txf)",
             me, ntxf->dim);
    return 1;
  }
  rangeStr = ntxf->axis[0].label;
  if (0 == airStrlen(rangeStr)) {
    biffAddf(MITE, "%s: axis[0]'s label doesn't specify txf range", me);
    return 1;
  }
  if (airStrlen(rangeStr) != ntxf->axis[0].size) {
    char stmp1[AIR_STRLEN_SMALL], stmp2[AIR_STRLEN_SMALL];
    biffAddf(MITE, "%s: axis[0]'s size %s, but label specifies %s values", me,
             airSprintSize_t(stmp1, ntxf->axis[0].size),
             airSprintSize_t(stmp2, airStrlen(rangeStr)));
    return 1;
  }
  for (rii=0; rii<airStrlen(rangeStr); rii++) {
    if (!strchr(miteRangeChar, rangeStr[rii])) {
      biffAddf(MITE, "%s: char %d of axis[0]'s label (\"%c\") isn't a valid "
               "transfer function range specifier (not in \"%s\")",
               me, rii, rangeStr[rii], miteRangeChar);
      return 1;
    }
  }
  for (axi=1; axi<ntxf->dim; axi++) {
    if (1 == ntxf->axis[axi].size) {
      biffAddf(MITE, "%s: # samples on axis %d must be > 1", me, axi);
      return 1;
    }
    domStr = ntxf->axis[axi].label;
    if (0 == airStrlen(domStr)) {
      biffAddf(MITE, "%s: axis[%d] of txf didn't specify a domain variable",
               me, axi);
      return 1;
    }
    if (miteVariableParse(&isp, domStr)) {
      biffAddf(MITE, "%s: couldn't parse txf domain \"%s\" for axis %d\n",
               me, domStr, axi);
      return 1;
    }
    if (!( 1 == isp.kind->table[isp.item].answerLength ||
           3 == isp.kind->table[isp.item].answerLength )) {
      biffAddf(MITE, "%s: %s (item %d) not a scalar or vector "
               "(answerLength = %d): "
               "can't be a txf domain variable", me, domStr, isp.item,
               isp.kind->table[isp.item].answerLength);
      return 1;
    }
    if (3 == isp.kind->table[isp.item].answerLength) {
      /* has to be right length for one of the quantization schemes */
      ilog2 = airLog2(ntxf->axis[axi].size);
      if (-1 == ilog2) {
        char stmp[AIR_STRLEN_SMALL];
        biffAddf(MITE, "%s: txf axis size for %s must be power of 2 (not %s)",
                 me, domStr, airSprintSize_t(stmp, ntxf->axis[axi].size));
        return 1;
      } else {
        if (!( AIR_IN_CL(8, ilog2, 16) )) {
          biffAddf(MITE, "%s: log_2 of txf axis size for %s should be in "
                   "range [8,16] (not %d)", me, domStr, ilog2);
          return 1;
        }
      }
    } else {
      if (!( AIR_EXISTS(ntxf->axis[axi].min) &&
             AIR_EXISTS(ntxf->axis[axi].max) )) {
        biffAddf(MITE, "%s: min and max of axis %d aren't both set", me, axi);
        return 1;
      }
      if (!( ntxf->axis[axi].min < ntxf->axis[axi].max )) {
        biffAddf(MITE, "%s: min (%g) not less than max (%g) on axis %d",
                 me, ntxf->axis[axi].min, ntxf->axis[axi].max, axi);
        return 1;
      }
    }
  }

  return 0;
}

/*
******** miteQueryAdd()
**
** This looks a given gageItemSpec and sets the bits in the
** gageKindScl and tenGageKind queries that are required to calculate
** the quantity
**
** NOTE: This does NOT initialize the query{Scl,Vec,Ten}: it
** just adds on new items to the existing queries
**
** HEY: this is really unfortunate: each new gage type use for
** volume rendering in mite will have to explicitly added as
** arguments here, which is part of the reason that mite may end
** up explicitly depending on the libraries supplying those gageKinds
** (like how mite now must depend on ten)
**
** The queryMite argument is a little odd- its not a real gage kind,
** but we use it to organize which of the miteVal quantities we take
** the time to compute in miteSample().
*/
void
miteQueryAdd(gageQuery queryScl, gageQuery queryVec,
             gageQuery queryTen, gageQuery queryMite,
             gageItemSpec *isp) {
  static const char me[]="miteQueryAdd";

  if (NULL == isp->kind) {
    /* nothing to add */
  } else if (gageKindScl == isp->kind) {
    GAGE_QUERY_ITEM_ON(queryScl, isp->item);
  } else if (gageKindVec == isp->kind) {
    GAGE_QUERY_ITEM_ON(queryVec, isp->item);
  } else if (tenGageKind == isp->kind) {
    GAGE_QUERY_ITEM_ON(queryTen, isp->item);
  } else if (miteValGageKind == isp->kind) {
    /* regardless of whether the mite query requires scl, vec, or ten
       queries, we add it to the quantites that have to be computed
       per-sample */
    GAGE_QUERY_ITEM_ON(queryMite, isp->item);
    /* HEY: some these have useful analogs for tensor data, but I
       won't be able to express them.  This means that while Phong
       shading of *scalar* volumes can be implemented with transfer
       functions, it is currently not possible in *tensor* volumes
       (for instance, using the gradient of fractional anisotropy) */
    switch(isp->item) {
    case miteValVrefN:
    case miteValNdotV:
    case miteValNdotL:
      /* the "N" can be a normalized vector from any of the
         available kinds (except miteValGageKind!), and its
         associated query will be handled elsewhere, so there's
         nothing to add here */
      break;
    case miteValGTdotV:
      GAGE_QUERY_ITEM_ON(queryScl, gageSclGeomTens);
      break;
    case miteValVdefT:
      GAGE_QUERY_ITEM_ON(queryTen, tenGageTensor);
    case miteValVdefTdotV:
      GAGE_QUERY_ITEM_ON(queryTen, tenGageTensor);
      break;
    }
  } else {
    fprintf(stderr, "%s: PANIC: unrecognized non-NULL gageKind\n", me);
    exit(1);
  }
  return;
}

int
_miteNtxfCopy(miteRender *mrr, miteUser *muu) {
  static const char me[]="_miteNtxfCopy";
  int ni, E;

  mrr->ntxf = AIR_CALLOC(muu->ntxfNum, Nrrd *);
  if (!mrr->ntxf) {
    biffAddf(MITE, "%s: couldn't calloc %d ntxf pointers", me, muu->ntxfNum);
    return 1;
  }
  mrr->ntxfNum = muu->ntxfNum;
  airMopAdd(mrr->rmop, mrr->ntxf, airFree, airMopAlways);
  E = 0;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    mrr->ntxf[ni] = nrrdNew();
    if (!E) airMopAdd(mrr->rmop, mrr->ntxf[ni],
                      (airMopper)nrrdNuke, airMopAlways);
    if (!( nrrdTypeUChar == muu->ntxf[ni]->type
           || nrrdTypeFloat == muu->ntxf[ni]->type
           || nrrdTypeDouble == muu->ntxf[ni]->type )) {
      biffAddf(MITE,
               "%s: sorry, can't handle txf of type %s (only %s, %s, %s)",
               me, airEnumStr(nrrdType, muu->ntxf[ni]->type),
               airEnumStr(nrrdType, nrrdTypeUChar),
               airEnumStr(nrrdType, nrrdTypeFloat),
               airEnumStr(nrrdType, nrrdTypeDouble));
      return 1;
    }
    /* note that key/values need to be copied for the sake of
       identifying a non-default miteStageOp */
    switch(muu->ntxf[ni]->type) {
    case nrrdTypeUChar:
      if (!E) E |= nrrdUnquantize(mrr->ntxf[ni], muu->ntxf[ni], nrrdTypeUChar);
      if (!E) E |= nrrdKeyValueCopy(mrr->ntxf[ni], muu->ntxf[ni]);
      break;
    case mite_nt:
      if (!E) E |= nrrdCopy(mrr->ntxf[ni], muu->ntxf[ni]);
      break;
    default:  /* will be either float or double (whatever mite_nt isn't) */
      if (!E) E |= nrrdConvert(mrr->ntxf[ni], muu->ntxf[ni], mite_nt);
      if (!E) E |= nrrdKeyValueCopy(mrr->ntxf[ni], muu->ntxf[ni]);
      break;
    }
  }
  if (E) {
    biffMovef(MITE, NRRD, "%s: troubling copying/converting all ntxfs", me);
    return 1;
  }
  return 0;
}

int
_miteNtxfAlphaAdjust(miteRender *mrr, miteUser *muu) {
  static const char me[]="_miteNtxfAlphaAdjust";
  int ni, ei, ri, nnum, rnum;
  Nrrd *ntxf;
  mite_t *data, alpha, frac;

  if (_miteNtxfCopy(mrr, muu)) {
    biffAddf(MITE, "%s: trouble copying/converting transfer functions", me);
    return 1;
  }
  frac = muu->rayStep/muu->refStep;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    ntxf = mrr->ntxf[ni];
    if (!strchr(ntxf->axis[0].label, miteRangeChar[miteRangeAlpha])) {
      continue;
    }
    /* else this txf sets opacity */
    data = (mite_t *)ntxf->data;
    rnum = ntxf->axis[0].size;
    nnum = nrrdElementNumber(ntxf)/rnum;
    for (ei=0; ei<nnum; ei++) {
      for (ri=0; ri<rnum; ri++) {
        if (ntxf->axis[0].label[ri] == miteRangeChar[miteRangeAlpha]) {
          alpha = data[ri + rnum*ei];
          data[ri + rnum*ei] = 1 - pow(AIR_MAX(0, 1-alpha), frac);
        }
      }
    }
  }
  return 0;
}

int
_miteStageNum(miteRender *mrr) {
  int num, ni;

  num = 0;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    num += mrr->ntxf[ni]->dim - 1;
  }
  return num;
}

void
_miteStageInit(miteStage *stage) {
  int rii;

  stage->val = NULL;
  stage->size = -1;
  stage->op = miteStageOpUnknown;
  stage->qn = NULL;
  stage->min = stage->max = AIR_NAN;
  stage->data = NULL;
  for (rii=0; rii<=MITE_RANGE_NUM-1; rii++) {
    stage->rangeIdx[rii] = -1;
  }
  stage->rangeNum = -1;
  stage->label = NULL;
  return;
}

double *
_miteAnswerPointer(miteThread *mtt, gageItemSpec *isp) {
  static const char me[]="_miteAnswerPointer";
  double *ret;

  if (!isp->kind) {
    /* we got a NULL kind (as happens with output of
       gageItemSpecNew(), or miteVariableParse of an
       empty string); only NULL return is sensible */
    return NULL;
  }

  if (gageKindScl == isp->kind) {
    ret = mtt->ansScl;
  } else if (gageKindVec == isp->kind) {
    ret = mtt->ansVec;
  } else if (tenGageKind == isp->kind) {
    ret = mtt->ansTen;
  } else if (miteValGageKind == isp->kind) {
    ret = mtt->ansMiteVal;
  } else {
    fprintf(stderr, "\nPANIC: %s: unknown gageKind!\n", me);
    exit(1);
  }
  ret += gageKindAnswerOffset(isp->kind, isp->item);
  return ret;
}

/*
** _miteStageSet
**
** ALLOCATES and initializes stage array in a miteThread
*/
int
_miteStageSet(miteThread *mtt, miteRender *mrr) {
  static const char me[]="_miteStageSet";
  char *value;
  int ni, di, stageIdx, rii, stageNum, ilog2;
  Nrrd *ntxf;
  miteStage *stage;
  gageItemSpec isp;
  char rc;

  stageNum = _miteStageNum(mrr);
  /* fprintf(stderr, "!%s: stageNum = %d\n", me, stageNum); */
  mtt->stage = AIR_CALLOC(stageNum, miteStage);
  if (!mtt->stage) {
    biffAddf(MITE, "%s: couldn't alloc array of %d stages", me, stageNum);
    return 1;
  }
  airMopAdd(mtt->rmop, mtt->stage, airFree, airMopAlways);
  mtt->stageNum = stageNum;
  stageIdx = 0;
  for (ni=0; ni<mrr->ntxfNum; ni++) {
    ntxf = mrr->ntxf[ni];
    for (di=ntxf->dim-1; di>=1; di--) {
      stage = mtt->stage + stageIdx;
      _miteStageInit(stage);
      miteVariableParse(&isp, ntxf->axis[di].label);
      stage->val = _miteAnswerPointer(mtt, &isp);
      stage->label = ntxf->axis[di].label;
      /*
      fprintf(stderr, "!%s: ans=%p + offset[%d]=%d == %p\n", me,
              mtt->ans, dom, kind->ansOffset[dom], stage->val);
      */
      stage->size = ntxf->axis[di].size;
      stage->min =  ntxf->axis[di].min;
      stage->max =  ntxf->axis[di].max;
      if (di > 1) {
        stage->data = NULL;
      } else {
        stage->data = (mite_t *)ntxf->data;
        value = nrrdKeyValueGet(ntxf, "miteStageOp");
        if (value) {
          stage->op = airEnumVal(miteStageOp, value);
          if (miteStageOpUnknown == stage->op) {
            stage->op = miteStageOpMultiply;
          }
        } else {
          stage->op = miteStageOpMultiply;
        }
        if (1 == isp.kind->table[isp.item].answerLength) {
          stage->qn = NULL;
        } else if (3 == isp.kind->table[isp.item].answerLength) {
          char stmp[AIR_STRLEN_SMALL];
          ilog2 = airLog2(ntxf->axis[di].size);
          switch(ilog2) {
          case 8:  stage->qn = limnVtoQN_d[ limnQN8octa]; break;
          case 9:  stage->qn = limnVtoQN_d[ limnQN9octa]; break;
          case 10: stage->qn = limnVtoQN_d[limnQN10octa]; break;
          case 11: stage->qn = limnVtoQN_d[limnQN11octa]; break;
          case 12: stage->qn = limnVtoQN_d[limnQN12octa]; break;
          case 13: stage->qn = limnVtoQN_d[limnQN13octa]; break;
          case 14: stage->qn = limnVtoQN_d[limnQN14octa]; break;
          case 15: stage->qn = limnVtoQN_d[limnQN15octa]; break;
          case 16: stage->qn = limnVtoQN_d[limnQN16octa]; break;
          default:
            biffAddf(MITE, "%s: txf axis %d size %s not usable for "
                     "vector txf domain variable %s", me, di,
                     airSprintSize_t(stmp, ntxf->axis[di].size),
                     ntxf->axis[di].label);
            return 1;
            break;
          }
        } else {
          biffAddf(MITE, "%s: %s not scalar or vector (len = %d): can't be "
                   "a txf domain variable", me,
                   ntxf->axis[di].label,
                   isp.kind->table[isp.item].answerLength);
          return 1;
        }
        stage->rangeNum = ntxf->axis[0].size;
        for (rii=0; rii<stage->rangeNum; rii++) {
          rc = ntxf->axis[0].label[rii];
          stage->rangeIdx[rii] = strchr(miteRangeChar, rc) - miteRangeChar;
          /*
          fprintf(stderr, "!%s: range: %c -> %d\n", "_miteStageSet",
                  ntxf->axis[0].label[rii], stage->rangeIdx[rii]);
          */
        }
      }
      stageIdx++;
    }
  }
  return 0;
}

void
_miteStageRun(miteThread *mtt, miteUser *muu) {
  static const char me[]="_miteStageRun";
  int stageIdx, ri, rii;
  unsigned int txfIdx, finalIdx;
  miteStage *stage;
  mite_t *rangeData;
  double *dbg=NULL;

  finalIdx = 0;
  if (mtt->verbose) {
    dbg = muu->debug + muu->debugIdx;
  }
  for (stageIdx=0; stageIdx<mtt->stageNum; stageIdx++) {
    stage = &(mtt->stage[stageIdx]);
    if (stage->qn) {
      /* its a vector-valued txf domain variable */
      txfIdx = stage->qn(stage->val);
      /* right now, we can't store vector-valued txf domain variables */
    } else {
      /* its a scalar txf domain variable */
      txfIdx = airIndexClamp(stage->min, *(stage->val),
                             stage->max, stage->size);
      if (mtt->verbose) {
        fprintf(stderr, "!%s: %s=%g in [%g,%g]/%u -> %u\n", me,
                stage->label, *(stage->val),
                stage->min, stage->max, stage->size, txfIdx);
        dbg[0 + 2*stageIdx] = *(stage->val);
      }
    }
    finalIdx = stage->size*finalIdx + txfIdx;
    if (mtt->verbose) {
      dbg[1 + 2*stageIdx] = txfIdx;
    }
    if (stage->data) {
      rangeData = stage->data + stage->rangeNum*finalIdx;
      for (rii=0; rii<stage->rangeNum; rii++) {
        ri = stage->rangeIdx[rii];
        switch(stage->op) {
        case miteStageOpMin:
          mtt->range[ri] = AIR_MIN(mtt->range[ri], rangeData[rii]);
          break;
        case miteStageOpMax:
          mtt->range[ri] = AIR_MAX(mtt->range[ri], rangeData[rii]);
          break;
        case miteStageOpAdd:
          mtt->range[ri] += rangeData[rii];
          break;
        case miteStageOpMultiply:
        default:
          mtt->range[ri] *= rangeData[rii];
          break;
        }
      }
      finalIdx = 0;
    }
  }
  return;
}
