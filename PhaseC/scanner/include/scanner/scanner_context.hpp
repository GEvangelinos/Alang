#ifndef SCANNER_CONTEXT_HPP
#define SCANNER_CONTEXT_HPP

#include <optional>
#include <stack>

#include "core/konstants.hpp"
#include "core/numeric_types.hpp"
#include "core/source_location.hpp"
#include "parser/alpha_parser.gen.hpp"
#include "scanner/scanner_token.hpp"

namespace alpha
{
struct TokenInfo
{
    alpha_yytoken_kind_t id;
    SourceLocation loc;
};

class LexerCtx : private Immobile
{
public:
    u32 index_ = 0;

    LexerCtx() = default;

    ~LexerCtx() { TokenID::clearLastId(); }

    void register_token(TokenInfo token_info);

    [[nodiscard]] std::optional<TokenInfo> last_token_info() const noexcept;
    [[nodiscard]] std::optional<TokenInfo> second_last_token_info() const noexcept;
    [[nodiscard]] std::optional<SourceLocation> lastest_open_parenthesis_loc() const;
    [[nodiscard]] std::optional<SourceLocation> lastest_open_bracket_loc() const;
    [[nodiscard]] std::optional<SourceLocation> latest_open_brace_loc() const;

private:
    std::optional<TokenInfo> last_token_info_;
    std::optional<TokenInfo> second_last_token_info_;

    // These fields are used to improve diagnostic messages. For example, if a brace remains
    // unclosed, we can point to the exact source location of the most recent opening token.

    using TokenLocStack = std::stack<SourceLocation, std::vector<SourceLocation>>;
    TokenLocStack open_left_parentheses_locs;
    TokenLocStack open_left_brackets_locs;
    TokenLocStack open_left_braces_locs;
};

inline std::optional<TokenInfo>
LexerCtx::last_token_info() const noexcept { return last_token_info_; }

inline std::optional<TokenInfo>
LexerCtx::second_last_token_info() const noexcept { return second_last_token_info_; }

} // namespace alpha
#endif // SCANNER_CONTEXT_HPP
