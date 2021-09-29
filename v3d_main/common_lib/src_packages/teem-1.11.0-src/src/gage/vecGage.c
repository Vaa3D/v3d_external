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

#include "gage.h"
#include "privateGage.h"

/*
 * highly inefficient computation of the imaginary part of complex
 * conjugate eigenvalues of a 3x3 non-symmetric matrix
 */
double
gage_imaginary_part_eigenvalues(double *M ) {
    double A, B, C, scale, frob, m[9], _eval[3];
    double beta, gamma;
    int roots;

    frob = ELL_3M_FROB(M);
    scale = frob > 10 ? 10.0/frob : 1.0;
    ELL_3M_SCALE(m, scale, M);
    /*
    ** from gordon with mathematica; these are the coefficients of the
    ** cubic polynomial in x: det(x*I - M).  The full cubic is
    ** x^3 + A*x^2 + B*x + C.
    */
    A = -m[0] - m[4] - m[8];
    B = m[0]*m[4] - m[3]*m[1]
        + m[0]*m[8] - m[6]*m[2]
        + m[4]*m[8] - m[7]*m[5];
    C = (m[6]*m[4] - m[3]*m[7])*m[2]
        + (m[0]*m[7] - m[6]*m[1])*m[5]
        + (m[3]*m[1] - m[0]*m[4])*m[8];
    roots = ell_cubic(_eval, A, B, C, AIR_TRUE);
    if ( roots != ell_cubic_root_single )
        return 0.;

    /* 2 complex conjuguate eigenvalues */
    beta = A + _eval[0];
    gamma = -C/_eval[0];
    return sqrt( 4.*gamma - beta*beta );
}


gageItemEntry
_gageVecTable[GAGE_VEC_ITEM_MAX+1] = {
  /* enum value         len, deriv, prereqs,                                                  parent item, parent index, needData */
  {gageVecUnknown,         0,  0,   {0},                                                                0,      0,       AIR_FALSE},
  {gageVecVector,          3,  0,   {0},                                                                0,      0,       AIR_FALSE},
  {gageVecVector0,         1,  0,   {gageVecVector},                                        gageVecVector,      0,       AIR_FALSE},
  {gageVecVector1,         1,  0,   {gageVecVector},                                        gageVecVector,      1,       AIR_FALSE},
  {gageVecVector2,         1,  0,   {gageVecVector},                                        gageVecVector,      2,       AIR_FALSE},
  {gageVecLength,          1,  0,   {gageVecVector},                                                    0,      0,       AIR_FALSE},
  {gageVecNormalized,      3,  0,   {gageVecVector, gageVecLength},                                     0,      0,       AIR_FALSE},
  {gageVecJacobian,        9,  1,   {0},                                                                0,      0,       AIR_FALSE},
  {gageVecStrain,          9,  1,   {gageVecJacobian},                                                  0,      0,       AIR_FALSE},
  {gageVecDivergence,      1,  1,   {gageVecJacobian},                                                  0,      0,       AIR_FALSE},
  {gageVecCurl,            3,  1,   {gageVecJacobian},                                                  0,      0,       AIR_FALSE},
  {gageVecCurlNorm,        1,  1,   {gageVecCurl},                                                      0,      0,       AIR_FALSE},
  {gageVecHelicity,        1,  1,   {gageVecVector, gageVecCurl},                                       0,      0,       AIR_FALSE},
  {gageVecNormHelicity,    1,  1,   {gageVecNormalized, gageVecCurl},                                   0,      0,       AIR_FALSE},
  {gageVecSOmega,          9,  1,   {gageVecJacobian,gageVecStrain},                                    0,      0,       AIR_FALSE},
  {gageVecLambda2,         1,  1,   {gageVecSOmega},                                                    0,      0,       AIR_FALSE},
  {gageVecImaginaryPart,   1,  1,   {gageVecJacobian},                                                  0,      0,       AIR_FALSE},
  {gageVecHessian,        27,  2,   {0},                                                                0,      0,       AIR_FALSE},
  {gageVecDivGradient,     3,  1,   {gageVecHessian},                                                   0,      0,       AIR_FALSE},
  {gageVecCurlGradient,    9,  2,   {gageVecHessian},                                                   0,      0,       AIR_FALSE},
  {gageVecCurlNormGrad,    3,  2,   {gageVecHessian, gageVecCurl},                                      0,      0,       AIR_FALSE},
  {gageVecNCurlNormGrad,   3,  2,   {gageVecCurlNormGrad},                                              0,      0,       AIR_FALSE},
  {gageVecHelGradient,     3,  2,   {gageVecVector, gageVecJacobian, gageVecCurl, gageVecCurlGradient}, 0,      0,       AIR_FALSE},
  {gageVecDirHelDeriv,     1,  2,   {gageVecNormalized, gageVecHelGradient},                            0,      0,       AIR_FALSE},
  {gageVecProjHelGradient, 3,  2,   {gageVecNormalized, gageVecHelGradient, gageVecDirHelDeriv},        0,      0,       AIR_FALSE},
  /* HEY: these should change to sub-items!!! */
  {gageVecGradient0,       3,  1,   {gageVecJacobian},                                                  0,      0,       AIR_FALSE},
  {gageVecGradient1,       3,  1,   {gageVecJacobian},                                                  0,      0,       AIR_FALSE},
  {gageVecGradient2,       3,  1,   {gageVecJacobian},                                                  0,      0,       AIR_FALSE},
  {gageVecMultiGrad,       9,  1,   {gageVecGradient0, gageVecGradient1, gageVecGradient2},             0,      0,       AIR_FALSE},
  {gageVecMGFrob,          1,  1,   {gageVecMultiGrad},                                                 0,      0,       AIR_FALSE},
  {gageVecMGEval,          3,  1,   {gageVecMultiGrad},                                                 0,      0,       AIR_FALSE},
  {gageVecMGEvec,          9,  1,   {gageVecMultiGrad, gageVecMGEval},                                  0,      0,       AIR_FALSE}
};

