#ifndef BIDIRECTIONAL_REGISTRY_HPP
#define BIDIRECTIONAL_REGISTRY_HPP

#include <concepts>
#include <optional>
#include <unordered_map>
#include <vector>
#include "debug_tools.hpp"

namespace alpha::support
{
template <typename T>
concept Hashable = requires(T a)
{
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template <Hashable KeyType, typename IdType>
    requires std::is_integral_v<IdType> && std::is_unsigned_v<IdType>
class BidirectionalRegistry

{
public:
    [[nodiscard]] IdType reg(const KeyType& content)
    {
        DMASSERT(id_map.size() == value_registry.size());
        const IdType new_id = id_map.size();
        const auto [it, inserted] = id_map.try_emplace(content, new_id);
        if (inserted)
        {
            value_registry.emplace_back(content);
            DMASSERT(value_registry[new_id] == content);
        }
        return it->second;
    }

    [[nodiscard]] const KeyType* get_by_id(const IdType id) const
    {
        const auto index = static_cast<std::size_t>(id);
        return index < value_registry.size() ? &value_registry[index] : nullptr;
    }

    [[nodiscard]] std::optional<IdType> get_id_of(const KeyType& content) const
    {
        if (const auto it = id_map.find(content); it != id_map.end())
            return it->second;
        return std::nullopt;
    }

    [[nodiscard]] std::size_t size() const noexcept { return id_map.size(); }

    [[nodiscard]] const auto& from_key_view() const noexcept { return id_map; }
    [[nodiscard]] const auto& from_value_view() const noexcept { return value_registry; }

private:
    std::unordered_map<KeyType, IdType> id_map;
    std::vector<KeyType> value_registry;
};
} // namespace alpha::support
#endif //BIDIRECTIONAL_REGISTRY_HPP
