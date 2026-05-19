#include "ir_editor.hpp"

namespace alpha
{
IREditor::IREditor(ir::QuadStream& qstream)
    : qstream_(qstream),
      is_dead_(qstream.size()),
      translation_map_(qstream.size(), k_tombstone) {}

bool
IREditor::apply()
{
    ir::QuadStream alive_qstream;
    alive_qstream.reserve(qstream_.size() - kill_counter_);

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

        const auto none = CodeAddress::none();
        if (q.label.is_none())
            continue;

        auto ql_index = ir::Quad::label_to_index(q.label);
        auto mapped_index = translation_map_[ql_index];
        const auto new_label = ir::Quad::index_to_label(mapped_index);
        alive_qstream.back().label = new_label;
    }
    qstream_ = std::move(alive_qstream);
    return kill_counter_ > 0;
}

} // namespace alpha