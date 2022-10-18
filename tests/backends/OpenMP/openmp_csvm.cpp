/**
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the functionality related to the OpenMP backend.
 */

#include "backends/OpenMP/mock_openmp_csvm.hpp"

#include "plssvm/backends/OpenMP/csvm.hpp"        // plssvm::openmp::csvm
#include "plssvm/backends/OpenMP/exceptions.hpp"  // plssvm::openmp::backend_exception
#include "plssvm/data_set.hpp"                    // plssvm::data_set
#include "plssvm/kernel_function_types.hpp"       // plssvm::kernel_function_type
#include "plssvm/parameter.hpp"                   // plssvm::detail::data_set
#include "plssvm/target_platforms.hpp"            // plssvm::target_platform

#include "backends/generic_tests.hpp"  // generic::{test_solve_system_of_linear_equations, test_predict_values, test_predict, test_score}
#include "custom_test_macros.hpp"      // EXPECT_THROW_WHAT, EXPECT_FLOATING_POINT_VECTOR_NEAR
#include "naming.hpp"                  // naming::{real_type_kernel_function_to_name, real_type_to_name}
#include "types_to_test.hpp"           // util::{real_type_kernel_function_gtest, real_type_gtest}
#include "utility.hpp"                 // util::{redirect_output, generate_random_vector}

#include "gtest/gtest.h"  // TEST_F, EXPECT_NO_THROW, TYPED_TEST_SUITE, TYPED_TEST, ::testing::Test

#include <vector>  // std::vector

// TODO: kernel??!??!?

class OpenMPCSVM : public ::testing::Test, private util::redirect_output {};

// check whether the constructor correctly fails when using an incompatible target platform
TEST_F(OpenMPCSVM, construct_parameter_invalid_target_platform) {
    // only automatic or cpu are allowed as target platform for the OpenMP backend
    EXPECT_NO_THROW(mock_openmp_csvm{ plssvm::target_platform::automatic });
    EXPECT_NO_THROW(mock_openmp_csvm{ plssvm::target_platform::cpu });

    // all other target platforms must throw
    EXPECT_THROW_WHAT(mock_openmp_csvm{ plssvm::target_platform::gpu_nvidia },
                      plssvm::openmp::backend_exception,
                      "Invalid target platform 'gpu_nvidia' for the OpenMP backend!");
    EXPECT_THROW_WHAT(mock_openmp_csvm{ plssvm::target_platform::gpu_amd },
                      plssvm::openmp::backend_exception,
                      "Invalid target platform 'gpu_amd' for the OpenMP backend!");
    EXPECT_THROW_WHAT(mock_openmp_csvm{ plssvm::target_platform::gpu_intel },
                      plssvm::openmp::backend_exception,
                      "Invalid target platform 'gpu_intel' for the OpenMP backend!");
}

// TODO: test constructors?!

template <typename T>
class OpenMPCSVMSolveSystemOfLinearEquations : public OpenMPCSVM {};
TYPED_TEST_SUITE(OpenMPCSVMSolveSystemOfLinearEquations, util::real_type_kernel_function_gtest, naming::real_type_kernel_function_to_name);

TYPED_TEST(OpenMPCSVMSolveSystemOfLinearEquations, solve_system_of_linear_equations_diagonal) {
    generic::test_solve_system_of_linear_equations<typename TypeParam::real_type, mock_openmp_csvm>(TypeParam::kernel_type);
}

template <typename T>
class OpenMPCSVMPredictValues : public OpenMPCSVM {};
TYPED_TEST_SUITE(OpenMPCSVMPredictValues, util::real_type_kernel_function_gtest, naming::real_type_kernel_function_to_name);

TYPED_TEST(OpenMPCSVMPredictValues, predict_values) {
    generic::test_predict_values<typename TypeParam::real_type, mock_openmp_csvm>(TypeParam::kernel_type);
}

template <typename T>
class OpenMPCSVMGenerateQ : public OpenMPCSVM {};
TYPED_TEST_SUITE(OpenMPCSVMGenerateQ, util::real_type_kernel_function_gtest, naming::real_type_kernel_function_to_name);

