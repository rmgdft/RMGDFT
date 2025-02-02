/*
 *
 * Copyright (c) 1995,2011,2014 Emil Briggs
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
*/

#include <cmath>
#include <complex>
#include <type_traits>
#include "Lattice.h"
#include "FiniteDiff.h"
#include "Gpufuncs.h"
#include "LaplacianCoeff.h"
#include "RmgTimer.h"
#include "rmg_error.h"

// Force instantiation of float, double and complex versions.
template double FiniteDiff::app_cil_sixth<float>(float *, float *, int, int, int, double, double, double);
template double FiniteDiff::app_cil_sixth<double>(double *, double *, int, int, int, double, double, double);
template double FiniteDiff::app_cil_sixth<std::complex<float> >(std::complex <float> *, std::complex <float> *, int, int, int, double, double, double);
template double FiniteDiff::app_cil_sixth<std::complex<double> >(std::complex <double> *, std::complex <double> *, int, int, int, double, double, double);

template void FiniteDiff::app_cir_sixth<float>(float *, float *, int, int, int);
template void FiniteDiff::app_cir_sixth<double>(double *, double *, int, int, int);
template void FiniteDiff::app_cir_sixth<std::complex<float> >(std::complex <float> *, std::complex <float> *, int, int, int);
template void FiniteDiff::app_cir_sixth<std::complex<double> >(std::complex <double> *, std::complex <double> *, int, int, int);


template double FiniteDiff::app_cil_fourth<float>(float *, float *, int, int, int, double, double, double);
template double FiniteDiff::app_cil_fourth<double>(double *, double *, int, int, int, double, double, double);
template double FiniteDiff::app_cil_fourth<std::complex<float> >(std::complex<float> *, std::complex<float> *, int, int, int, double, double, double);
template double FiniteDiff::app_cil_fourth<std::complex<double> >(std::complex<double> *, std::complex<double> *, int, int, int, double, double, double);

template void FiniteDiff::app_cir_fourth<float>(float *, float *, int, int, int);
template void FiniteDiff::app_cir_fourth<double>(double *, double *, int, int, int);
template void FiniteDiff::app_cir_fourth<std::complex<float> >(std::complex<float> *, std::complex<float> *, int, int, int);
template void FiniteDiff::app_cir_fourth<std::complex<double> >(std::complex<double> *, std::complex<double> *, int, int, int);


