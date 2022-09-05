/**
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for functions related to the default_value wrapper.
 */

#include "plssvm/default_value.hpp"

#include "plssvm/detail/utility.hpp"  // plssvm::detail::contains
#include "utility.hpp"                // util::{convert_to_string, convert_from_string}

#include "gtest/gtest.h"  // TEST, EXPECT_EQ, EXPECT_TRUE, EXPECT_FALSE

#include <sstream>  // std::istringstream
#include <vector>   // std::vector

//*************************************************************************************************************************************//
//                                                            default_init                                                             //
//*************************************************************************************************************************************//

template <typename T>
class DefaultInitDefault : public ::testing::Test {};
using default_init_default_types = ::testing::Types<short, unsigned char, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, std::string>;
TYPED_TEST_SUITE(DefaultInitDefault, default_init_default_types);

TYPED_TEST(DefaultInitDefault, default_construct) {
    using type = TypeParam;

    // check for correct default construction
    EXPECT_EQ(plssvm::default_init<type>{}.value, type{});
}

class DefaultInitExplicit : public ::testing::Test {};
class DefaultInitIntegral : public DefaultInitExplicit, public ::testing::WithParamInterface<int> {};
TEST_P(DefaultInitIntegral, explicit_construct) {
    const auto val = GetParam();

    // check for correct construction
    EXPECT_EQ(plssvm::default_init{ val }.value, val);
}
INSTANTIATE_TEST_SUITE_P(DefaultInitExplicit, DefaultInitIntegral, ::testing::Values(0, 1, 2, 3, 42, -1, -5));

class DefaultInitFloatingPoint : public DefaultInitExplicit, public ::testing::WithParamInterface<double> {};
TEST_P(DefaultInitFloatingPoint, explicit_construct) {
    const auto val = GetParam();

    // check for correct construction
    EXPECT_EQ(plssvm::default_init{ val }.value, val);
}
INSTANTIATE_TEST_SUITE_P(DefaultInitExplicit, DefaultInitFloatingPoint, ::testing::Values(0.0, 1.2, 2.5, 3.38748, 42.1, -1, -5.22));

class DefaultInitString : public DefaultInitExplicit, public ::testing::WithParamInterface<std::string> {};
TEST_P(DefaultInitString, explicit_construct) {
    const auto val = GetParam();

    // check for correct construction
    EXPECT_EQ(plssvm::default_init{ val }.value, val);
}
INSTANTIATE_TEST_SUITE_P(DefaultInitExplicit, DefaultInitString, ::testing::Values("", "foo", "bar", "baz", "Hello, World!"));

//*************************************************************************************************************************************//
//                                                            default_value                                                            //
//*************************************************************************************************************************************//

TEST(DefaultValue, default_init) {
    // create default_value
    plssvm::default_value val{ plssvm::default_init{ 42 } };

    // a default value has been assigned
    EXPECT_TRUE(val.is_default());
    EXPECT_EQ(val.value(), 42);
    EXPECT_EQ(val.get_default(), 42);
}

TEST(DefaultValue, assign_non_default) {
    // create default_value
    plssvm::default_value<double> val{};

    // must be default
    EXPECT_TRUE(val.is_default());

    // assign non-default value
    val = 3.1415;
    // value must now be a non-default value
    EXPECT_FALSE(val.is_default());
    EXPECT_EQ(val.value(), 3.1415);
    EXPECT_EQ(val.get_default(), 0.0);
}

TEST(DefaultValue, copy_construct_default) {
    // create first default_value
    const plssvm::default_value val1{ plssvm::default_init{ 3.1415 } };

    // copy-construct second default value
    const plssvm::default_value<int> val2{ val1 };

    // check for correct values
    EXPECT_TRUE(val2.is_default());
    EXPECT_EQ(val2.value(), 3);
    EXPECT_EQ(val2.get_default(), 3);
    // val1 must not have changed
    EXPECT_TRUE(val1.is_default());
    EXPECT_EQ(val1.value(), 3.1415);
    EXPECT_EQ(val1.get_default(), 3.1415);
}
TEST(DefaultValue, copy_construct_non_default) {
    // create first default_value
    plssvm::default_value<double> val1{};
    val1 = 3.1415;

    // copy-construct second default value
    const plssvm::default_value<int> val2{ val1 };

    // check for correct values
    EXPECT_FALSE(val2.is_default());
    EXPECT_EQ(val2.value(), 3);
    EXPECT_EQ(val2.get_default(), 0);
    // val1 must not have changed
    EXPECT_FALSE(val1.is_default());
    EXPECT_EQ(val1.value(), 3.1415);
    EXPECT_EQ(val1.get_default(), 0.0);
}