void
_gageVecFilter(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_gageVecFilter";
  double *fw00, *fw11, *fw22, *vec, *jac, *hes;
  int fd;
  gageScl3PFilter_t *filter[5] = {NULL, gageScl3PFilter2, gageScl3PFilter4,
                                  gageScl3PFilter6, gageScl3PFilter8};
  unsigned int valIdx;

  fd = 2*ctx->radius;
  vec  = pvl->directAnswer[gageVecVector];
  jac  = pvl->directAnswer[gageVecJacobian];
  hes  = pvl->directAnswer[gageVecHessian];
  if (!ctx->parm.k3pack) {
    fprintf(stderr, "!%s: sorry, 6pack filtering not implemented\n", me);
    return;
  }
  fw00 = ctx->fw + fd*3*gageKernel00;
  fw11 = ctx->fw + fd*3*gageKernel11;
  fw22 = ctx->fw + fd*3*gageKernel22;
  /* perform the filtering */
  if (fd <= 8) {
    for (valIdx=0; valIdx<3; valIdx++) {
      filter[ctx->radius](ctx->shape,
                          pvl->iv3 + valIdx*fd*fd*fd,
                          pvl->iv2 + valIdx*fd*fd,
                          pvl->iv1 + valIdx*fd,
                          fw00, fw11, fw22,
                          vec + valIdx, jac + valIdx*3, hes + valIdx*9,
                          pvl->needD);
    }
  } else {
    for (valIdx=0; valIdx<3; valIdx++) {
      gageScl3PFilterN(ctx->shape, fd,
                       pvl->iv3 + valIdx*fd*fd*fd,
                       pvl->iv2 + valIdx*fd*fd,
                       pvl->iv1 + valIdx*fd,
                       fw00, fw11, fw22,
                       vec + valIdx, jac + valIdx*3, hes + valIdx*9,
                       pvl->needD);
    }
  }

  return;
}

