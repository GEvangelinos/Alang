#ifndef SCANNER_CONTEXT_HPP
#define SCANNER_CONTEXT_HPP

#include <optional>

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

    void add_token(TokenInfo token_info);

    [[nodiscard]] std::optional<TokenInfo> get_last_token() const;
    [[nodiscard]] std::optional<TokenInfo> get_second_last_token() const;

private:
    std::optional<TokenInfo> last;
    std::optional<TokenInfo> second_last;
};

inline void
LexerCtx::add_token(TokenInfo token_info)
{
    second_last = std::move(last);
    last = std::move(token_info);
}

inline std::optional<TokenInfo>
LexerCtx::get_last_token() const { return last; }

inline std::optional<TokenInfo>
LexerCtx::get_second_last_token() const { return second_last; }
} // namespace alpha
#endif // SCANNER_CONTEXT_HPP
