/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/

#include "main.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

static void app_cir_sixth_global (REAL * a, REAL * b);



// Compilers can generate much better code if they know the loop dimensions at compile
// as opposed to run time. Therefore since most of the finite difference stencils
// are applied at the global level we check at the top level to see if the grid
// dimensions correpsond to the global case. If so we call a routine with those
// dimensions set at compile time. If not we just fall through to the general case.

void app_cir_sixth (REAL * a, REAL * b, int dimx, int dimy, int dimz)
{

    int ix, iy, iz, numgrid;
    int ixs, iys, ixms, ixps, iyms, iyps;
    int incy, incx, ixmms, ixpps, iymms, iypps;
    int incyr, incxr;
    REAL *rptr;
    REAL c000, c100, c110, c200;

    numgrid = dimx * dimy * dimz;
    if(numgrid == P0_BASIS) {
        app_cir_sixth_global (a, b);
        return;
    }

    incx = (dimz + 4) * (dimy + 4);
    incy = dimz + 4;
    incxr = dimz * dimy;
    incyr = dimz;

    my_malloc (rptr, (dimx + 4) * (dimy + 4) * (dimz + 4), REAL);

    /*trade_images2(a, SSptr); */
    trade_imagesx (a, rptr, dimx, dimy, dimz, 2);

    c000 = 61.0 / 120.0;
    c100 = 13.0 / 180.0;
    c110 = 1.0 / 144.0;
    c200 = -1.0 / 240.0;
    for (ix = 2; ix < dimx + 2; ix++)
    {
        ixs = ix * incx;
        ixms = (ix - 1) * incx;
        ixps = (ix + 1) * incx;
        ixmms = (ix - 2) * incx;
        ixpps = (ix + 2) * incx;

        for (iy = 2; iy < dimy + 2; iy++)
        {
            iys = iy * incy;
            iyms = (iy - 1) * incy;
            iyps = (iy + 1) * incy;
            iymms = (iy - 2) * incy;
            iypps = (iy + 2) * incy;

            for (iz = 2; iz < dimz + 2; iz++)
            {

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] =
                    c100 * (rptr[ixs + iys + (iz - 1)] +
                            rptr[ixs + iys + (iz + 1)] +
                            rptr[ixms + iys + iz] +
                            rptr[ixps + iys + iz] +
                            rptr[ixs + iyms + iz] +
                            rptr[ixs + iyps + iz]) + c000 * rptr[ixs + iys + iz];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    c110 * (rptr[ixps + iyps + iz] +
                            rptr[ixps + iyms + iz] +
                            rptr[ixms + iyps + iz] +
                            rptr[ixms + iyms + iz] +
                            rptr[ixps + iys + (iz + 1)] +
                            rptr[ixps + iys + (iz - 1)] +
                            rptr[ixms + iys + (iz + 1)] +
                            rptr[ixms + iys + (iz - 1)] +
                            rptr[ixs + iyps + (iz + 1)] +
                            rptr[ixs + iyps + (iz - 1)] +
                            rptr[ixs + iyms + (iz + 1)] + rptr[ixs + iyms + (iz - 1)]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    c200 * (rptr[ixs + iys + (iz - 2)] +
                            rptr[ixs + iys + (iz + 2)] +
                            rptr[ixmms + iys + iz] +
                            rptr[ixpps + iys + iz] +
                            rptr[ixs + iymms + iz] + rptr[ixs + iypps + iz]);

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */

    my_free (rptr);
}


void app_cir_sixth_global (REAL * a, REAL * b)
{

    int ix, iy, iz;
    int ixs, iys, ixms, ixps, iyms, iyps;
    int incy, incx, ixmms, ixpps, iymms, iypps;
    int incyr, incxr;
    REAL *rptr;
    REAL c000, c100, c110, c200;

    incx = (PZ0_GRID + 4) * (PY0_GRID + 4);
    incy = PZ0_GRID + 4;
    incxr = PZ0_GRID * PY0_GRID;
    incyr = PZ0_GRID;

    my_malloc (rptr, (PX0_GRID + 4) * (PY0_GRID + 4) * (PZ0_GRID + 4), REAL);

    /*trade_images2(a, SSptr); */
    trade_imagesx (a, rptr, PX0_GRID, PY0_GRID, PZ0_GRID, 2);

    c000 = 61.0 / 120.0;
    c100 = 13.0 / 180.0;
    c110 = 1.0 / 144.0;
    c200 = -1.0 / 240.0;
    for (ix = 2; ix < PX0_GRID + 2; ix++)
    {
        ixs = ix * incx;
        ixms = (ix - 1) * incx;
        ixps = (ix + 1) * incx;
        ixmms = (ix - 2) * incx;
        ixpps = (ix + 2) * incx;

        for (iy = 2; iy < PY0_GRID + 2; iy++)
        {
            iys = iy * incy;
            iyms = (iy - 1) * incy;
            iyps = (iy + 1) * incy;
            iymms = (iy - 2) * incy;
            iypps = (iy + 2) * incy;

            for (iz = 2; iz < PZ0_GRID + 2; iz++)
            {

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] =
                    c100 * (rptr[ixs + iys + (iz - 1)] +
                            rptr[ixs + iys + (iz + 1)] +
                            rptr[ixms + iys + iz] +
                            rptr[ixps + iys + iz] +
                            rptr[ixs + iyms + iz] +
                            rptr[ixs + iyps + iz]) + c000 * rptr[ixs + iys + iz];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    c110 * (rptr[ixps + iyps + iz] +
                            rptr[ixps + iyms + iz] +
                            rptr[ixms + iyps + iz] +
                            rptr[ixms + iyms + iz] +
                            rptr[ixps + iys + (iz + 1)] +
                            rptr[ixps + iys + (iz - 1)] +
                            rptr[ixms + iys + (iz + 1)] +
                            rptr[ixms + iys + (iz - 1)] +
                            rptr[ixs + iyps + (iz + 1)] +
                            rptr[ixs + iyps + (iz - 1)] +
                            rptr[ixs + iyms + (iz + 1)] + rptr[ixs + iyms + (iz - 1)]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    c200 * (rptr[ixs + iys + (iz - 2)] +
                            rptr[ixs + iys + (iz + 2)] +
                            rptr[ixmms + iys + iz] +
                            rptr[ixpps + iys + iz] +
                            rptr[ixs + iymms + iz] + rptr[ixs + iypps + iz]);

            }                   /* end for */

        }                       /* end for */

    }                           /* end for */

    my_free (rptr);
}
