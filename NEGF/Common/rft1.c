/************************** SVN Revision Information **************************
 **    $Id: rft1.c 1242 2011-02-02 18:55:23Z luw $    **
******************************************************************************/
 
/****f* QMD-MGDFT/rft1.c *****
 * NAME
 *   Ab initio real space code with multigrid acceleration
 *   Quantum molecular dynamics package.
 *   Version: 2.1.5
 * COPYRIGHT
 *   Copyright (C) 1995  Emil Briggs
 *   Copyright (C) 1998  Emil Briggs, Charles Brabec, Mark Wensell, 
 *                       Dan Sullivan, Chris Rapcewicz, Jerzy Bernholc
 *   Copyright (C) 2001  Emil Briggs, Wenchang Lu,
 *                       Marco Buongiorno Nardelli,Charles Brabec, 
 *                       Mark Wensell,Dan Sullivan, Chris Rapcewicz,
 *                       Jerzy Bernholc
 * FUNCTION
 *   void rft1(REAL cparm, REAL *f, REAL *r, REAL *ffil, REAL al, int rg_points, 
  *           int lval, REAL dr, REAL width, int lrg_points)
 *   This function is used to filter the high frequencies from a radial function 
 *   defined on a logarithmic grid. It computes a DFT, applies a cutoff function
 *   defined in gcutoff to the transform and then reconstructs the filtered function
 *   on a linear grid. (The linear grid is used for fast interpolation of the
 *   radial function onto a 3-dimensional grid).
 *
 *   The radial function must be short ranged.
 * INPUTS
 *   f: a radial function to be filtered
 *   r: array stored r values
 *   al: log mesh parameter
 *   rg_points: number of radial mesh
 *   lval: momentum l (0=s, 1=p, 2=d)
 *   dr: linear grid space
 *   width: width for G-cutoff
 *   lrg_point: number of linear grid 
 * OUTPUT
 *   ffil:  filtered potential in linear grid
 * PARENTS
 *   init_kbr.c
 * CHILDREN
 *   global_sums.c radint.c
 * SOURCE
 */


#include <float.h>
#include <math.h>
#include <stdlib.h>
#include "md.h"



/* G-vector cutoff function */
REAL gcutoff (REAL g1, REAL gcut, REAL width)
{
    REAL t1;

    if (g1 < gcut)
        return 1.0;

    t1 = (g1 - gcut) / gcut;
    return exp (-width * (t1 * t1));
}




