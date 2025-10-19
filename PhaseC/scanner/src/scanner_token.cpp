#include "scanner/scanner_token.hpp"
#include <algorithm> // for transform
#include <cctype>    // for toupper
#include <cstring>   // for strcpy, strlen, size_t
#include <limits>    // for numeric_limits
#include <sstream>   // for basic_ostream, basic_stringstream, operator<<
#include <stdexcept> // for overflow_error, runtime_error, underflow_error

#include "core/konstants.hpp"

namespace alpha
{
/*** STARTOF: Local to the file (static) definitions: ***/
[[maybe_unused]] static std::string to_upper_case(const std::string &input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) -> unsigned char { return std::toupper(c); });
    return result;
}

/*** ENDOF: Local to the file (static) definitions: ***/

/*** STARTOF: class Token definitions: ***/
unsigned int Token::valid_token_counter_ = 0;

void Token::export_token(
    void *yylval,
    const u32 line_number,
    const std::string &content,
    const std::string &code_name)
{
    AlphaToken *casted_yylval = (struct AlphaToken *) yylval;
    casted_yylval->line_number = line_number;
    casted_yylval->token_number = Token::get_valid_token_counter();
    casted_yylval->content = content;
    casted_yylval->code_name = code_name;
}

void Token::increment_valid_token_counter()
{
    if (valid_token_counter_ == std::numeric_limits<unsigned int>::max())
        throw std::overflow_error(ATTACH_CONTEXT(
            "validTokenCounter has reached its maximum value and will wrap-around."));
    ++valid_token_counter_;
}

void Token::decrement_valid_token_counter()
{
    if (valid_token_counter_ == std::numeric_limits<unsigned int>::min())
        /* This should never happen. */
        throw std::underflow_error(ATTACH_CONTEXT(
            "validTokenCounter has reached its minimum value and will wrap-around."));
    --valid_token_counter_;
}

Token::Token(
    const u32 line_number,
    const u32 token_number,
    const std::string &token_content)
    : line_number_(line_number), token_number_(token_number), token_content_(token_content)
{
    increment_valid_token_counter();
}

std::string Token::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << line_number_
        << ":\t"
        << "#"
        << token_number_
        << "\t"
        << "\""
        << token_content_
        << "\"";
    return string_buffer.str();
}

/*** ENDOF: class Token definitions. ***/

/*** STARTOF: class TokenKeyword definitions: ***/
TokenKeyword::TokenKeyword(
    const u32 line_number,
    const u32 token_number,
    const std::string &keyword_content,
    const std::string &keyword_code_name)
    : Token(line_number, token_number, keyword_content), token_code_name_(keyword_code_name) {}

std::string TokenKeyword::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "KEYWORD"
        << "\t"
        << token_code_name_;
    return string_buffer.str();
}

/*** ENDOF: class TokenKeyword definitions. ***/

/*** STARTOF: class TokenOperator definitions: ***/
TokenOperator::TokenOperator(
    const u32 line_number,
    const u32 token_number,
    const std::string &operator_content,
    const std::string &operator_code_name)
    : Token(line_number, token_number, operator_content),
      token_code_name_(operator_code_name) {}

std::string TokenOperator::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "OPERATOR"
        << "\t"
        << token_code_name_;
    return string_buffer.str();
}

/*** ENDOF: class TokenOperator definitions. ***/

/*** STARTOF: class TokenPunctuation definitions: ***/
TokenPunctuation::TokenPunctuation(
    const u32 line_number,
    const u32 token_number,
    const std::string &punctuation_content,
    const std::string &punctuation_code_name)
    : Token(line_number, token_number, punctuation_content),
      token_code_name_(punctuation_code_name) {}

std::string TokenPunctuation::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "PUNCTUATION"
        << "\t"
        << token_code_name_;
    return string_buffer.str();
}

/*** ENDOF: class TokenKeyword definitions. ***/

/*** STARTOF: class TokenIntegerNumber definitions. ***/
TokenIntegerNumber::TokenIntegerNumber(
    const u32 line_number,
    const u32 token_number,
    const std::string &integer_content,
    const std::string number_of_token)
    : Token(line_number, token_number, integer_content),
      number_of_token_(number_of_token) {}

std::string TokenIntegerNumber::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "INTCONST"
        << "\t"
        << number_of_token_;
    return string_buffer.str();
}

/*** ENDOF: class TokenIntegerNumber definitions. ***/

/*** STARTOF: class TokenRealNumber definitions. ***/
TokenRealNumber::TokenRealNumber(
    const u32 line_number,
    const u32 token_number,
    const std::string &real_content,
    const std::string &number_of_token)
    : Token(line_number, token_number, real_content),
      number_of_token_(number_of_token) {}

std::string TokenRealNumber::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "REALCONST"
        << "\t"
        << number_of_token_;
    return string_buffer.str();
}

/*** ENDOF: class TokenIntegerNumber definitions. ***/

/*** STARTOF: class TokenID definitions. ***/
char *TokenID::last_id_ = nullptr;

