#include "scanner/scanner_context.hpp"

namespace alpha
{
void
LexerCtx::register_token(TokenInfo token_info)
{
    switch (token_info.id)
    {
    case alpha_yytoken_kind_t::TKN_LEFT_PAREN:
        open_left_parentheses_locs.push(token_info.loc);
        break;
    case alpha_yytoken_kind_t::TKN_LEFT_BRACKET:
        open_left_brackets_locs.push(token_info.loc);
        break;
    case alpha_yytoken_kind_t::TKN_LEFT_BRACE:
        open_left_braces_locs.push(token_info.loc);
        break;
    case alpha_yytoken_kind_t::TKN_RIGHT_PAREN:
        if (!open_left_parentheses_locs.empty()) open_left_parentheses_locs.pop();
        break;
    case alpha_yytoken_kind_t::TKN_RIGHT_BRACKET:
        if (!open_left_brackets_locs.empty()) open_left_brackets_locs.pop();
        break;
    case alpha_yytoken_kind_t::TKN_RIGHT_BRACE:
        if (!open_left_braces_locs.empty()) open_left_braces_locs.pop();
        break;
    default:
        break;
    }

    second_last_token_info_ = std::move(last_token_info_);
    last_token_info_ = std::move(token_info);
}

std::optional<SourceLocation>
LexerCtx::lastest_open_parenthesis_loc() const
{
    if (open_left_parentheses_locs.empty())
        return {};
    DEBUG_SMART_ASSERT(open_left_parentheses_locs.top() != k_no_loc);
    return open_left_parentheses_locs.top();
}

std::optional<SourceLocation>
LexerCtx::lastest_open_bracket_loc() const
{
    if (open_left_brackets_locs.empty())
        return {};
    DEBUG_SMART_ASSERT(open_left_brackets_locs.top() != k_no_loc);
    return open_left_brackets_locs.top();
}

std::optional<SourceLocation>
LexerCtx::latest_open_brace_loc() const
{
    if (open_left_braces_locs.empty())
        return {};
    DEBUG_SMART_ASSERT(open_left_braces_locs.top() != k_no_loc);
    return open_left_braces_locs.top();
}
} // namespace alpha
