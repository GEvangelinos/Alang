#ifndef prsr_ctx_HPP
#define prsr_ctx_HPP

#include <stack>
#include "core/alpha_types.hpp"

namespace Alpha
{
        static constexpr u32 INITIAL_FUNCTION_NESTING_DEPTH = 0;
        static constexpr u32 INITIAL_LOOP_NESTING_DEPTH = 0;

        class PrsrCTX
        {
                struct StateFlags
                {
                        bool flag1 : 1;
                        bool flag2 : 1;
                        bool flag3 : 1;
                        bool flag4 : 1;
                        bool flag5 : 1;
                        bool flag6: 1;
                        bool flag7 : 1;
                        bool flag8 : 1;
                        bool flag9 : 10;
                        bool flag10 : 10;
                        bool flag11 : 10;
                        bool flag12 : 10;
                };

                static constexpr int ff = sizeof(StateFlags);

        public:
                PrsrCTX()
                    : function_nesting_depth_(INITIAL_FUNCTION_NESTING_DEPTH) {}

                // clang-format off
                u32 function_nesting_depth() const noexcept { return function_nesting_depth_; }
                void enter_function()              noexcept {      ++function_nesting_depth_; }
                void exit_function()               noexcept {      --function_nesting_depth_; }

                /* TODO: Can these go easily on parser's context? (Easily? and elegantly?) */
                void push_new_loop_depth_counter();
                void pop_loop_depth_counter();
                u32 loop_depth() const;
                void enter_loop();
                void exit_loop();

                u32 current_scope() const noexcept { return current_scope_; }
                void enter_scope()        noexcept { ++current_scope_; }
                void exit_scope()         noexcept { --current_scope_; }
                // clang-format on

        private:
                u32 current_scope_;
                u32 function_nesting_depth_;
                std::stack<u32> loop_depth_counters_; // New counter appended with each function.
        };

}
// clang-format off
                // public:
                //       lvalue_is_member_(false),
                //       is_function_block_(false)
                //
                // bool lvalue_is_member() const     noexcept { return lvalue_is_member_; }
                // void set_lvalue_is_member()       noexcept { lvalue_is_member_ = true; }
                // void clear_lvalue_is_member()     noexcept { lvalue_is_member_ = false; }
                //
                // bool is_function_block() const    noexcept { return is_function_block_; }
                // void set_is_function_block()      noexcept { is_function_block_ = true; }
                // void clear_is_function_block()    noexcept { is_function_block_ = false; }
                //
                // private:
                // bool lvalue_is_member_;
                // bool is_function_block_;
// clang-format-on

        // class Const
        // {
        //         enum class Terminal
        //         {
        //                 INT_CONST,
        //                 REAL_CONST,
        //                 STRING_LITERAL,
        //                 NIL,
        //                 TRUE,
        //                 FALSE,
        //         };
        // };
// ConstTerminal last_const_terminal() const noexcept { return last_const_terminal_; }
                // void update_last_const_terminal(ConstTerminal terminal) noexcept { last_const_terminal_ = terminal; }
                // ConstTerminal last_const_terminal_;
#endif// prsr_ctx_MANAGER_HPP
