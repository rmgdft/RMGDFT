#ifndef RMG_ErrorFuncs_H
#define RMG_ErrorFuncs_H 1


#if CUDA_ENABLED
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cublas_v2.h>

void RmgCudaError(const char *file, int line, const cudaError_t cudaStatus, const char * errorMessage);
void RmgCudaError(const char *file, int line, const cublasStatus_t status, const char * errorMessage);
void ProcessCublasError(cublasStatus_t custat);
#endif

#if HIP_ENABLED
#include <hip/hip_runtime.h>
#include <hipblas.h>
void RmgHipError(const char *file, int line, const hipError_t hipStatus, const char * errorMessage);
void RmgHipError(const char *file, int line, const hipblasStatus_t status, const char * errorMessage);
#endif

#endif

