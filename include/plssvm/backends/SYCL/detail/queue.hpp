/**
 * @file
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief PImpl class used to hide the "sycl/sycl.hpp" header from the public interface.
 */

#pragma once

#include <memory>  // std::shared_ptr

namespace plssvm::@PLSSVM_SYCL_BACKEND_NAMESPACE_NAME@::detail {

/**
 * @brief PImpl class to hide the ::sycl::queue class from the public interface (and, therefore, the "sycl/sycl.hpp" header).
 */
class queue {
  public:
    /// The struct used in the PImpl idiom (encapsulates the actual ::sycl::queue).
    struct queue_impl;
    /// A pointer to the implementation hidden in a private header.
    std::shared_ptr<queue_impl> impl{};
};

}  // namespace plssvm::@PLSSVM_SYCL_BACKEND_NAMESPACE_NAME@::detail
