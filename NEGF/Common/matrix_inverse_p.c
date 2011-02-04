/************************** SVN Revision Information **************************
 **    $Id: matrix_inverse_p.c 1242 2011-02-02 18:55:23Z luw $    **
******************************************************************************/
 
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "md.h"
#include "pmo.h"


void matrix_inverse_p (doublecomplex * H_tri, doublecomplex * G_tri)
{
/*  Calculate the inverse of a semi-tridiagonal complex matrix
 *
 *    H00   H01   0    0   ...		0
 *    H10   H11   H12  0  ...		0	
 *    0     H21   H22  H23 ....		0
 *    .     .     .    .		.	
 *    .     .     .    .		.
 *    .     .     .    .     Hn-1,n-1	Hn-1,n
 *    0     0     0          Hn,n-1     Hnn
 *
 *   H_tri: store the input matrix in the order of H00, H01, H11, H12, .... Hnn
 *   Hi,i+1  = transpose (Hi+1, i)
 *
 *   G_tri: output inverse matrix in the same order as H_tri
 *   N:   number of blocks
 *   ni:  dimension of each block
 *   for example Hii is a ni[i] x ni[i] matrix
 *               Hi,i+1 is a ni[i] x ni[i+1] matrix 
 */

    int  i, j, n1, n2, n3, n4, n5, n6, n7, n8;
    int *ipiv;
    doublecomplex *Hii, *Gii, *G_tem, *temp, *Hii1, *Gii0;
    doublecomplex mone, one, zero;
    int ione = 1, *ntem_begin;
    int ntot_row;
    int maxrow, maxcol;
    int *desca, *descb, *descc, *descd;
    int *ni, N;

        ni = ct.block_dim;
        N = ct.num_blocks;
    mone.r = -1.0, mone.i = 0.0;
    one.r = 1.0, one.i = 0.0;
    zero.r = 0.0, zero.i = 0.0;

/*  find the maximum dimension of the blocks  */


    ntot_row = 0;
    maxrow=0;
    maxcol=0;
    for (i = 0; i < ct.num_blocks; i++)
    {
        ntot_row += pmo.mxllda_cond[i];
        maxrow = max(maxrow, pmo.mxllda_cond[i]);
        maxcol = max(maxcol, pmo.mxlocc_cond[i]);

    }

    my_malloc_init( ipiv, maxrow + pmo.mblock, int );


    my_malloc_init( ntem_begin, ct.num_blocks, int);
    my_malloc_init( Hii, maxrow * maxcol, doublecomplex );
    my_malloc_init( Gii, maxrow * maxcol, doublecomplex );
    my_malloc_init( temp, maxrow * maxcol, doublecomplex );
    my_malloc_init( G_tem, ntot_row * maxcol, doublecomplex );


    /*
     *  ntem_begin: starting address of one column of G_tem for each block
     */

    ntem_begin[0] = 0;
    for (i = 1; i < ct.num_blocks; i++)
    {
        ntem_begin[i] = ntem_begin[i - 1] + pmo.mxllda_cond[i - 1] * maxcol;
    }

    /*  calculate the inverse of the first block  */

    for (i = 0; i < pmo.mxllda_cond[0] * pmo.mxlocc_cond[0]; i++)
    {
        Hii[i].r = H_tri[i].r;
        Hii[i].i = H_tri[i].i;
    }

    desca = &pmo.desc_cond[0];
    get_inverse_block_p (Hii, Gii, ipiv, desca);

    n1 = pmo.mxllda_cond[0] * pmo.mxlocc_cond[0];
    zcopy (&n1, Gii, &ione, G_tri, &ione);

    /*  iterate to get one more block  */

    for (i = 0; i < N - 1; i++)
    {
        /* get the interaction  Hi,i+1  from input H_tri 
         * Hii1 is a pointer only
         */
        Hii1 = &H_tri[pmo.offdiag_begin[i] ];
        Gii0 = &G_tri[pmo.diag_begin[i] ];

        /* Hii now has the matrix Hi+1,i+1  */

        for (j = 0; j < pmo.mxllda_cond[i+1] * pmo.mxlocc_cond[i + 1]; j++)
        {
            Hii[j].r = H_tri[j + pmo.diag_begin[i+ 1]].r;
            Hii[j].i = H_tri[j + pmo.diag_begin[i+ 1]].i;
        }


        /* calculate Hi+1,i+1 - Hi+1,i * Gii^0 * Hi,i+1  */

        n1 = ni[i + 1];
        n2 = ni[i];

        desca = &pmo.desc_cond[ (i   + (i+1) * ct.num_blocks) * DLEN];
        descb = &pmo.desc_cond[ (i   +     i * ct.num_blocks) * DLEN];
        descc = &pmo.desc_cond[ (i+1 +     i * ct.num_blocks) * DLEN];
        descd = &pmo.desc_cond[ (i+1 + (i+1) * ct.num_blocks) * DLEN];

        PZGEMM ("T", "N", &n1, &n2, &n2, &one, Hii1, &ione, &ione, desca,
                Gii0, &ione, &ione, descb, &zero, temp, &ione, &ione, descc);
        PZGEMM ("N", "N", &n1, &n1, &n2, &mone, temp, &ione, &ione, descc,
                Hii1, &ione, &ione, desca, &one, Hii, &ione, &ione, descd);


        /* now Hii store the matrix Hi+1,i+1 - Hi+1,i * Gii^0 * Hi,i+1
         * Gi+1,i+1, stored in Gii, = Hii^(-1)
         */

        get_inverse_block_p (Hii, Gii, ipiv, descd);

        n1 = pmo.mxllda_cond[i + 1] * pmo.mxlocc_cond[i + 1];
        zcopy (&n1, Gii, &ione, &G_tri[pmo.diag_begin[i + 1]], &ione);

        /*  Gi,i+1 =  Gii^0 * Hi,i+1 * Gi+1,i+1  */

        n1 = ni[i];
        n2 = ni[i + 1];
        n3 = pmo.offdiag_begin[i];

        /* temp = Gii^0 * Hi,i+1 */
        PZGEMM ("N", "N", &n1, &n2, &n1, &mone, Gii0, &ione, &ione, descb,
                Hii1, &ione, &ione, desca, &zero, temp, &ione, &ione, desca);

        /* G(i,i+1) = temp * G(i+1,i+1)  also == G_tem(i,i+1)  */
        PZGEMM ("N", "N", &n1, &n2, &n2, &one, temp, &ione, &ione, desca,
                Gii, &ione, &ione, descd, &zero, &G_tri[n3], &ione, &ione, desca);

        n4 = pmo.mxllda_cond[i] * pmo.mxlocc_cond[i + 1];
        zcopy (&n4, &G_tri[n3], &ione, &G_tem[ntem_begin[i]], &ione);

        /* update Gii  */

        PZGEMM ("N", "T", &n1, &n1, &n2, &one, temp, &ione, &ione, desca,
                &G_tri[n3], &ione, &ione, desca, &one, &G_tri[pmo.diag_begin[i]], &ione, &ione, descb);

        for (j = i - 1; j >= 0; j--)
        {

            n1 = ni[j];         /* dimension of block j */
            n2 = ni[i];         /* dimension of block i */
            n3 = ni[i + 1];     /* dimension of block i+1 */
            n4 = ntem_begin[j]; /* starting address of Gtem(j,*) in G_tem */
            n5 = pmo.diag_begin[j];    /* starting address of G(j,j) in G_tri */
            n6 = pmo.offdiag_begin[j];    /* starting address of G(j,j+1) in G_tri */
            n7 = ntem_begin[j + 1];     /* starting address of Gtem(j+1,*) in G_tem */
            n8 = ni[j + 1];     /* dimension of block j+1 */

            /* temp = -G0(j,i) * Hi,i+1  */
            desca = &pmo.desc_cond[ ( j + i * ct.num_blocks) * DLEN];
            descb = &pmo.desc_cond[ ( i + (i+1) * ct.num_blocks) * DLEN];
            descc = &pmo.desc_cond[ ( j + (i+1) * ct.num_blocks) * DLEN];

            PZGEMM ("N", "N", &n1, &n3, &n2, &mone, &G_tem[n4], &ione, &ione, desca,
                    Hii1, &ione, &ione, descb, &zero, temp, &ione, &ione, descc);

            /* G0(j, i+1) = temp * G(i+1,i+1) */

            desca = &pmo.desc_cond[ ( j + (i+1) * ct.num_blocks) * DLEN];
            descb = &pmo.desc_cond[ ( i+1 + (i+1) * ct.num_blocks) * DLEN];
            PZGEMM ("N", "N", &n1, &n3, &n3, &one, temp, &ione, &ione, desca,
                    Gii, &ione, &ione, descb, &zero, &G_tem[n4], &ione, &ione, desca);

            /* G(j,j) = G0(j,j) + temp * G(i+1,j) */
            desca = &pmo.desc_cond[ ( j + (i+1) * ct.num_blocks) * DLEN];
            descb = &pmo.desc_cond[ ( j + j * ct.num_blocks) * DLEN];
            PZGEMM ("N", "T", &n1, &n1, &n3, &one, temp, &ione, &ione, desca, 
                    &G_tem[n4], &ione, &ione, desca, &one, &G_tri[n5], &ione, &ione, descb);

            /* G(j,j+1) = G0(j,j+1) + temp * G(i+1,j+1)  */
            desca = &pmo.desc_cond[ ( j + (i+1) * ct.num_blocks) * DLEN];
            descb = &pmo.desc_cond[ ( j+1 + (i+1) * ct.num_blocks) * DLEN];
            descc = &pmo.desc_cond[ ( j + (j+1) * ct.num_blocks) * DLEN];
            PZGEMM ("N", "T", &n1, &n8, &n3, &one, temp, &ione, &ione, desca, 
                    &G_tem[n7], &ione, &ione, descb, &one, &G_tri[n6],
                    &ione, &ione, descc);
        }                       /* end for (j--) */

    }                           /* end  for(i = 0; i < N-1; i++) */


    my_free( ntem_begin );
    my_free( ipiv );
    my_free( Hii );
    my_free( Gii );
    my_free( temp );
    my_free( G_tem );
}
