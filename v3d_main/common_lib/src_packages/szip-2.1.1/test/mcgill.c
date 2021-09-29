#include "mcgill.h"
/*************************************************************************

             M c G i l l -- S U P E R   D U P E R

               Random number generation package
            -------------------------------------
          G. Marsaglia, K. Ananthanarayanan, N. Paul

         Incorparating the Ziggurat metho of sampling
    from decreasing or symmetric unimodal density functions
                  G. Marsaglia, W. W. Tsang

           Rewritten into C for the AT&T PC 6300 by
                         E. Schneider

*************************************************************************/

#define MULT 69069L

#define MASK32 0xFFFFFFFFUL

static unsigned long mcgn, srgn;

void
rstart(i1, i2)
long i1, i2;
{
	mcgn = (i1 == 0) ? 0 : i1 | 1;
	mcgn &= MASK32;

	srgn = (i2 == 0) ? 0 : (i2 & 0x7ff) | 1;
	srgn &= MASK32;
}

long
iuni()
{
	register unsigned long r0, r1;

	r0 = (srgn >> 15);
	r1 = srgn ^ r0;
	r0 = (r1 << 17);
	r0 &= MASK32;
	srgn = r0 ^ r1;
	mcgn = MULT * mcgn;
	mcgn &= MASK32;
	r1 = mcgn ^ srgn;
	return ((r1 >> 1));
}

long
ivni()
{
	register unsigned long r0, r1;

	r0 = (srgn >> 15);
	r1 = srgn ^ r0;
	r0 = (r1 << 17);
	r0 &= MASK32;
	srgn = r0 ^ r1;
	mcgn = MULT * mcgn;
	mcgn &= MASK32;
	r1 = mcgn ^ srgn;
	return (r1);
}

double
uni()
{
	register unsigned long r0, r1;

	r0 = (srgn >> 15);
	r1 = srgn ^ r0;
	r0 = (r1 << 17);
	r0 &= 0xFFFFFFFFUL;
	srgn = r0 ^ r1;
	mcgn = MULT * mcgn;
	mcgn &= 0xFFFFFFFFUL;
	r1 = mcgn ^ srgn;
	return ((double)(r1 >> 1) / 2147483648.);
}

double
vni()
{
	register unsigned long r0, r1;

	r0 = (srgn >> 15);
	r1 = srgn ^ r0;
	r0 = (r1 << 17);
	r0 &= 0xFFFFFFFFUL;
	srgn = r0 ^ r1;
	mcgn = MULT * mcgn;
	mcgn &= 0xFFFFFFFFUL;
	r1 = mcgn ^ srgn;
	return ((double)(r1) / 2147483648.);
}
