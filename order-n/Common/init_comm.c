/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/*

init_comm.c

(1) we build up a matrix NPES * NPES and see which pairs need communications
    and how many orbits we need send
(2) pick up the smallest number first, build up a sendrecv pairs.
(3) pick up the smallest number again, and build up another sendrecv pairs.
    until there is nothing left.

num_sendrecv_loop: 
send_to[loop * (ct.num_state_per_proc +2) ]:
	for each loop, send_to[loop][0]: the processor rank we send to
	               send_to[loop][1]: number of state we need to send
		       send_to[loop][2, 3, ..]: state index we send
recv_from[loop * (ct.num_state_per_proc +2) ]:
	for each loop, recv_from[loop][0]: the processor rank we recev from
	               recv_from[loop][1]: number of state we need to recv
		       recv_from[loop][2, 3, ..]: state index we recv
		       



*/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "md.h"


void init_comm(STATE * states)
{
    int i, j, proc1, proc2, proc3;
    int communication_pair, state_per_proc, loop;
    int *matrix_pairs, *proc_recv;
    int st1, st2, idx;
    int max_send_states;
    int send_order;
    int recv_order;
    int num_overlap;
    int recv_proc;
	double tem, tem1;

    state_per_proc = ct.state_per_proc + 2;
    my_calloc( matrix_pairs, NPES * NPES, int );
    my_calloc( proc_recv, NPES, int );
    recv_proc = MPI_PROC_NULL;

    if (send_to != NULL)
        my_free(send_to);
    my_calloc( send_to, NPES * state_per_proc, int );

    if (recv_from != NULL)
        my_free(recv_from);
    my_calloc( recv_from, NPES * state_per_proc, int );

    for (loop = 0; loop < NPES; loop++)
    {
        send_to[loop * state_per_proc] = MPI_PROC_NULL;
        send_to[loop * state_per_proc + 1] = 0;
        recv_from[loop * state_per_proc] = MPI_PROC_NULL;
        recv_from[loop * state_per_proc + 1] = 0;
    }


    for (proc1 = 0; proc1 < NPES * NPES; proc1++)
        matrix_pairs[proc1] = 0;

    proc1 = pct.gridpe;
    for (proc2 = 0; proc2 < NPES; proc2++)
    {
	num_overlap = 0;
	for (st1 = ct.state_begin; st1 < ct.state_end; st1++)
		    for (st2 = 0; st2 < ct.num_states; st2++)
			    if ( (states[st2].pe == proc2)
					    && (proc2 != proc1)
					    && (state_overlap_or_not[st1 * ct.num_states + st2] == 1))
			    {
				    num_overlap++;
				    break;
			    }
	    matrix_pairs[proc1 * NPES + proc2] = num_overlap;
    }

 idx = NPES * NPES;
 global_sums_int(matrix_pairs, &idx);


    /*
     *if (pct.gridpe == 0)
     *{
     *    printf("\n initial communication matrix ");
     *    for (i = 0; i < NPES; i++)
     *    {
     *        printf("\n");
     *        for (j = 0; j < NPES; j++)
     *            printf(" %d ", matrix_pairs[i * NPES + j]);
     *    }
     *}
     */

    for (loop = 0; loop < NPES; loop++)
    {
	    for (proc1 = 0; proc1 < NPES; proc1++)
		    proc_recv[proc1] = 0;
	    communication_pair = 0;
	    for (proc1 = 0; proc1 < NPES; proc1++)
	    {
		    max_send_states = 0;
		    for (proc3 = proc1 + 1; proc3 < proc1 + NPES; proc3++)
		    {
			    proc2 = proc3;
			    if (proc2 > NPES - 1)
				    proc2 = proc2 - NPES;
			    if (matrix_pairs[proc1 * NPES + proc2] > max_send_states && proc_recv[proc2] == 0)
			    {
				    max_send_states = matrix_pairs[proc1 * NPES + proc2];
				    recv_proc = proc2;
			    }
		    }

		    if (max_send_states > 0)
		    {
			    proc_recv[recv_proc] = 1;
			    if (pct.gridpe == proc1)
			    {
				    send_to[loop * state_per_proc] = recv_proc;
				    send_to[loop * state_per_proc + 1] = max_send_states;
			    }
			    if (pct.gridpe == recv_proc)
			    {
				    recv_from[loop * state_per_proc] = proc1;
				    recv_from[loop * state_per_proc + 1] = max_send_states;
			    }
			    matrix_pairs[proc1 * NPES + recv_proc] = 0;
			    matrix_pairs[recv_proc * NPES + proc1] = 0;
			    communication_pair++;
		    }
	    }
	    if (communication_pair == 0)
	    {
		    num_sendrecv_loop = loop;
		    loop += NPES;       /* break */
	    }
    }


    for (loop = 0; loop < num_sendrecv_loop; loop++)
    {
	    proc1 = pct.gridpe;
	    proc2 = send_to[loop * state_per_proc];
	    send_order = 0;
	    if (proc2 != MPI_PROC_NULL)
		    for (st1 = ct.state_begin; st1 < ct.state_end; st1++)
		    {
			    for (st2 = 0; st2 < ct.num_states; st2++)
			    {
				    if (states[st2].pe == proc2
						    && state_overlap_or_not[st1 * ct.num_states + st2] == 1)
				    {
					    send_to[loop * state_per_proc + send_order + 2] = st1;
					    send_order++;
					    break;
				    }
			    }
		    }
	    assert(send_order == send_to[loop * state_per_proc + 1]);

	    proc1 = pct.gridpe;
	    proc2 = recv_from[loop * state_per_proc];
	    recv_order = 0;
	    if (proc2 != MPI_PROC_NULL)
		    for (st2 = 0; st2 < ct.num_states; st2++)
		    {
			    if (states[st2].pe == proc2)
				    for (st1 = ct.state_begin; st1 < ct.state_end; st1++)
					    if (state_overlap_or_not[st1 * ct.num_states + st2] == 1)
					    {
						    recv_from[loop * state_per_proc + recv_order + 2] = st2;
						    recv_order++;
						    break;
					    }
		    }
	    assert(recv_order == recv_from[loop * state_per_proc + 1]);
    }

#if 	DEBUG
    for (loop = 0; loop < num_sendrecv_loop; loop++)
    {

	    printf("\nLoop: %d  PE:%d send %d states to PE:%d ---- ", loop,
			    pct.gridpe, send_to[loop * state_per_proc + 1], send_to[loop * state_per_proc]);
	    for (st2 = 0; st2 < send_to[loop * state_per_proc + 1]; st2++)
		    printf("  %d ", send_to[loop * state_per_proc + 2 + st2]);

	    printf("\nLoop: %d  PE:%d receive %d states from PE:%d ---- ", loop,
			    pct.gridpe, recv_from[loop * state_per_proc + 1], recv_from[loop * state_per_proc]);
	    for (st2 = 0; st2 < recv_from[loop * state_per_proc + 1]; st2++)
		    printf("  %d ", recv_from[loop * state_per_proc + 2 + st2]);

    }
#endif

    my_free(matrix_pairs);
    my_free(proc_recv);

}
