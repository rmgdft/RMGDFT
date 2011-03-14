/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/*


distribute_to_global_soft.c

from distributed array FPX0_BASIS * FPY0_BASIS * FPZ0_BASIS
get  global array  FNX_GRID * FNY_GRID * FNZ_GRID

*/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "md.h"


void global_to_distribute_soft(REAL * global_array, REAL * distr_array)
{

    int ix, iy, iz, ii, jj, kk;
    int idx2, idx1, incx, incx1, incy, incy1;

    incx = FPY0_GRID * FPZ0_GRID;
    incy = FPZ0_GRID;
    incx1 = FNY_GRID * FNZ_GRID;
    incy1 = FNZ_GRID;

    pe2xyz(pct.gridpe, &ii, &jj, &kk);
    ii *= FPX0_GRID;
    jj *= FPY0_GRID;
    kk *= FPZ0_GRID;

    idx1 = FNX_GRID * FNY_GRID * FNZ_GRID;

    for (ix = 0; ix < FPX0_GRID; ix++)
        for (iy = 0; iy < FPY0_GRID; iy++)
            for (iz = 0; iz < FPZ0_GRID; iz++)
            {
                idx1 = ix * incx + iy * incy + iz;
                idx2 = (ix + ii) * incx1 + (iy + jj) * incy1 + iz + kk;
                distr_array[idx1] = global_array[idx2];
            }

}
