  double mean, norm, rnorm, Q, R, QQQ, D, theta,
    M00, M01, M02, M11, M12, M22;
  double epsilon = 1.0E-12;
  int roots;

  /* copy the given matrix elements */
  M00 = _M00;
  M01 = _M01;
  M02 = _M02;
  M11 = _M11;
  M12 = _M12;
  M22 = _M22;

  /*
  ** subtract out the eigenvalue mean (will add back to evals later);
  ** helps with numerical stability
  */
  mean = (M00 + M11 + M22)/3.0;
  M00 -= mean;
  M11 -= mean;
  M22 -= mean;
  
  /* 
  ** divide out L2 norm of eigenvalues (will multiply back later);
  ** this too seems to help with stability
  */
  norm = sqrt(M00*M00 + 2*M01*M01 + 2*M02*M02 +
              M11*M11 + 2*M12*M12 +
              M22*M22);
  rnorm = norm ? 1.0/norm : 1.0;
  M00 *= rnorm;
  M01 *= rnorm;
  M02 *= rnorm;
  M11 *= rnorm;
  M12 *= rnorm;
  M22 *= rnorm;

  /* this code is a mix of prior Teem code and ideas from Eberly's
     "Eigensystems for 3 x 3 Symmetric Matrices (Revisited)" */
  Q = (M01*M01 + M02*M02 + M12*M12 - M00*M11 - M00*M22 - M11*M22)/3.0;
  QQQ = Q*Q*Q;
  R = (M00*M11*M22 + M02*(2*M01*M12 - M02*M11) 
       - M00*M12*M12 - M01*M01*M22)/2.0;
  D = QQQ - R*R;
  if (D > epsilon) {
    /* three distinct roots- this is the most common case */
    double mm, ss, cc;
    theta = atan2(sqrt(D), R)/3.0;
    mm = sqrt(Q);
    ss = sin(theta);
    cc = cos(theta);
    eval[0] = 2*mm*cc;
    eval[1] = mm*(-cc + sqrt(3.0)*ss);
    eval[2] = mm*(-cc - sqrt(3.0)*ss);
    roots = ROOT_THREE;
    /* else D is near enough to zero */
  } else if (R < -epsilon || epsilon < R) {
    double U;
    /* one double root and one single root */
    U = airCbrt(R); /* cube root function */
    if (U > 0) {
      eval[0] = 2*U;
      eval[1] = -U;
      eval[2] = -U;
    } else {
      eval[0] = -U;
      eval[1] = -U;
      eval[2] = 2*U;
    }
    roots = ROOT_SINGLE_DOUBLE;
  } else {
    /* a triple root! */
    eval[0] = eval[1] = eval[2] = 0.0;
    roots = ROOT_TRIPLE;
  }