void
_gageVecAnswer(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_gageVecAnswer";
  double cmag, tmpMat[9], mgevec[9], mgeval[3];
  double asym[9], tran[9], eval[3], tmpVec[3], norm;
  double *vecAns, *normAns, *jacAns, *strainAns, *somegaAns,
    *curlAns, *hesAns, *curlGradAns,
    *helGradAns, *dirHelDirAns, *curlnormgradAns;
  /* int asw; */

  vecAns          = pvl->directAnswer[gageVecVector];
  normAns         = pvl->directAnswer[gageVecNormalized];
  jacAns          = pvl->directAnswer[gageVecJacobian];
  strainAns       = pvl->directAnswer[gageVecStrain];
  somegaAns       = pvl->directAnswer[gageVecSOmega];
  curlAns         = pvl->directAnswer[gageVecCurl];
  hesAns          = pvl->directAnswer[gageVecHessian];
  curlGradAns     = pvl->directAnswer[gageVecCurlGradient];
  curlnormgradAns = pvl->directAnswer[gageVecCurlNormGrad];
  helGradAns      = pvl->directAnswer[gageVecHelGradient];
  dirHelDirAns    = pvl->directAnswer[gageVecDirHelDeriv];

  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecVector)) {
    /* done if doV */
    if (ctx->verbose) {
      fprintf(stderr, "vec = ");
      ell_3v_print_d(stderr, vecAns);
    }
  }
  /* done if doV
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecVector{0,1,2})) {
  }
  */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecLength)) {
    pvl->directAnswer[gageVecLength][0] = ELL_3V_LEN(vecAns);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecNormalized)) {
    if (pvl->directAnswer[gageVecLength][0]) {
      ELL_3V_SCALE(normAns, 1.0/pvl->directAnswer[gageVecLength][0], vecAns);
    } else {
      ELL_3V_COPY(normAns, gageZeroNormal);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecJacobian)) {
    /* done if doD1 */
    /*
      0:dv_x/dx  1:dv_x/dy  2:dv_x/dz
      3:dv_y/dx  4:dv_y/dy  5:dv_y/dz
      6:dv_z/dx  7:dv_z/dy  8:dv_z/dz
    */
    if (ctx->verbose) {
      fprintf(stderr, "%s: jac = \n", me);
      ell_3m_print_d(stderr, jacAns);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecDivergence)) {
    pvl->directAnswer[gageVecDivergence][0] = jacAns[0] + jacAns[4] + jacAns[8];
    if (ctx->verbose) {
      fprintf(stderr, "%s: div = %g + %g + %g  = %g\n", me,
              jacAns[0], jacAns[4], jacAns[8],
              pvl->directAnswer[gageVecDivergence][0]);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecCurl)) {
    ELL_3V_SET(curlAns,
               jacAns[7] - jacAns[5],
               jacAns[2] - jacAns[6],
               jacAns[3] - jacAns[1]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecCurlNorm)) {
    pvl->directAnswer[gageVecCurlNorm][0] = ELL_3V_LEN(curlAns);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecHelicity)) {
    pvl->directAnswer[gageVecHelicity][0] =
      ELL_3V_DOT(vecAns, curlAns);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecNormHelicity)) {
    cmag = ELL_3V_LEN(curlAns);
    pvl->directAnswer[gageVecNormHelicity][0] =
      cmag ? ELL_3V_DOT(normAns, curlAns)/cmag : 0;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecStrain)) {
    ELL_3M_TRANSPOSE(tran, jacAns);
    /* symmetric part */
    ELL_3M_SCALE_ADD2(strainAns, 0.5, jacAns,  0.5, tran);
    /* nested these ifs to make dependency explicit to the compiler */
    if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecSOmega)) {
      /* antisymmetric part */
      ELL_3M_SCALE_ADD2(asym, 0.5, jacAns, -0.5, tran);
      /* square symmetric part */
      ELL_3M_MUL(tmpMat, strainAns, strainAns);
      ELL_3M_COPY(somegaAns, tmpMat);
      /* square antisymmetric part */
      ELL_3M_MUL(tmpMat, asym, asym);
      /* sum of both */
      ELL_3M_ADD2(somegaAns, somegaAns, tmpMat);
      if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecLambda2)) {
        /* get eigenvalues in sorted order */
        /* asw = */ ell_3m_eigenvalues_d(eval, somegaAns, AIR_TRUE);
        pvl->directAnswer[gageVecLambda2][0] = eval[1];
      }
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecImaginaryPart)) {
    pvl->directAnswer[gageVecImaginaryPart][0] =
      gage_imaginary_part_eigenvalues(jacAns);
  }
  /* 2nd order vector derivative continued */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecHessian)) {
    /* done if doD2 */
    /* the ordering is induced by the scalar hessian computation :
       0:d2v_x/dxdx   1:d2v_x/dxdy   2:d2v_x/dxdz
       3:d2v_x/dydx   4:d2v_x/dydy   5:d2v_x/dydz
       6:d2v_x/dzdx   7:d2v_x/dzdy   8:d2v_x/dzdz
       9:d2v_y/dxdx      [. . .]
          [. . .]
       24:dv2_z/dzdx  25:d2v_z/dzdy  26:d2v_z/dzdz
    */
    if (ctx->verbose) {
      fprintf(stderr, "%s: hes = \n", me);
      ell_3m_print_d(stderr, hesAns); /* ?? */
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecDivGradient)) {
      pvl->directAnswer[gageVecDivGradient][0] = hesAns[0] + hesAns[12] + hesAns[24];
      pvl->directAnswer[gageVecDivGradient][1] = hesAns[1] + hesAns[13] + hesAns[25];
      pvl->directAnswer[gageVecDivGradient][2] = hesAns[2] + hesAns[14] + hesAns[26];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecCurlGradient)) {
      pvl->directAnswer[gageVecCurlGradient][0] = hesAns[21]-hesAns[15];
      pvl->directAnswer[gageVecCurlGradient][1] = hesAns[22]-hesAns[16];
      pvl->directAnswer[gageVecCurlGradient][2] = hesAns[23]-hesAns[17];
      pvl->directAnswer[gageVecCurlGradient][3] = hesAns[ 6]-hesAns[18];
      pvl->directAnswer[gageVecCurlGradient][4] = hesAns[ 7]-hesAns[19];
      pvl->directAnswer[gageVecCurlGradient][5] = hesAns[ 8]-hesAns[20];
      pvl->directAnswer[gageVecCurlGradient][6] = hesAns[ 9]-hesAns[ 1];
      pvl->directAnswer[gageVecCurlGradient][7] = hesAns[10]-hesAns[ 2];
      pvl->directAnswer[gageVecCurlGradient][8] = hesAns[11]-hesAns[ 3];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecCurlNormGrad)) {
      norm = 1./ELL_3V_LEN(curlAns);

      tmpVec[0] = hesAns[21] - hesAns[15];
      tmpVec[1] = hesAns[ 6] - hesAns[18];
      tmpVec[2] = hesAns[ 9] - hesAns[ 3];
      pvl->directAnswer[gageVecCurlNormGrad][0] =
        norm*ELL_3V_DOT(tmpVec, curlAns);

      tmpVec[0] = hesAns[22] - hesAns[16];
      tmpVec[1] = hesAns[ 7] - hesAns[19];
      tmpVec[2] = hesAns[10] - hesAns[ 4];
      pvl->directAnswer[gageVecCurlNormGrad][1]=
        norm*ELL_3V_DOT(tmpVec, curlAns);

      tmpVec[0] = hesAns[23] - hesAns[17];
      tmpVec[1] = hesAns[ 8] - hesAns[20];
      tmpVec[2] = hesAns[11] - hesAns[ 5];
      pvl->directAnswer[gageVecCurlNormGrad][2]=
        norm*ELL_3V_DOT(tmpVec, curlAns);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecNCurlNormGrad)) {
      norm = 1./ELL_3V_LEN(curlnormgradAns);
      ELL_3V_SCALE(pvl->directAnswer[gageVecNCurlNormGrad],
                   norm, pvl->directAnswer[gageVecCurlNormGrad]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecHelGradient)) {
      pvl->directAnswer[gageVecHelGradient][0] =
          jacAns[0]*curlAns[0]+
          jacAns[3]*curlAns[1]+
          jacAns[6]*curlAns[2]+
          curlGradAns[0]*vecAns[0]+
          curlGradAns[3]*vecAns[1]+
          curlGradAns[6]*vecAns[2];
      pvl->directAnswer[gageVecHelGradient][1] =
          jacAns[1]*curlAns[0]+
          jacAns[4]*curlAns[1]+
          jacAns[7]*curlAns[2]+
          curlGradAns[1]*vecAns[0]+
          curlGradAns[4]*vecAns[1]+
          curlGradAns[7]*vecAns[2];
      pvl->directAnswer[gageVecHelGradient][0] =
          jacAns[2]*curlAns[0]+
          jacAns[5]*curlAns[1]+
          jacAns[8]*curlAns[2]+
          curlGradAns[2]*vecAns[0]+
          curlGradAns[5]*vecAns[1]+
          curlGradAns[8]*vecAns[2];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecDirHelDeriv)) {
      pvl->directAnswer[gageVecDirHelDeriv][0] =
        ELL_3V_DOT(normAns, helGradAns);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecProjHelGradient)) {
      pvl->directAnswer[gageVecDirHelDeriv][0] =
          helGradAns[0]-dirHelDirAns[0]*normAns[0];
      pvl->directAnswer[gageVecDirHelDeriv][1] =
          helGradAns[1]-dirHelDirAns[0]*normAns[1];
      pvl->directAnswer[gageVecDirHelDeriv][2] =
          helGradAns[2]-dirHelDirAns[0]*normAns[2];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecGradient0)) {
    ELL_3V_SET(pvl->directAnswer[gageVecGradient0],
               jacAns[0],
               jacAns[1],
               jacAns[2]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecGradient1)) {
    ELL_3V_SET(pvl->directAnswer[gageVecGradient1],
               jacAns[3],
               jacAns[4],
               jacAns[5]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecGradient2)) {
    ELL_3V_SET(pvl->directAnswer[gageVecGradient2],
               jacAns[6],
               jacAns[7],
               jacAns[8]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecMultiGrad)) {
    ELL_3M_IDENTITY_SET(pvl->directAnswer[gageVecMultiGrad]);
    ELL_3MV_OUTER_INCR(pvl->directAnswer[gageVecMultiGrad],
                       pvl->directAnswer[gageVecGradient0],
                       pvl->directAnswer[gageVecGradient0]);
    ELL_3MV_OUTER_INCR(pvl->directAnswer[gageVecMultiGrad],
                       pvl->directAnswer[gageVecGradient1],
                       pvl->directAnswer[gageVecGradient1]);
    ELL_3MV_OUTER_INCR(pvl->directAnswer[gageVecMultiGrad],
                       pvl->directAnswer[gageVecGradient2],
                       pvl->directAnswer[gageVecGradient2]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecMGFrob)) {
    pvl->directAnswer[gageVecMGFrob][0]
      = ELL_3M_FROB(pvl->directAnswer[gageVecMultiGrad]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecMGEval)) {
    ELL_3M_COPY(tmpMat, pvl->directAnswer[gageVecMultiGrad]);
    /* HEY: look at the return value for root multiplicity? */
    ell_3m_eigensolve_d(mgeval, mgevec, tmpMat, AIR_TRUE);
    ELL_3V_COPY(pvl->directAnswer[gageVecMGEval], mgeval);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageVecMGEvec)) {
    ELL_3M_COPY(pvl->directAnswer[gageVecMGEvec], mgevec);
  }

  return;
}