void rft1 (REAL cparm, REAL * f, REAL * r, REAL * ffil, REAL * rab, int rg_points,
           int lval, REAL dr, REAL width, int lrg_points)
{

    int idx, ift, gnum, istep, alloc;
    REAL gmesh, gmax, gcut, t1, t2, *work1, *work2, *gcof, *gvec;
    REAL rfil, rstep;


    /* Get some temporary memory */
    alloc = rg_points;
    if (alloc < lrg_points)
        alloc = lrg_points;
    my_malloc_init( work1, 4 * alloc, REAL );
    work2 = work1 + alloc;
    gcof = work2 + alloc;
    gvec = gcof + alloc;

    for (idx = 0; idx < alloc; idx++)
    {
        work1[idx] = 0.0;
        work2[idx] = 0.0;
        gcof[idx] = 0.0;
        gvec[idx] = 0.0;
    }
    gnum = 1500;

    /* G-vectors are defined on a log grid with the smallest value set */
    /* by the largest real-space value of r.                           */
    gvec[0] = PI / (r[rg_points - 1]);

    /* The largest g-vector that we generate is defined by the global */
    /* grid spacing.                                                  */
    gcut = PI / (cparm * ct.hmaxgrid);
    gmax = PI / r[0];

    gmesh = (log (gmax) - log (gvec[0])) / gnum;
    t1 = exp (gmesh);


    /* Generate g-vectors */
    for (idx = 1; idx < gnum; idx++)
        gvec[idx] = gvec[0] * pow (t1, (REAL) idx);




    /* Loop over frequency components */
    for (ift = 0; ift < gnum; ift++)
    {


        switch (lval)
        {

        case S_STATE:

            for (idx = 0; idx < rg_points; idx++)
            {

                t1 = r[idx] * gvec[ift];
                work1[idx] = f[idx] * sin (t1) / t1;
                t2 = sin (t1) / t1;
                if (t1 < 1.0e-8)
                    t2 = 1.0;
                work1[idx] = f[idx] * t2;

            }

            break;

        case P_STATE:

            for (idx = 0; idx < rg_points; idx++)
            {

                t1 = r[idx] * gvec[ift];
                t2 = cos (t1) / t1 - sin (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;
                work1[idx] = f[idx] * t2;

            }

            break;

        case D_STATE:

            for (idx = 0; idx < rg_points; idx++)
            {

                t1 = r[idx] * gvec[ift];
                t2 = (3.0 / (t1 * t1) - 1.0) * sin (t1) / t1;
                t2 -= 3.0 * cos (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;
                work1[idx] = f[idx] * t2;

            }

            break;

        case F_STATE:

            for (idx = 0; idx < rg_points; idx++)
            {
                t1 = r[idx] * gvec[ift];
                t2 = (15.0 / (t1 * t1) - 6.0) * sin (t1) / (t1 * t1);
                t2 += (1.0 - 15.0 / (t1 * t1)) * cos (t1) / t1;
                if (t1 < 1.0e-8)
                    t2 = 0.0;
                work1[idx] = f[idx] * t2;
            }

            break;

        case G_STATE:

            for (idx = 0; idx < rg_points; idx++)
            {
                t1 = r[idx] * gvec[ift];
                t2 = (105.0 / (t1 * t1 * t1 * t1) - 45.0 / (t1 * t1) + 1.0) * sin (t1) / t1;
                t2 += (10.0 - 105.0 / (t1 * t1)) * cos (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;
                work1[idx] = f[idx] * t2;
            }

            break;

        default:

            error_handler ("angular momentum state not programmed");


        }                       /* end switch */


        /* Get coefficients */
        gcof[ift] = radint1 (work1, r, rab, rg_points);

    }                           /* end for */

    /* Fouerier Filter the transform and store in work2 */
    for (idx = 0; idx < gnum; idx++)
        work2[idx] = gcof[idx] * gcutoff (gvec[idx], gcut, width);



    /* Zero out the array in which the filtered function is to be returned */
    for (idx = 0; idx < lrg_points; idx++)
        ffil[idx] = 0.0;



    /* Now we reconstruct the filtered function */
    istep = lrg_points / NPES;
    t1 = (REAL) istep;
    rstep = t1 * dr;
    t1 = (REAL) pct.thispe;
    rfil = t1 * rstep + 1.0e-10;

    switch (lval)
    {

    case S_STATE:

        for (idx = istep * pct.thispe; idx < istep * pct.thispe + istep; idx++)
        {


            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = sin (t1) / t1;
                if (t1 < 1.0e-8)
                    t2 = 1.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }


            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;

            rfil += dr;

        }


        istep = NPES * istep;
        global_sums (ffil, &istep);

        t1 = (REAL) NPES;
        rfil = t1 * rstep + 1.0e-10;

        for (idx = istep; idx < lrg_points; idx++)
        {


            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = sin (t1) / t1;
                if (t1 < 1.0e-8)
                    t2 = 1.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }


            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;

            rfil += dr;

        }




        break;

    case P_STATE:

        for (idx = istep * pct.thispe; idx < istep * pct.thispe + istep; idx++)
        {


            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = cos (t1) / t1 - sin (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }


            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;

            rfil += dr;

        }


        istep = NPES * istep;
        global_sums (ffil, &istep);

        t1 = (REAL) NPES;
        rfil = t1 * rstep + 1.0e-10;

        for (idx = istep; idx < lrg_points; idx++)
        {

            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = cos (t1) / t1 - sin (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }


            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;
            rfil += dr;

        }

        break;

    case D_STATE:

        for (idx = istep * pct.thispe; idx < istep * pct.thispe + istep; idx++)
        {


            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = (3.0 / (t1 * t1) - 1.0) * sin (t1) / t1;
                t2 -= 3.0 * cos (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }


            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;

            rfil += dr;

        }


        istep = NPES * istep;
        global_sums (ffil, &istep);

        t1 = (REAL) NPES;
        rfil = t1 * rstep + 1.0e-10;

        for (idx = istep; idx < lrg_points; idx++)
        {

            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = (3.0 / (t1 * t1) - 1.0) * sin (t1) / t1;
                t2 -= 3.0 * cos (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }

            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;
            rfil += dr;

        }

        break;

    case F_STATE:

        for (idx = istep * pct.thispe; idx < istep * pct.thispe + istep; idx++)
        {

            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = (15.0 / (t1 * t1) - 6.0) * sin (t1) / (t1 * t1);
                t2 += (1.0 - 15.0 / (t1 * t1)) * cos (t1) / t1;
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }

            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;

            rfil += dr;

        }

        istep = NPES * istep;
        global_sums (ffil, &istep);

        t1 = (REAL) NPES;
        rfil = t1 * rstep + 1e-10;

        for (idx = istep; idx < lrg_points; idx++)
        {

            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = (15.0 / (t1 * t1) - 6.0) * sin (t1) / (t1 * t1);
                t2 += (1.0 - 15.0 / (t1 * t1)) * cos (t1) / t1;
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }

            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;
            rfil += dr;

        }

        break;

    case G_STATE:

        for (idx = istep * pct.thispe; idx < istep * pct.thispe + istep; idx++)
        {

            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = (105.0 / (t1 * t1 * t1 * t1) - 45.0 / (t1 * t1) + 1.0) * sin (t1) / t1;
                t2 += (10.0 - 105.0 / (t1 * t1)) * cos (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }

            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;

            rfil += dr;

        }

        istep = NPES * istep;
        global_sums (ffil, &istep);

        t1 = (REAL) NPES;
        rfil = t1 * rstep + 1e-10;

        for (idx = istep; idx < lrg_points; idx++)
        {

            for (ift = 0; ift < gnum; ift++)
            {

                t1 = rfil * gvec[ift];
                t2 = (105.0 / (t1 * t1 * t1 * t1) - 45.0 / (t1 * t1) + 1.0) * sin (t1) / t1;
                t2 += (10.0 - 105.0 / (t1 * t1)) * cos (t1) / (t1 * t1);
                if (t1 < 1.0e-8)
                    t2 = 0.0;

                /* G-space cutoff */
                work1[ift] = work2[ift] * t2;

            }

            /* Integrate it */
            t1 = radint (work1, gvec, gnum, gmesh);
            ffil[idx] = t1 * 2.0 / PI;
            rfil += dr;

        }

        break;

    default:

        error_handler ("angular momentum state not programmed");


    }                           /* end switch */


    /* Release memory */
    my_free(work1);

}                               /* end rft1 */
