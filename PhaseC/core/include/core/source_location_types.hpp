#ifndef SOURCE_LOCATION_TYPES_HPP
#define SOURCE_LOCATION_TYPES_HPP

#include "core/numeric_types.hpp" // for word

namespace alpha
{
#ifdef DEFINE_SRC_IDX_TYPE
#error "Macro collision detected"
#endif
#define DEFINE_SRC_IDX_TYPE_STRUCT(StructName)                                      \
    struct StructName                                                               \
    {                                                                               \
        using UnderlyingType = Word;                                                \
        static_assert(std::is_unsigned_v<UnderlyingType>);                          \
                                                                                    \
        static constexpr UnderlyingType none = 0;                                   \
        UnderlyingType value;                                                       \
                                                                                    \
        constexpr StructName() : value(none)                                     {} \
        explicit constexpr StructName(const UnderlyingType value) : value(value) {} \
        constexpr StructName(const StructName & rhs) : value(rhs.value) {}          \
        constexpr auto operator<=>(const StructName &rhs) const = default;          \
        constexpr StructName &operator=(const StructName &rhs);                     \
        constexpr StructName &operator+=(const StructName &rhs);                    \
        constexpr StructName &operator-=(const StructName &rhs);                    \
        constexpr StructName &operator++();                                         \
        constexpr StructName operator++(int);                                       \
        constexpr StructName &operator--();                                         \
        constexpr StructName operator--(int);                                       \
    };

DEFINE_SRC_IDX_TYPE_STRUCT(SrcLineIdx)
DEFINE_SRC_IDX_TYPE_STRUCT(SrcColumnIdx)
DEFINE_SRC_IDX_TYPE_STRUCT(SrcBufferIdx)
#undef DEFINE_SRC_IDX_TYPE_STRUCT

#ifdef DEFINE_SRC_IDX_TYPE_METHOD_IMPLS
  #error "Macro collision detected"
#endif
#define DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(StructName)                         \
    constexpr StructName &StructName::operator=(const StructName &rhs)       \
    {                                                                        \
        value = rhs.value;                                                   \
        return *this;                                                        \
    }                                                                        \
                                                                             \
    constexpr StructName &StructName::operator+=(const StructName &rhs)      \
    {                                                                        \
        value += rhs.value;                                                  \
        return *this;                                                        \
    }                                                                        \
                                                                             \
    constexpr StructName &StructName::operator-=(const StructName &rhs)      \
    {                                                                        \
        value -= rhs.value;                                                  \
        return *this;                                                        \
    }                                                                        \
                                                                             \
    constexpr StructName &StructName::operator++()                           \
    {                                                                        \
        ++value;                                                             \
        return *this;                                                        \
    }                                                                        \
                                                                             \
    constexpr StructName StructName::operator++(int)                         \
    {                                                                        \
        auto tmp = *this;                                                    \
        ++*this;                                                             \
        return tmp;                                                          \
    }                                                                        \
                                                                             \
    constexpr StructName &StructName::operator--()                           \
    {                                                                        \
        --value;                                                             \
        return *this;                                                        \
    }                                                                        \
                                                                             \
    constexpr StructName StructName::operator--(int)                         \
    {                                                                        \
        auto tmp = *this;                                                    \
        --*this;                                                             \
        return tmp;                                                          \
    }

DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(SrcLineIdx)
DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(SrcColumnIdx)
DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(SrcBufferIdx)
#undef DEFINE_SRC_IDX_TYPE_METHOD_IMPLS
} // namespace alpha
#endif // SOURCE_LOCATION_TYPES_HPP
