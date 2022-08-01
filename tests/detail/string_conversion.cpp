/**
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright 2018-today The PLSSVM project - All Rights Reserved
 * @license This file is part of the PLSSVM project which is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the sha256 implementation.
 */

#include "plssvm/detail/string_conversion.hpp"     // plssvm::detail::convert_to
#include "plssvm/detail/arithmetic_type_name.hpp"  // plssvm::detail::arithmetic_type_name

#include "../utility.hpp"  // EXPECT_THROW_WHAT

#include "fmt/format.h"   // fmt::format
#include "gtest/gtest.h"  // TEST, ASSERT_EQ, EXPECT_EQ, EXPECT_TRUE

#include <stdexcept>  // std::invalid_argument
#include <string>     // std::string
#include <vector>     // std::vector

template <typename T>
void check_convert_to(const std::vector<std::string_view> &input, const std::vector<T> &correct_output) {
    ASSERT_EQ(input.size(), correct_output.size());

    for (std::vector<std::string_view>::size_type i = 0; i < input.size(); ++i) {
        const T conv = plssvm::detail::convert_to<T>(input[i]);
        EXPECT_EQ(conv, correct_output[i]) << fmt::format(R"(input: "{}", output: "{}", correct: "{}")", input[i], conv, correct_output[i]);
    }
}

TEST(StringConversion, string_conversion) {
    using namespace plssvm::detail;

    std::vector<std::string_view> input = { "-3", "-1.5", "0.0", "1.5", "3", "   5", "  6 ", "7  " };
    std::vector<std::string_view> input_unsigned = { "0.0", "1.5", "3", "   5", "  6 ", "7  " };
    std::vector<std::string_view> input_char = { "0", "48", "65.2", "66", "122", "   119", "  120 ", "121  " };

    // boolean
    // std::from_chars seems to not support bool
    //    check_convert_to(input, std::vector<bool>{ true, true, false, true, true, true, true, true });

    // character types
    check_convert_to(input_char, std::vector<char>{ '\0', '0', 'A', 'B', 'z', 'w', 'x', 'y' });
    check_convert_to(input_char, std::vector<signed char>{ '\0', '0', 'A', 'B', 'z', 'w', 'x', 'y' });
    check_convert_to(input_char, std::vector<unsigned char>{ '\0', '0', 'A', 'B', 'z', 'w', 'x', 'y' });
    // std::from_chars seems to not support char16_t, char32_t, and wchar_t
    //    check_convert_to(input_char, std::vector<char16_t>{ '\0', '0', 'A', 'B', 'z', 'w', 'x', 'y' });
    //    check_convert_to(input_char, std::vector<char32_t>{ '\0', '0', 'A', 'B', 'z', 'w', 'x', 'y' });
    //    check_convert_to(input_char, std::vector<wchar_t>{ '\0', '0', 'A', 'B', 'z', 'w', 'x', 'y' });

    // integer types
    check_convert_to(input, std::vector<short>{ -3, -1, 0, 1, 3, 5, 6, 7 });
    check_convert_to(input_unsigned, std::vector<unsigned short>{ 0, 1, 3, 5, 6, 7 });
    check_convert_to(input, std::vector<int>{ -3, -1, 0, 1, 3, 5, 6, 7 });
    check_convert_to(input_unsigned, std::vector<unsigned int>{ 0, 1, 3, 5, 6, 7 });
    check_convert_to(input, std::vector<long>{ -3, -1, 0, 1, 3, 5, 6, 7 });
    check_convert_to(input_unsigned, std::vector<unsigned long>{ 0, 1, 3, 5, 6, 7 });
    check_convert_to(input, std::vector<long long>{ -3, -1, 0, 1, 3, 5, 6, 7 });
    check_convert_to(input_unsigned, std::vector<unsigned long long>{ 0, 1, 3, 5, 6, 7 });

    // floating point types
    check_convert_to(input, std::vector<float>{ -3.0f, -1.5f, 0.0f, 1.5f, 3.0f, 5.0f, 6.0f, 7.0f });
    check_convert_to(input, std::vector<double>{ -3.0, -1.5, 0.0, 1.5, 3.0, 5.0, 6.0, 7.0 });
    // fast_float::from_chars seems to not support long double
    //    check_convert_to(input, std::vector<long double>{ -3.0l, -1.5l, 0.0l, 1.5l, 3.0l, 5.0l, 6.0l, 7.0l });

    // std::string
    check_convert_to(input, std::vector<std::string>{ "-3", "-1.5", "0.0", "1.5", "3", "5", "6", "7" });
}

template <typename T>
class StringConversionException : public ::testing::Test {};
using string_conversion_exception_types = ::testing::Types<short, unsigned char, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double>;
TYPED_TEST_SUITE(StringConversionException, string_conversion_exception_types);

