#ifndef PARSER_CONTEXT_MANAGER_HPP
#define PARSER_CONTEXT_MANAGER_HPP

#include <list>
#include <string>
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"

namespace Alpha
{
        class ParserContext
        {
        public:
                ParserContext()
                    : active_function_depth_(0),
                      lvalue_is_member_(false),
                      is_function_block_(false) {}

                inline bool lvalue_is_member() const noexcept { return lvalue_is_member_; }
                inline void set_lvalue_is_member() noexcept { lvalue_is_member_ = true; }
                inline void clear_lvalue_is_member() noexcept { lvalue_is_member_ = false; }

                inline bool is_function_block() const noexcept { return is_function_block_; }
                inline void set_is_function_block() noexcept { is_function_block_ = true; }
                inline void clear_is_function_block() noexcept { is_function_block_ = false; }

                inline u32 active_function_depth() const noexcept { return active_function_depth_; }
                inline void enter_function() noexcept { active_function_depth_++; }
                inline void exit_function() noexcept { active_function_depth_--; }

                const std::list<std::pair<std::string, Location>> &
                function_argument_list() const
                {
                        return function_argument_list_;
                }

                inline void append_function_argument(const std::string &argument_name, const Location &argument_location)
                {
                        function_argument_list_.push_back(std::make_pair(argument_name, argument_location));
                }

                inline void clear_function_argument_list()
                {
                        function_argument_list_.clear();
                }

        private:
                u32 active_function_depth_;
                bool lvalue_is_member_;
                bool is_function_block_;
                std::list<std::pair<std::string, Location>> function_argument_list_;
        };
}

#endif /* PARSER_CONTEXT_MANAGER_HPP */
