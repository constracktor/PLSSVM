/**
 * @file
 * @author Alexander Van Craen
 * @author Marcel Breyer
 * @copyright
 *
 * @brief Implements utility functions for string manipulation and conversion.
 */

#pragma once

#include "plssvm/detail/arithmetic_type_name.hpp"  // plssvm::detail::arithmetic_type_name
#include "plssvm/detail/utility.hpp"               // plssvm::detail::always_false_v

#include "fast_float/fast_float.h"  // fast_float::from_chars_result, fast_float::from_chars (floating point types)
#include "fmt/core.h"               // fmt::format

#include <algorithm>     // std::min, std::transform
#include <cctype>        // std::tolower
#include <charconv>      // std::from_chars_result, std::from_chars (integral types)
#include <stdexcept>     // std::runtime_error
#include <string>        // std::char_traits
#include <string_view>   // std::string_view
#include <system_error>  // std:errc
#include <type_traits>   // std::is_floating_point_v, std::is_integral_v

namespace plssvm::detail {

/**
 * @brief Checks if the string @p str starts with the prefix @p sv.
 * @param[in] str the string to check
 * @param[in] sv the string to match against the start of @p str
 * @return `true` if @p str starts with the string @p sv, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] inline bool starts_with(const std::string_view str, const std::string_view sv) noexcept {
    return str.substr(0, sv.size()) == sv;
}
/**
 * @brief Checks if the string @p str starts with the character @p c.
 * @param[in] str the string to check
 * @param[in] c the character to match against the first character of @p str
 * @return `true` if @p str starts with the character @p c, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] inline bool starts_with(const std::string_view str, const char c) noexcept {
    return !str.empty() && std::char_traits<char>::eq(str.front(), c);
}
/**
 * @brief Checks if the string @p str ends with the suffix @p sv.
 * @param[in] str the string to check
 * @param[in] sv the string to match against the end of @p str
 * @return `true` if @p str ends with the string @p sv, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] inline bool ends_with(const std::string_view str, const std::string_view sv) noexcept {
    return str.size() >= sv.size() && str.compare(str.size() - sv.size(), std::string_view::npos, sv) == 0;
}
/**
 * @brief Checks if the string @p str ends with the character @p c.
 * @param[in] str the string to check
 * @param[in] c the character to match against the last character of @p str
 * @return `true` if @p str ends with the character @p c, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] inline bool ends_with(const std::string_view str, const char c) noexcept {
    return !str.empty() && std::char_traits<char>::eq(str.back(), c);
}
/**
 * @brief Checks if the string @p str contains the string @p sv.
 * @param[in] str the string to check
 * @param[in] sv the string to find
 * @return `true` if @p str contains the string @p sv, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] inline bool contains(const std::string_view str, const std::string_view sv) noexcept {
    return str.find(sv) != std::string_view::npos;
}
/**
 * @brief Checks if the string @p str contains the character @p c.
 * @param[in] str the string to check
 * @param[in] c the character to find
 * @return `true` if @p str contains the character @p c, otherwise `false` (`[[nodiscard]]`)
 */
[[nodiscard]] inline bool contains(const std::string_view str, const char c) noexcept {
    return str.find(c) != std::string_view::npos;
}

/**
 * @brief Returns a new [`std::string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view) equal to @p str where all leding whitespaces are removed.
 * @param[in] str the string to remove the leading whitespaces
 * @return the string @p str without leading whitespace (`[[nodiscard]]`)
 */
[[nodiscard]] inline std::string_view trim_left(const std::string_view str) noexcept {
    std::string_view::size_type pos = std::min(str.find_first_not_of(' '), str.size());
    return str.substr(pos);
}

/**
 * @brief Converts the string @p str to a value of type @p T.
 * @details If @p T is an integral type [`std::from_chars`](https://en.cppreference.com/w/cpp/utility/from_chars) is used,
 *          if @p T is a floating point type [`float_fast::from_chars`](https://github.com/fastfloat/fast_float) is used
 *          and if @p T is neither of both, an exception is thrown.
 * @tparam T the type to convert the value of @p str to, must be an arithmetic type
 * @tparam Exception the exception type to throw in case that @p str can't be converted to a value of @p T
 *         (default: [`std::runtime_error`](https://en.cppreference.com/w/cpp/error/runtime_error)).
 * @param[in] str the string to convert
 * @throws Exception if @p str can't be converted to a value of type @p T
 * @return the value of type @p T denoted by @p str (`[[nodiscard]]`)
 */
template <typename T, typename Exception = std::runtime_error>
[[nodiscard]] inline T convert_to(std::string_view str) {
    // select conversion function depending on the provided type
    const auto convert_from_chars = [](const std::string_view sv, auto &val) {
        if constexpr (std::is_floating_point_v<T>) {
            // convert the string to a floating point value
            return fast_float::from_chars(sv.data(), sv.data() + sv.size(), val);
        } else if constexpr (std::is_integral_v<T>) {
            // convert the string to an integral value
            return std::from_chars(sv.data(), sv.data() + sv.size(), val);
        } else {
            // can't convert the string to a non-arithmetic type
            static_assert(always_false_v<T>, "Can only convert arithmetic types!");
        }
    };

    // remove leading whitespaces
    str = trim_left(str);

    // convert string to value fo type T
    T val;
    auto res = convert_from_chars(str, val);
    if (res.ec != std::errc{}) {
        throw Exception{ fmt::format("Can't convert '{}' to a value of type {}!", str, plssvm::detail::arithmetic_type_name<T>()) };
    }
    return val;
}

/**
 * @brief Replaces all occurrences of @p what with @p with in the string @p str.
 * @param[inout] str the string to replace the values
 * @param[in] what the string to replace
 * @param[in] with the string to replace with
 */
inline void replace_all(std::string &str, const std::string_view what, const std::string_view with) {
    for (std::string::size_type pos = 0; std::string::npos != (pos = str.find(what.data(), pos, what.length())); pos += with.length()) {
        str.replace(pos, what.length(), with.data(), with.length());
    }
}

/**
 * @brief Convert the string @p str to its all lower case representation.
 * @param[inout] str the string to transform
 * @return the transformed string
 */
inline std::string &to_lower_case(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), [](const char c) { return std::tolower(c); });
    return str;
}

}  // namespace plssvm::detail