TYPED_TEST(OpenMPCSVMGenerateQ, generate_q) {
    using real_type = typename TypeParam::real_type;
    const plssvm::kernel_function_type kernel_type = TypeParam::kernel_type;

    // create parameter struct
    const plssvm::detail::parameter<real_type> params{ kernel_type, 2, 0.001, 1.0, 0.1 };

    // create the data that should be used
    const plssvm::data_set<real_type> data{ PLSSVM_TEST_FILE };

    // calculate correct q vector (ground truth)
    const std::vector<real_type> ground_truth = compare::generate_q(params, data.data());

    // create C-SVM: must be done using the mock class, since plssvm::openmp::csvm::generate_q is protected
    const mock_openmp_csvm svm;

    // calculate the q vector using the OpenMP backend
    const std::vector<real_type> calculated = svm.generate_q(params, data.data());

    // check the calculated result for correctness
    EXPECT_FLOATING_POINT_VECTOR_NEAR(ground_truth, calculated);
}

template <typename T>
class OpenMPCSVMCalculateW : public OpenMPCSVM {};
TYPED_TEST_SUITE(OpenMPCSVMCalculateW, util::real_type_gtest, naming::real_type_to_name);

TYPED_TEST(OpenMPCSVMCalculateW, calculate_w) {
    using real_type = TypeParam;

    // create the data that should be used
    const plssvm::data_set<real_type> support_vectors{ PLSSVM_TEST_FILE };
    const std::vector<real_type> weights = util::generate_random_vector<real_type>(support_vectors.num_data_points());

    // calculate the correct w vector
    const std::vector<real_type> ground_truth = compare::calculate_w(support_vectors.data(), weights);

    // create C-SVM: must be done using the mock class, since plssvm::openmp::csvm::calculate_w is protected
    const mock_openmp_csvm svm;

    // calculate the w vector using the OpenMP backend
    const std::vector<real_type> calculated = svm.calculate_w(support_vectors.data(), weights);

    // check the calculated result for correctness
    EXPECT_FLOATING_POINT_VECTOR_NEAR(ground_truth, calculated);
}

template <typename T>
class OpenMPCSVMRunDeviceKernel : public OpenMPCSVM {};
TYPED_TEST_SUITE(OpenMPCSVMRunDeviceKernel, util::real_type_kernel_function_gtest, naming::real_type_kernel_function_to_name);

TYPED_TEST(OpenMPCSVMRunDeviceKernel, run_device_kernel) {
    using real_type = typename TypeParam::real_type;
    const plssvm::kernel_function_type kernel_type = TypeParam::kernel_type;

    // create parameter struct
    const plssvm::detail::parameter<real_type> params{ kernel_type, 2, 0.001, 1.0, 0.1 };

    // create the data that should be used
    const plssvm::data_set<real_type> data{ PLSSVM_TEST_FILE };
    const std::vector<real_type> rhs = util::generate_random_vector<real_type>(data.num_data_points() - 1, real_type{ 1.0 }, real_type{ 2.0 });
    const std::vector<real_type> q = compare::generate_q(params, data.data());
    const real_type QA_cost = compare::kernel_function(params, data.data().back(), data.data().back()) + 1 / params.cost;

    // create C-SVM: must be done using the mock class, since plssvm::openmp::csvm::calculate_w is protected
    const mock_openmp_csvm svm;

    for (const real_type add : { real_type{ -1.0 }, real_type{ 1.0 } }) {
        // calculate the correct device function result
        const std::vector<real_type> ground_truth = compare::device_kernel_function(params, data.data(), rhs, q, QA_cost, add);

        // perform the kernel calculation on the device
        std::vector<real_type> calculated(data.num_data_points() - 1);
        svm.run_device_kernel(params, q, calculated, rhs, data.data(), QA_cost, add);

        // check the calculated result for correctness
        EXPECT_FLOATING_POINT_VECTOR_NEAR(ground_truth, calculated);
    }
}

template <typename T>
class OpenMPCSVMPredictAndScore : public OpenMPCSVM {};
TYPED_TEST_SUITE(OpenMPCSVMPredictAndScore, util::real_type_kernel_function_gtest, naming::real_type_kernel_function_to_name);

TYPED_TEST(OpenMPCSVMPredictAndScore, predict) {
    generic::test_predict<typename TypeParam::real_type, plssvm::openmp::csvm>(TypeParam::kernel_type);
}

TYPED_TEST(OpenMPCSVMPredictAndScore, score) {
    generic::test_score<typename TypeParam::real_type, plssvm::openmp::csvm>(TypeParam::kernel_type);
}