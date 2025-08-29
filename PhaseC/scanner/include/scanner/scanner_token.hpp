#ifndef ALPHA_LANG_HPP
#define ALPHA_LANG_HPP

#include <iosfwd> // for stringstream
#include <string> // for string, basic_string
#include "core/source_location.hpp"

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
    Token(u32 line_number, u32 token_number, const std::string &token_content);
    u32 get_token_number() const noexcept { return token_number_;}
    u32 get_line_number() const noexcept {return line_number_; }
    const std::string &get_token_content() const noexcept { return token_content_; };

public:
    virtual ~Token() = default;
    static u32 get_valid_token_counter() noexcept {return valid_token_counter_; }
    virtual std::string to_string() const = 0;
    static void export_token(
        void *yylval, u32 line_number, const std::string &content, const std::string &code_name);
};

class TokenKeyword final : public Token
{
private:
    const std::string token_code_name_;

public:
    TokenKeyword(
        u32 line_number,
        u32 token_number,
        const std::string &keyword_content,
        const std::string &keyword_code_name);
    std::string to_string() const override;
};

class TokenOperator final : public Token
{
private:
    const std::string token_code_name_;

public:
    TokenOperator(
        u32 line_number,
        u32 token_number,
        const std::string &operator_content,
        const std::string &operator_code_name);
    std::string to_string() const override;
};

class TokenPunctuation final : public Token
{
private:
    const std::string token_code_name_;

public:
    TokenPunctuation(
        u32 line_number,
        u32 token_number,
        const std::string &punctuation_content,
        const std::string &punctuation_code_name);
    std::string to_string() const override;
};

class TokenIntegerNumber final : public Token
{
private:
    const std::string number_of_token_;

public:
    TokenIntegerNumber(
        u32 line_number,
        u32 token_number,
        const std::string &integer_content,
        std::string number_of_token);
    std::string to_string() const override;
};

class TokenRealNumber final : public Token
{
private:
    const std::string number_of_token_;

public:
    TokenRealNumber(
        u32 line_number,
        u32 token_number,
        const std::string &real_content,
        const std::string &number_of_token);
    std::string to_string() const override;
};

class TokenID final : public Token
{
private:
    const std::string id_name_;
    static char *last_id_;

public:
    TokenID(
        u32 line_number,
        u32 token_number,
        const std::string &id_content,
        const std::string &id_name);
    static char *refresh_last_id(const char *id);
    static void clear_last_id();
    std::string to_string() const override;
};

class TokenString final : public Token
{
public:
    static void set_starting_location(SourceLocation location);
    static SourceLocation get_starting_location();
    static void append_to_assembling_buffer(std::string string_chunk);
    static const char *export_string_token();
    TokenString(u32 line_number, u32 token_number, const std::string &string_content);
    std::string to_string() const override;

private:
    static std::string latest_assembled_string_;
    static std::stringstream string_assembling_buffer_;
    static SourceLocation string_starting_location_;
    static void flush_assembling_buffer();
    static std::string convert_content_escapes_to_ascii();
};

class TokenComment final : public Token
{
private:
    std::string comment_type_;
    static SourceLocation comment_starting_location_;

public:
    static void set_starting_location(SourceLocation location);
    static SourceLocation get_starting_location();
    TokenComment(
        u32 line_number,
        u32 token_number,
        const std::string &comment_description,
        const std::string &comment_type);
    std::string to_string() const override;
};

class TokenInvalid final : public Token
{
public:
    TokenInvalid(
        u32 line_number,
        u32 token_number,
        const std::string &the_invalid_token);
    std::string to_string() const override;
};
} /* namespace Alpha */

#endif /* ALPHA_LANG_HPP */
