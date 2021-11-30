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

#include "meet.h"

meetPullVol *
meetPullVolNew(void) {
  meetPullVol *ret;

  ret = AIR_CALLOC(1, meetPullVol);
  if (ret) {
    ret->kind = NULL;
    ret->fileName = ret->volName = NULL;
    ret->derivNormSS = AIR_FALSE;
    ret->uniformSS = AIR_FALSE;
    ret->optimSS = AIR_FALSE;
    ret->leeching = AIR_FALSE;
    ret->numSS = 0;
    ret->rangeSS[0] = ret->rangeSS[1] = AIR_NAN;
    ret->derivNormBiasSS = 0.0;
    ret->posSS = NULL;
    ret->nin = NULL;
    ret->ninSS = NULL;
  }
  return ret;
}

/*
******** meetPullVolParse
**
** parses a string to extract all the information necessary to create
** the pullVolume (this function originated in Deft/src/main-pull.c)
*/
int
meetPullVolParse(meetPullVol *mpv, const char *_str) {
  static const char me[]="meetPullVolParse";
#define VFMT_SHRT "<fileName>:<kind>:<volName>"
#define SFMT "<minScl>-<#smp>-<maxScl>[-no|u]"
#define VFMT_LONG "<fileName>:<kind>:" SFMT ":<volName>"
  char *str, *ctok, *clast=NULL, *dtok, *dlast=NULL;
  airArray *mop;
  int wantSS;

  if (!(mpv && _str)) {
    biffAddf(MEET, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( str = airStrdup(_str) )) {
    biffAddf(MEET, "%s: couldn't strdup input", me);
    return 1;
  }

  mop = airMopNew();
  airMopAdd(mop, str, airFree, airMopAlways);
  if (!( 3 == airStrntok(str, ":") || 4 == airStrntok(str, ":") )) {
    biffAddf(MEET, "%s: didn't get 3 or 4 \":\"-separated tokens in \"%s\"; "
             "not of form " VFMT_SHRT " or " VFMT_LONG , me, _str);
    airMopError(mop); return 1;
  }
  /* mpv->nin is set elsewhere */
  wantSS = (4 == airStrntok(str, ":"));

  ctok = airStrtok(str, ":", &clast);
  if (!(mpv->fileName = airStrdup(ctok))) {
    biffAddf(MEET, "%s: couldn't strdup fileName", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, &(mpv->fileName), (airMopper)airSetNull, airMopOnError);
  airMopAdd(mop, mpv->fileName, airFree, airMopOnError);
  ctok = airStrtok(NULL, ":", &clast);
  if (!(mpv->kind = meetConstGageKindParse(ctok))) {
    biffAddf(MEET, "%s: couldn't parse \"%s\" as kind", me, ctok);
    airMopError(mop); return 1;
  }
  if (wantSS) {
    int haveFlags;
    ctok = airStrtok(NULL, ":", &clast);
    if (!( 3 == airStrntok(ctok, "-") || 4 == airStrntok(ctok, "-") )) {
      biffAddf(MEET, "%s: didn't get 3 or 4 \"-\"-separated tokens in \"%s\"; "
               "not of form SFMT" , me, ctok);
      airMopError(mop); return 1;
    }
    haveFlags = (4 == airStrntok(ctok, "-"));
    dtok = airStrtok(ctok, "-", &dlast);
    if (1 != sscanf(dtok, "%lg", &(mpv->rangeSS[0]))) {
      biffAddf(MEET, "%s: couldn't parse \"%s\" as min scale", me, dtok);
      airMopError(mop); return 1;
    }
    dtok = airStrtok(NULL, "-", &dlast);
    if (1 != sscanf(dtok, "%u", &(mpv->numSS))) {
      biffAddf(MEET, "%s: couldn't parse \"%s\" as # scale samples", me, dtok);
      airMopError(mop); return 1;
    }
    if (!( mpv->numSS >= 2 )) {
      biffAddf(MEET, "%s: need # scale samples >= 2 (not %u)", me, mpv->numSS);
      airMopError(mop); return 1;
    }
    dtok = airStrtok(NULL, "-", &dlast);
    if (1 != sscanf(dtok, "%lg", &(mpv->rangeSS[1]))) {
      biffAddf(MEET, "%s: couldn't parse \"%s\" as max scale", me, dtok);
      airMopError(mop); return 1;
    }
    /* initialize things as if there were no flags */
    mpv->derivNormSS = AIR_FALSE;
    mpv->uniformSS = AIR_FALSE;
    mpv->optimSS = AIR_FALSE;
    mpv->derivNormBiasSS = 0.0;
    if (haveFlags) {
      char *flags, *bias;
      /* look for various things in flags */
      flags = airToLower(airStrdup(airStrtok(NULL, "-", &dlast)));
      airMopAdd(mop, flags, airFree, airMopAlways);
      if (strchr(flags, 'n')) {
        mpv->derivNormSS = AIR_TRUE;
      }
      if (strchr(flags, 'u')) {
        mpv->uniformSS = AIR_TRUE;
      }
      if (strchr(flags, 'o')) {
        mpv->optimSS = AIR_TRUE;
      }
      if (mpv->optimSS && mpv->uniformSS) {
        biffAddf(MEET, "%s: can't have both optimal ('o') and uniform ('u') "
                 "flags set in \"%s\"", me, flags);
        airMopError(mop); return 1;
      }
      if ((bias = strchr(flags, '+'))) {
        /* indicating a bias, unfortunately only a positive one is
           possible here, because of the way that other fields are
           tokenized by '-' */
        bias++;
        if (1 != sscanf(bias, "%lf", &(mpv->derivNormBiasSS))) {
          biffAddf(MEET, "%s: couldn't parse bias \"%s\"", me, bias);
          airMopError(mop); return 1;
        }
      }
    }
    /* mpv->ninSS and mpv->posSS are allocated and filled elsewhere */
    mpv->ninSS = NULL;
    mpv->posSS = NULL;
    /* we don't actually create nrrds nor load the volumes here,
       because we don't know cachePath, and because we might want
       different pullVolumes to share the same underlying Nrrds */
  } else {
    /* no scale-space stuff wanted */
    mpv->numSS = 0;
    mpv->rangeSS[0] = mpv->rangeSS[1] = AIR_NAN;
    mpv->ninSS = NULL;
    mpv->posSS = NULL;
  }
  ctok = airStrtok(NULL, ":", &clast);
  if (!(mpv->volName = airStrdup(ctok))) {
    biffAddf(MEET, "%s: couldn't strdup volName", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, &(mpv->volName), (airMopper)airSetNull, airMopOnError);
  airMopAdd(mop, mpv->volName, airFree, airMopOnError);

  if (strchr(ctok, '-')) {
    biffAddf(MEET, "%s: you probably don't want \"-\" in volume name \"%s\"; "
             "forgot last \":\" in scale sampling specification?", me, ctok);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

int
meetHestPullVolParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  static const char me[]="meetHestPullVolParse";
  meetPullVol *mpv, **mpvP;
  airArray *mop;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  mpvP = AIR_CAST(meetPullVol **, ptr);
  *mpvP = mpv = meetPullVolNew();
  airMopAdd(mop, mpvP, (airMopper)airSetNull, airMopOnError);
  airMopAdd(mop, mpv, (airMopper)meetPullVolNix, airMopOnError);
  if (meetPullVolParse(mpv, str)) {
    char *ler;
    airMopAdd(mop, ler = biffGetDone(MEET), airFree, airMopOnError);
    airStrcpy(err, AIR_STRLEN_HUGE, ler);
    airMopError(mop);
    return 1;
  }
  airMopOkay(mop);
  return 0;
}

/*
******** meetPullVolNix
**
** this frees stuff allocated meetPullVolParse and meetPullVolLoadMulti
*/
meetPullVol *
meetPullVolNix(meetPullVol *mpv) {

  if (mpv) {
    if (!mpv->leeching && mpv->nin) {
      nrrdNuke(mpv->nin);
    }
    if (mpv->numSS) {
      unsigned int ssi;
      if (mpv->ninSS) {
        /* need this check because the mpv may not have benefitted
           from meetPullVolLoadMulti, so it might be incomplete */
        for (ssi=0; ssi<mpv->numSS; ssi++) {
          if (!mpv->leeching) {
            nrrdNuke(mpv->ninSS[ssi]);
          }
        }
        airFree(mpv->ninSS);
      }
      airFree(mpv->posSS);
    }
    airFree(mpv->fileName);
    airFree(mpv->volName);
    airFree(mpv);
  }
  return NULL;
}

hestCB
_meetHestPullVol = {
  sizeof(meetPullVol *),
  "meetPullVol",
  meetHestPullVolParse,
  (airMopper)meetPullVolNix,
};

hestCB *
meetHestPullVol = &_meetHestPullVol;

/*
******** meetPullVolLeechable
**
** indicates whether lchr can leech from orig
*/
int
meetPullVolLeechable(const meetPullVol *lchr,
                     const meetPullVol *orig) {
  static const char me[]="meetPullVolLeechable";
  int can, verbose;

  verbose = 0;
  can = !!strcmp(orig->fileName, "-");  /* can, if not reading from stdin */
  if (verbose && !can) {
    fprintf(stderr, "%s: no: from stdin\n", me);
  }
  can &= !strcmp(orig->fileName, lchr->fileName);  /* come from same file */
  if (verbose && !can) {
    fprintf(stderr, "%s: no: not from same file\n", me);
  }
  can &= (orig->kind == lchr->kind);               /* same kind */
  if (verbose && !can) {
    fprintf(stderr, "%s: no: not same kind\n", me);
  }
  /* need to have different volname */
  can &= (orig->numSS == lchr->numSS);             /* same scale space */
  if (verbose && !can) {
    fprintf(stderr, "%s: no: not same scale space\n", me);
  }
  if (lchr->numSS) {
    /* DO allow difference in derivNormSS (the main reason for leeching) */
    /* same SS sampling strategy */
    can &= (orig->uniformSS == lchr->uniformSS);
    if (verbose && !can) {
      fprintf(stderr, "%s: no: not same uniformSS\n", me);
    }
    can &= (orig->optimSS == lchr->optimSS);
    if (verbose && !can) {
      fprintf(stderr, "%s: no: not same optimSS\n", me);
    }
    /* same SS range */
    can &= (orig->rangeSS[0] == lchr->rangeSS[0]);
    can &= (orig->rangeSS[1] == lchr->rangeSS[1]);
    if (verbose && !can) {
      fprintf(stderr, "%s: no: not same rangeSS\n", me);
    }
  }
  return can;
}

void
meetPullVolLeech(meetPullVol *vol,
                 const meetPullVol *volPrev) {

  if (vol && volPrev) {
    vol->nin = volPrev->nin;
    if (vol->numSS) {
      unsigned int ni;
      /* have to allocate ninSS here; in volPrev it was probably allocated
         by gageStackBlurManage */
      vol->ninSS = AIR_CALLOC(vol->numSS, Nrrd *);
      /* have to allocate posSS here; in volPrev is was probably allocated
         by meetPullVolLoadMulti */
      vol->posSS = AIR_CALLOC(vol->numSS, double);
      for (ni=0; ni<vol->numSS; ni++) {
        vol->ninSS[ni] = volPrev->ninSS[ni];
        vol->posSS[ni] = volPrev->posSS[ni];
      }
    }
    vol->leeching = AIR_TRUE;
  }
  return;
}

/*
******** meetPullVolLoadMulti
**
** at this point the per-pullVolume information required for
** loading/creating the volumes, which is NOT in the meetPullVol, is
** the cachePath and the info we have to set in the gageStackBlurParm,
** so these have to be passed explicitly.
*/
int
meetPullVolLoadMulti(meetPullVol **mpv, unsigned int mpvNum,
                     char *cachePath, NrrdKernelSpec *kSSblur,
                     int boundary, double padValue, int verbose) {
  static const char me[]="meetPullVolLoadMulti";
  char formatSS[AIR_STRLEN_LARGE];
  unsigned int mpvIdx;
  gageStackBlurParm *sbp;
  airArray *mop;
  meetPullVol *vol;

  if (!( mpv && cachePath && kSSblur )) {
    biffAddf(MEET, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();

  /* this can be re-used for different volumes */
  sbp = gageStackBlurParmNew();
  airMopAdd(mop, sbp, (airMopper)gageStackBlurParmNix, airMopAlways);

  for (mpvIdx=0; mpvIdx<mpvNum; mpvIdx++) {
    unsigned int pvi, ssi;
    vol = mpv[mpvIdx];
    for (pvi=0; pvi<mpvIdx; pvi++) {
      if (meetPullVolLeechable(vol, mpv[pvi])) {
        meetPullVolLeech(vol, mpv[pvi]);
        break; /* prevent a chain of leeching */
      }
    }
    if (pvi < mpvIdx) {
      /* we leeched this one, move on */
      if (verbose) {
        fprintf(stderr, "%s: vspec[%u] (%s) leeching off vspec[%u] (%s)\n",
                me, mpvIdx, vol->volName, pvi, mpv[pvi]->volName);
      }
      continue;
    }
    /* else we're not leeching */
    vol->leeching = AIR_FALSE;
    if (verbose) {
      fprintf(stderr, "%s: vspec[%u] (%s) cannot leech\n",
              me, mpvIdx, vol->volName);
    }
    /* if this is the pull we only have to learn the scale samples,
       but these might even be used */
    vol->nin = nrrdNew();
    airMopAdd(mop, &(vol->nin), (airMopper)airSetNull, airMopOnError);
    airMopAdd(mop, vol->nin, (airMopper)nrrdNuke, airMopOnError);
    if (nrrdLoad(vol->nin, vol->fileName, NULL)) {
      biffMovef(MEET, NRRD, "%s: trouble loading mpv[%u]->nin (\"%s\")",
                me, mpvIdx, vol->volName);
      airMopError(mop); return 1;
    }
    if (vol->numSS) {
      sprintf(formatSS, "%s/%s-%%03u-%03u.nrrd",
              cachePath, vol->volName, vol->numSS);
      if (verbose) {
        fprintf(stderr, "%s: managing %s ... \n", me, formatSS);
      }
      if (gageStackBlurParmScaleSet(sbp, vol->numSS,
                                    vol->rangeSS[0], vol->rangeSS[1],
                                    vol->uniformSS, vol->optimSS)
          || gageStackBlurParmKernelSet(sbp, kSSblur, AIR_TRUE)
          || gageStackBlurParmBoundarySet(sbp, boundary, padValue)
          || gageStackBlurParmVerboseSet(sbp, verbose)
          || gageStackBlurManage(&(vol->ninSS), &(vol->recomputedSS), sbp,
                                 formatSS, AIR_TRUE, NULL,
                                 vol->nin, vol->kind)) {
        biffMovef(MEET, GAGE, "%s: trouble getting volume stack (\"%s\")",
                  me, formatSS);
        airMopError(mop); return 1;
      }
      if (verbose) {
        fprintf(stderr, "%s: ... done\n", me);
      }
    }
    /* allocate and set vol->posSS from sbp-scale regardless of kind */
    if (vol->numSS) {
      vol->posSS = AIR_CALLOC(sbp->num, double);
      for (ssi=0; ssi<sbp->num; ssi++) {
        vol->posSS[ssi] = sbp->scale[ssi];
      }
    }
  }
  airMopOkay(mop);
  return 0;
}

/*
******** meetPullVolAddMulti
**
** the spatial (k00, k11, k22) and scale (kSSrecon) reconstruction
** kernels are not part of the meetPullVol, so have to be passed in here
*/
int
meetPullVolAddMulti(pullContext *pctx,
                    meetPullVol **mpv, unsigned int mpvNum,
                    const NrrdKernelSpec *k00,
                    const NrrdKernelSpec *k11,
                    const NrrdKernelSpec *k22,
                    const NrrdKernelSpec *kSSrecon) {
  static const char me[]="meetPullVolAddMulti";
  unsigned int mpvIdx;

  if (!( pctx && mpv )) {
    biffAddf(MEET, "%s: got NULL pointer", me);
    return 1;
  }
  for (mpvIdx=0; mpvIdx<mpvNum; mpvIdx++) {
    meetPullVol *vol;
    int E;
    vol = mpv[mpvIdx];
    if (!vol->numSS) {
      E = pullVolumeSingleAdd(pctx, vol->kind, vol->volName,
                              vol->nin, k00, k11, k22);
    } else {
      E = pullVolumeStackAdd(pctx, vol->kind, vol->volName, vol->nin,
                             AIR_CAST(const Nrrd *const *,
                                      vol->ninSS),
                             vol->posSS, vol->numSS,
                             vol->derivNormSS, vol->derivNormBiasSS,
                             k00, k11, k22, kSSrecon);
    }
    if (E) {
      biffMovef(MEET, PULL, "%s: trouble adding volume %u/%u (\"%s\")",
                me, mpvIdx, mpvNum, vol->volName);
      return 1;
    }
  }

  return 0;
}

meetPullInfo *
meetPullInfoNew(void) {
  meetPullInfo *ret;

  ret = AIR_CALLOC(1, meetPullInfo);
  if (ret) {
    ret->info = 0;
    ret->source = pullSourceUnknown;
    ret->prop = pullPropUnknown;
    ret->constraint = AIR_FALSE;
    ret->volName = ret->itemStr = NULL;
    ret->zero = ret->scale = AIR_NAN;
  }
  return ret;
}

meetPullInfo *
meetPullInfoNix(meetPullInfo *minf) {

  if (minf) {
    airFree(minf->volName);
    airFree(minf->itemStr);
    free(minf);
  }
  return NULL;
}

static int
zeroScaleSet(meetPullInfo *minf, int haveZS, char **lastP) {
  static const char me[]="_zeroScaleSet";
  char *tok;

  if (haveZS) {
    tok = airStrtok(NULL, ":", lastP);
    if (1 != sscanf(tok, "%lf", &(minf->zero))) {
      biffAddf(MEET, "%s: couldn't parse %s as zero (double)", me, tok);
      return 1;
    }
    tok = airStrtok(NULL, ":", lastP);
    if (1 != sscanf(tok, "%lf", &(minf->scale))) {
      biffAddf(MEET, "%s: couldn't parse %s as scale (double)", me, tok);
      return 1;
    }
  } else {
    minf->zero = minf->scale = AIR_NAN;
  }
  return 0;
}

int
meetPullInfoParse(meetPullInfo *minf, const char *_str) {
  static const char me[]="meetPullInfoParse";
#define IFMT_GAGE "<info>[-c]:<volname>:<item>[:<zero>:<scale>]"
#define IFMT_PROP "<info>:prop=<prop>[:<zero>:<scale>]"
#define PROP_PREFIX "prop="   /* has to end with = */
  char *str, *tok, *last=NULL, *iflags;
  airArray *mop;
  int haveZS, source;

  if (!(minf && _str)) {
    biffAddf(MEET, "%s: got NULL pointer", me);
    return 1;
  }
  if ( (3 == airStrntok(_str, ":") || 5 == airStrntok(_str, ":"))
       && 1 == airStrntok(_str, "=") ) {
    source = pullSourceGage;
    haveZS = (5 == airStrntok(_str, ":"));
  } else if ( (2 == airStrntok(_str, ":") || 4 == airStrntok(_str, ":"))
              && 2 == airStrntok(_str, "=") ) {
    source = pullSourceProp;
    haveZS = (4 == airStrntok(_str, ":"));
  } else {
    biffAddf(MEET, "%s: \"%s\" not of form " IFMT_GAGE " or " IFMT_PROP,
             me, _str);
    return 1;
  }

  mop = airMopNew();
  if (!( str = airStrdup(_str) )) {
    biffAddf(MEET, "%s: couldn't strdup input", me);
    return 1;
  }
  airMopAdd(mop, str, airFree, airMopAlways);

  minf->source = source;
  if (pullSourceGage == source) {
    tok = airStrtok(str, ":", &last);
    iflags = strchr(tok, '-');
    if (iflags) {
      *iflags = '\0';
      iflags++;
    }
    if (!(minf->info = airEnumVal(pullInfo, tok))) {
      biffAddf(MEET, "%s: couldn't parse \"%s\" as %s",
               me, tok, pullInfo->name);
      airMopError(mop); return 1;
    }
    if (iflags) {
      if (strchr(iflags, 'c')) {
        minf->constraint = AIR_TRUE;
      }
    }
    tok = airStrtok(NULL, ":", &last);
    airFree(minf->volName);
    minf->volName = airStrdup(tok);
    airMopAdd(mop, minf->volName, airFree, airMopOnError);
    tok = airStrtok(NULL, ":", &last);
    airFree(minf->itemStr);
    minf->itemStr = airStrdup(tok);
    airMopAdd(mop, minf->itemStr, airFree, airMopOnError);
    if (zeroScaleSet(minf, haveZS, &last)) {
      biffAddf(MEET, "%s: couldn't parse zero or scale",  me);
      airMopError(mop); return 1;
    }
  } else if (pullSourceProp == source) {
    /* "<info>:prop=<prop>[:<zero>:<scale>]" */
    tok = airStrtok(str, ":", &last);
    if (!(minf->info = airEnumVal(pullInfo, tok))) {
      biffAddf(MEET, "%s: couldn't parse \"%s\" as %s",
               me, tok, pullInfo->name);
      airMopError(mop); return 1;
    }
    tok = airStrtok(NULL, ":", &last);
    if (strncmp(PROP_PREFIX, tok, strlen(PROP_PREFIX))) {
      biffAddf(MEET, "%s: property info didn't start with %s",
               me, PROP_PREFIX);
    }
    tok += strlen(PROP_PREFIX);
    if (!(minf->prop = airEnumVal(pullProp, tok))) {
      biffAddf(MEET, "%s: couldn't parse \"%s\" as %s",
               me, tok, pullProp->name);
      airMopError(mop); return 1;
    }
    if (zeroScaleSet(minf, haveZS, &last)) {
      biffAddf(MEET, "%s: couldn't parse zero or scale",  me);
      airMopError(mop); return 1;
    }
  } else {
    biffAddf(MEET, "%s: sorry, source %s not handled",
             me, airEnumStr(pullSource, source));
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

int
meetHestPullInfoParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  static const char me[]="meetHestPullInfoParse";
  airArray *mop;
  meetPullInfo **minfP, *minf;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  minfP = AIR_CAST(meetPullInfo **, ptr);
  *minfP = minf = meetPullInfoNew();
  airMopAdd(mop, minfP, (airMopper)airSetNull, airMopOnError);
  airMopAdd(mop, minf, (airMopper)meetPullInfoNix, airMopOnError);
  if (meetPullInfoParse(minf, str)) {
    char *ler;
    airMopAdd(mop, ler = biffGetDone(MEET), airFree, airMopOnError);
    airStrcpy(err, AIR_STRLEN_HUGE, ler);
    airMopError(mop);
    return 1;
  }
  airMopOkay(mop);
  return 0;
}

hestCB
_meetHestPullInfo = {
  sizeof(meetPullInfo *),
  "meetPullInfo",
  meetHestPullInfoParse,
  (airMopper)meetPullInfoNix
};

hestCB *
meetHestPullInfo = &_meetHestPullInfo;

int
meetPullInfoAddMulti(pullContext *pctx,
                     meetPullInfo **minf, unsigned int minfNum) {
  static const char me[]="meetPullInfoAddMulti";
  const pullVolume *vol;
  unsigned int ii;
  airArray *mop;

  if (!( pctx && minf )) {
    biffAddf(MEET, "%s: got NULL pointer", me);
    return 1;
  }

  mop = airMopNew();
  for (ii=0; ii<minfNum; ii++) {
    pullInfoSpec *ispec;
    ispec = pullInfoSpecNew();
    airMopAdd(mop, ispec, (airMopper)pullInfoSpecNix, airMopOnError);
    ispec->volName = airStrdup(minf[ii]->volName);
    ispec->source = minf[ii]->source;
    ispec->info = minf[ii]->info;
    ispec->prop = minf[ii]->prop;
    ispec->zero = minf[ii]->zero;
    ispec->scale = minf[ii]->scale;
    ispec->constraint = minf[ii]->constraint;
    /* the item is the one thing that takes some work to recover;
       we need to find the volume and find the item from its kind->enm */
    if (pullSourceGage == ispec->source) {
      if (!( vol = pullVolumeLookup(pctx, minf[ii]->volName) )) {
        biffMovef(MEET, PULL, "%s: can't find volName \"%s\" for minf[%u]",
                  me, minf[ii]->volName, ii);
        airMopError(mop); return 1;
      }
      if (!( ispec->item = airEnumVal(vol->kind->enm, minf[ii]->itemStr))) {
        biffAddf(MEET, "%s: can't parse \"%s\" as item of %s kind (minf[%u])\n",
                 me, minf[ii]->itemStr, vol->kind->name, ii);
        airMopError(mop); return 1;
      }
    }
    if (pullInfoSpecAdd(pctx, ispec)) {
      biffMovef(MEET, PULL, "%s: trouble adding ispec from minf[%u]", me, ii);
      airMopError(mop); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}
