/************************** SVN Revision Information **************************
 **    $Id: 
******************************************************************************/

/* calculating Hamiltonian matrix Hij and 
 * overlap matrix matB togather
 */

 
#include <float.h>
#include <stdio.h>
#include <assert.h>
#include "md.h"



#include "my_scalapack.h"



void get_HS(STATE * states, STATE * states1, double *vtot_c, double *Aij, double *Bij)
{
    int idx, st1, st2, idx1, idx2;
    int maxst, n2;
    STATE *sp;
    int ione = 1;
    REAL tem;
    int ixx, iyy, izz;

    int IA=1, JA=1, IB=1, JB=1, numst = ct.num_states;

    maxst = ct.num_states;

    distribute_to_global(vtot_c, vtot_global);

    for (st1 = 0; st1 < (ct.state_end-ct.state_begin) * ct.num_states; st1++)
    {
        Hij_00[st1] = 0.;
        Bij_00[st1] = 0.;
    }

    /* Loop over states */
    /* calculate the H |phi> on this processor and stored in states1[].psiR[] */

    for (st1 = ct.state_begin; st1 < ct.state_end; st1++)
    {
        sp = &states[st1];
        ixx = states[st1].ixmax - states[st1].ixmin + 1;
        iyy = states[st1].iymax - states[st1].iymin + 1;
        izz = states[st1].izmax - states[st1].izmin + 1;

        /* Generate 2*V*psi and store it  in orbit_tem */
        genvlocpsi(states[st1].psiR, st1, states1[st1].psiR, vtot_global, states);


        /* A operating on psi stored in orbit_tem */

        /* Eighth-order finite-differenital method for Laplatian operating on psi stored in orbit_tem */
        app10_del2(sp->psiR, orbit_tem, ixx, iyy, izz, ct.hxgrid, ct.hygrid, ct.hzgrid);

        /*  A |psi > + 0.5 (B V|psi> + V B |psi>) */

        for (idx = 0; idx < ixx * iyy * izz; idx++)
        {
            states1[st1].psiR[idx] = 0.5 * states1[st1].psiR[idx] - 0.5 * orbit_tem[idx];

        }                       
    }                           /* end for st1 = .. */

    /* print_sum(pct.psi_size, states1[ct.state_begin].psiR, "states1 sum get_Hij");
     * print_state_sum(states1); 
     */

    /* calculate the < states.psiR | states1.psiR>  */

    my_barrier();

    orbit_dot_orbit(states, states1, Hij_00, Bij_00);


    my_barrier();

    get_all_kbpsi(states, states, 0);

    get_Hvnlij(Hij_00);


        /** Add sum_n,m,I(q_n,m * <phi|beta_n,I> * <beta_m,I|phi>) **/
    get_matB_qnm(Bij_00);          /* shuchun wang */

    n2 = (ct.state_end-ct.state_begin) * ct.num_states;
    sscal (&n2, &ct.vel, Hij_00, &ione);
    sscal (&n2, &ct.vel, Bij_00, &ione);

    Cpdgemr2d(numst, numst, Hij_00, IA, JA, pct.descb, Aij, IB, JB,
            pct.desca, pct.desca[1]);
    Cpdgemr2d(numst, numst, Bij_00, IA, JA, pct.descb, Bij, IB, JB,
            pct.desca, pct.desca[1]);


    if (pct.gridpe == 0)
    {
        printf(" matrix Hij\n");
        print_matrix(Hij_00, 5, maxst);
        printf(" matrix Bij\n");
        print_matrix(Bij_00, 5, maxst);
    }


}
