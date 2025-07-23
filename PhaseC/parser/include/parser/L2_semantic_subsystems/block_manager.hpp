#ifndef BLOCK_MANAGER_HPP
#define BLOCK_MANAGER_HPP
#include "semantic_subsystem.hpp"
#include "L1_driver/semantic_system_dispatcher_dsl.hpp"

namespace Alpha
{
class BlockManager final : private SemanticSubsystem
{
public:
    explicit BlockManager(const SemanticSystemServices &ss_services);

    DISPATCH_DECLARE_HANDLER();

private:
    void enter_block() noexcept;
    void exit_block() noexcept;

    BlockSourceLocation make_block_location(
        SourceLocation begin, SourceLocation end) const noexcept;
};

inline BlockManager::BlockManager(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

DISPATCH_DEFINE_HANDLER_BEGIN(BlockManager);
    DISPATCH_BEGIN_CALLS();
    DISPATCH_SLAVE_METHOD_CALL(enter_block);
    DISPATCH_SLAVE_METHOD_CALL(exit_block);
    DISPATCH_SLAVE_METHOD_CALL(make_block_location);
    DISPATCH_END_CALLS();
DISPATCH_DEFINE_HANDLER_END(BlockManager);

inline void BlockManager::enter_block() noexcept { parse_ctx_->scope_handler.enter_scope(); }

inline void BlockManager::exit_block() noexcept
{
    symbol_table_->hide_scope_symbols(parse_ctx_->scope_handler.scope());
    parse_ctx_->scope_handler.exit_scope();
}

inline BlockSourceLocation
BlockManager::make_block_location(
    const SourceLocation begin,
    const SourceLocation end) noexcept
{
    return {
        .begin = begin,
        .end = end,
    };
}
} // namespace Alpha
#endif //BLOCK_MANAGER_HPP