const char *
_gageVecStr[] = {
  "(unknown gageVec)",
  "vector",
  "vector0",
  "vector1",
  "vector2",
  "length",
  "normalized",
  "Jacobian",
  "strain",
  "divergence",
  "curl",
  "curl norm",
  "helicity",
  "normalized helicity",
  "SOmega",
  "lambda2",
  "imag",
  "vector hessian",
  "div gradient",
  "curl gradient",
  "curl norm gradient",
  "normalized curl norm gradient",
  "helicity gradient",
  "directional helicity derivative",
  "projected helicity gradient",
  "gradient0",
  "gradient1",
  "gradient2",
  "multigrad",
  "frob(multigrad)",
  "multigrad eigenvalues",
  "multigrad eigenvectors",
};

const char *
_gageVecDesc[] = {
  "unknown gageVec query",
  "component-wise-interpolated vector",
  "vector[0]",
  "vector[1]",
  "vector[2]",
  "length of vector",
  "normalized vector",
  "3x3 Jacobian",
  "rate-of-strain tensor",
  "divergence",
  "curl",
  "curl magnitude",
  "helicity: dot(vector,curl)",
  "normalized helicity",
  "S squared plus Omega squared for vortex characterization",
  "lambda2 value of SOmega",
  "imaginary part of complex-conjugate eigenvalues of Jacobian",
  "3x3x3 second-order vector derivative",
  "gradient of divergence",
  "3x3 derivative of curl",
  "gradient of curl norm",
  "normalized gradient of curl norm",
  "gradient of helicity",
  "directional derivative of helicity along flow",
  "projection of the helicity gradient onto plane orthogonal to flow",
  "gradient of 1st component of vector",
  "gradient of 2nd component of vector",
  "gradient of 3rd component of vector",
  "multi-gradient: sum of outer products of gradients",
  "frob norm of multi-gradient",
  "eigenvalues of multi-gradient",
  "eigenvectors of multi-gradient"
};

