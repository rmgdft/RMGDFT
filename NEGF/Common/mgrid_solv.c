/************************** SVN Revision Information **************************
 **    $Id: mgrid_solv.c 1242 2011-02-02 18:55:23Z luw $    **
******************************************************************************/
 
/****f* QMD-MGDFT/mgrid_solv.c *****
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
 *   void mgrid_solv(REAL *v_mat, REAL *f_mat, REAL *Work, 
 *                   int dimx, int dimy, int dimz,
 *                   REAL gridhx, REAL gridhy, REAL gridhz,
 *                   int level, int *nb_ids, int max_levels, int *pre_cyc,
 *                   int *post_cyc, int mu_cyc, REAL step)
 *	solves the Poisson equation: del^2 v = f for v. 
 *      This routine is called recursively for each level of multi-grid.
 * INPUTS
 *   f_mat[(xdim+2)*(dimy+2)*(dimz+2)]: the charge density (or residual) data
 *   work: workspace, dimensioned at least as large as the above matricies
 *   dimx,dimy,dimz:  size of the data arrays
 *   gridhx:  grid spacing for the x plane
 *   gridhy:  grid spacing for the y plane
 *   gridhz:  grid spacing for the z plane
 *   level:   indicates current level of multigrid
 *   nb_ids:  PE neighbor list
 *   max_levels:  maximum multigrid level
 *   pre_cyc: 
 *   post_cyc: number of post-smoothing
 *   mu_cyc:   number of iteration on the same level
 *   step:  time step
 * OUTPUT
 *   v_mat[(xdim+2)*(dimy+2)*(dimz+2)]: array to contain the solution
 * PARENTS
 *   get_vh.c mg_eig_state.c
 * CHILDREN
 *   trade_images.c solv_pois.c eval_residual.c mg_restrict.c mg_prolong.c solv_pois.c
 * SOURCE
 */


#include <stdio.h>

#include "md.h"


