#include "ir_postprocess/ir_optimizer.hpp"

namespace alpha
{
namespace
{
using namespace alpha;

class FunctionHoister
{
public:
    [[nodiscard]] static ir::QuadStream hoist(const ir::QuadStream& unhoisted_stream);

private:
    struct SourcedQuad
    {
        ir::Quad quad;
        u64 original_index;
    };

    static constexpr u64 k_tombstone = std::numeric_limits<u64>::max();

    using SourcedQuadStream = std::vector<SourcedQuad>;
    VectorStack<SourcedQuadStream> open_qstreams;
    std::vector<SourcedQuadStream> closed_qstreams;
    std::vector<u64> translation_map_;
    ir::QuadStream hoisted_stream;

    FunctionHoister();

    void unnest_functions(const ir::QuadStream& qstream);
    void linearize_to_single_qstream(const ir::QuadStream& qstream);
    void resolve_tombstones(const ir::QuadStream& qstream);
    void patch_labels(const ir::QuadStream& qstream);
};

FunctionHoister::FunctionHoister()
{
    open_qstreams.emplace(); /* Initialize global scope */
}

void
FunctionHoister::unnest_functions(const ir::QuadStream& qstream)
{
    for (ir::QuadStream::size_type i = 0; i < qstream.size(); ++i)
    {
        const ir::Quad& q = qstream[i];

        // Before function hoisting this JUMP is necessary to bypass over the function's code.
        // After function hoisting this JUMP is obsolete dead code.
        const bool is_bypass_function_jump =
            q.opcode == ir::Opcode::JUMP &&
            i + 1 < qstream.size() &&
            qstream[i + 1].opcode == ir::Opcode::FUNCSTART;

        if (is_bypass_function_jump)
            continue;

        if (q.opcode == ir::Opcode::FUNCSTART)
        {
            open_qstreams.emplace(); // Push clean frame for nested function
            open_qstreams.top().emplace_back(q, i);
            continue;
        }

        if (q.opcode == ir::Opcode::FUNCEND)
        {
            open_qstreams.top().emplace_back(q, i);
            closed_qstreams.push_back(std::move(open_qstreams.top()));
            open_qstreams.pop();
            continue;
        }

        DMASSERT(!open_qstreams.empty());
        SourcedQuadStream& current_open_qstream = open_qstreams.top();
        current_open_qstream.emplace_back(q, i);
    }

    DMASSERT(!open_qstreams.empty() && "First open scope was added in constructor");
    closed_qstreams.push_back(std::move(open_qstreams.top()));
    open_qstreams.pop();
}

void
FunctionHoister::linearize_to_single_qstream(const ir::QuadStream& qstream)
{
    hoisted_stream.reserve(qstream.size());

    // Initialize map with sentinel values to detect deleted/tombstone quads
    translation_map_.resize(qstream.size(), FunctionHoister::k_tombstone);

    for (SourcedQuadStream& scope : closed_qstreams)
        for (SourcedQuad& sq : scope)
        {
            const u64 new_index = hoisted_stream.size();
            translation_map_[sq.original_index] = new_index;
            hoisted_stream.push_back(std::move(sq.quad));
        }
}

void
FunctionHoister::resolve_tombstones(const ir::QuadStream& qstream)
{
    // Resolve Tombstones by Chasing Jump Targets
    for (ir::QuadStream::size_type i = 0; i < qstream.size(); ++i)
    {
        if (translation_map_[i] != FunctionHoister::k_tombstone)
            continue;
        DMASSERT(qstream[i].opcode == ir::Opcode::JUMP);

        // Chase the jump chain until we find a quad that survived hoisting
        u64 curr = i;
        while (translation_map_[curr] == FunctionHoister::k_tombstone)
            curr = ir::Quad::label_to_index(qstream[curr].label);
        translation_map_[i] = translation_map_[curr];
    }
}

void
FunctionHoister::patch_labels(const ir::QuadStream& qstream)
{
    // Fix jump labels instantly using O(1) array direct-mapping
    for (ir::Quad& q : hoisted_stream)
    {
        if (q.label.is_none())
            continue;

        const u64 old_target = ir::Quad::label_to_index(q.label);
        DMASSERT(old_target < translation_map_.size());

        const u64 new_target = translation_map_[old_target];
        q.label = ir::Quad::index_to_label(new_target);
    }
}

ir::QuadStream
FunctionHoister::hoist(const ir::QuadStream& unhoisted_stream)
{
    if (unhoisted_stream.empty())
        return {};

    FunctionHoister hoister;
    hoister.unnest_functions(unhoisted_stream);
    hoister.linearize_to_single_qstream(unhoisted_stream);
    hoister.resolve_tombstones(unhoisted_stream);
    hoister.patch_labels(unhoisted_stream);
    return std::move(hoister.hoisted_stream);
}
} // namespace

ir::QuadStream
IROptimizer::hoist_functions(const ir::QuadStream& unhoisted_stream)
{
    return FunctionHoister::hoist(unhoisted_stream);
}

} // namespace alpha