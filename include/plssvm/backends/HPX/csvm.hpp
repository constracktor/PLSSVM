/**
 * @file
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Defines a C-SVM using the HPX backend.
 */

#ifndef PLSSVM_BACKENDS_HPX_CSVM_HPP_
#define PLSSVM_BACKENDS_HPX_CSVM_HPP_
#pragma once

#include "plssvm/csvm.hpp"                // plssvm::csvm
#include "plssvm/detail/type_traits.hpp"  // PLSSVM_REQUIRES
#include "plssvm/parameter.hpp"           // plssvm::parameter, plssvm::detail::{parameter, has_only_parameter_named_args_v}
#include "plssvm/target_platforms.hpp"    // plssvm::target_platform

#include <type_traits>                    // std::true_type
#include <utility>                        // std::forward, std::pair
#include <vector>                         // std::vector

namespace plssvm {

namespace hpx {

/**
 * @brief A C-SVM implementation using HPX as backend.
 */
class csvm : public ::plssvm::csvm {
  public:
    /**
     * @brief Construct a new C-SVM using the HPX backend with the parameters given through @p params.
     * @param[in] params struct encapsulating all possible SVM parameters
     * @throws plssvm::exception all exceptions thrown in the base class constructor
     * @throws plssvm::hpx::backend_exception if the target platform isn't plssvm::target_platform::automatic or plssvm::target_platform::cpu
     * @throws plssvm::hpx::backend_exception if the plssvm::target_platform::cpu target isn't available
     */
    explicit csvm(parameter params = {});
    /**
     * @brief Construct a new C-SVM using the HPX backend on the @p target platform with the parameters given through @p params.
     * @param[in] target the target platform used for this C-SVM
     * @param[in] params struct encapsulating all possible SVM parameters
     * @throws plssvm::exception all exceptions thrown in the base class constructor
     * @throws plssvm::hpx::backend_exception if the target platform isn't plssvm::target_platform::automatic or plssvm::target_platform::cpu
     * @throws plssvm::hpx::backend_exception if the plssvm::target_platform::cpu target isn't available
     */
    explicit csvm(target_platform target, parameter params = {});

    /**
     * @brief Construct a new C-SVM using the HPX backend and the optionally provided @p named_args.
     * @param[in] named_args the additional optional named-parameters
     * @throws plssvm::exception all exceptions thrown in the base class constructor
     * @throws plssvm::hpx::backend_exception if the target platform isn't plssvm::target_platform::automatic or plssvm::target_platform::cpu
     * @throws plssvm::hpx::backend_exception if the plssvm::target_platform::cpu target isn't available
     */
    template <typename... Args, PLSSVM_REQUIRES(detail::has_only_parameter_named_args_v<Args...>)>
    explicit csvm(Args &&...named_args) :
        ::plssvm::csvm{ std::forward<Args>(named_args)... } {
        // the default target is the automatic one
        this->init(plssvm::target_platform::automatic);
    }
    /**
     * @brief Construct a new C-SVM using the HPX backend on the @p target platform and the optionally provided @p named_args.
     * @param[in] target the target platform used for this C-SVM
     * @param[in] named_args the additional optional named-parameters
     * @throws plssvm::exception all exceptions thrown in the base class constructor
     * @throws plssvm::hpx::backend_exception if the target platform isn't plssvm::target_platform::automatic or plssvm::target_platform::cpu
     * @throws plssvm::hpx::backend_exception if the plssvm::target_platform::cpu target isn't available
     */
    template <typename... Args, PLSSVM_REQUIRES(detail::has_only_parameter_named_args_v<Args...>)>
    explicit csvm(const target_platform target, Args &&...named_args) :
        ::plssvm::csvm{ std::forward<Args>(named_args)... } {
        this->init(target);
    }

    /**
     * @copydoc plssvm::csvm::csvm(const plssvm::csvm &)
     */
    csvm(const csvm &) = delete;
    /**
     * @copydoc plssvm::csvm::csvm(plssvm::csvm &&) noexcept
     */
    csvm(csvm &&) noexcept = default;
    /**
     * @copydoc plssvm::csvm::operator=(const plssvm::csvm &)
     */
    csvm &operator=(const csvm &) = delete;
    /**
     * @copydoc plssvm::csvm::operator=(plssvm::csvm &&) noexcept
     */
    csvm &operator=(csvm &&) noexcept = default;
    /**
     * @brief Default destructor since the copy and move constructors and copy- and move-assignment operators are defined.
     */
     ~csvm() override = default;

