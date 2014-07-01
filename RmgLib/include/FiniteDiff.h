/*
 *
 * Copyright (c) 2014, Emil Briggs
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

#ifndef RMG_FiniteDiff_H
#define RMG_FiniteDiff_H 1

/* Order of finite differencing for driver routines */
#define APP_CI_FOURTH 4
#define APP_CI_SIXTH 6

#ifdef __cplusplus
#include "Lattice.h"
#include "TradeImages.h"

template <typename RmgType>
void CPP_app_cir_driver (Lattice *L, TradeImages *T, RmgType * a, RmgType * b, int dimx, int dimy, int dimz, int order);
template <typename RmgType>
double CPP_app_cil_driver (Lattice *L, TradeImages *T, RmgType * a, RmgType * b, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz, int order);
template <typename RmgType>
void CPP_app_grad_driver (Lattice *L, TradeImages *T, RmgType * a, RmgType * bx, RmgType * by, RmgType * bz, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz);



#include "rmg_error.h"

class FiniteDiff {

private:
    Lattice *L;

public:
    FiniteDiff(Lattice *lptr);

    bool check_anisotropy(double hx, double hy, double hz, double limit);

    template <typename RmgType>
    double app_cil_sixth (RmgType *rptr, RmgType *b, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz);

    template <typename RmgType>
    void app_cir_sixth (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz);

    template <typename RmgType>
    double app_del2c (RmgType * a, RmgType * b, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz);

    template <typename RmgType>
    double app6_del2 (RmgType * a, RmgType * b, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz);

    template <typename RmgType>
    double app_cil_fourth (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz, double gridhx, double gridhy, double gridhz);

    template <typename RmgType>
    void app_cir_fourth (RmgType * rptr, RmgType * b, int dimx, int dimy, int dimz);

    template <typename RmgType>
    void app_cir_fcc (RmgType * a, RmgType * b, int dimx, int dimy, int dimz);

    template <typename RmgType>
    void app_cir_bcc (RmgType * a, RmgType * b, int dimx, int dimy, int dimz);

    template <typename RmgType>
    void app_cir_hex (RmgType * a, RmgType * b, int dimx, int dimy, int dimz);

    template <typename RmgType>
    void app_gradient (RmgType * rptr, RmgType * wxr, RmgType *wyr, RmgType *wzr, int dimx, int dimy, int dimz,
                               double gridhx, double gridhy, double gridhz);



};
#endif

#endif
