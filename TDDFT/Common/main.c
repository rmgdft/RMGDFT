/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/****f* QMD-MGDFT/md.c *****
 * NAME
 *   Ab initio O(n) real space code with 
 *   localized orbitals and multigrid acceleration
 *   Version: 3.0.0
 * COPYRIGHT
 *   Copyright (C) 2001  Wenchang Lu,
 *                       Jerzy Bernholc
 * FUNCTION
 *   int main(int argc, char **argv)
 *   Main program
 *   Read-in all informations, structures, pseudopentials, etc. 
 *   Then enters the main driver loop. 
 * INPUTS
 *   when we run it, we need to give the input control 
 *   file name in the first argument
 *   for example, md in.diamond8
 * OUTPUT
 *   nothing
 * PARENTS
 *   This is grand-grand-....
 * CHILDREN
 *   run.c
 * SEE ALSO
 *   main.h for structure defination
 * SOURCE
 */

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "prototypes_on.h"
#include "init_var.h"
#include "svnrev.h"



/* Main control structure which is declared extern in main.h so any module */
/* may access it.					                 */
CONTROL ct;

/* PE control structure which is also declared extern in main.h */
PE_CONTROL pct;


void eldyn_(int *num_states, double *, double *, double *, double *, int *, int*);

int main(int argc, char **argv)
{



    time_t tt;
    char *timeptr;

    double *Hmatrix, *Smatrix, *Xij_00, *Yij_00, *Zij_00;
    double *Xij_dist, *Yij_dist, *Zij_dist;
    
    int Ieldyn=1, iprint = 0;
    int n2, numst,i;
    double *Pn0, *Pn1;
    double dipole_ion[3], dipole_ele[3];
    int IA=1, JA=1, IB=1, JB=1;

    time(&tt);
    timeptr = ctime(&tt);
    ct.time0 = my_crtc();

    ct.images_per_node = 1;
    init_IO(argc, argv);


    my_barrier();



    /*  Begin to do the real calculations */
    init_TDDFT();

    numst = ct.num_states;

    get_HS(states, states1, vtot_c, Hij_00, Bij_00);

    Cpdgemr2d(numst, numst, Hij_00, IA, JA, pct.descb, Hij, IB, JB,
            pct.desca, pct.desca[1]);
    Cpdgemr2d(numst, numst, Bij_00, IA, JA, pct.descb, matB, IB, JB,
            pct.desca, pct.desca[1]);


    get_cholesky_real(matB);
    get_dm_diag_p(states, l_s, mat_X, Hij);

    write_eigs(states);

    n2 = (ct.state_end - ct.state_begin) * ct.num_states;
    my_malloc( Xij_00, n2, double );
    my_malloc( Yij_00, n2, double );
    my_malloc( Zij_00, n2, double );


    n2 = MXLLDA * MXLCOL;
    my_malloc( Xij_dist, n2, double );
    my_malloc( Yij_dist, n2, double );
    my_malloc( Zij_dist, n2, double );

    get_phi_xyz_phi(states, Xij_00, Yij_00, Zij_00);

    Cpdgemr2d(numst, numst, Xij_00, IA, JA, pct.descb, Xij_dist, IB, JB,
            pct.desca, pct.desca[1]);
    Cpdgemr2d(numst, numst, Yij_00, IA, JA, pct.descb, Yij_dist, IB, JB,
            pct.desca, pct.desca[1]);
    Cpdgemr2d(numst, numst, Zij_00, IA, JA, pct.descb, Zij_dist, IB, JB,
            pct.desca, pct.desca[1]);

    n2 = numst * numst;
    my_malloc( Pn0, 2*n2, double );
    my_malloc( Pn1, 2*n2, double );
    my_malloc( Hmatrix, n2, double );
    my_malloc( Smatrix, n2, double );
    




    /*  matB: overlap matrix, Hij:  Hamiltonian matrix  distributed in
     *  scalapack way*/

    dipole_calculation(rhoc, dipole_ion);
    dipole_calculation(rho, dipole_ele);


    printf("\n  x dipolll  %f %f", dipole_ion[0], dipole_ele[0]);
    printf("\n  y dipolll  %f %f", dipole_ion[1], dipole_ele[1]);
    printf("\n  z dipolll  %f %f", dipole_ion[2], dipole_ele[2]);






    //for(i = 0; i < n2; i++) mat_X[i]= Pn1[i];
    //update_TDDFT(mat_X);
    //  get_dm_diag_p(states, l_s, mat_X, Hij);

    //  write_eigs(states);
    /*  Xij = <phi|x|phi>, Yij = <phi|y|phi>, Zij = <phi|z|phi>  */ 

    mat_dist_to_global(mat_X, Pn0, pct.desca);
    mat_dist_to_global(matB, Smatrix, pct.desca);

    for(i = 0; i < n2; i++) Pn0[i+n2] = 0.0;

    double time_step = 0.20;
    double efield = 0.001;
    double fs= 0.02418884;  // 1fs = 0.02418884 *10^-15 second 
    FILE *dfi;
    dfi = fopen("dipole.dat.ykick", "w+");
    for(ct.scf_steps = 0; ct.scf_steps < ct.max_scf_steps; ct.scf_steps++)
    {

        if(ct.scf_steps > 0) 
        {
            efield = 0.0;
        }

        for(i = 0; i < MXLLDA * MXLCOL; i++) Hij[i] = time_step*Hij[i] + efield * Yij_dist[i];

        mat_dist_to_global(Hij, Hmatrix, pct.desca);

        eldyn_(&numst, Smatrix, Hmatrix, Pn0, Pn1, &Ieldyn, &iprint);

        //for(i = 0; i < n2; i++) mat_X[i]= Pn1[i];
        mat_global_to_dist(Pn1, mat_X, pct.desca);
        for(i = 0; i < 2*n2; i++) Pn0[i]= Pn1[i];
        update_TDDFT(mat_X);

        get_HS(states, states1, vtot_c, Hij_00, Bij_00);
        Cpdgemr2d(numst, numst, Hij_00, IA, JA, pct.descb, Hij, IB, JB,
            pct.desca, pct.desca[1]);
        dipole_calculation(rho, dipole_ele);

        dipole_ele[0] -= dipole_ion[0];
        dipole_ele[1] -= dipole_ion[1];
        dipole_ele[2] -= dipole_ion[2];


        fprintf(dfi, "\n  %f  %18.10f  %18.10f  %18.10f ",
                ct.scf_steps*time_step, dipole_ele[0], dipole_ele[1], dipole_ele[2]);
        // get_dm_diag_p(states, l_s, mat_X, Hij);
        // write_eigs(states);


    }

    //    my_free(Xij);
    //    my_free(Yij);
    //    my_free(Zij);

    MPI_Finalize();

    return 0;
}