const int
_gageVecVal[] = {
  gageVecUnknown,
  gageVecVector,
  gageVecVector0,
  gageVecVector1,
  gageVecVector2,
  gageVecLength,
  gageVecNormalized,
  gageVecJacobian,
  gageVecStrain,
  gageVecDivergence,
  gageVecCurl,
  gageVecCurlNorm,
  gageVecHelicity,
  gageVecNormHelicity,
  gageVecSOmega,
  gageVecLambda2,
  gageVecImaginaryPart,
  gageVecHessian,
  gageVecDivGradient,
  gageVecCurlGradient,
  gageVecCurlNormGrad,
  gageVecNCurlNormGrad,
  gageVecHelGradient,
  gageVecDirHelDeriv,
  gageVecProjHelGradient,
  gageVecGradient0,
  gageVecGradient1,
  gageVecGradient2,
  gageVecMultiGrad,
  gageVecMGFrob,
  gageVecMGEval,
  gageVecMGEvec,
};

#define GV_V   gageVecVector
#define GV_V0  gageVecVector0
#define GV_V1  gageVecVector1
#define GV_V2  gageVecVector2
#define GV_L   gageVecLength
#define GV_N   gageVecNormalized
#define GV_J   gageVecJacobian
#define GV_S   gageVecStrain
#define GV_D   gageVecDivergence
#define GV_C   gageVecCurl
#define GV_CM  gageVecCurlNorm
#define GV_H   gageVecHelicity
#define GV_NH  gageVecNormHelicity
#define GV_SO  gageVecSOmega
#define GV_LB  gageVecLambda2
#define GV_IM  gageVecImaginaryPart
#define GV_VH  gageVecHessian
#define GV_DG  gageVecDivGradient
#define GV_CG  gageVecCurlGradient
#define GV_CNG gageVecCurlNormGrad
#define GV_NCG gageVecNCurlNormGrad
#define GV_HG  gageVecHelGradient
#define GV_DH  gageVecDirHelDeriv
#define GV_PH  gageVecProjHelGradient
#define GV_G0  gageVecGradient0
#define GV_G1  gageVecGradient1
#define GV_G2  gageVecGradient2
#define GV_MG  gageVecMultiGrad
#define GV_MF  gageVecMGFrob
#define GV_ML  gageVecMGEval
#define GV_MC  gageVecMGEvec

