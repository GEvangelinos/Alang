#ifndef ALPHA_BASICS_HPP
#define ALPHA_BASICS_HPP
#include <stack>
#include <vector>

#include "support/debug_tools.hpp"
#include "support/format_adapter.hpp"

namespace alpha
{
template <typename T>
using VectorStack = std::stack<T, std::vector<T>>;

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

    Immobile(const Immobile&) = delete;            ///< Disable copy-construction
    Immobile(Immobile&&) = delete;                 ///< Disable move-construction
    Immobile& operator=(const Immobile&) = delete; ///< Disable copy-assignment
    Immobile& operator=(Immobile&&) = delete;      ///< Disable move-assignment
};

class ToggleSwitch
{
public:
    ToggleSwitch() : state_(false) {}
    explicit ToggleSwitch(const bool init_state): state_(init_state) {}

    constexpr void enable() noexcept
    {
        DMASSERT(state_ == false);
        state_ = true;
    }

    constexpr void disable() noexcept
    {
        DMASSERT(state_ == true);
        state_ = false;
    }

    constexpr operator bool() const noexcept { return state_; }

private:
    bool state_ = false; // Initially the switch is off.
};


template <typename T>
class Once
{
public:
    explicit Once(const T& t) { set(t); }
    explicit Once(T&& t) { set(std::move(t)); }

    Once() = default; // No T is constructed here

    // variadic template to construct T in-place
    template <typename... Args>
    void set(Args&&... args)
    {
        if (assigned_) throw std::logic_error("BUG: Once already assigned");

        // Placement new: constructs T directly into `storage`
        new(&storage) T(std::forward<Args>(args)...);
        assigned_ = true;
    }

    operator const T&() const & { return get(); }

    const T& get() const
    {
        if (!assigned_) throw std::logic_error("BUG: Once not assigned");
        return *reinterpret_cast<const T*>(&storage);
    }

    ~Once() { if (assigned_) reinterpret_cast<T*>(&storage)->~T(); }

    const Once& operator=(const T& value)
    {
        set(value);
        return *this;
    }

    template <typename U = T>
    std::enable_if_t<std::is_pointer_v<U>, U> operator->() { return get(); }

    template <typename U = T>
    std::enable_if_t<std::is_pointer_v<U>, U> operator->() const { return get(); }

    // We prefer use addressof instead of & to avoid overloads on & operator.
    template <typename U = T>
    std::enable_if_t<!std::is_pointer_v<U>, U> operator->() { return std::addressof(get()); }

    // We prefer use addressof instead of & to avoid overloads on & operator.
    template <typename U = T>
    std::enable_if_t<!std::is_pointer_v<U>, U> operator->() const { return std::addressof(get()); }

    explicit operator bool() { return is_assigned(); }

    [[nodiscard]] bool is_assigned() const noexcept { return assigned_; }

private:
    alignas(T) std::byte storage[sizeof(T)];
    bool assigned_ = false;
};

template <typename T>
class DebugOnce : private Immobile
{
public:
    DebugOnce() = default;
    explicit DebugOnce(const T& t) { set(t); }
    explicit DebugOnce(T&& t) { set(std::move(t)); }

    ~DebugOnce() = default;

    void set(const T& value)
    {
        DEBUG(
            if (assigned_) throw std::logic_error(ATTACH_CONTEXT("BUG: `Once` already assigned"));
        )
        value_ = value;
        DEBUG(assigned_ = true;)
    }

    void set(T&& value)
    {
        DEBUG(
            if (assigned_) throw std::logic_error(ATTACH_CONTEXT("BUG: `Once` already assigned"));
        )
        value_ = std::move(value);
        DEBUG(assigned_ = true;)
    }

    operator const T&() const & { return get(); }

    const DebugOnce& operator=(const T& value)
    {
        set(value);
        return *this;
    }

    template <typename U = T>
    std::enable_if_t<std::is_pointer_v<U>, U> operator->() { return get(); }

    template <typename U = T>
    std::enable_if_t<std::is_pointer_v<U>, U> operator->() const { return get(); }

    // We prefer use addressof instead of  & to avoid overloads on & operator.
    template <typename U = T>
    std::enable_if_t<!std::is_pointer_v<U>, U> operator->() { return std::addressof(get()); }

    // We prefer use addressof instead of  & to avoid overloads on & operator.
    template <typename U = T>
    std::enable_if_t<!std::is_pointer_v<U>, U> operator->() const { return std::addressof(get()); }

    explicit operator bool() { return is_assigned(); }

    [[nodiscard]] const T& get() const
    {
        DEBUG(
            if (!assigned_) throw std::logic_error(ATTACH_CONTEXT("BUG: `Once` not assigned yet"));
        )
        return value_;
    }

    [[nodiscard]] bool is_assigned() const noexcept
    {
        #ifdef DEBUG_MODE
        return assigned_;
        #else
        UNREACHABLE("DebugOnce::is_assigned() is only available in DEBUG_MODE");
        #endif
    }

private:
    T value_;
    DEBUG(bool assigned_ = false;)
};

class OnceFlag : private Immobile
{
public:
    OnceFlag() = default;

    [[nodiscard]] bool is_raised() const noexcept { return raised_; }
    void raise() noexcept { raised_ = true; }

    constexpr operator bool() const noexcept { return is_raised(); }

private:
    bool raised_ = false;
};
} // namespace alpha

#endif //ALPHA_BASICS_HPP