template <typename RmgType>
double FiniteDiff::app_cil_sixth (RmgType *rptr, RmgType *b, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz)
{

    RmgTimer RT("App_cil: computation");
    int iz, ix, iy, incx, incy, incxr, incyr, ibrav;
    int ixs, iys, ixms, ixps, iyms, iyps, ixmms, ixpps, iymms, iypps;
    double ihx, ihy, ihz;
    RmgType rz, rzms, rzps, rzpps;
    RmgType rfc1, rbc1, rbc2, rd1, rd2, rd3, rd4;
    RmgType td1, td2, td3, td4, td5, td6, td7, td8, tdx;
    double c0 = -116.0 / 90.0;
    double c1 = 31.0 / 232.0;
    double c2 = 49.0 / 60.0;
    double c3 = -31.0 / 464.0;
    double c4 = 1.0 / 10.0;
    double c5 = 1.0 / 120.0;
    double c6 = -1.0 / 240.0;
    double c7 = 1.0 / 144.0;

    ibrav = L->get_ibrav_type();

    incx = (dimz + 4) * (dimy + 4);
    incy = dimz + 4;
    incxr = dimz * dimy;
    incyr = dimz;

    ihx = 1.0 / (gridhx * gridhx * L->get_xside() * L->get_xside());
    ihy = 1.0 / (gridhy * gridhy * L->get_yside() * L->get_yside());
    ihz = 1.0 / (gridhz * gridhz * L->get_zside() * L->get_zside());

    double ccd (c0 * (ihx + ihy + ihz));
    RmgType cc (std::real(c0 * (ihx + ihy + ihz)));
    RmgType fcx ( std::real(c1 * ccd + c2 * ihx));
    RmgType fcy ( std::real(c1 * ccd + c2 * ihy));
    RmgType fcz ( std::real(c1 * ccd + c2 * ihz));

    RmgType ecxy ( std::real(c3 * ccd - c4 * ihz));
    RmgType ecxz ( std::real(c3 * ccd - c4 * ihy));
    RmgType ecyz ( std::real(c3 * ccd - c4 * ihx));

    RmgType cor ( std::real(c7 * (ihx + ihy + ihz)));

    RmgType fc2x ( std::real(c5 * (ihy + ihz)));
    RmgType fc2y ( std::real(c5 * (ihx + ihz)));
    RmgType fc2z ( std::real(c5 * (ihx + ihy)));

    RmgType tcx ( std::real(c6 * ihx));
    RmgType tcy ( std::real(c6 * ihy));
    RmgType tcz ( std::real(c6 * ihz));

    // Handle the general case first
    if((dimz % 4) || (ibrav != CUBIC_PRIMITIVE)) {

        for (ix = 2; ix < dimx + 2; ix++)
        {
            ixs = ix * incx;
            ixms = (ix - 1) * incx;
            ixps = (ix + 1) * incx;
            ixmms = (ix - 2) * incx;
            ixpps = (ix + 2) * incx;

            for (iy = 2; iy < dimy + 2; iy++)
            {
                iys = iy * incy;
                iyms = (iy - 1) * incy;
                iyps = (iy + 1) * incy;
                iymms = (iy - 2) * incy;
                iypps = (iy + 2) * incy;

                for (iz = 2; iz < dimz + 2; iz++)
                {

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] = cc * rptr[ixs + iys + iz] +
                        fcx * (rptr[ixms + iys + iz] + rptr[ixps + iys + iz]) +
                        fcy * (rptr[ixs + iyms + iz] + rptr[ixs + iyps + iz]) +
                        fcz * (rptr[ixs + iys + (iz - 1)] + rptr[ixs + iys + (iz + 1)]);

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                        ecxz * (rptr[ixms + iys + (iz - 1)] + rptr[ixps + iys + (iz - 1)] +
                                rptr[ixms + iys + (iz + 1)] + rptr[ixps + iys + (iz + 1)]) +
                        ecyz * (rptr[ixs + iyms + (iz - 1)] + rptr[ixs + iyps + (iz - 1)] +
                                rptr[ixs + iyms + (iz + 1)] + rptr[ixs + iyps + (iz + 1)]) +
                        ecxy * (rptr[ixms + iyms + iz] + rptr[ixms + iyps + iz] +
                                rptr[ixps + iyms + iz] + rptr[ixps + iyps + iz]);

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                        cor * (rptr[ixms + iyms + (iz - 1)] + rptr[ixps + iyms + (iz - 1)] +
                               rptr[ixms + iyps + (iz - 1)] + rptr[ixps + iyps + (iz - 1)] +
                               rptr[ixms + iyms + (iz + 1)] + rptr[ixps + iyms + (iz + 1)] +
                               rptr[ixms + iyps + (iz + 1)] + rptr[ixps + iyps + (iz + 1)]);

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                        fc2x * (rptr[ixmms + iys + iz] + rptr[ixpps + iys + iz]) +
                        fc2y * (rptr[ixs + iymms + iz] + rptr[ixs + iypps + iz]) +
                        fc2z * (rptr[ixs + iys + (iz - 2)] + rptr[ixs + iys + (iz + 2)]);

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                        tcx * (rptr[ixps + iypps + iz] + rptr[ixps + iymms + iz] +
                               rptr[ixms + iypps + iz] + rptr[ixms + iymms + iz] +
                               rptr[ixps + iys + (iz + 2)] + rptr[ixps + iys + (iz - 2)] +
                               rptr[ixms + iys + (iz + 2)] + rptr[ixms + iys + (iz - 2)]) +
                        tcy * (rptr[ixpps + iyps + iz] + rptr[ixmms + iyps + iz] +
                               rptr[ixpps + iyms + iz] + rptr[ixmms + iyms + iz] +
                               rptr[ixs + iyps + (iz + 2)] + rptr[ixs + iyps + (iz - 2)] +
                               rptr[ixs + iyms + (iz + 2)] + rptr[ixs + iyms + (iz - 2)]) +
                        tcz * (rptr[ixpps + iys + (iz + 1)] + rptr[ixmms + iys + (iz + 1)] +
                               rptr[ixpps + iys + (iz - 1)] + rptr[ixmms + iys + (iz - 1)] +
                               rptr[ixs + iypps + (iz + 1)] + rptr[ixs + iymms + (iz + 1)] +
                               rptr[ixs + iypps + (iz - 1)] + rptr[ixs + iymms + (iz - 1)]);

                }                   /* end for */

            }                       /* end for */

        }                           /* end for */

        return (double)std::real(cc);

    }

    // Optimized case for dimz divisible by 4 and cubic primitive grid

    for (ix = 2; ix < dimx + 2; ix++)
    {
        ixs = ix * incx;
        ixms = (ix - 1) * incx;
        ixps = (ix + 1) * incx;
        ixmms = (ix - 2) * incx;
        ixpps = (ix + 2) * incx;

        for (iy = 2; iy < dimy + 2; iy++)
        {
            iys = iy * incy;
            iyms = (iy - 1) * incy;
            iyps = (iy + 1) * incy;
            iymms = (iy - 2) * incy;
            iypps = (iy + 2) * incy;

            // Compute the middle set of edges (2nd nn) before entry to the loop
            rz = rptr[ixms + iys + 2] +
                 rptr[ixps + iys + 2] +
                 rptr[ixs + iyms + 2] +
                 rptr[ixs + iyps + 2];

            // Compute the trailing set of edges (2nd nn) before entry to the  loop
            rzms = rptr[ixps + iys + 1] +
                   rptr[ixms + iys + 1] +
                   rptr[ixs + iyps + 1] +
                   rptr[ixs + iyms + 1];

            // Compute the 4 nn with same z value as loop index on entry
            rfc1 = rptr[ixms + iyms + 2] + rptr[ixms + iyps + 2] +
                   rptr[ixps + iyms + 2] + rptr[ixps + iyps + 2];

            // Compute a pair trailing sets of corners on entry
            rbc1 = rptr[ixms + iyms + 1] + rptr[ixps + iyms + 1] +
                   rptr[ixms + iyps + 1] + rptr[ixps + iyps + 1];
            rbc2 = rptr[ixms + iyms + 2] + rptr[ixps + iyms + 2] +
                   rptr[ixms + iyps + 2] + rptr[ixps + iyps + 2];

            rd1 = rptr[ixpps + iys + 1] + rptr[ixmms + iys + 1] +
                  rptr[ixs + iypps + 1] + rptr[ixs + iymms + 1];

            rd4 = rptr[ixpps + iys + 2] + rptr[ixmms + iys + 2] +
                  rptr[ixs + iypps + 2] + rptr[ixs + iymms + 2];


            td2 =   rptr[ixps + iys] +
                           rptr[ixms + iys] +
                           rptr[ixs + iyps] +
                           rptr[ixs + iyms];

            td4 =   rptr[ixps + iys + 1] +
                           rptr[ixms + iys + 1] +
                           rptr[ixs + iyps + 1] +
                           rptr[ixs + iyms + 1];

            td6 =   rptr[ixps + iys + 2] +
                           rptr[ixms + iys + 2] +
                           rptr[ixs + iyps + 2] +
                           rptr[ixs + iyms + 2];

            td8 =   rptr[ixps + iys + 3] +
                           rptr[ixms + iys + 3] +
                           rptr[ixs + iyps + 3] +
                           rptr[ixs + iyms + 3];

            td1 =   rptr[ixps + iys + 4] +
                           rptr[ixms + iys + 4] +
                           rptr[ixs + iyps + 4] +
                           rptr[ixs + iyms + 4];

            td3 =   rptr[ixps + iys + 5] +
                           rptr[ixms + iys + 5] +
                           rptr[ixs + iyps + 5] +
                           rptr[ixs + iyms + 5];


            td5 =   rptr[ixps + iys + 6] +
                           rptr[ixms + iys + 6] +
                           rptr[ixs + iyps + 6] +
                           rptr[ixs + iyms + 6];


            td7 =   rptr[ixps + iys + 7] +
                           rptr[ixms + iys + 7] +
                           rptr[ixs + iyps + 7] +
                           rptr[ixs + iyms + 7];



            for (iz = 2; iz < dimz + 2; iz+=4)
            {

                tdx =   rptr[ixps + iys + iz + 6] +
                        rptr[ixms + iys + iz + 6] +
                        rptr[ixs + iyps + iz + 6] +
                        rptr[ixs + iyms + iz + 6];

                // Forward set of edges
                rzps = rptr[ixps + iys + iz + 1] +
                       rptr[ixms + iys + iz + 1] +
                       rptr[ixs + iyps + iz + 1] +
                       rptr[ixs + iyms + iz + 1];

                // First
                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] = cc * rptr[ixs + iys + iz] +
                    fcx * rz +
                    fcx * (rptr[ixs + iys + iz - 1] + rptr[ixs + iys + iz + 1]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    ecxz * rzms + ecxz * rzps + ecxz * rfc1;

                // Compute the forward set of corners
                rfc1 = rptr[ixms + iyms + iz + 1] + rptr[ixps + iyms + iz + 1] +
                       rptr[ixms + iyps + iz + 1] + rptr[ixps + iyps + iz + 1];

                rd3 = rptr[ixpps + iys + iz + 1] + rptr[ixmms + iys + iz + 1] +
                          rptr[ixs + iypps + iz + 1] + rptr[ixs + iymms + iz + 1];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    cor * rbc1 + cor * rfc1;
                rbc1 = rfc1;

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    fc2x * (rptr[ixmms + iys + iz] + rptr[ixpps + iys + iz] +
                            rptr[ixs + iymms + iz] + rptr[ixs + iypps + iz]) +
                    fc2x * (rptr[ixs + iys + iz - 2] + rptr[ixs + iys + iz + 2]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    tcx * rptr[ixps + iypps + iz] + tcx * rptr[ixps + iymms + iz] +
                    tcx * rptr[ixms + iypps + iz] + tcx * rptr[ixms + iymms + iz] +
                    tcx * rptr[ixpps + iyps + iz] + tcx * rptr[ixmms + iyps + iz] +
                    tcx * rptr[ixpps + iyms + iz] + tcx * rptr[ixmms + iyms + iz] +
                    tcx * rd1 +
                    tcx * rd3 +
                    tcx * (td1 + td2);

                td2 = td1;
                td1 = tdx;
                tdx =   rptr[ixps + iys + iz + 7] +
                        rptr[ixms + iys + iz + 7] +
                        rptr[ixs + iyps + iz + 7] +
                        rptr[ixs + iyms + iz + 7];

                // Compute another set of forward edges here
                rzpps = rptr[ixps + iys + iz + 2] +
                        rptr[ixms + iys + iz + 2] +
                        rptr[ixs + iyps + iz + 2] +
                        rptr[ixs + iyms + iz + 2];

                // Second
                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] = cc * rptr[ixs + iys + iz + 1] +
                    fcx * rzps +
                    fcx * (rptr[ixs + iys + iz] + rptr[ixs + iys + iz + 2]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] +=
                    ecxz * rz +  ecxz * rzpps +  ecxz * rfc1;

                // Another set of forward corners here
                rfc1 = rptr[ixms + iyms + iz + 2] + rptr[ixps + iyms + iz + 2] +
                       rptr[ixms + iyps + iz + 2] + rptr[ixps + iyps + iz + 2];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] +=
                    cor * rbc2 + cor * rfc1;
                rbc2 = rfc1;

                rd2 = rptr[ixpps + iys + iz + 2] + rptr[ixmms + iys + iz + 2] +
                      rptr[ixs + iypps + iz + 2] + rptr[ixs + iymms + iz + 2];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] +=
                    fc2x * (rptr[ixmms + iys + iz + 1] + rptr[ixpps + iys + iz + 1] +
                            rptr[ixs + iymms + iz + 1] + rptr[ixs + iypps + iz + 1]) +
                    fc2x *  (rptr[ixs + iys + iz - 1] + rptr[ixs + iys + iz + 3]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] +=
                    tcx * rptr[ixps + iypps + iz + 1] + tcx * rptr[ixps + iymms + iz + 1] +
                    tcx * rptr[ixms + iypps + iz + 1] + tcx * rptr[ixms + iymms + iz + 1] +
                    tcx * rptr[ixpps + iyps + iz + 1] + tcx * rptr[ixmms + iyps + iz + 1] +
                    tcx * rptr[ixpps + iyms + iz + 1] + tcx * rptr[ixmms + iyms + iz + 1] +
                    tcx * rd4 +
                    tcx * rd2 +
                    tcx * (td3 + td4);

                td4 = td3;
                td3 = tdx;
                tdx =   rptr[ixps + iys + iz + 8] +
                           rptr[ixms + iys + iz + 8] +
                           rptr[ixs + iyps + iz + 8] +
                           rptr[ixs + iyms + iz + 8];

                // Compute the forward set of edges here which becomes the trailing set at the top
                rzms = rptr[ixms + iys + iz+3] +
                       rptr[ixps + iys + iz+3] +
                       rptr[ixs + iyms + iz+3] +
                       rptr[ixs + iyps + iz+3];

                // Third
                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz)] = cc * rptr[ixs + iys + iz + 2] +
                    fcx * rzpps +
                    fcx * (rptr[ixs + iys + iz + 1] + rptr[ixs + iys + iz + 3]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz)] +=
                    ecxz * rzps + ecxz * rzms + ecxz * rfc1;

                // Another set of forward corners here
                rfc1 = rptr[ixms + iyms + iz + 3] + rptr[ixps + iyms + iz + 3] +
                       rptr[ixms + iyps + iz + 3] + rptr[ixps + iyps + iz + 3];

                rd1 = rptr[ixpps + iys + iz + 3] + rptr[ixmms + iys + iz + 3] +
                      rptr[ixs + iypps + iz + 3] + rptr[ixs + iymms + iz + 3];

                b[(ix - 2) * incxr + (iy - 2) * incyr + iz] +=
                    cor * rbc1 + cor * rfc1;
                rbc1 = rfc1;

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz)] +=
                    fc2x * (rptr[ixmms + iys + iz + 2] + rptr[ixpps + iys + iz + 2] +
                            rptr[ixs + iymms + iz + 2] + rptr[ixs + iypps + iz + 2]) +
                    fc2x *  (rptr[ixs + iys + iz] + rptr[ixs + iys + iz + 4]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz)] +=
                    tcx * rptr[ixps + iypps + iz + 2] + tcx * rptr[ixps + iymms + iz + 2] +
                    tcx * rptr[ixms + iypps + iz + 2] + tcx * rptr[ixms + iymms + iz + 2] +
                    tcx * rptr[ixpps + iyps + iz + 2] + tcx * rptr[ixmms + iyps + iz + 2] +
                    tcx * rptr[ixpps + iyms + iz + 2] + tcx * rptr[ixmms + iyms + iz + 2] +
                    tcx * rd3 +
                    tcx * rd1 +
                    tcx * (td5 + td6);

                td6 = td5;
                td5 = tdx;
                tdx =   rptr[ixps + iys + iz + 9] +
                        rptr[ixms + iys + iz + 9] +
                        rptr[ixs + iyps + iz + 9] +
                        rptr[ixs + iyms + iz + 9];

                // Compute the forward set of edges here which becomes the middle set at the top
                rz = rptr[ixps + iys + iz + 4] +
                     rptr[ixms + iys + iz + 4] +
                     rptr[ixs + iyps + iz + 4] +
                     rptr[ixs + iyms + iz + 4];

                // Fourth
                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz + 1)] = cc * rptr[ixs + iys + iz + 3] +
                    fcx * rzms +
                    fcx * (rptr[ixs + iys + iz + 2] + rptr[ixs + iys + iz + 4]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz + 1)] +=
                    ecxz * rzpps + ecxz * rz + ecxz * rfc1;

                // Another set of forward corners here
                rfc1 = rptr[ixms + iyms + iz + 4] + rptr[ixps + iyms + iz + 4] +
                       rptr[ixms + iyps + iz + 4] + rptr[ixps + iyps + iz + 4];

                rd4 = rptr[ixpps + iys + iz + 4] + rptr[ixmms + iys + iz + 4] +
                      rptr[ixs + iypps + iz + 4] + rptr[ixs + iymms + iz + 4];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz + 1)] +=
                    cor * rbc2 + cor * rfc1;
                rbc2 = rfc1;

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz + 1)] +=
                    fc2x * (rptr[ixmms + iys + iz + 3] + rptr[ixpps + iys + iz + 3] +
                            rptr[ixs + iymms + iz + 3] + rptr[ixs + iypps + iz + 3]) +
                    fc2x * (rptr[ixs + iys + iz + 1] + rptr[ixs + iys + iz + 5]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz + 1)] +=
                    tcx * rptr[ixps + iypps + iz + 3] + tcx * rptr[ixps + iymms + iz + 3] +
                    tcx * rptr[ixms + iypps + iz + 3] + tcx * rptr[ixms + iymms + iz + 3] +
                    tcx * rptr[ixpps + iyps + iz + 3] + tcx * rptr[ixmms + iyps + iz + 3] +
                    tcx * rptr[ixpps + iyms + iz + 3] + tcx * rptr[ixmms + iyms + iz + 3] +
                    tcx * rd2 +
                    tcx * rd4 +
                    tcx * (td7 + td8);

                td8 = td7;
                td7 = tdx;



            }                   /* end for */

        }                       /* end for */

    }                           /* end for */


    return (double)std::real(cc);

}



