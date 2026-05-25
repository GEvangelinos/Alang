#ifndef MACHINE_HPP
#define MACHINE_HPP

#include "core/basics.hpp"
#include "core/numeric_types.hpp"

namespace alpha::vm
{
struct Bytes
{
    u64 count;

    static constexpr Bytes from_KB(const u64 kilobytes) noexcept
    {
        return Bytes{(1ULL << 10) * kilobytes};
    }

    static constexpr Bytes from_MB(const u64 megabytes) noexcept
    {
        return Bytes{(1ULL << 20) * megabytes};
    }

    static constexpr Bytes from_GB(const u64 gigabytes) noexcept
    {
        return Bytes{(1ULL << 30) * gigabytes};
    }

    static constexpr Bytes from_TB(const u64 terabytes) noexcept
    {
        return Bytes{(1ULL << 40) * terabytes};
    }

};

class Machine
{
    void set_stack_size(Bytes bytes);
    void init();

private:
    OnceFlag initialized;
    u64 stack_size = 0;
};
} // namespace alpha::vm

#endif //MACHINE_HPP