void mgrid_solv (REAL * v_mat, REAL * f_mat, REAL * work,
                 int dimx, int dimy, int dimz,
                 REAL gridhx, REAL gridhy, REAL gridhz,
                 int level, int *nb_ids, int max_levels, int *pre_cyc,
                 int *post_cyc, int mu_cyc, int istate, int *iion, int flag_local)
{
    int i;
    int cycl;
    int size, idx;
    REAL scale;
    int ione = 1;
    int dx2, dy2, dz2, siz2;
    REAL *resid, *newf, *newv, *newwork;

    int ncycl;




/* precalc some boundaries */
    size = (dimx + 2) * (dimy + 2) * (dimz + 2);

    resid = work + 2 * size;

    /*   my_malloc_init( resid, size, REAL ); */

    scale = 2.0 / (gridhx * gridhx * ct.xside * ct.xside);
    scale = scale + (2.0 / (gridhy * gridhy * ct.yside * ct.yside));
    scale = scale + (2.0 / (gridhz * gridhz * ct.zside * ct.zside));
    scale = 1.0 / scale;



    if (flag_local == 1)
    {
        fill_orbit_borders (f_mat, dimx, dimy, dimz);
    }
    else
    {
        trade_images (f_mat, dimx, dimy, dimz, nb_ids);
    }

    for (idx = 0; idx < size; idx++)
    {

        /*   Lu Wenchang 
           v_mat[idx] = - scale * f_mat[idx];
         */

        v_mat[idx] = 0.0;
    }                           /* end for */


/*
 * solve on this grid level 
 */

/*  Lu Wenchang */
    if (level == max_levels)
    {
        ncycl = pre_cyc[level] + post_cyc[level];
    }
    else
    {
        ncycl = pre_cyc[level];
    }                           /* end if */

    for (cycl = 0; cycl < ncycl; cycl++)
    {

        /* solve once */
        solv_pois (v_mat, f_mat, work, dimx, dimy, dimz, gridhx, gridhy, gridhz);


        /* trade boundary info */
        if (flag_local == 1)
        {
            pack_stop (v_mat, work, dimx, dimy, dimz);

            /* Localization the work array */
            app_mask (istate, work, level);

            pack_ptos (v_mat, work, dimx, dimy, dimz);

            fill_orbit_borders (v_mat, dimx, dimy, dimz);
        }
        else
        {
            trade_images (v_mat, dimx, dimy, dimz, nb_ids);
        }
    }



/*
 * on coarsest grid, we are finished
 */

    if (level == max_levels)
    {

        return;

    }                           /* end if */


/* evaluate residual */
    eval_residual (v_mat, f_mat, dimx, dimy, dimz, gridhx, gridhy, gridhz, resid);

    if (flag_local == 1)
    {
        pack_stop (resid, work, dimx, dimy, dimz);

        /* Localization the work array */
        app_mask (istate, work, level);

        pack_ptos (resid, work, dimx, dimy, dimz);

        fill_orbit_borders (resid, dimx, dimy, dimz);
    }
    else
    {
        trade_images (resid, dimx, dimy, dimz, nb_ids);
    }

/* size for next smaller grid */
    dx2 = dimx / 2;
    dy2 = dimy / 2;
    dz2 = dimz / 2;
    siz2 = (dx2 + 2) * (dy2 + 2) * (dz2 + 2);

/* set storage pointers in the current workspace */
    newv = &work[0];
    newf = &work[siz2];
    newwork = &work[2 * siz2];


    for (i = 0; i < mu_cyc; i++)
    {

        mg_restrict (resid, newf, dimx, dimy, dimz);

        if (flag_local == 1)
        {
            pack_stop (newf, newwork, dx2, dy2, dz2);

            /* Localization the work array */
            app_mask (istate, newwork, level + 1);

            pack_ptos (newf, newwork, dx2, dy2, dz2);
            fill_orbit_borders (newf, dx2, dy2, dz2);
        }                       /* end if flag_local */

        /* call mgrid solver on new level */
        mgrid_solv (newv, newf, newwork, dx2, dy2, dz2, gridhx * 2.0,
                    gridhy * 2.0, gridhz * 2.0, level + 1, nb_ids,
                    max_levels, pre_cyc, post_cyc, 1, istate, iion, flag_local);

        if (flag_local == 1)
        {
            pack_stop (newv, newwork, dx2, dy2, dz2);

            /* Localization the work array */
            app_mask (istate, newwork, level + 1);

            pack_ptos (newv, newwork, dx2, dy2, dz2);

            fill_orbit_borders (newv, dx2, dy2, dz2);
        }
        else
        {
            trade_images (newv, dx2, dy2, dz2, nb_ids);
        }

        mg_prolong (resid, newv, dimx, dimy, dimz);

        if (flag_local == 1)
        {
            pack_stop (resid, work, dimx, dimy, dimz);

            /* Localization the work array */
            app_mask (istate, work, level);

            pack_ptos (resid, work, dimx, dimy, dimz);
        }                       /* end if flag_local */


        scale = 1.0;

        QMD_saxpy (size, scale, resid, ione, v_mat, ione);

        /* re-solve on this grid level */

        if (flag_local == 1)
        {
            fill_orbit_borders (v_mat, dimx, dimy, dimz);
        }
        else
        {
            trade_images (v_mat, dimx, dimy, dimz, nb_ids);
        }

        for (cycl = 0; cycl < post_cyc[level]; cycl++)
        {

            /* solve once */
            solv_pois (v_mat, f_mat, work, dimx, dimy, dimz, gridhx, gridhy, gridhz);


            /* trade boundary info */
            if (flag_local == 1)
            {
                pack_stop (v_mat, work, dimx, dimy, dimz);

                /* Localization the work array */
                app_mask (istate, work, level);

                pack_ptos (v_mat, work, dimx, dimy, dimz);

                fill_orbit_borders (v_mat, dimx, dimy, dimz);
            }
            else
            {
                trade_images (v_mat, dimx, dimy, dimz, nb_ids);
            }
        }                       /* end for */

        /* evaluate max residual */
        if (i < (mu_cyc - 1))
        {

            eval_residual (v_mat, f_mat, dimx, dimy, dimz, gridhx, gridhy, gridhz, resid);


            if (flag_local == 1)
            {
                pack_stop (resid, work, dimx, dimy, dimz);

                /* Localization the work array */
                app_mask (istate, work, level);

                pack_ptos (resid, work, dimx, dimy, dimz);

                fill_orbit_borders (resid, dimx, dimy, dimz);
            }
            else
            {
                trade_images (resid, dimx, dimy, dimz, nb_ids);
            }                   /* end if flag_local */

        }                       /* end if */


    }                           /* for mu_cyc */


/* my_free(resid); */
}

/******/