TEST(DefaultValue, move_construct_default) {
    // create first default_value
    plssvm::default_value val1{ plssvm::default_init<std::string>{ "Hello, World!" } };

    // copy-construct second default value
    const plssvm::default_value<std::string> val2{ std::move(val1) };

    // check for correct values
    EXPECT_TRUE(val2.is_default());
    EXPECT_EQ(val2.value(), "Hello, World!");
    EXPECT_EQ(val2.get_default(), "Hello, World!");
}
TEST(DefaultValue, move_construct_non_default) {
    // create first default_value
    plssvm::default_value<std::string> val1{};
    val1 = "foo bar baz";

    // copy-construct second default value
    const plssvm::default_value<std::string> val2{ std::move(val1) };

    // check for correct values
    EXPECT_FALSE(val2.is_default());
    EXPECT_EQ(val2.value(), "foo bar baz");
    EXPECT_EQ(val2.get_default(), "");
}

TEST(DefaultValue, copy_assign_default) {
    // create two default_values
    const plssvm::default_value val1{ plssvm::default_init{ 3.1415 } };
    plssvm::default_value val2{ plssvm::default_init{ 42 } };

    // copy-assign second default value
    val2 = val1;

    // check for correct values
    EXPECT_TRUE(val2.is_default());
    EXPECT_EQ(val2.value(), 3);
    EXPECT_EQ(val2.get_default(), 3);
    // val1 must not have changed!
    EXPECT_TRUE(val1.is_default());
    EXPECT_EQ(val1.value(), 3.1415);
    EXPECT_EQ(val1.get_default(), 3.1415);
}
TEST(DefaultValue, copy_assign_non_default) {
    // create two default_values
    plssvm::default_value val1{ plssvm::default_init{ 3.1415 } };
    val1 = 2.7182;
    plssvm::default_value val2{ plssvm::default_init{ 42 } };

    // copy-assign second default value
    val2 = val1;

    // check for correct values
    EXPECT_FALSE(val2.is_default());
    EXPECT_EQ(val2.value(), 2);
    EXPECT_EQ(val2.get_default(), 3);
    // val1 must not have changed!
    EXPECT_FALSE(val1.is_default());
    EXPECT_EQ(val1.value(), 2.7182);
    EXPECT_EQ(val1.get_default(), 3.1415);
}

TEST(DefaultValue, move_assign_default) {
    // create two default_values
    plssvm::default_value val1{ plssvm::default_init<std::string>{ "AAA" } };
    plssvm::default_value val2{ plssvm::default_init<std::string>{ "BBB" } };

    // copy-assign second default value
    val2 = std::move(val1);

    // check for correct values
    EXPECT_TRUE(val2.is_default());
    EXPECT_EQ(val2.value(), "AAA");
    EXPECT_EQ(val2.get_default(), "AAA");
}
TEST(DefaultValue, move_assign_non_default) {
    // create two default_values
    plssvm::default_value val1{ plssvm::default_init<std::string>{ "AAA" } };
    val1 = "CCC";
    plssvm::default_value val2{ plssvm::default_init<std::string>{ "BBB" } };

    // copy-assign second default value
    val2 = val1;

    // check for correct values
    EXPECT_FALSE(val2.is_default());
    EXPECT_EQ(val2.value(), "CCC");
    EXPECT_EQ(val2.get_default(), "AAA");
}

TEST(DefaultValue, value_default) {
    // create default_value
    const plssvm::default_value val{ plssvm::default_init{ 42 } };

    // check for correct value
    EXPECT_EQ(val.value(), 42);
}
TEST(DefaultValue, value_non_default) {
    // create default_value
    plssvm::default_value val{ plssvm::default_init<std::string>{ "AAA" } };
    val = "BBB";

    // check for correct values
    EXPECT_EQ(val.value(), "BBB");
}

TEST(DefaultValue, implicit_conversion_default) {
    // create default_value
    const plssvm::default_value val{ plssvm::default_init{ 42 } };

    // check for correct value
    EXPECT_EQ(static_cast<int>(val), 42);
}
TEST(DefaultValue, implicit_conversion_non_default) {
    // create default_value
    plssvm::default_value val{ plssvm::default_init<std::string>{ "AAA" } };
    val = "BBB";

    // check for correct values
    EXPECT_EQ(static_cast<std::string>(val), "BBB");
}

TEST(DefaultValue, get_default_default) {
    // create default_value
    const plssvm::default_value val{ plssvm::default_init{ 42 } };

    // default_init -> must be default
    EXPECT_EQ(val.get_default(), 42);
}
TEST(DefaultValue, get_default_non_default) {
    // create default_value
    plssvm::default_value val{ plssvm::default_init{ "Hello, World!" } };
    val = "foo bar baz";

    // default value overwritten -> must not be default
    EXPECT_EQ(val.get_default(), "Hello, World!");
}

