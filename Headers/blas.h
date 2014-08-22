/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/****f* QMD-MGDFT/blas.h *****
 * NAME
 *   Ab initio real space code with multigrid acceleration
 *   Quantum molecular dynamics package.
 *   Version: 2.1.5
 * COPYRIGHT
 *   Copyright (C) 1995  Emil Briggs
 *   Copyright (C) 1998  Emil Briggs, Charles Brabec, Mark Wensell, 
 *                       Dan Sullivan, Chris Rapcewicz, Jerzy Bernholc
 *   Copyright (C) 2001  Emil Briggs, Wenchang Lu,
 *                       Marco Buongiorno Nardelli,Charles Brabec, 
 *                       Mark Wensell,Dan Sullivan, Chris Rapcewicz,
 *                       Jerzy Bernholc
 * FUNCTION
 *   
 * INPUTS
 *
 * OUTPUT
 *  
 * PARENTS
 *
 * CHILDREN
 * 
 * SEE ALSO
 *  
 * SOURCE
 */
/*

			blas.h


    Lets us call blas routines from c. The Crays require that the function
    call be in Capital letters while the RS6000's require lowercase. Also
    we use the double precision form on the IBM's and the single precision
    form on the crays. (Even though a double in C is actually a fortran 
    single.)

*/

#ifdef LINUX 

#define                 sger            dger_
//#  define		global_sums     global_sums_
#  define		saxpy		daxpy_
#  define		daxpy		daxpy_
#  define		zaxpy		zaxpy_
#  define		ZAXPY		zaxpy_
#  define		scopy   	dcopy_
#  define		dcopy   	dcopy_
#  define		zcopy   	zcopy_
#  define		ZCOPY   	zcopy_
#  define		sdot    	ddot_
#  define		ddot    	ddot_
#  define		sscal   	dscal_
#  define		dscal   	dscal_
#  define		ssyrk   	dsyrk_
#  define		spotrf  	dpotrf_
#  define		dpotrf  	dpotrf_
#  define		spotri  	dpotri_
#  define		cpotrf  	zpotrf_
#  define		cpotri  	zpotri_
#  define		snrm2   	dnrm2_
#  define		sswap   	dswap_
#  define		dswap   	dswap_
#  define		ssyev		dsyev_
#  define		cheev		zheev_
#  define		xbecke		xbecke_
#  define		corlyp_f90      corlyp_f90_
#  define		corpbe  	corpbe_
#  define		exchpbe 	exchpbe_
#  define		exch		exch_
#  define		corlsd		corlsd_
#  define		corgga		corgga_
#  define		symrho  	symrho_
#  define		fsymforces  	fsymforces_
#  define		symmetry 	symmetry_
#  define		fgram   	dfgram_
#  define		dlamch  	dlamch_
#  define		dgemm  	        dgemm_
#  define		zgemm  	        zgemm_
#  define		ZGEMM  	        zgemm_
#  define		dgetrf 	        dgetrf_
#  define		dgetri 	        dgetri_
#  define		zgetrf 	        zgetrf_
#  define		zgetri 	        zgetri_
#  define		dgesv 	        dgesv_
#  define               sgesv           dgesv_
#  define		zgesv 	        zgesv_
#  define		ZGESV 	        zgesv_
#  define               dgemv           dgemv_
#  define               dsygvx          dsygvx_
#  define               zhegvx          zhegvx_
#  define               zhegst          zhegst_
#  define               zheevd          zheevd_

#else
#ifdef AIX

#  define		saxpy		daxpy
#  define		scopy   	dcopy
#  define		sdot    	ddot
#  define		sscal   	dscal
#  define		ssyrk   	dsyrk
#  define		spotrf  	dpotrf
#  define		spotri  	dpotri
#  define		snrm2   	dnrm2
#  define		sswap   	dswap
#  define		ssyev		dsyev
#  define		cpotrf  	zpotrf
#  define		cpotri  	zpotri
#  define		cheev   	zheev
#  define		fgram   	dfgram

#endif
#endif


