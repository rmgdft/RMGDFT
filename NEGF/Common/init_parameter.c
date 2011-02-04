/************************** SVN Revision Information **************************
 **    $Id: init_parameter.c 1242 2011-02-02 18:55:23Z luw $    **
******************************************************************************/
 
/****f* QMD-MGDFT/init_parameter.c *****
 * NAME
 *   Ab initio O(n) real space code with 
 *   localized orbitals and multigrid acceleration
 *   Version: 3.0.0
 * COPYRIGHT
 *   Copyright (C) 2001  Wenchang Lu,
 *                       Jerzy Bernholc
 * FUNCTION
 *   void init_parameter()
 *   Basic initialization of some parameters.
 * INPUTS
 *
 * OUTPUT
 *   
 * PARENTS
 *   init.c
 * CHILDREN
 *
 * SOURCE
 */


#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "md.h"

void init_parameter (STATE * states)
{

    int kpt, kst1, state, ion, st1;
    ION *iptr;
    REAL v1, v2, v3;
    int ispin;

    ct.psi_nbasis = NX_GRID * NY_GRID * NZ_GRID;
    ct.psi_nxgrid = NX_GRID;
    ct.psi_nygrid = NY_GRID;
    ct.psi_nzgrid = NZ_GRID;

    /* Set hartree boundary condition stuff */
    ct.vh_pxgrid = FPX0_GRID;
    ct.vh_pygrid = FPY0_GRID;
    ct.vh_pzgrid = FPZ0_GRID;

    ct.vh_pbasis = ct.vh_pxgrid * ct.vh_pygrid * ct.vh_pzgrid;
    if (ct.vh_ext == NULL)
        my_malloc_init( ct.vh_ext, ct.vh_pbasis, REAL );


    ct.vh_nbasis = FNX_GRID * FNY_GRID * FNZ_GRID;
    ct.vh_nxgrid = FNX_GRID;
    ct.vh_nygrid = FNY_GRID;
    ct.vh_nzgrid = FNZ_GRID;


    /* Initialize some k-point stuff */
    for (kpt = 0; kpt < ct.num_kpts; kpt++)
    {

        v1 = 2.0 * PI * ct.kp[kpt].kpt[0] / ct.xside;
        v2 = 2.0 * PI * ct.kp[kpt].kpt[1] / ct.yside;
        v3 = 2.0 * PI * ct.kp[kpt].kpt[2] / ct.zside;

        ct.kp[kpt].kvec[0] = v1;
        ct.kp[kpt].kvec[1] = v2;
        ct.kp[kpt].kvec[2] = v3;

        ct.kp[kpt].kmag = v1 * v1 + v2 * v2 + v3 * v3;

        ct.kp[kpt].kstate = &states[kpt * ct.num_states];
        ct.kp[kpt + ct.spin * ct.num_states].kstate =
            &states[kpt * ct.num_states + ct.spin * ct.num_kpts * ct.num_states];
        ct.kp[kpt].kidx = kpt;

    }                           /* end for */

    /* Count up the total number of electrons */
    ct.ionic_charge = 0.0;
    for (ion = 0; ion < ct.num_ions; ion++)
    {

        iptr = &ct.ions[ion];
        ct.ionic_charge += ct.sp[iptr->species].zvalence;

    }                           /* end for */


    /* Count up the total number of electrons */
    ct.nel = 0.0;
    for (state = 0; state < ct.num_states; state++)
    {

        ct.nel += states[state].occupation;

    }                           /* end for */


    /* calculation the compensating background charge
       for charged supercell calculations */

    ct.nel = ct.background_charge + ct.ionic_charge;

    ct.hmaxgrid = ct.xside * ct.hxgrid;
    if (ct.yside * ct.hygrid > ct.hmaxgrid)
        ct.hmaxgrid = ct.yside * ct.hygrid;
    if (ct.zside * ct.hzgrid > ct.hmaxgrid)
        ct.hmaxgrid = ct.zside * ct.hzgrid;

    ct.hmingrid = ct.xside * ct.hxgrid;
    if (ct.yside * ct.hygrid < ct.hmingrid)
        ct.hmingrid = ct.yside * ct.hygrid;
    if (ct.zside * ct.hzgrid < ct.hmingrid)
        ct.hmingrid = ct.zside * ct.hzgrid;

    ct.anisotropy = ct.hmaxgrid / ct.hmingrid;

    if (ct.anisotropy > 1.1)
        error_handler (" Anisotropy too large");

    /* Set discretization array */
    ct.xcstart = 0.0;
    ct.ycstart = 0.0;
    ct.zcstart = 0.0;


    /* Some multigrid parameters */
    ct.poi_parm.sb_step = 1.0;
    ct.eig_parm.sb_step = 1.0;
    for (ispin = 0; ispin <= ct.spin; ispin++)
    {
        for (kpt = pct.kstart; kpt < pct.kend; kpt++)
        {
            for (st1 = 0; st1 < ct.num_states; st1++)
            {
                kst1 = ispin * ct.num_states * ct.num_kpts + kpt * ct.num_states + st1;
                states[kst1].kidx = kpt;
                states[kst1].istate = st1;
                states[kst1].firstflag = 0;

            }                   /* end for */
        }                       /* end for */
    }                           /* end for ispin */



}                               /* end init */

/******/