TEST(DefaultValue, is_default_default) {
    // create default_value
    const plssvm::default_value val{ plssvm::default_init{ 42 } };

    // default_init -> must be default
    EXPECT_TRUE(val.is_default());
}
TEST(DefaultValue, is_default_non_default) {
    // create default_value
    plssvm::default_value val{ plssvm::default_init{ "Hello, World!" } };
    val = "foo bar baz";

    // default value overwritten -> must not be default
    EXPECT_FALSE(val.is_default());
}

TEST(DefaultValue, swap_member_function) {
    // create two default_values
    plssvm::default_value val1{ plssvm::default_init{ 1 } };
    plssvm::default_value val2{ plssvm::default_init{ 2 } };
    val2 = 3;

    // check values before swap
    EXPECT_TRUE(val1.is_default());
    EXPECT_EQ(val1.value(), 1);
    EXPECT_EQ(val1.get_default(), 1);
    EXPECT_FALSE(val2.is_default());
    EXPECT_EQ(val2.value(), 3);
    EXPECT_EQ(val2.get_default(), 2);

    // swap contents
    val1.swap(val2);

    // check if contents were correctly swapped
    EXPECT_FALSE(val1.is_default());
    EXPECT_EQ(val1.value(), 3);
    EXPECT_EQ(val1.get_default(), 2);
    EXPECT_TRUE(val2.is_default());
    EXPECT_EQ(val2.value(), 1);
    EXPECT_EQ(val2.get_default(), 1);
}

TEST(DefaultValue, reset_default) {
    // create default_value
    plssvm::default_value val{ plssvm::default_init{ 42 } };

    // reset value
    val.reset();

    // check values
    EXPECT_TRUE(val.is_default());
    EXPECT_EQ(val.value(), 42);
    EXPECT_EQ(val.get_default(), 42);
}
TEST(DefaultValue, reset_non_default) {
    // create default_value
    plssvm::default_value val{ plssvm::default_init{ 42 } };
    val = 64;

    // reset value
    val.reset();

    // check values
    EXPECT_TRUE(val.is_default());
    EXPECT_EQ(val.value(), 42);
    EXPECT_EQ(val.get_default(), 42);
}

TEST(DefaultValue, to_string) {
    // check conversions to std::string
    EXPECT_EQ(util::convert_to_string(plssvm::default_value{ plssvm::default_init{ 1 } }), "1");
    EXPECT_EQ(util::convert_to_string(plssvm::default_value{ plssvm::default_init{ 3.1415 } }), "3.1415");
    EXPECT_EQ(util::convert_to_string(plssvm::default_value{ plssvm::default_init{ -4 } }), "-4");
    EXPECT_EQ(util::convert_to_string(plssvm::default_value{ plssvm::default_init{ "Hello, World!" } }), "Hello, World!");
}
TEST(DefaultValue, from_string) {
    // check conversion from std::string
    plssvm::default_value<int> val1{};
    val1 = 1;
    EXPECT_EQ(util::convert_from_string<plssvm::default_value<int>>("1"), val1);
    EXPECT_FALSE(val1.is_default());
    EXPECT_EQ(val1.get_default(), 0);

    plssvm::default_value<double> val2{};
    val2 = 3.1415;
    EXPECT_EQ(util::convert_from_string<plssvm::default_value<double>>("3.1415"), val2);
    EXPECT_FALSE(val2.is_default());
    EXPECT_EQ(val2.get_default(), 0);

    plssvm::default_value<int> val3{ plssvm::default_init{ 42 } };
    val3 = -4;
    EXPECT_EQ(util::convert_from_string<plssvm::default_value<int>>("-4"), val3);
    EXPECT_FALSE(val3.is_default());
    EXPECT_EQ(val3.get_default(), 42);

    plssvm::default_value<std::string> val4{};
    val4 = "foo";
    EXPECT_EQ(util::convert_from_string<plssvm::default_value<std::string>>("foo"), val4);
    EXPECT_FALSE(val4.is_default());
    EXPECT_EQ(val4.get_default(), std::string{ "" });
}

TEST(DefaultValue, swap_free_function) {
    // create two default_values
    plssvm::default_value val1{ plssvm::default_init{ 1 } };
    plssvm::default_value val2{ plssvm::default_init{ 2 } };
    val2 = 3;

    // check values before swap
    EXPECT_TRUE(val1.is_default());
    EXPECT_EQ(val1.value(), 1);
    EXPECT_EQ(val1.get_default(), 1);
    EXPECT_FALSE(val2.is_default());
    EXPECT_EQ(val2.value(), 3);
    EXPECT_EQ(val2.get_default(), 2);

    // swap contents
    std::swap(val1, val2);

    // check if contents were correctly swapped
    EXPECT_FALSE(val1.is_default());
    EXPECT_EQ(val1.value(), 3);
    EXPECT_EQ(val1.get_default(), 2);
    EXPECT_TRUE(val2.is_default());
    EXPECT_EQ(val2.value(), 1);
    EXPECT_EQ(val2.get_default(), 1);
}