template <typename RmgType>
void FiniteDiff::app_cir_sixth (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz)
{

    RmgTimer RT("App_cir: computation");
    int ix, iy, iz;
    int ixs, iys, ixms, ixps, iyms, iyps;
    int incy, incx, ixmms, ixpps, iymms, iypps;
    int incyr, incxr;
    RmgType rz, rzps, rzms, rzpps;
    RmgType c000, c100, c110, c200;

    incx = (dimz + 4) * (dimy + 4);
    incy = dimz + 4;
    incxr = dimz * dimy;
    incyr = dimz;

    c000 = 61.0 / 120.0;
    c100 = 13.0 / 180.0;
    c110 = 1.0 / 144.0;
    c200 = -1.0 / 240.0;

    // Handle the general case first
    if(dimz % 4) {

        for (ix = 2; ix < dimx + 2; ix++)
        {
            ixs = ix * incx;
            ixms = (ix - 1) * incx;
            ixps = (ix + 1) * incx;
            ixmms = (ix - 2) * incx;
            ixpps = (ix + 2) * incx;

            for (iy = 2; iy < dimy + 2; iy++)
            {
                iys = iy * incy;
                iyms = (iy - 1) * incy;
                iyps = (iy + 1) * incy;
                iymms = (iy - 2) * incy;
                iypps = (iy + 2) * incy;

                for (iz = 2; iz < dimz + 2; iz++)
                {

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] =
                        c100 * (rptr[ixs + iys + (iz - 1)] +
                                rptr[ixs + iys + (iz + 1)] +
                                rptr[ixms + iys + iz] +
                                rptr[ixps + iys + iz] +
                                rptr[ixs + iyms + iz] +
                                rptr[ixs + iyps + iz]) + c000 * rptr[ixs + iys + iz];

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                        c110 * (rptr[ixps + iyps + iz] +
                                rptr[ixps + iyms + iz] +
                                rptr[ixms + iyps + iz] +
                                rptr[ixms + iyms + iz] +
                                rptr[ixps + iys + (iz + 1)] +
                                rptr[ixps + iys + (iz - 1)] +
                                rptr[ixms + iys + (iz + 1)] +
                                rptr[ixms + iys + (iz - 1)] +
                                rptr[ixs + iyps + (iz + 1)] +
                                rptr[ixs + iyps + (iz - 1)] +
                                rptr[ixs + iyms + (iz + 1)] + rptr[ixs + iyms + (iz - 1)]);

                    b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                        c200 * (rptr[ixs + iys + (iz - 2)] +
                                rptr[ixs + iys + (iz + 2)] +
                                rptr[ixmms + iys + iz] +
                                rptr[ixpps + iys + iz] +
                                rptr[ixs + iymms + iz] + rptr[ixs + iypps + iz]);

                }                   /* end for */

            }                       /* end for */

        }                           /* end for */

        return;
    }


    // Optimized case for dimz divisible by 4
    for (ix = 2; ix < dimx + 2; ix++)
    {
        ixs = ix * incx;
        ixms = (ix - 1) * incx;
        ixps = (ix + 1) * incx;
        ixmms = (ix - 2) * incx;
        ixpps = (ix + 2) * incx;

        for (iy = 2; iy < dimy + 2; iy++)
        {
            iys = iy * incy;
            iyms = (iy - 1) * incy;
            iyps = (iy + 1) * incy;
            iymms = (iy - 2) * incy;
            iypps = (iy + 2) * incy;

            // Compute the middle set of edges before entry to the loop
            rz = rptr[ixms + iys + 2] +
                 rptr[ixps + iys + 2] +
                 rptr[ixs + iyms + 2] +
                 rptr[ixs + iyps + 2];

            // Compute the trailing set of edges before entry to the  loop
            rzms = rptr[ixps + iys + 1] +
                   rptr[ixms + iys + 1] +
                   rptr[ixs + iyps + 1] +
                   rptr[ixs + iyms + 1];


            for (iz = 2; iz < dimz + 2; iz+=4)
            {

                // Forward set of edges
                rzps = rptr[ixps + iys + iz + 1] +
                       rptr[ixms + iys + iz + 1] +
                       rptr[ixs + iyps + iz + 1] +
                       rptr[ixs + iyms + iz + 1];

                // First
                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] =
                    c100 * (rptr[ixs + iys + iz - 1] +
                            rptr[ixs + iys + iz + 1] +
                            rz) + c000 * rptr[ixs + iys + iz];


                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    c110 * (rptr[ixps + iyps + iz] +
                            rptr[ixps + iyms + iz] +
                            rptr[ixms + iyps + iz] +
                            rptr[ixms + iyms + iz] +
                            rzms +
                            rzps);

                // Compute another set of forward edges here
                rzpps = rptr[ixps + iys + iz + 2] +
                        rptr[ixms + iys + iz + 2] +
                        rptr[ixs + iyps + iz + 2] +
                        rptr[ixs + iyms + iz + 2];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] +=
                    c200 * (rptr[ixs + iys + iz - 2] +
                            rptr[ixs + iys + iz + 2]) +
                    c200 * (rptr[ixmms + iys + iz] +
                            rptr[ixpps + iys + iz] +
                            rptr[ixs + iymms + iz] + 
                            rptr[ixs + iypps + iz]);


                // Second
                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] =
                    c100 * (rptr[ixs + iys + (iz)] +
                            rptr[ixs + iys + iz + 2] +
                            rzps) + 
                            c000 * rptr[ixs + iys + iz + 1];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] +=
                    c110 * (rptr[ixps + iyps + iz+1] +
                            rptr[ixps + iyms + iz+1] +
                            rptr[ixms + iyps + iz+1] +
                            rptr[ixms + iyms + iz+1] +
                            rzpps +
                            rz);

                // Compute the forward set of edges here which becomes the trailing set at the top
                rzms = rptr[ixms + iys + iz+3] +
                       rptr[ixps + iys + iz+3] +
                       rptr[ixs + iyms + iz+3] +
                       rptr[ixs + iyps + iz+3];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 1)] +=
                    c200 * (rptr[ixs + iys + iz - 1] +
                            rptr[ixs + iys + iz + 3]) +
                    c200 * (rptr[ixmms + iys + iz+1] +
                            rptr[ixpps + iys + iz+1] +
                            rptr[ixs + iymms + iz+1] + 
                            rptr[ixs + iypps + iz+1]);

                // Third
                b[(ix - 2) * incxr + (iy - 2) * incyr + iz] =
                    c100 * (rptr[ixs + iys + iz+1] +
                            rptr[ixs + iys + iz + 3] +
                            rzpps) +
                            c000 * rptr[ixs + iys + iz + 2];

                b[(ix - 2) * incxr + (iy - 2) * incyr + iz] +=
                    c110 * (rptr[ixps + iyps + iz+2] +
                            rptr[ixps + iyms + iz+2] +
                            rptr[ixms + iyps + iz+2] +
                            rptr[ixms + iyms + iz+2] +
                            rzms +
                            rzps);

                // Compute the forward set of edges here which becomes the middle set at the top
                rz = rptr[ixps + iys + iz + 4] +
                     rptr[ixms + iys + iz + 4] +
                     rptr[ixs + iyps + iz + 4] +
                     rptr[ixs + iyms + iz + 4];

                b[(ix - 2) * incxr + (iy - 2) * incyr + iz] +=
                    c200 * (rptr[ixs + iys + (iz)] +
                            rptr[ixs + iys + (iz + 4)]) +
                    c200 * (rptr[ixmms + iys + iz+2] +
                            rptr[ixpps + iys + iz+2] +
                            rptr[ixs + iymms + iz+2] + 
                            rptr[ixs + iypps + iz+2]);

                // Fourth
                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz+1)] =
                    c100 * (rptr[ixs + iys + iz+2] +
                            rptr[ixs + iys + iz + 4] +
                            rzms) + 
                            c000 * rptr[ixs + iys + iz + 3];

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz+1)] +=
                    c110 * (rptr[ixps + iyps + iz+3] +
                            rptr[ixps + iyms + iz+3] +
                            rptr[ixms + iyps + iz+3] +
                            rptr[ixms + iyms + iz+3] +
                            rzpps +
                            rz);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz+1)] +=
                    c200 * (rptr[ixs + iys + iz+1] +
                            rptr[ixs + iys + iz + 5]) +
                    c200 * (rptr[ixmms + iys + iz+3] +
                            rptr[ixpps + iys + iz+3] +
                            rptr[ixs + iymms + iz+3] + 
                            rptr[ixs + iypps + iz+3]);


            }                   /* end for */

        }                       /* end for */

    }                           /* end for */

}


