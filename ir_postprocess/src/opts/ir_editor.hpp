#ifndef IR_EDITOR_HPP
#define IR_EDITOR_HPP

#include "core/ir/ir_quad.hpp"

namespace alpha
{
    class IREditor
    {
    public:

        explicit IREditor(ir::QuadStream& qstream);

        [[nodiscard]] bool is_dead(ir::QuadStream::size_type qidx) const noexcept;
        void kill(ir::QuadStream::size_type qidx) noexcept;

        /**
         *
         * @return returns `true` if it modified the given ir::QuadStream, `false` otherwise.
         */
        [[nodiscard]] bool apply();

    private:
        static constexpr auto k_tombstone = std::numeric_limits<u64>::max();

        ir::QuadStream& qstream_;
        u64 kill_counter_ = 0;
        std::vector<OnceFlag> is_dead_;
        std::vector<u64> translation_map_;
    };

inline void
IREditor::kill(const ir::QuadStream::size_type qidx) noexcept
{
    DMASSERT(qidx < qstream_.size(), kill_counter_ < qstream_.size());
    is_dead_[qidx].raise();
    ++kill_counter_;
}
} // namespace alpha


#endif //IR_EDITOR_HPP
