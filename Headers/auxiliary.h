#ifndef RMG_Auxiliary_H
#define RMG_Auxiliary_H 1

template <typename RmgType>
void CPP_genvpsi (RmgType * psi, RmgType * sg_twovpsi, double * vtot, double * vnl, double * kd,
              double kmag, int dimx, int dimy, int dimz);

template <typename RmgType>
void CPP_pack_stop_axpy (RmgType * sg, RmgType * pg, double alpha, int dimx, int dimy, int dimz);

template <typename RmgType>
void CPP_pack_stop (RmgType * sg, RmgType * pg, int dimx, int dimy, int dimz);

template <typename RmgType>
void CPP_pack_ptos(RmgType * sg, RmgType * pg, int dimx, int dimy, int dimz);

template <typename RmgType>
void CPP_app_smooth (RmgType * f, RmgType * work, int dimx, int dimy, int dimz);

template <typename RmgType>
void CPP_app_smooth1 (RmgType * f, RmgType * work, int dimx, int dimy, int dimz);

void CPP_pack_dtos (double * s, double * d, int dimx, int dimy, int dimz, int boundaryflag);

void CPP_pack_stod (double * s, double * d, int dimx, int dimy, int dimz, int boundaryflag);

#endif