TYPED_TEST(StringConversionException, string_conversion_exception) {
    using namespace plssvm::detail;

    [[maybe_unused]] TypeParam res;
    EXPECT_THROW_WHAT(res = convert_to<TypeParam>("a"), std::runtime_error, fmt::format("Can't convert 'a' to a value of type {}!", arithmetic_type_name<TypeParam>()));
    EXPECT_THROW_WHAT(res = convert_to<TypeParam>("  abc 1"), std::runtime_error, fmt::format("Can't convert '  abc 1' to a value of type {}!", arithmetic_type_name<TypeParam>()));
    EXPECT_THROW_WHAT((res = convert_to<TypeParam, std::invalid_argument>("a")), std::invalid_argument, fmt::format("Can't convert 'a' to a value of type {}!", arithmetic_type_name<TypeParam>()));
}

class StringConversionExtract : public ::testing::TestWithParam<std::pair<std::string_view, int>> {};
TEST_P(StringConversionExtract, extract_first_integer_from_string) {
    auto [input, output] = GetParam();
    EXPECT_EQ(plssvm::detail::extract_first_integer_from_string<int>(input), output);
}
INSTANTIATE_TEST_SUITE_P(StringUtility, StringConversionExtract, ::testing::Values(std::make_pair("111", 111), std::make_pair("111 222", 111), std::make_pair("-111 222", 111), std::make_pair(" 111 222 333", 111), std::make_pair("abcd 111", 111), std::make_pair("abcd111 222", 111), std::make_pair("111_222", 111), std::make_pair("111 abcd 222", 111), std::make_pair("abc123def456", 123)));

TEST(StringConversion, extract_first_integer_from_string_exception) {
    [[maybe_unused]] int res;
    EXPECT_THROW_WHAT(res = plssvm::detail::extract_first_integer_from_string<int>("abc"), std::runtime_error, "String \"abc\" doesn't contain any integer!");
    EXPECT_THROW_WHAT(res = plssvm::detail::extract_first_integer_from_string<int>(""), std::runtime_error, "String \"\" doesn't contain any integer!");
}

template <typename T>
class StringConversionSplitAs : public ::testing::Test {};
using split_as_types = ::testing::Types<short, int, long, long long, float, double>;
TYPED_TEST_SUITE(StringConversionSplitAs, split_as_types);

TYPED_TEST(StringConversionSplitAs, split_default_delimiter) {
    // split string using the default delimiter
    const std::string string_to_split = "1.5 2.0 -3.5 4.0 5.0 -6.0 7.5";

    const std::vector<TypeParam> split_correct = { static_cast<TypeParam>(1.5), static_cast<TypeParam>(2.0), static_cast<TypeParam>(-3.5), static_cast<TypeParam>(4.0), static_cast<TypeParam>(5.0), static_cast<TypeParam>(-6.0), static_cast<TypeParam>(7.5) };
    const std::vector<TypeParam> split = plssvm::detail::split_as<TypeParam>(string_to_split);
    ASSERT_EQ(split.size(), split_correct.size());
    for (typename std::vector<TypeParam>::size_type i = 0; i < split_correct.size(); ++i) {
        EXPECT_EQ(split[i], split_correct[i]) << fmt::format("pos: {}, split: {}, correct: {}", i, split[i], split_correct[i]);
    }
}

TYPED_TEST(StringConversionSplitAs, split_custom_delimiter) {
    // split string using a custom delimiter
    const std::string string_to_split = "1.5,2.0,-3.5,4.0,5.0,-6.0,7.5";

    const std::vector<TypeParam> split_correct = { static_cast<TypeParam>(1.5), static_cast<TypeParam>(2.0), static_cast<TypeParam>(-3.5), static_cast<TypeParam>(4.0), static_cast<TypeParam>(5.0), static_cast<TypeParam>(-6.0), static_cast<TypeParam>(7.5) };
    const std::vector<TypeParam> split = plssvm::detail::split_as<TypeParam>(string_to_split, ',');
    ASSERT_EQ(split.size(), split_correct.size());
    for (typename std::vector<TypeParam>::size_type i = 0; i < split_correct.size(); ++i) {
        EXPECT_EQ(split[i], split_correct[i]) << fmt::format("pos: {}, split: {}, correct: {}", i, split[i], split_correct[i]);
    }
}

TYPED_TEST(StringConversionSplitAs, split_single_value) {
    // split string containing a single value
    const std::vector<TypeParam> split = plssvm::detail::split_as<TypeParam>("42");
    ASSERT_EQ(split.size(), 1);
    EXPECT_EQ(split.front(), static_cast<TypeParam>(42)) << fmt::format("split: {}, correct: {}", split.front(), static_cast<TypeParam>(42));
}

TYPED_TEST(StringConversionSplitAs, split_empty_string) {
    // split the empty string
    const std::vector<TypeParam> split = plssvm::detail::split_as<TypeParam>("");
    EXPECT_TRUE(split.empty());
}