template <typename RmgType>
double FiniteDiff::app_cil_fourth (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz)
{

    RmgTimer RT("App_cil: computation");
    int incx, incy, incxr, incyr;
    int ixs, iys, ixms, ixps, iyms, iyps;
    RmgType ecxy, ecxz, ecyz, cc = 0.0, fcx, fcy, fcz;
    RmgType ihx, ihy, ihz;
    RmgType a1, a2, a3;
    RmgType ONE_t = 1.0;
    RmgType TWO_t = 2.0;
    RmgType THREE_t = 3.0;
    RmgType FOUR_t = 4.0;
    RmgType FIVE_t = 5.0;
    RmgType SIX_t = 6.0;
    RmgType EIGHT_t = 8.0;
    RmgType NINE_t = 9.0;
    RmgType TWELVE_t = 12.0;
    RmgType EIGHTTEEN_t = 18.0;
    RmgType TWENTYFOUR_t = 24.0;
    RmgType THIRTYFOUR_t = 34.0;
    RmgType THIRTYSIX_t = 36.0;
    RmgType FORTYEIGHT_t = 48.0;

    int ibrav = L->get_ibrav_type();


    ihx = 1.0 / (gridhx * gridhx * L->get_xside() * L->get_xside());
    ihy = 1.0 / (gridhy * gridhy * L->get_yside() * L->get_yside());
    ihz = 1.0 / (gridhz * gridhz * L->get_zside() * L->get_zside());


    incx = (dimz + 2) * (dimy + 2);
    incy = dimz + 2;
    incxr = dimz * dimy;
    incyr = dimz;

    switch(ibrav) {

        case CUBIC_PRIMITIVE:
        case ORTHORHOMBIC_PRIMITIVE:
        case TETRAGONAL_PRIMITIVE:

            if (FiniteDiff::check_anisotropy(gridhx, gridhy, gridhz, 0.0000001))
            {

                cc = (-FOUR_t / THREE_t) * (ihx + ihx + ihx);
                fcx = (FIVE_t / SIX_t) * ihx + (cc / EIGHT_t);
                ecxy = (ONE_t / TWELVE_t) * (ihx + ihx);
                incy = dimz + 2;
                incx = (dimz + 2) * (dimy + 2);
                incyr = dimz;
                incxr = dimz * dimy;

                for (int ix = 1; ix <= dimx; ix++)
                {
                    ixs = ix * incx;
                    ixms = (ix - 1) * incx;
                    ixps = (ix + 1) * incx;
                    for (int iy = 1; iy <= dimy; iy++)
                    {
                        iys = iy * incy;
                        iyms = (iy - 1) * incy;
                        iyps = (iy + 1) * incy;

                        for (int iz = 1; iz <= dimz; iz++)
                        {

                            b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] =
                                cc * rptr[ixs + iys + iz] +
                                fcx * (rptr[ixms + iys + iz] +
                                        rptr[ixps + iys + iz] +
                                        rptr[ixs + iyms + iz] +
                                        rptr[ixs + iyps + iz] +
                                        rptr[ixs + iys + (iz - 1)] + rptr[ixs + iys + (iz + 1)]);

                            b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] +=
                                ecxy * (rptr[ixms + iys + iz - 1] +
                                        rptr[ixps + iys + iz - 1] +
                                        rptr[ixs + iyms + iz - 1] +
                                        rptr[ixs + iyps + iz - 1] +
                                        rptr[ixms + iyms + iz] +
                                        rptr[ixms + iyps + iz] +
                                        rptr[ixps + iyms + iz] +
                                        rptr[ixps + iyps + iz] +
                                        rptr[ixms + iys + iz + 1] +
                                        rptr[ixps + iys + iz + 1] +
                                        rptr[ixs + iyms + iz + 1] + rptr[ixs + iyps + iz + 1]);


                        }           /* end for */

                    }               /* end for */

                }                   /* end for */
            }
            else
            {

                /* Compute coefficients for this grid spacing */
                cc = (-FOUR_t / THREE_t) * (ihx + ihy + ihz);

                fcx = FIVE_t/SIX_t * ihx + (cc / EIGHT_t);
                fcy = FIVE_t/SIX_t * ihy + (cc / EIGHT_t);
                fcz = FIVE_t/SIX_t * ihz + (cc / EIGHT_t);

                ecxy = (ONE_t / TWELVE_t) * (ihx + ihy);
                ecxz = (ONE_t / TWELVE_t) * (ihx + ihz);
                ecyz = (ONE_t / TWELVE_t) * (ihy + ihz);


                incy = dimz + 2;
                incx = (dimz + 2) * (dimy + 2);
                incyr = dimz;
                incxr = dimz * dimy;



                for (int ix = 1; ix <= dimx; ix++)
                {
                    ixs = ix * incx;
                    ixms = (ix - 1) * incx;
                    ixps = (ix + 1) * incx;
                    for (int iy = 1; iy <= dimy; iy++)
                    {
                        iys = iy * incy;
                        iyms = (iy - 1) * incy;
                        iyps = (iy + 1) * incy;

                        for (int iz = 1; iz <= dimz; iz++)
                        {

                            b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] =
                                cc * rptr[ixs + iys + iz] +
                                fcx * rptr[ixms + iys + iz] +
                                fcx * rptr[ixps + iys + iz] +
                                fcy * rptr[ixs + iyms + iz] +
                                fcy * rptr[ixs + iyps + iz] +
                                fcz * rptr[ixs + iys + (iz - 1)] + fcz * rptr[ixs + iys + (iz + 1)];

                            b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] +=
                                ecxz * rptr[ixms + iys + iz - 1] +
                                ecxz * rptr[ixps + iys + iz - 1] +
                                ecyz * rptr[ixs + iyms + iz - 1] +
                                ecyz * rptr[ixs + iyps + iz - 1] +
                                ecxy * rptr[ixms + iyms + iz] +
                                ecxy * rptr[ixms + iyps + iz] +
                                ecxy * rptr[ixps + iyms + iz] +
                                ecxy * rptr[ixps + iyps + iz] +
                                ecxz * rptr[ixms + iys + iz + 1] +
                                ecxz * rptr[ixps + iys + iz + 1] +
                                ecyz * rptr[ixs + iyms + iz + 1] + ecyz * rptr[ixs + iyps + iz + 1];


                        }           /* end for */

                    }               /* end for */

                }                   /* end for */

            }                       /* end if */
            break;

        case HEXAGONAL2:
        case HEXAGONAL:

            cc = ((-THREE_t / FOUR_t) * ihz) - ((FIVE_t / THREE_t) * ihx);
            a1 = ((THREE_t / EIGHT_t) * ihz) - ((ONE_t / SIX_t) * ihx);
            a2 = ((FIVE_t / EIGHTTEEN_t) * ihx) - ((ONE_t / TWENTYFOUR_t) * ihz);
            a3 = ((ONE_t / FORTYEIGHT_t) * ihz) + ((ONE_t / THIRTYSIX_t) * ihx);
            cc = TWO_t * cc;
            a1 = TWO_t * a1;
            a2 = TWO_t * a2;
            a3 = TWO_t * a3;

            for (int ix = 1; ix <= dimx; ix++)
            {
                ixs = ix * incx;
                ixms = (ix - 1) * incx;
                ixps = (ix + 1) * incx;
                for (int iy = 1; iy <= dimy; iy++)
                {
                    iys = iy * incy;
                    iyms = (iy - 1) * incy;
                    iyps = (iy + 1) * incy;

                    for (int iz = 1; iz <= dimz; iz++)
                    {

                        b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] =
                            cc * rptr[ixs + iys + iz] +
                            a3 * (rptr[ixps + iys + iz - 1] +
                                    rptr[ixps + iyms + iz - 1] +
                                    rptr[ixs + iyms + iz - 1] +
                                    rptr[ixms + iys + iz - 1] +
                                    rptr[ixms + iyps + iz - 1] +
                                    rptr[ixs + iyps + iz - 1] +
                                    rptr[ixps + iys + iz + 1] +
                                    rptr[ixps + iyms + iz + 1] +
                                    rptr[ixs + iyms + iz + 1] +
                                    rptr[ixms + iys + iz + 1] +
                                    rptr[ixms + iyps + iz + 1] + 
                                    rptr[ixs + iyps + iz + 1]);


                        b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] +=
                            a2 * (rptr[ixps + iys + iz] +
                                    rptr[ixps + iyms + iz] +
                                    rptr[ixs + iyms + iz] +
                                    rptr[ixms + iys + iz] + 
                                    rptr[ixms + iyps + iz] + 
                                    rptr[ixs + iyps + iz]);

                        b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] +=
                            a1 * (rptr[ixs + iys + iz - 1] + rptr[ixs + iys + iz + 1]);

                    }               /* end for */

                }                   /* end for */

            }                       /* end for */

            break;

        case CUBIC_FC:

            cc = -(THIRTYFOUR_t / SIX_t) * ihx;
            a1 = (FOUR_t / NINE_t) * ihx;
            a2 = (ONE_t / EIGHTTEEN_t) * ihx;

            for (int ix = 1; ix <= dimx; ix++)
            {
                ixs = ix * incx;
                ixms = (ix - 1) * incx;
                ixps = (ix + 1) * incx;
                for (int iy = 1; iy <= dimy; iy++)
                {
                    iys = iy * incy;
                    iyms = (iy - 1) * incy;
                    iyps = (iy + 1) * incy;

                    for (int iz = 1; iz <= dimz; iz++)
                    {

                        b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] =
                            cc * rptr[ix * incx + iys + iz] +
                            a1 * rptr[ixms + iys + iz] +
                            a1 * rptr[ixms + iys + iz + 1] +
                            a1 * rptr[ixms + iyps + iz] +
                            a1 * rptr[ixs + iyms + iz] +
                            a1 * rptr[ixs + iyms + iz + 1] +
                            a1 * rptr[ixs + iys + iz - 1] +
                            a1 * rptr[ixs + iys + iz + 1] +
                            a1 * rptr[ixs + iyps + iz - 1] +
                            a1 * rptr[ixs + iyps + iz] +
                            a1 * rptr[ixps + iyms + iz] +
                            a1 * rptr[ixps + iys + iz - 1] + 
                            a1 * rptr[ixps + iys + iz];


                        b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] +=
                            a2 * rptr[ixms + iyms + iz + 1] +
                            a2 * rptr[ixms + iyps + iz - 1] +
                            a2 * rptr[ixms + iyps + iz + 1] +
                            a2 * rptr[ixps + iyms + iz - 1] +
                            a2 * rptr[ixps + iyms + iz + 1] + 
                            a2 * rptr[ixps + iyps + iz - 1];

                    }               /* end for */

                }                   /* end for */

            }                       /* end for */
            break;

        default:
            rmg_error_handler (__FILE__, __LINE__, "Grid symmetry not programmed yet in app_cil_fourth.\n");

    } // end switch

    return (double)std::real(cc);

}                               /* end app_cil */