#if __cplusplus
extern "C" {
#endif

void my_copy(rmg_double_t *in, rmg_double_t *out, int length);
void my_scal(rmg_double_t alpha, rmg_double_t *vect, int length);
void my_axpy(rmg_double_t alpha, rmg_double_t *in, rmg_double_t *out, int length);
void my_swap(rmg_double_t *vec1, rmg_double_t *vec2, int length);


void fsymforces (rmg_double_t * force, int *s, int *irg, int *irt,
                 int *nat, int *ibrav, int *nsym,
                 rmg_double_t * celldm, int *nr1, int *nr2, int *nr3);
int ilaenv (int *ispec, char *name, char *opts, int *n1, int *n2, int *n3,
            int *n4);
void saxpy (int *n, double * alpha, double * x, int *incx, double * y, int *incy);
void sscal (int *n, rmg_double_t * alpha, rmg_double_t * x, int *incx);
void scopy (int *n, rmg_double_t * x, int *incx, rmg_double_t * y, int *incy);
rmg_double_t sdot (int *n, rmg_double_t * x, int *incx, rmg_double_t * y, int *incy);
rmg_double_t snrm2 (int *n, rmg_double_t * x, int *incx);
void ssyrk (const char *uplo, const char *trans, int *n, int *k, rmg_double_t * alpha, rmg_double_t * A,
            int *lda, rmg_double_t * beta, rmg_double_t * c, int *ldc);
void sswap (int *n, rmg_double_t * x, int *incx, rmg_double_t * y, int *incy);
void xbecke (rmg_double_t * d, rmg_double_t * s, rmg_double_t * u, rmg_double_t * v, rmg_double_t * ex, rmg_double_t * vx);
void corlyp_f90 (rmg_double_t * dp, rmg_double_t * dm, rmg_double_t * dp1, rmg_double_t * dm1, rmg_double_t * dp2,
                 rmg_double_t * dm2, rmg_double_t * ec, rmg_double_t * vcp0, rmg_double_t * vcm0, int *ndm);

void corlsd (rmg_double_t * rs, rmg_double_t * zet, rmg_double_t * ec, rmg_double_t * vcup, rmg_double_t * vcdn,
             rmg_double_t * ecrs, rmg_double_t * eczet, rmg_double_t * alfc);
void corgga (rmg_double_t * rs, rmg_double_t * zet, rmg_double_t * t, rmg_double_t * uu, rmg_double_t * vv,
             rmg_double_t * ww, rmg_double_t * h, rmg_double_t * dvcup, rmg_double_t * dvcdn,
             rmg_double_t * fk, rmg_double_t * sk, rmg_double_t * g, rmg_double_t * ec, rmg_double_t * ecrs,
             rmg_double_t * eczet);
void corpbe (rmg_double_t * rs, rmg_double_t * zet, rmg_double_t * t, rmg_double_t * uu, rmg_double_t * vv, rmg_double_t * ww,
             int *lgga, int *lpot, rmg_double_t * ec, rmg_double_t * vcup, rmg_double_t * vcdn,
             rmg_double_t * h, rmg_double_t * dvcup, rmg_double_t * DVCDN);
void cpotrf (char *uplo, int *n, rmg_double_t * a, int *lda, int *info);
void exch (rmg_double_t * d, rmg_double_t * s, rmg_double_t * u, rmg_double_t * v, rmg_double_t * ex, rmg_double_t * vx);
void exchpbe (rmg_double_t * d, rmg_double_t * s, rmg_double_t * u, rmg_double_t * v, int *lgga, int *lpot,
              rmg_double_t * ex, rmg_double_t * vx);
void symrho (rmg_double_t * rho, int *nr1, int *nr2, int *nr3, int *nsym, int *s,
             int *irg, int *ftau);
void ssyev (char *jobz, char *uplo, int *numm, rmg_double_t * ss, int *numn,
            rmg_double_t * work1, rmg_double_t * work2, int *lwork, int *info);
void cheev (char *jobz, char *uplo, int *numm, rmg_double_t * ss, int *numn,
            rmg_double_t * work1, rmg_double_t * work2, int *lwork, rmg_double_t *, int *info);
void spotri (char *, int *, rmg_double_t *, int *, int *);
void dgemm(const char *, const char *, int *, int *, int *, rmg_double_t *, rmg_double_t *, int *, rmg_double_t *, int *, rmg_double_t *, rmg_double_t *, int *);
void zgemm(const char *, const char *, int *, int *, int *, rmg_double_t *, rmg_double_t *, int *, rmg_double_t *, int *, rmg_double_t *, rmg_double_t *, int *);
void dgetrf( int *, int *, rmg_double_t *, int *, int *, int *);
void dgetri(int *, rmg_double_t *, int *, int *, rmg_double_t *, int *, int *);
void zgetrf( int *, int *, rmg_double_t *, int *, int *, int *);
void zgetri(int *, rmg_double_t *, int *, int *, rmg_double_t *, int *, int *);
void zhegst(int *, const char *, int *, double *, int *, double *, int *, int *);
void zheevd(const char *, const char *, int *, double *, int *, double *, double *, int *, double *, int *, int *, int *, int *);
void dgesv (int *, int*, rmg_double_t *, int *, int *, rmg_double_t *, int *, int *);
void zgesv (int *, int*, rmg_double_t *, int *, int *, rmg_double_t *, int *, int *);
void dgemv ( char *, int *, int *, rmg_double_t *, rmg_double_t *, int *, rmg_double_t *, int *, rmg_double_t *, rmg_double_t *, int *);
void dsygvx (int *itype, char *jobz, char *range, char *uplo, int *n, double *A, int *lda, double *B, int *ldb, double *vl, double *vu, 
             int *il, int *iu, double *tol, int *m, double *w, double *z, int *ldz, double *work, int *lwork, int *iwork, int *ifail, int *info);
void zhegvx (int *itype, char *jobz, char *range, char *uplo, int *n, double *A, int *lda, double *B, int *ldb, double *vl, double *vu, 
             int *il, int *iu, double *tol, int *m, double *w, double *z, int *ldz, double *work, int *lwork, double *rwork, int *iwork, int *ifail, int *info);

#if __cplusplus
}
#endif
/******/
