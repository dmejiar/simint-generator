#ifndef SIMINT_SHELL_H
#define SIMINT_SHELL_H

#define MAX_CONT 50
#define MAX_CONT2 (MAX_CONT*MAX_CONT)

struct gaussian_shell
{
  int am;
  double x, y, z;
  int nprim;
  double * alpha;
  double * coef;
};

struct shell_pair
{
  int n, n1, n2;
  int am1, am2;

  double * x;
  double * y;
  double * z;
  double * alpha;
  double * prefac;
};

#endif