template <typename RmgType>
void FiniteDiff::app_cir_fourth (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz)
{

    RmgTimer RT("App_cir: computation");
    int ixs, iys, ixms, ixps, iyms, iyps;
    RmgType c000(0.5), c100(1.0/12.0);
    RmgType Bc(2.0 / 3.0);
    RmgType Bf(1.0 / 36.0);
    RmgType Bch(7.0 / 12.0);
    RmgType Bfh(1.0 / 24.0);
    RmgType Bz(1.0 / 12.0);

    int ibrav = L->get_ibrav_type();
    int incx = (dimz + 2) * (dimy + 2);
    int incy = dimz + 2;
    int incxr = dimz * dimy;
    int incyr = dimz;

    switch(ibrav) {

        case CUBIC_PRIMITIVE: 
        case ORTHORHOMBIC_PRIMITIVE:
        case TETRAGONAL_PRIMITIVE:

            for (int ix = 1; ix < dimx + 1; ix++)
            {
                ixs = ix * incx;
                ixms = (ix - 1) * incx;
                ixps = (ix + 1) * incx;

                for (int iy = 1; iy < dimy + 1; iy++)
                {
                    iys = iy * incy;
                    iyms = (iy - 1) * incy;
                    iyps = (iy + 1) * incy;

                    for (int iz = 1; iz < dimz + 1; iz++)
                    {

                        b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] =
                            c100 * (rptr[ixs + iys + (iz - 1)] +
                                    rptr[ixs + iys + (iz + 1)] +
                                    rptr[ixms + iys + iz] +
                                    rptr[ixps + iys + iz] +
                                    rptr[ixs + iyms + iz] +
                                    rptr[ixs + iyps + iz]) + 
                            c000 *  rptr[ixs + iys + iz];

                    }                   /* end for */

                }                       /* end for */

            }                           /* end for */
            break;

        case HEXAGONAL2:
        case HEXAGONAL:

            for(int ix = 1;ix < dimx + 1;ix++) {

                ixs = ix * incx;
                ixms = (ix - 1) * incx;
                ixps = (ix + 1) * incx;
                for(int iy = 1;iy < dimy + 1;iy++) {

                    iys = iy * incy;
                    iyms = (iy - 1) * incy;
                    iyps = (iy + 1) * incy;
                    for(int iz = 1;iz < dimz + 1;iz++) {

                        b[(ix - 1)*incxr + (iy - 1)*incyr + (iz - 1)] =
                            Bch * rptr[ixs + iys + iz] +
                            Bz * rptr[ixs + iys + iz - 1] +
                            Bz * rptr[ixs + iys + iz + 1] +
                            Bfh * rptr[ixps + iys + iz] +
                            Bfh * rptr[ixps + iyms + iz] +
                            Bfh * rptr[ixs + iyms + iz] +
                            Bfh * rptr[ixms + iys + iz] +
                            Bfh * rptr[ixms + iyps + iz] +
                            Bfh * rptr[ixs + iyps + iz];

                    } /* end for */

                } /* end for */

            } /* end for */
            break;

        case CUBIC_FC:

            for (int ix = 1; ix <= dimx; ix++)
            {
                ixs = ix * incx;
                ixms = (ix - 1) * incx;
                ixps = (ix + 1) * incx;
                for (int iy = 1; iy <= dimy; iy++)
                {
                    iys = iy * incy;
                    iyms = (iy - 1) * incy;
                    iyps = (iy + 1) * incy;
                    for (int iz = 1; iz <= dimz; iz++)
                    {

                        b[(ix - 1) * incxr + (iy - 1) * incyr + (iz - 1)] =
                            Bc * rptr[ixs + iys + iz] +
                            Bf * (rptr[ixms + iys + iz] +
                                    rptr[ixms + iys + iz + 1] +
                                    rptr[ixms + iyps + iz] +
                                    rptr[ixs + iyms + iz] +
                                    rptr[ixs + iyms + iz + 1] +
                                    rptr[ixs + iys + iz - 1] +
                                    rptr[ixs + iys + iz + 1] +
                                    rptr[ixs + iyps + iz - 1] +
                                    rptr[ixs + iyps + iz] +
                                    rptr[ixps + iyms + iz] + 
                                    rptr[ixps + iys + iz - 1] + 
                                    rptr[ixps + iys + iz]);


                    }                   /* end for */

                }                       /* end for */

            }                           /* end for */

            break;

        default:
            rmg_error_handler (__FILE__, __LINE__, "Grid symmetry not programmed yet in app_cir_fourth.\n");

    }

}
