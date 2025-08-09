#ifndef ALPHA_BASICS_HPP
#define ALPHA_BASICS_HPP
#include "utils/debug_tools.hpp"
#include "utils/format_adapter.hpp"

namespace alpha
{
/// @brief A mixin that prevents copying, moving, or reassigning derived classes.
/// @details
///   Inherit from this class to make your type “immobile”:
///   once constructed, instances cannot be copied, moved, or reassigned.
///   Note that this does *not* freeze internal state—mutable members
///   or methods marked `const` may still modify their own data.
///
/// @note
///   Copy-construction, move-construction, copy-assignment, and move-assignment
///   are all deleted. Derived types remain fully mutable through their own
///   member functions.
class Immobile
{
public:
    Immobile() = default;

    ~Immobile() = default;

    Immobile(const Immobile &) = delete;            ///< Disable copy-construction
    Immobile(Immobile &&) = delete;                 ///< Disable move-construction
    Immobile &operator=(const Immobile &) = delete; ///< Disable copy-assignment
    Immobile &operator=(Immobile &&) = delete;      ///< Disable move-assignment
};

class ToggleSwitch
{
public:
    void enable() noexcept
    {
        DEBUG_SMART_ASSERT(is_disabled());
        state_ = true;
    }

    void disable() noexcept
    {
        DEBUG_SMART_ASSERT(is_enabled());
        state_ = false;
    }

    [[nodiscard]] bool is_enabled() const noexcept { return state_; }
    [[nodiscard]] bool is_disabled() const noexcept { return !state_; }

private:
    bool state_ = false; // Initially the switch is off.
};

template<typename T>
class Once : private Immobile
{
public:
    Once() = default;

    ~Once() = default;

    void set(T value)
    {
        if (assigned_)
            throw std::logic_error(ATTACH_CONTEXT("BUG: `Once` already assigned"));
        value_ = value;
        assigned_ = true;
    }

    template<typename U = T>
    std::enable_if_t<std::is_pointer_v<U>, U> operator->() { return get(); }

    template<typename U = T>
    std::enable_if_t<std::is_pointer_v<U>, U> operator->() const { return get(); }

    // We prefer use addressof instead of  & to avoid overloads on & operator.
    template<typename U = T>
    std::enable_if_t<!std::is_pointer_v<U>, U> operator->() { return std::addressof(get()); }

    // We prefer use addressof instead of  & to avoid overloads on & operator.
    template<typename U = T>
    std::enable_if_t<!std::is_pointer_v<U>, U> operator->() const { return std::addressof(get()); }

    explicit operator bool() { return assigned(); }

    [[nodiscard]] const T &get() const
    {
        if (!assigned_)
            throw std::logic_error(ATTACH_CONTEXT("BUG: `Once` not assigned yet"));
        return value_;
    }

    [[nodiscard]] bool assigned() const noexcept { return assigned_; }

private:
    T value_;
    bool assigned_ = false;
};
} // namespace alpha

#endif //ALPHA_BASICS_HPP
