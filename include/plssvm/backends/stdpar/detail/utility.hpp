/**
 * @file
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Utility functions specific to the stdpar backend.
 */

#ifndef PLSSVM_BACKENDS_STDPAR_DETAIL_UTILITY_HPP_
#define PLSSVM_BACKENDS_STDPAR_DETAIL_UTILITY_HPP_
#pragma once

#include "plssvm/target_platforms.hpp"  // plssvm::target_platform

#if defined(PLSSVM_STDPAR_BACKEND_HAS_ACPP)
    #include "plssvm/backends/SYCL/detail/atomics.hpp"  // plssvm::sycl::detail::atomic_op

    #include "sycl/sycl.hpp"  // ::sycl::device
#else
    #include "boost/atomic/atomic_ref.hpp"  // boost::atomic_ref
#endif

#include <string>  // std::string

namespace plssvm::stdpar::detail {

#if defined(PLSSVM_STDPAR_BACKEND_HAS_ACPP)
template <typename T>
using atomic_ref = plssvm::sycl::detail::atomic_op<T>;
#else
using boost::atomic_ref;
#endif

#if defined(PLSSVM_STDPAR_BACKEND_HAS_ACPP)
[[nodiscard]] bool default_device_equals_target(const ::sycl::device &device, plssvm::target_platform target);
#endif

/**
 * @brief Return the version of the used stdpar implementation.
 * @return the stdpar version (`[[nodiscard]]`)
 */
[[nodiscard]] std::string get_stdpar_version();

}  // namespace plssvm::stdpar::detail

#endif  // PLSSVM_BACKENDS_STDPAR_DETAIL_UTILITY_HPP_
