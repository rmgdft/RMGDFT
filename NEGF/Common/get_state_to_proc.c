/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/*


get_state_to_proc.c


*/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "md.h"


void get_state_to_proc (STATE * states)
{
    int st, ion_per_proc, ion1, ion2;
    int st_local;
    int ion_mode;

    assert (ct.num_ions >= NPES);

    my_malloc(state_begin, NPES, int);
    my_malloc(state_end, NPES, int);


    ion_per_proc = ct.num_ions / NPES;
    ion_mode = ct.num_ions - ion_per_proc * NPES;

    if (ion_mode > 0)
    {
        if (pct.gridpe < ion_mode)
        {
            ion1 = (ion_per_proc + 1) * pct.gridpe;
            ion2 = ion1 + ion_per_proc + 1;
        }
        else
        {
            ion1 = ion_per_proc * pct.gridpe + ion_mode;
            ion2 = ion1 + ion_per_proc;
        }
    }
    else
    {

        ion1 = ion_per_proc * pct.gridpe;
        ion2 = ion1 + ion_per_proc;
    }
    assert (ion2 <= ct.num_ions);
    ct.ion_begin = ion1;
    ct.ion_end = ion2;

    ct.state_begin = 1000000;
    ct.state_end = -1;

    for (st = 0; st < ct.num_states; st++)
    {

        state_to_proc[st] = 0;
        if (state_to_ion[st] >= ion1 && state_to_ion[st] < ion2)
        {
            ct.state_begin = min (ct.state_begin, st);
            ct.state_end = max (ct.state_end, st);
            state_to_proc[st] = pct.gridpe;
        }
    }

    global_sums_int (state_to_proc, &ct.num_states);
    for (st = 0; st < ct.num_states; st++)
        states[st].pe = state_to_proc[st];

    for (st = 0; st < ct.num_states; st++)
        states[st].index = st;


    ct.state_end += 1;
    ct.state_per_proc = ct.state_end - ct.state_begin;

    ct.state_per_proc = int_max_all (ct.state_per_proc);



    for (ion1 = ct.ion_begin; ion1 < ct.ion_end; ion1++)
    {
        st_local = 0;
        for (st = ct.state_begin; st < ct.state_end; st++)
        {
            ion2 = state_to_ion[st];
            if (ion1 == ion2)
            {
                states[st].loc_index = st_local;
                st_local += 1;
            }
        }
    }

    for (st = 0; st < NPES; st++)
    {
        state_begin[st] = 0;
        state_end[st] = 0;
    }

    st = pct.gridpe;
    state_begin[st] = ct.state_begin;
    state_end[st] = ct.state_end;

    st = NPES;
    global_sums_int (state_begin, &st);
    global_sums_int (state_end, &st);

}
