
  /* multiply back by eigenvalue L2 norm */
  eval[0] /= rnorm;
  eval[1] /= rnorm;
  eval[2] /= rnorm;

  /* add back in the eigenvalue mean */
  eval[0] += mean;
  eval[1] += mean;
  eval[2] += mean;