TokenID::TokenID(
    const u32 line_number,
    const u32 token_number,
    const std::string &id_content,
    const std::string &id_name)
    : Token(line_number, token_number, id_content),
      id_name_(id_name) {}

char *TokenID::refresh_last_id(const char *const id)
{
    // TODO in future driver.. just malloc a buffer, and write the same (grow only if needed..);
    if (!id)
        throw std::runtime_error("Error in lexer (probably): `id` is null.");

    delete[] TokenID::last_id_; /* delete[] is safe with nullptr too (in first initialization). */
    TokenID::last_id_ = new char[std::strlen(id) + 1]; /* +1 for '\0'. */
    std::strcpy(TokenID::last_id_, id);
    return TokenID::last_id_;
}

void TokenID::clear_last_id() { delete[] TokenID::last_id_; }

std::string TokenID::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "ID"
        << "\t"
        << "\""
        << id_name_
        << "\"";
    return string_buffer.str();
}

/*** ENDOF: class TokenID definitions. ***/

/*** STARTOF: class TokenComment definitions. ***/
SourceLocation TokenComment::comment_starting_location_ = k_no_loc;

void TokenComment::set_starting_location(const SourceLocation location)
{
    TokenComment::comment_starting_location_ = location;
}

SourceLocation TokenComment::get_starting_location()
{
    return TokenComment::comment_starting_location_;
}

TokenComment::TokenComment(
    const u32 line_number,
    const u32 token_number,
    const std::string &comment_description,
    const std::string &comment_type)
    : Token(line_number, token_number, comment_description), comment_type_(comment_type) {}

std::string TokenComment::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "COMMENT"
        << "\t"
        << comment_type_;
    return string_buffer.str();
}

/*** ENDOF: class TokenComment definitions. ***/

/*** STARTOF: class TokenString definitions. ***/
std::stringstream TokenString::string_assembling_buffer_;
SourceLocation TokenString::string_starting_location_ = k_no_loc;
std::string TokenString::latest_assembled_string_;

void TokenString::set_starting_location(const SourceLocation location)
{
    TokenString::string_starting_location_ = location;
}

SourceLocation TokenString::get_starting_location()
{
    return TokenString::string_starting_location_;
}

/* TODO: FIXME: I AM UGLY AS FUCK... AT LEAST PUT replacement and replacee  into list and iterate.. jeez. */
std::string TokenString::convert_content_escapes_to_ascii()
{
    std::string str = string_assembling_buffer_.str();
    std::string to_replace = "\\n";
    char replacement = '\n';
    size_t pos = 0;
    while ((pos = str.find(to_replace, pos)) != std::string::npos)
    {
        str.replace(pos, to_replace.length(), 1, replacement);
        pos += 1; // Move past the replacement
    }

    to_replace = "\\t";
    replacement = '\t';
    pos = 0;
    while ((pos = str.find(to_replace, pos)) != std::string::npos)
    {
        str.replace(pos, to_replace.length(), 1, replacement);
        pos += 1; // Move past the replacement
    }

    to_replace = "\\\\";
    replacement = '\\';
    pos = 0;
    while ((pos = str.find(to_replace, pos)) != std::string::npos)
    {
        str.replace(pos, to_replace.length(), 1, replacement);
        pos += 1; // Move past the replacement
    }

    to_replace = "\\\"";
    replacement = '\"';
    pos = 0;
    while ((pos = str.find(to_replace, pos)) != std::string::npos)
    {
        str.replace(pos, to_replace.length(), 1, replacement);
        pos += 1; // Move past the replacement
    }
    return str;
}

void TokenString::append_to_assembling_buffer(std::string string_chunk)
{
    string_assembling_buffer_ << string_chunk;
}

void TokenString::flush_assembling_buffer()
{
    string_assembling_buffer_.str("");
    string_assembling_buffer_.clear();
}

const char *TokenString::export_string_token()
{
    latest_assembled_string_ = convert_content_escapes_to_ascii();
    TokenString::flush_assembling_buffer();
    return latest_assembled_string_.c_str();
}

SourceLocation
TokenString::export_location(SourceLocation closing_quote_loc)
{
    return merge(get_starting_location(), closing_quote_loc);
}

TokenString::TokenString(
    const u32 line_number,
    const u32 token_number,
    const std::string &string_content)
    : Token(line_number, token_number, string_content) {}

std::string TokenString::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "STRING"
        << "\t"
        << "\""
        << Token::get_token_content()
        << "\"";
    return string_buffer.str();
}

/*** ENDOF: class TokenString definitions. ***/

/*** STARTOF: class TokenInvalid definitions. ***/
TokenInvalid::TokenInvalid(
    const u32 line_number,
    const u32 token_number,
    const std::string &the_invalid_token)
    : Token(line_number, token_number, the_invalid_token)
{
    /* Empty Constructor Body */
}

std::string TokenInvalid::to_string() const
{
    std::stringstream string_buffer;
    string_buffer << Token::to_string()
        << "\t"
        << "TOKEN_INVALID"
        << "\t"
        << Token::get_token_content();
    return string_buffer.str();
}

/*** ENDOF: class TokenInvalid definitions. ***/
} /* namespace Alpha */
