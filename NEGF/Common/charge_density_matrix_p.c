/************************** SVN Revision Information **************************
 **    $Id$    **
 ******************************************************************************/

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "main.h"
#include "init_var.h"
#include "LCR.h"
#include "pmo.h"

#define 	LDEBUG 	0

void charge_density_matrix_p (complex double * sigma_all)
{
    int iprobe, jprobe, nprobe;
    int iene;
    int st1, idx_sigma, idx_C, nC_1, *desca;
    complex double ene;
    complex double weight;
    complex double *green_C, *rho_mn;
    complex double *sigma, *gamma;
    double denominator, numerator, dum, sum, *wmn;
    int nC, nL, i, ntot, *sigma_idx, idx_delta, j;
    complex double *green_C_row, *green_C_col;
    int maxrow, maxcol;
    double one, zero;
    int ione =1;
    one =1.0;
    zero =0.0;


    void *RT = BeginRmgTimer("4-ChargeDensityMatrix");

    nL = lcr[1].num_states;
    if (nL != ct.block_dim[0])
    {
        printf (" lcr[1].num_states & ct.block_dim[0] are unequal \n");
    }

    nL = lcr[2].num_states;

    if (nL != ct.block_dim[ct.num_blocks - 1])
    {
        printf (" lcr[2].num_states & ct.block_dim[%d] are unequal \n", ct.num_blocks - 1);
    }

    /*  allocate memory for green_C, grenn_C is tri-diagonal */
    ntot = pmo.ntot;
    my_malloc_init( green_C, pmo.ntot_low, complex double );
    my_malloc_init( sigma_idx, cei.num_probe, int );

    /*   Calculating the equilibrium term eq. 32 of PRB 65, 165401  */


    idx_sigma = 0;
    iprobe = cei.probe_noneq;
    for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
    {
        for (st1 = 0; st1 < ntot; st1++)
        {
            lcr[iprobe].density_matrix_tri[st1] = 0.0;

        }
    }

    void *RT1 = BeginRmgTimer("4-ChargeDensityMatrix: equilibrium");
    for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
    {
        if(cei.probe_noneq > 0)  iprobe = cei.probe_noneq;

        /*   parallel for the processor on energy grid */
        for (iene = pmo.myblacs; iene < lcr[iprobe].nenergy; iene += pmo.npe_energy)
        {

            ene = lcr[iprobe].ene[iene];
            weight = lcr[iprobe].weight[iene];


            /* sigma is a complex matrix with dimension ct.num_states * ct.num_states 
             * it sums over all probes
             * tot, tott, green_tem is also a complex matrix, 
             * their memory should be the maximum of probe dimensions, lcr[1,...].num_states
             */


            for (jprobe = 1; jprobe <= cei.num_probe; jprobe++)
            {
                sigma_idx[jprobe - 1] = idx_sigma;

                idx_C = cei.probe_in_block[jprobe-1];
                idx_sigma += pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C];
            }


            Sgreen_c_p (lcr[0].Htri, lcr[0].Stri, sigma_all, sigma_idx, ene, green_C);


            for (st1 = 0; st1 < ntot; st1++)
            {

                lcr[iprobe].density_matrix_tri[st1] += cimag( weight * green_C[st1]);
            }


        }                       /* end for energy points */

        comm_sums (lcr[iprobe].density_matrix_tri, &ntot, COMM_EN1);


        if(cei.probe_noneq > 0) break;
                if (ct.runflag == 111 | ct.runflag == 112 |ct.runflag ==1121)
                   break;

    }

    my_free( green_C );


    EndRmgTimer(RT1);

    void *RT2 = BeginRmgTimer("4-ChargeDensityMatrix: non-equilibrium");
    /* ======================= Non-equilibrium part ===================== */

    if (ct.runflag == 113)
    {

        maxrow = 0;
        maxcol = 0;
        for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
        {
            idx_C = cei.probe_in_block[iprobe - 1];  /* block index */
            maxrow = rmg_max(maxrow, pmo.mxllda_cond[idx_C]);
            maxcol = rmg_max(maxcol, pmo.mxlocc_cond[idx_C]);
        }

        my_malloc_init( sigma, maxrow * maxcol, complex double ); 
        my_malloc_init( gamma, maxrow * maxcol, complex double ); 


        maxrow = 0;
        maxcol = 0;
        int totrow = 0;
        int totcol = 0;
        for(i = 0; i < ct.num_blocks; i++)
        {
            maxrow = rmg_max(maxrow, pmo.mxllda_cond[i]);
            maxcol = rmg_max(maxcol, pmo.mxlocc_cond[i]);
            totrow += pmo.mxllda_cond[i];
            totcol += pmo.mxlocc_cond[i];
        }

        my_malloc_init( green_C_row, maxcol * totrow, complex double ); 
        my_malloc_init( green_C_col, maxrow * totcol, complex double ); 
        my_malloc_init( rho_mn, ntot, complex double );

        /*   Calculating the non-equilibrium term eq. 33 of PRB 65, 165401  */
        for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
        {
            if(cei.probe_noneq > 0) iprobe = cei.probe_noneq;
            j = 0;	
            for (idx_delta = 1; idx_delta <= cei.num_probe; idx_delta++)
            {
                if(idx_delta != iprobe)
                {

                    for (st1 = 0; st1 < ntot; st1++)
                        lcr[iprobe].lcr_ne[j].density_matrix_ne_tri[st1] = 0.0;

                    /*  parallel for the processor on energy grid */
                    for (iene = pmo.myblacs; iene < lcr[iprobe].lcr_ne[j].nenergy_ne; iene += pmo.npe_energy)
                    {

                        ene = lcr[iprobe].lcr_ne[j].ene_ne[iene];
                        weight = lcr[iprobe].lcr_ne[j].weight_ne[iene];

                        for (jprobe = 1; jprobe <= cei.num_probe; jprobe++)
                        {
                            sigma_idx[jprobe - 1] = idx_sigma;
                            idx_C = cei.probe_in_block[jprobe - 1];  /* block index */  
                            idx_sigma += pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C];
                        }


                        nC = ct.num_states;

                        Sgreen_c_noneq_p (lcr[0].Htri, lcr[0].Stri, sigma_all, sigma_idx, ene,
                                green_C_row, green_C_col, nC, idx_delta);



                        /* sigma is the rhomn in output  */

                        idx_C = cei.probe_in_block[idx_delta - 1];
                        for (st1 = 0; st1 < pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C]; st1++)
                        {
                            sigma[st1] = sigma_all[sigma_idx[idx_delta - 1] + st1];
                        }


                        desca = &pmo.desc_cond[( idx_C + idx_C * ct.num_blocks) * DLEN];   /* nC_1 * nC_1 matrix */
                        nC_1 = ct.block_dim[idx_C];
                        PZTRANC(&nC_1, &nC_1, &one, sigma, &ione, &ione, desca,
                                &zero, gamma, &ione, &ione, desca);
                        for (i = 0; i < pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C]; i++)
                        {
                            gamma[i] = I * (sigma[i] - gamma[i]);
                        }

                        rho_munu (rho_mn, green_C_row, green_C_col, gamma, idx_delta); 


                        for (st1 = 0; st1 < ntot; st1++)
                        {

                            lcr[iprobe].lcr_ne[j].density_matrix_ne_tri[st1] +=
                                creal(weight) * creal(rho_mn[st1]);   /* check -ve sign ??? */

                        }


                    }                   /* end for energy points */

                    comm_sums (lcr[iprobe].lcr_ne[j].density_matrix_ne_tri, &ntot, COMM_EN1);

                    j++;	
                }     /* if statement ends here */	
            }     /* idx_delta loop ends here */

            if(cei.probe_noneq > 0) break;
        }      /* iprobe loop ends here */

        my_free( green_C_row );
        my_free( green_C_col );
        my_free( rho_mn );
        my_free( sigma_idx );

        my_free( sigma );
        my_free( gamma );


        EndRmgTimer(RT2);
        /* ========== Calculation of the density matrix ============= */		


        for (st1 = 0; st1 < ntot; st1++)
            lcr[0].density_matrix_tri[st1] = 0.0;

        my_malloc_init( wmn, cei.num_probe, double );
        for (st1 = 0; st1 < ntot; st1++)
        {

            /* Calculates the denominator part for the weight*/
            denominator = 0.0;
            for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
            {
                for (idx_delta = 1; idx_delta < cei.num_probe; idx_delta++)
                {
                    dum = lcr[iprobe].lcr_ne[idx_delta - 1].density_matrix_ne_tri[st1];
                    dum = dum * dum;
                    denominator += dum;

                }  /* idx_delta loop ends here */
            }   /* iprobe loop ends here    */

            denominator = denominator * (cei.num_probe - 1.0); 


            /* Calculates the numerator part for the weight */
            for (nprobe = 1; nprobe <= cei.num_probe; nprobe++)
            {

                numerator = 0.0;
                for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
                {
                    if(iprobe != nprobe)
                    {
                        for (idx_delta = 1; idx_delta < cei.num_probe; idx_delta++)
                        {
                            dum = lcr[iprobe].lcr_ne[idx_delta - 1].density_matrix_ne_tri[st1];
                            dum = dum * dum;
                            numerator += dum;

                        }  /* idx_delta loop ends here */
                    }	 /* if statement ends here  */
                }   /* iprobe loop ends here    */

                wmn[nprobe - 1] = numerator / denominator;
                /* if(nprobe != 1) wmn[nprobe - 1] = 0.0; */
                if(denominator < 0.0000001) wmn[nprobe - 1] = 1.0 / cei.num_probe;  /* Check it */
            }   /* nprobe loop ends here    */


     //       if(pct.gridpe == 0) 
     //           printf (" \n omega %d %f %f %f %f %f \n", st1, wmn[0], wmn[1], wmn[2], wmn[3], wmn[0]+wmn[1]+wmn[2]+wmn[3]);
      //      printf (" \n omega %d %f %f %f \n", st1, wmn[0], wmn[1], wmn[0]+wmn[1]); 

            /* Finally, calculates density matrix */ 

            for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
            {
                sum = lcr[iprobe].density_matrix_tri[st1];	

                for (idx_delta = 1; idx_delta < cei.num_probe; idx_delta++)
                {

                    sum -= lcr[iprobe].lcr_ne[idx_delta - 1].density_matrix_ne_tri[st1];

                }  
                lcr[0].density_matrix_tri[st1] += wmn[iprobe - 1] * sum;
            }   


        } /* st1 loop ends here */
        my_free( wmn );


        /* =================== Non-equilibrium part ends here ================= */

        //        for (st1 = 0; st1 < ntot; st1++)
        //        {
        //           iprobe = cei.probe_noneq;
        //          sum = lcr[iprobe].density_matrix_tri[st1];	
        //
        //           for (idx_delta = 1; idx_delta < cei.num_probe; idx_delta++)
        //          {
        //
        //               sum -= lcr[iprobe].lcr_ne[idx_delta - 1].density_matrix_ne_tri[st1];
        //
        //           }  
        //          lcr[0].density_matrix_tri[st1] =  sum;
        //     }   
        //
    }     /* if (ct.runflag == 113) statement ends here */ 


    for (st1 = 0; st1 < ntot; st1++)
        lcr[0].density_matrix_tri[st1] *= ct.kp[pct.kstart].kweight /PI ;

    comm_sums (lcr[0].density_matrix_tri, &ntot, pct.kpsub_comm);

    EndRmgTimer(RT);

    if (cei.num_probe > 4)
        error_handler ("probe > 4");


}
