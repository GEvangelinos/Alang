#ifndef ALPHA_LANG_HPP
#define ALPHA_LANG_HPP

#include <iosfwd> // for stringstream
#include <string> // for string, basic_string
#include "core/source_location.hpp"
#include "core/string_span.hpp"

// TODO: if planning to scan multiple files and have multiple threads..
// you need to remove the static buffer variables and either move to thread_local (for many threads)
// or to private  (per class)
namespace alpha
{
struct AlphaToken
{
public:
    u32 line_number;
    u32 token_number;
    std::string content;
    std::string code_name;
};

class Token
{
private:
    static u32 valid_token_counter_;
    static void increment_valid_token_counter();
    static void decrement_valid_token_counter();
    const u32 line_number_;
    const u32 token_number_;
    const std::string token_content_;

protected:
    Token(u32 line_number, u32 token_number, const std::string& token_content);
    [[nodiscard]] u32 get_token_number() const noexcept { return token_number_; }
    [[nodiscard]] u32 get_line_number() const noexcept { return line_number_; }
    [[nodiscard]] const std::string& get_token_content() const noexcept { return token_content_; }

public:
    virtual ~Token() = default;
    [[nodiscard]] static u32 get_valid_token_counter() noexcept { return valid_token_counter_; }
    virtual std::string to_string() const = 0;
    static void export_token(
        void* yylval, u32 line_number, const std::string& content, const std::string& code_name);
};

class TokenKeyword final : public Token
{
private:
    const std::string token_code_name_;

public:
    TokenKeyword(
        u32 line_number,
        u32 token_number,
        const std::string& keyword_content,
        const std::string& keyword_code_name);
    [[nodiscard]] std::string to_string() const override;
};

class TokenOperator final : public Token
{
private:
    const std::string token_code_name_;

public:
    TokenOperator(
        u32 line_number,
        u32 token_number,
        const std::string& operator_content,
        const std::string& operator_code_name);
    [[nodiscard]] std::string to_string() const override;
};

class TokenPunctuation final : public Token
{
private:
    const std::string token_code_name_;

public:
    TokenPunctuation(
        u32 line_number,
        u32 token_number,
        const std::string& punctuation_content,
        const std::string& punctuation_code_name);
    [[nodiscard]] std::string to_string() const override;
};

class TokenIntegerNumber final : public Token
{
private:
    const std::string number_of_token_;

public:
    TokenIntegerNumber(
        u32 line_number,
        u32 token_number,
        const std::string& integer_content,
        std::string number_of_token);
    [[nodiscard]] std::string to_string() const override;
};

class TokenRealNumber final : public Token
{
private:
    const std::string number_of_token_;

public:
    TokenRealNumber(
        u32 line_number,
        u32 token_number,
        const std::string& real_content,
        const std::string& number_of_token);
    std::string to_string() const override;
};

class TokenID final : public Token
{
public:
    TokenID(
        u32 line_number,
        u32 token_number,
        const std::string& id_content,
        const std::string& id_name);
    [[nodiscard]] std::string to_string() const override;

private:
    const std::string id_name_;
};

class TokenString final : public Token
{
public:
    static void set_starting_location(SourceLocation location);
    [[nodiscard]] static SourceLocation get_starting_location();
    static void set_string_begin_addr(const char *str_addr);
    static void append_to_assembling_buffer(std::string string_chunk);
    [[nodiscard]] static StringSpan assemble_string_span();
    [[nodiscard]] static SourceLocation export_location(SourceLocation closing_quote_loc);
    TokenString(u32 line_number, u32 token_number, const std::string& string_content);
    [[nodiscard]] std::string to_string() const override;

private:
    // TODO: bomb waiting to explode... (uses global buffer, with many TranslationUnit == CHAOS)
    static std::string latest_assembled_string_;
    // TODO: use std::vector<char> stringstream is locale aware and slow.
    static std::stringstream string_assembling_buffer_;
    static const char *last_string_begin_addr_;
    static SourceLocation string_starting_location_;
    static void flush_assembling_buffer();
    [[nodiscard]] static std::string convert_content_escapes_to_ascii();
};

class TokenComment final : public Token
{
private:
    std::string comment_type_;
    static SourceLocation comment_starting_location_;

public:
    static void set_starting_location(SourceLocation location);
    [[nodiscard]] static SourceLocation get_starting_location();
    TokenComment(
        u32 line_number,
        u32 token_number,
        const std::string& comment_description,
        const std::string& comment_type);
    [[nodiscard]] std::string to_string() const override;
};

class TokenInvalid final : public Token
{
public:
    TokenInvalid(
        u32 line_number,
        u32 token_number,
        const std::string& the_invalid_token);
    [[nodiscard]] std::string to_string() const override;
};

inline void
TokenString::set_starting_location(const SourceLocation location)
{
    TokenString::string_starting_location_ = location;
}

inline void
TokenString::set_string_begin_addr(const char* const str_addr)
{
    TokenString::last_string_begin_addr_ = str_addr;

}

inline SourceLocation
TokenString::get_starting_location() { return TokenString::string_starting_location_; }
} /* namespace alpha */

#endif /* ALPHA_LANG_HPP */
