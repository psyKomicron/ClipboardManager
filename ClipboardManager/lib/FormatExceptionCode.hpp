#pragma once
#include <cstdint>

namespace clip
{
    enum class FormatExceptionCode : uint32_t
    {
        Unknown = 0,
        MissingOpenBraces = 1,
        MissingClosingBraces = 1 << 1,
        InvalidFormat = 1 << 2,
        EmptyString = 1 << 3,
        BadBraceOrder = 1 << 4,
        ArgumentNotFound = 1 << 5,
        OutOfRangeFormatArgument = 1 << 6,
        InvalidFormatArgument = 1 << 7,
    };

    inline constexpr FormatExceptionCode operator|(const FormatExceptionCode& left, const FormatExceptionCode& right)
    {
        return static_cast<FormatExceptionCode>(static_cast<int32_t>(left) | static_cast<int32_t>(right));
    }

    inline constexpr FormatExceptionCode operator&(const FormatExceptionCode& left, const FormatExceptionCode& right)
    {
        return static_cast<FormatExceptionCode>(static_cast<int32_t>(left) & static_cast<int32_t>(right));
    }

    inline bool hasFlag(const FormatExceptionCode& left, const FormatExceptionCode& right)
    {
        return (left & right) == right;
    }
}
