#ifndef ALPHA_PARSER_CONTEXT_HPP
#define ALPHA_PARSER_CONTEXT_HPP

#include <stack>
#include <limits>
#include "core/alpha_types.hpp"
#include "misc/smart_assert.h"
#include "core/alpha_types.hpp"
#include "core/alpha_konstants.hpp"
#include "core/alpha_templates.hpp"

namespace Alpha
{
        // clang-format off
        struct ScopeTag{};
        struct FunctionTag{};
        struct LoopTag {};
        // clang-format on
        class ControlFlowCtx
        {
        public:
                ControlFlowCtx() = default;
                ~ControlFlowCtx() = default;

                ControlFlowCtx(const ControlFlowCtx &) = delete;
                ControlFlowCtx(ControlFlowCtx &&) = delete;
                ControlFlowCtx &operator=(const ControlFlowCtx &) = delete;
                ControlFlowCtx &operator=(ControlFlowCtx &&) = delete;

                void enter_function();
                void exit_function();
                void enter_loop() noexcept;
                void exit_loop() noexcept;
                u32 loop_depth() const noexcept;
                u32 function_depth() const noexcept;

        private:
                std::stack<Counter<LoopTag>> frame_stack_;
        };

        // struct StateFlags
        // {
        //         bool flag1 : 1;
        //         bool flag2 : 1;
        //         bool flag3 : 1;
        // };

        class ParseCtx
        {
        public:
                Counter<ScopeTag> current_scope;
                ControlFlowCtx ctrl_flow_ctx;

                ParseCtx() = default;
                ~ParseCtx() = default;

                ParseCtx(const ParseCtx &) = delete;
                ParseCtx(ParseCtx &&) = delete;
                ParseCtx &operator=(const ParseCtx &) = delete;
                ParseCtx &operator=(ParseCtx &&) = delete;
        };

} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP
