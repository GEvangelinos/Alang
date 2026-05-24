#include "ir_editor.hpp"

namespace alpha
{
IREditor::IREditor(ir::QuadStream& qstream)
    : qstream_(qstream),
      is_dead_(qstream.size()),
      translation_map_(qstream.size(), k_tombstone) {}

bool
IREditor::is_dead(const ir::QuadStream::size_type qidx) const noexcept
{
    DMASSERT(qidx < qstream_.size());
    return is_dead_[qidx];
}

bool
IREditor::apply()
{
    ir::QuadStream alive_qstream;
    DMASSERT(qstream_.size() >= kill_counter_);
    const auto expected_alive_quads = qstream_.size() - kill_counter_;
    alive_qstream.reserve(expected_alive_quads);

    // Build new jump_mappings:
    u64 alive_count = 0;
    for (u64 i = 0; i < qstream_.size(); ++i)
    {
        if (is_dead(i))
            continue;
        translation_map_[i] = alive_count++;
    }

    // In case last quad, is also mark dead, we seed it to point outside valid quads by 1 position.
    if (translation_map_.back() == k_tombstone)
        translation_map_.back() = alive_count;
    for (i64 i = translation_map_.size() - 2; i >= 0; --i)
        if (translation_map_[i] == k_tombstone)
            translation_map_[i] = translation_map_[i + 1];

    for (u64 i = 0; i < qstream_.size(); ++i)
    {
        const ir::Quad& q = qstream_[i];
        if (is_dead(i))
            continue;
        alive_qstream.push_back(q);

        if (q.label.is_none())
            continue;

        const auto ql_index = ir::Quad::label_to_index(q.label);
        if (ql_index >= translation_map_.size())
        {
            alive_qstream.back().label.value = expected_alive_quads;
            continue;
        }
        const auto mapped_index = translation_map_[ql_index];
        const auto new_label = ir::Quad::index_to_label(mapped_index);
        alive_qstream.back().label = new_label;
    }
    DMASSERT(alive_qstream.size() == expected_alive_quads);
    qstream_ = std::move(alive_qstream);
    return kill_counter_ > 0;
}
} // namespace alpha
