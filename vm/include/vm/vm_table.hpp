#ifndef VM_TABLE_HPP
#define VM_TABLE_HPP

#include <unordered_map>
#include "vm_memcell.hpp"

namespace alpha::vm
{
struct HashTable
{
    // TODO: use a flat hashmap, currently there is a lot of pointer chasing.
    std::unordered_map<vm::Memcell, vm::Memcell> data;
    u32 ref_counter;

    void increase_ref() noexcept { ++ref_counter; }
    void decrease_ref() noexcept { --ref_counter; }
};

struct Table
{
    HashTable str_indexed;
    HashTable int_indexed;
    HashTable float_indexed;
    HashTable progfuncs;
    HashTable libfuncs;
};
} // namespace alpha::vm

#endif // VM_TABLE_HPP
