#include "ir_postprocess/ir_optimizer.hpp"

#include <ir_opcode_info_traits.gen.hpp>
#include <ranges>
#include <unordered_map>
#include "core/ir/ir_expr.hpp"
#include "ir_editor.hpp"

namespace alpha
{
struct FuncMetadata
{
    ir::QuadStream::size_type begin_idx;
    ir::QuadStream::size_type end_idx;
    bool used;
};

using FuncMetadataMap = std::unordered_map<const ProgFuncSymbol*, FuncMetadata>;

[[nodiscard]] static FuncMetadataMap
discover_function_boundaries(const ir::QuadStream& qstream)
{
    namespace IRIT = ir::info_traits;
    static_assert(IRIT::arg1(ir::Opcode::FUNCSTART) == IRIT::Requirement::REQUIRED, "for funcname");
    static_assert(IRIT::arg1(ir::Opcode::FUNCEND) == IRIT::Requirement::REQUIRED, "for funcname");

    std::unordered_map<const ProgFuncSymbol*, FuncMetadata> metadata_map;
    for (ir::QuadStream::size_type i = 0; i < qstream.size(); ++i)
    {
        const ir::Quad& q = qstream[i];
        if (q.opcode == ir::Opcode::FUNCSTART)
        {
            DMASSERT(q.arg1 && q.arg1->type == Expr::Type::PROGRAM_FUNCTION);
            const auto* const func_sym = static_cast<const ProgFuncExpr*>(q.arg1)->progfunc_symbol;
            DMASSERT(!metadata_map.contains(func_sym) && "Function symbol must be unique");
            metadata_map.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(func_sym),
                std::forward_as_tuple(i)
            );
        }
        else if (q.opcode == ir::Opcode::FUNCEND)
        {
            DMASSERT(q.arg1 && q.arg1->type == Expr::Type::PROGRAM_FUNCTION);
            const auto* const func_sym = static_cast<const ProgFuncExpr*>(q.arg1)->progfunc_symbol;
            DMASSERT(metadata_map.contains(func_sym) && "FUNCEND means FUNCSTART existed.");
            FuncMetadata& metadata = metadata_map[func_sym];
            metadata.end_idx = i;
            DMASSERT(metadata.begin_idx < metadata.end_idx);
        }
    }
    return metadata_map;
}

static void
mark_used_functions(const ir::QuadStream& qstream, FuncMetadataMap& func_metadata_map)
{
    for (ir::QuadStream::size_type i = 0; i < qstream.size(); ++i)
    {
        const ir::Quad& q = qstream[i];
        if (q.opcode == ir::Opcode::FUNCSTART || q.opcode == ir::Opcode::FUNCEND)
            continue;

        const auto mark_potential_function_usage = [&func_metadata_map](const Expr* const arg)
        {
            if (arg && arg->type == Expr::Type::PROGRAM_FUNCTION)
            {
                const auto* const func_sym = static_cast<const ProgFuncExpr*>(arg)->progfunc_symbol;
                DMASSERT(func_metadata_map.contains(func_sym));
                func_metadata_map[func_sym].used = true;
            }
        };
        DMASSERT(!q.result || q.result->type != Expr::Type::PROGRAM_FUNCTION);
        mark_potential_function_usage(q.arg1);
        mark_potential_function_usage(q.arg2);
    }
}

[[nodiscard]] static bool
remove_unused_functions(ir::QuadStream& qstream, FuncMetadataMap& func_metadata_map)
{
    IREditor ir_editor{qstream};
    for (const FuncMetadata& func_info : func_metadata_map | std::views::values)
    {
        if (func_info.used)
            continue;
        for (ir::QuadStream::size_type i = func_info.begin_idx; i <= func_info.end_idx; ++i)
            ir_editor.kill(i);
    }
    return ir_editor.apply();
}

bool
IROptimizer::do_dead_func_elimination(ir::QuadStream& qstream)
{
    FuncMetadataMap func_metadata_map = discover_function_boundaries(qstream);
    mark_used_functions(qstream, func_metadata_map);
    const bool changed_qstream = remove_unused_functions(qstream, func_metadata_map);
    return changed_qstream;
}
} // namespace alpha
