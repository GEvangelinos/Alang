#ifndef BLOCK_MANAGER_HPP
#define BLOCK_MANAGER_HPP

#include "semantic_subsystem.hpp"
#include "L1_driver/semantic_system_dispatcher_dsl.hpp"

namespace alpha
{
class BlockManager
{
    friend class SemanticSystem;

private:
    class Restricted final : private SemanticSubsystem
    {
        friend class BlockManager;

    private:
        explicit Restricted(const SemanticSystemServices &ss_services);

        void enter_block() noexcept;
        void exit_block() noexcept;

        [[nodiscard ]] static BlockSourceLocation
        make_block_location(SourceLocation begin_loc, SourceLocation end_loc) noexcept;
    };

    Restricted DISPATCH_TARGET;

    explicit BlockManager(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(enter_block);
    DISPATCH_SLAVE_METHOD_CALL(exit_block);
    DISPATCH_SLAVE_METHOD_CALL(make_block_location);
    DISPATCH_DEFINE_HANDLER_END();
};

inline void
BlockManager::Restricted::enter_block() noexcept { parse_ctx_->scope_handler.enter_scope(); }

inline void
BlockManager::Restricted::exit_block() noexcept
{
    symbol_table_->hide_scope_symbols(parse_ctx_->scope_handler.scope());
    parse_ctx_->scope_handler.exit_scope();
}

inline BlockSourceLocation
BlockManager::Restricted::make_block_location(
    const SourceLocation begin_loc,
    const SourceLocation end_loc) noexcept
{
    return BlockSourceLocation{
        .begin_raw_loc = SourceLocationRaw{
            .begin = begin_loc.begin.value,
            .end = begin_loc.end.value,
        },
        .end_raw_loc = SourceLocationRaw{
            .begin = end_loc.begin.value,
            .end = end_loc.end.value,
        },
    };
}
} // namespace alpha
#endif //BLOCK_MANAGER_HPP