const char *
_gageVecStrEqv[] = {
  "v", "vector", "vec",
  "v0", "vector0", "vec0",
  "v1", "vector1", "vec1",
  "v2", "vector2", "vec2",
  "l", "length", "len",
  "n", "normalized", "normalized vector",
  "jacobian", "jac", "j",
  "strain", "S",
  "divergence", "div", "d",
  "curl", "c",
  "curlnorm", "curl norm", "curl magnitude", "cm",
  "h", "hel", "hell", "helicity",
  "nh", "nhel", "normhel", "normhell", "normalized helicity",
  "SOmega",
  "lbda2", "lambda2",
  "imag", "imagpart",
  "vh", "vhes", "vhessian", "vector hessian",
  "dg", "divgrad", "div gradient",
  "cg", "curlgrad", "curlg", "curljac", "curl gradient",
  "cng", "curl norm gradient",
  "ncng", "norm curl norm gradient", "normalized curl norm gradient",
  "hg", "helg", "helgrad", "helicity gradient",
  "dirhelderiv", "dhd", "ddh", "directional helicity derivative",
  "phg", "projhel", "projhelgrad", "projected helicity gradient",
  "g0", "grad0", "gradient0",
  "g1", "grad1", "gradient1",
  "g2", "grad2", "gradient2",
  "mg", "multigrad",
  "mgfrob", "frob(multigrad)",
  "mgeval", "mg eval", "multigrad eigenvalues",
  "mgevec", "mg evec", "multigrad eigenvectors",
  ""
};

