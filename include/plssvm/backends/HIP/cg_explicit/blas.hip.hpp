/**
 * @file
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Functions for explicitly performing a BLAS GEMM like matrix-matrix multiplication using the HIP backend.
 */

#ifndef PLSSVM_BACKENDS_HIP_CG_EXPLICIT_BLAS_HIP_HPP_
#define PLSSVM_BACKENDS_HIP_CG_EXPLICIT_BLAS_HIP_HPP_
#pragma once

#include "plssvm/constants.hpp"  // plssvm::real_type, plssvm::THREAD_BLOCK_SIZE_OLD, plssvm::FEATURE_BLOCK_SIZE_OLD

#include "hip/hip_runtime.h"
#include "hip/hip_runtime_api.h"

namespace plssvm::hip {

/**
 * @brief Perform an explicit BLAS GEMM operation: `C = alpha * A * B + beta * C` where @p A is a `m x k` matrix, @p B is a `k x n` matrix, @p C is a `m x n` matrix, and @p alpha and @p beta are scalars.
 * @param[in] m the number of rows in @p A and @p C
 * @param[in] n the number of columns in @p B and @p C
 * @param[in] k the number of rows in @p A and number of columns in @p B
 * @param[in] alpha the scalar alpha value
 * @param[in] A the matrix @p A
 * @param[in] B the matrix @p B
 * @param[in] beta the scalar beta value
 * @param[in,out] C the matrix @p C, also used as result matrix
 */
__global__ void device_kernel_gemm(const unsigned long long m, const unsigned long long n, const unsigned long long k, const real_type alpha, const real_type *A, const real_type *B, const real_type beta, real_type *C) {
    // compute: C = alpha * A * B + beta * C with A in m x k, B in n x k, and C in n x m, alpha, beta as scalar
    const unsigned long long i = blockIdx.x * blockDim.x + threadIdx.x;  // # rhs
    const unsigned long long j = blockIdx.y * blockDim.y + threadIdx.y;  // # rows
    const unsigned long long j_cached_idx = blockIdx.y * blockDim.y + threadIdx.x;

    __shared__ real_type A_cache[FEATURE_BLOCK_SIZE_OLD][THREAD_BLOCK_SIZE_OLD];
    __shared__ real_type B_cache[FEATURE_BLOCK_SIZE_OLD][THREAD_BLOCK_SIZE_OLD];

    real_type temp{ 0.0 };

    for (unsigned long long dim = 0; dim < k; dim += FEATURE_BLOCK_SIZE_OLD) {
        // zero out shared memory
        if (threadIdx.y < FEATURE_BLOCK_SIZE_OLD) {
            A_cache[threadIdx.y][threadIdx.x] = real_type{ 0.0 };
            B_cache[threadIdx.y][threadIdx.x] = real_type{ 0.0 };
        }

        // load data into shared memory
        if (threadIdx.y < FEATURE_BLOCK_SIZE_OLD && dim + threadIdx.y < k) {
            if (dim + threadIdx.y < j_cached_idx) {
                if (j_cached_idx < k) {
                    A_cache[threadIdx.y][threadIdx.x] = A[(dim + threadIdx.y) * k + j_cached_idx - (dim + threadIdx.y) * (dim + threadIdx.y + 1) / 2];
                }
            } else {
                A_cache[threadIdx.y][threadIdx.x] = A[j_cached_idx * k + dim + threadIdx.y - j_cached_idx * (j_cached_idx + 1) / 2];
            }
            if (i < n) {
                B_cache[threadIdx.y][threadIdx.x] = B[(dim + threadIdx.y) * n + i];
            }
        }
        __syncthreads();

        // calculation
        for (unsigned long long block_dim = 0; block_dim < FEATURE_BLOCK_SIZE_OLD; ++block_dim) {
            temp += A_cache[block_dim][threadIdx.y] * B_cache[block_dim][threadIdx.x];
        }
        __syncthreads();
    }

    if (i < n && j < m) {
        C[j * n + i] = alpha * temp + beta * C[j * n + i];
    }
}

}  // namespace plssvm::hip

#endif  // PLSSVM_BACKENDS_HIP_CG_EXPLICIT_BLAS_HIP_HPP_
