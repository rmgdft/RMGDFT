/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <complex.h>

#include "main.h"
#include "init_var.h"
#include "LCR.h"
#include "pmo.h"


void sigma_all_energy_point (complex double * sigma_all, double kvecy, double kvecz)
{
    int iprobe, jprobe, idx_delta, j;
    int iene;
    int st1, n2;
    complex double *sigma, *g;          

    complex double *ch0, *ch01, *ch10;
    complex double *S00, *H00, *H10, *S10, *H01, *S01, *HCL, *SCL;
    complex double ene, ctem;

    int idx_sigma, idx_C;
    int  maxrow, maxcol, maxrow2, maxcol2;
    int max_sigma_col, max_sigma_row;
    int t1, t2;
    complex double one = 1.0, zero = 0.0;
    int ione = 1;
    int *desca, *descb, numst, numstC;



    maxrow =0;
    maxcol =0;
    for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
    {
        idx_C = cei.probe_in_block[iprobe - 1];  /* block index */
        maxrow = rmg_max( maxrow, pmo.mxllda_cond[idx_C]);
        maxcol = rmg_max( maxcol, pmo.mxlocc_lead[iprobe-1]);
    }


    my_malloc_init( g, maxrow * maxcol, complex double);
    my_malloc_init( ch0, maxrow * maxcol, complex double);
    my_malloc_init( ch01, maxrow * maxcol, complex double);
    my_malloc_init( ch10, maxrow * maxcol, complex double);
    my_malloc_init( S00, maxrow * maxcol,  complex double);
    my_malloc_init( H00, maxrow * maxcol,  complex double);
    my_malloc_init( S10, maxrow * maxcol,  complex double);
    my_malloc_init( S01, maxrow * maxcol,  complex double);
    my_malloc_init( H01, maxrow * maxcol,  complex double);
    my_malloc_init( H10, maxrow * maxcol,  complex double);
    my_malloc_init( SCL, maxrow * maxcol,  complex double);
    my_malloc_init( HCL, maxrow * maxcol,  complex double);

    max_sigma_col = 0;
    max_sigma_row = 0;
    for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
    {
        idx_C = cei.probe_in_block[iprobe - 1];  /* block index */
        max_sigma_row = rmg_max(max_sigma_row, pmo.mxllda_cond[idx_C]);
        max_sigma_col = rmg_max(max_sigma_col, pmo.mxlocc_cond[idx_C]);
    }

    my_malloc_init( sigma, max_sigma_row * max_sigma_col, complex double);

    /************************/
    int idx, i;



    /*  Calculating the equilibrium term eq. 32 of PRB 65, 165401  */

    idx_sigma = 0;
    for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
    {

        if(cei.probe_noneq > 0) iprobe = cei.probe_noneq;
        /*   parallel for the processor on energy grid */
        for (iene = pmo.myblacs; iene < lcr[iprobe].nenergy; iene += pmo.npe_energy)
        {

            ene = lcr[iprobe].ene[iene];


            /* sigma is a complex matrix with dimension ct.num_states * ct.num_states 
             * it sums over all probes
             * tot, tott,  is also a complex matrix, 
             * their memory should be the maximum of probe dimensions, lcr[1,...].num_states
             */


            for (jprobe = 1; jprobe <= cei.num_probe; jprobe++)
            {

                matrix_kpoint_lead(S00, H00, S01, H01, SCL, HCL,  kvecy, kvecz, jprobe);
                desca = &pmo.desc_lead[ (jprobe-1) * DLEN];

                numst = lcr[jprobe].num_states;

                PZTRANC(&numst, &numst, &one, S01, &ione, &ione, desca,
                        &zero, S10, &ione, &ione, desca);
                PZTRANC(&numst, &numst, &one, H01, &ione, &ione, desca,
                        &zero, H10, &ione, &ione, desca);


                idx = pmo.mxllda_lead[jprobe-1] * pmo.mxlocc_lead[jprobe-1];
                for (i = 0; i < idx; i++)
                {
                    ch0[i]  = ene * S00[i] - Ha_eV * H00[i];
                    ch01[i] = ene * S01[i] - Ha_eV * H01[i];
                    ch10[i] = ene * S10[i] - Ha_eV * H10[i];
                }

                //#if GPU_ENABLED
                //                    Sgreen_cuda (g, ch0, ch01, ch10, jprobe);

                //#else

                if (cimag(ene) >0.5 )
                {
                    Sgreen_semi_infinite_p (g, ch0, ch01, ch10, jprobe);
                }
                else
                {

                    green_lead(ch0, ch01, ch10, g, jprobe);

                }
                //#endif


                idx_C = cei.probe_in_block[jprobe - 1];  /* block index */
                idx = pmo.mxllda_cond[idx_C] * pmo.mxlocc_lead[jprobe-1];
                for (i = 0; i < idx; i++)
                {

                    ch01[i] = ene * SCL[i] - Ha_eV * HCL[i];
                }
                desca = &pmo.desc_cond_lead[ (idx_C + (jprobe-1) * ct.num_blocks) * DLEN];
                descb = &pmo.desc_lead_cond[ (idx_C + (jprobe-1) * ct.num_blocks) * DLEN];
                numst = lcr[jprobe].num_states;
                numstC = ct.block_dim[idx_C];


                PZTRANC(&numst, &numstC, &one, SCL, &ione, &ione, desca,
                        &zero, S10, &ione, &ione, descb);
                PZTRANC(&numst, &numstC, &one, HCL, &ione, &ione, desca,
                        &zero, H10, &ione, &ione, descb);
                idx = pmo.mxllda_lead[jprobe -1] * pmo.mxlocc_cond[idx_C];
                for (i = 0; i < idx; i++)
                {
                    ch10[i] = ene * S10[i] - Ha_eV * H10[i];
                }

                Sigma_p (sigma, ch0, ch01, ch10, g, jprobe);

                /*-------------------------------------------------------------------*/

                idx_C = cei.probe_in_block[jprobe - 1];  /* block index */
                for (st1 = 0; st1 < pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C]; st1++)
                {
                    sigma_all[idx_sigma + st1] = sigma[st1];
                }
                idx_sigma += pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C];


            }                   /* end for jprobe */  

        }                       /* end for energy points */

        if(cei.probe_noneq > 0) break;

    }                           /* end for iprobe */


    /*  Calculating the non-equilibrium term eq. 33 of PRB 65, 165401  */

    for (iprobe = 1; iprobe <= cei.num_probe; iprobe++)
    {

        if(cei.probe_noneq >0) iprobe = cei.probe_noneq;
        j = 0;
        for (idx_delta = 1; idx_delta <= cei.num_probe; idx_delta++)
        {
            if(idx_delta != iprobe)
            {

                /*  parallel for the processor on energy grid */
                for (iene = pmo.myblacs; iene < lcr[iprobe].lcr_ne[j].nenergy_ne; iene += pmo.npe_energy)
                {

                    ene = lcr[iprobe].lcr_ne[j].ene_ne[iene];


                    /* sigma is a complex matrix with dimension ct.num_states * ct.num_states 
                     * it sums over all probes
                     * tot, tott,  is also a complex matrix, 
                     * their memory should be the maximum of probe dimensions, lcr[1,...].num_states
                     */


                    for (jprobe = 1; jprobe <= cei.num_probe; jprobe++)
                    {

                        matrix_kpoint_lead(S00, H00, S01, H01, SCL, HCL,  kvecy, kvecz, jprobe);
                        desca = &pmo.desc_lead[ (jprobe-1) * DLEN];

                        numst = lcr[jprobe].num_states;

                        PZTRANC(&numst, &numst, &one, S01, &ione, &ione, desca,
                                &zero, S10, &ione, &ione, desca);
                        PZTRANC(&numst, &numst, &one, H01, &ione, &ione, desca,
                                &zero, H10, &ione, &ione, desca);


                        idx = pmo.mxllda_lead[jprobe-1] * pmo.mxlocc_lead[jprobe-1];
                        for (i = 0; i < idx; i++)
                        {
                            ch0[i] =  ene * S00[i] - Ha_eV * H00[i];
                            ch01[i] = ene * S01[i] - Ha_eV * H01[i];
                            ch10[i] = ene * S10[i] - Ha_eV * H10[i];
                        }


                        green_lead(ch0, ch01, ch10, g, jprobe);


                        idx_C = cei.probe_in_block[jprobe - 1];  /* block index */
                        idx = pmo.mxllda_cond[idx_C] * pmo.mxlocc_lead[jprobe-1];
                        for (i = 0; i < idx; i++)
                        {
                            ch01[i] = ene * SCL[i] - Ha_eV * HCL[i];
                        }
                        desca = &pmo.desc_cond_lead[ (idx_C + (jprobe-1) * ct.num_blocks) * DLEN];
                        descb = &pmo.desc_lead_cond[ (idx_C + (jprobe-1) * ct.num_blocks) * DLEN];
                        numst = lcr[jprobe].num_states;
                        numstC = ct.block_dim[idx_C];


                        PZTRANC(&numst, &numstC, &one, SCL, &ione, &ione, desca,
                                &zero, S10, &ione, &ione, descb);
                        PZTRANC(&numst, &numstC, &one, HCL, &ione, &ione, desca,
                                &zero, H10, &ione, &ione, descb);
                        idx = pmo.mxllda_lead[jprobe -1] * pmo.mxlocc_cond[idx_C];
                        for (i = 0; i < idx; i++)
                        {
                            ch10[i] = ene * S10[i] - Ha_eV * H10[i];
                        }

                        Sigma_p (sigma, ch0, ch01, ch10, g, jprobe);


                        idx_C = cei.probe_in_block[jprobe - 1];  /* block index */
                        for (st1 = 0; st1 < pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C]; st1++)
                        {
                            sigma_all[idx_sigma + st1] = sigma[st1];
                        }
                        idx_sigma += pmo.mxllda_cond[idx_C] * pmo.mxlocc_cond[idx_C];


                    }                         /* jprobe loop ends here */

                }                      /* energy loop ends here */

                j++;
            }            /* if statement ends here */

        }               /* idx_delta loop ends here */

        if(cei.probe_noneq >0) break;

    }        /* iprobe loop ends here */




    my_free(g);
    my_free(sigma);
    my_free(ch0);
    my_free(ch01);
    my_free(ch10);
    my_free(S10);
    my_free(S01);
    my_free(H10);
    my_free(H01);
    my_free(SCL);
    my_free(HCL);
    my_free(S00);
    my_free(H00);

}