const int
_gageVecValEqv[] = {
  GV_V, GV_V, GV_V,
  GV_V0, GV_V0, GV_V0,
  GV_V1, GV_V1, GV_V1,
  GV_V2, GV_V2, GV_V2,
  GV_L, GV_L, GV_L,
  GV_N, GV_N, GV_N,
  GV_J, GV_J, GV_J,
  GV_S, GV_S,
  GV_D, GV_D, GV_D,
  GV_C, GV_C,
  GV_CM, GV_CM, GV_CM, GV_CM,
  GV_H, GV_H, GV_H, GV_H,
  GV_NH, GV_NH, GV_NH, GV_NH, GV_NH,
  GV_SO,
  GV_LB, GV_LB,
  GV_IM, GV_IM,
  GV_VH, GV_VH, GV_VH, GV_VH,
  GV_DG, GV_DG, GV_DG,
  GV_CG, GV_CG, GV_CG, GV_CG, GV_CG,
  GV_CNG, GV_CNG,
  GV_NCG, GV_NCG, GV_NCG,
  GV_HG, GV_HG, GV_HG, GV_HG,
  GV_DH, GV_DH, GV_DH, GV_DH,
  GV_PH, GV_PH, GV_PH, GV_PH,
  GV_G0, GV_G0, GV_G0,
  GV_G1, GV_G1, GV_G1,
  GV_G2, GV_G2, GV_G2,
  GV_MG, GV_MG,
  GV_MF, GV_MF,
  GV_ML, GV_ML, GV_ML,
  GV_MC, GV_MC, GV_MC
};

const airEnum
_gageVec = {
  "gageVec",
  GAGE_VEC_ITEM_MAX,
  _gageVecStr, _gageVecVal,
  _gageVecDesc,
  _gageVecStrEqv, _gageVecValEqv,
  AIR_FALSE
};
const airEnum *const
gageVec = &_gageVec;

gageKind
_gageKindVec = {
  AIR_FALSE, /* statically allocated */
  "vector",
  &_gageVec,
  1, /* baseDim */
  3, /* valLen */
  GAGE_VEC_ITEM_MAX,
  _gageVecTable,
  _gageVecIv3Print,
  _gageVecFilter,
  _gageVecAnswer,
  NULL, NULL, NULL, NULL,
  NULL
};
gageKind *const
gageKindVec = &_gageKindVec;