  protected:
    /**
     * @copydoc plssvm::csvm::solve_system_of_linear_equations
     */
    [[nodiscard]] std::pair<std::vector<float>, float> solve_system_of_linear_equations(const detail::parameter<float> &params, const std::vector<std::vector<float>> &A, std::vector<float> b, float eps, unsigned long long max_iter) const override { return this->solve_system_of_linear_equations_impl(params, A, b, eps, max_iter); }
    /**
     * @copydoc plssvm::csvm::solve_system_of_linear_equations
     */
    [[nodiscard]] std::pair<std::vector<double>, double> solve_system_of_linear_equations(const detail::parameter<double> &params, const std::vector<std::vector<double>> &A, std::vector<double> b, double eps, unsigned long long max_iter) const override { return this->solve_system_of_linear_equations_impl(params, A, b, eps, max_iter); }
    /**
     * @copydoc plssvm::csvm::solve_system_of_linear_equations
     */
    template <typename real_type>
    [[nodiscard]] std::pair<std::vector<real_type>, real_type> solve_system_of_linear_equations_impl(const detail::parameter<real_type> &params, const std::vector<std::vector<real_type>> &A, std::vector<real_type> b, real_type eps, unsigned long long max_iter) const;

    /**
     * @copydoc plssvm::csvm::predict_values
     */
    [[nodiscard]] std::vector<float> predict_values(const detail::parameter<float> &params, const std::vector<std::vector<float>> &support_vectors, const std::vector<float> &alpha, float rho, std::vector<float> &w, const std::vector<std::vector<float>> &predict_points) const override { return this->predict_values_impl(params, support_vectors, alpha, rho, w, predict_points); }
    /**
     * @copydoc plssvm::csvm::predict_values
     */
    [[nodiscard]] std::vector<double> predict_values(const detail::parameter<double> &params, const std::vector<std::vector<double>> &support_vectors, const std::vector<double> &alpha, double rho, std::vector<double> &w, const std::vector<std::vector<double>> &predict_points) const override { return this->predict_values_impl(params, support_vectors, alpha, rho, w, predict_points); }
    /**
     * @copydoc plssvm::csvm::predict_values
     */
    template <typename real_type>
    [[nodiscard]] std::vector<real_type> predict_values_impl(const detail::parameter<real_type> &params, const std::vector<std::vector<real_type>> &support_vectors, const std::vector<real_type> &alpha, real_type rho, std::vector<real_type> &w, const std::vector<std::vector<real_type>> &predict_points) const;

    /**
     * @brief Calculate the `q` vector used in the dimensional reduction.
     * @details The template parameter `real_type` represents the type of the data points (either `float` or `double`).
     * @param[in] params the SVM parameter used to calculate `q` (e.g., kernel_type)
     * @param[in] data the data points used in the dimensional reduction.
     * @return the `q` vector (`[[nodiscard]]`)
     */
    template <typename real_type>
    [[nodiscard]] std::vector<real_type> generate_q(const detail::parameter<real_type> &params, const std::vector<std::vector<real_type>> &data) const;
    /**
     * @brief Precalculate the `w` vector to speedup up the prediction using the linear kernel function.
     * @details The template parameter `real_type` represents the type of the data points (either `float` or `double`).
     * @param[in] support_vectors the previously learned support vectors
     * @param[in] alpha the previously learned weights
     * @return the `w` vector (`[[nodiscard]]`)
     */
    template <typename real_type>
    [[nodiscard]] std::vector<real_type> calculate_w(const std::vector<std::vector<real_type>> &support_vectors, const std::vector<real_type> &alpha) const;

    /**
     * @brief Select the correct kernel based on the value of plssvm::parameter::kernel_type and run it on the CPU using HPX.
     * @details The template parameter `real_type` represents the type of the data points (either `float` or `double`).
     * @param[in] params the SVM parameter used to calculate `q` (e.g., kernel_type)
     * @param[in] q the `q` vector used in the dimensional reduction
     * @param[out] ret the result vector
     * @param[in] d the right-hand side of the equation
     * @param[in] data the data points
     * @param[in] QA_cost a value used in the dimensional reduction
     * @param[in] add denotes whether the values are added or subtracted from the result vector
     */
    template <typename real_type>
    void run_device_kernel(const detail::parameter<real_type> &params, const std::vector<real_type> &q, std::vector<real_type> &ret, const std::vector<real_type> &d, const std::vector<std::vector<real_type>> &data, real_type QA_cost, real_type add) const;

  private:
    /**
     * @brief Initializes the HPX backend and performs some sanity checks.
     * @param[in] target the target platform to use
     * @throws plssvm::hpx::backend_exception if the target platform isn't plssvm::target_platform::automatic or plssvm::target_platform::cpu
     * @throws plssvm::hpx::backend_exception if the plssvm::target_platform::cpu target isn't available
     */
    void init(target_platform target);
};

}  // namespace hpx

namespace detail {

/**
 * @brief Sets the `value` to `true` since C-SVMs using the HPX backend are available.
 */
template <>
struct csvm_backend_exists<hpx::csvm> : std::true_type {};

}  // namespace detail

}  // namespace plssvm

#endif  // PLSSVM_BACKENDS_HPX_CSVM_HPP_