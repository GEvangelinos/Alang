#include <alpha_parser.gen.hpp>
#include <fstream>
#include <driver/translation_unit.hpp>

#include "bytecode/abc_serializer.hpp"
#include "ir_postprocess/ir_optimizer.hpp"
#include "core/konstants.hpp"
#include "driver/exception.hpp"
#include "driver/konstants.hpp"
#include "core/exception.hpp"
#include "core/translation_unit_buffer.hpp"
#include "core/bytecode/vm_program.hpp"
#include "driver/translation_unit_buffer_loader.hpp"
#include "scanner/alpha_scanner.gen.hpp"
#include "scanner/scanner_adapter.hpp"
#include "bytecode/abc_generator.hpp"
#include "ir_postprocess/ir_validator.hpp"

#include "support/cli_color.h"

#define WARNING_BANNER_PREFIX COLOR_FG_ASCII_MAGENTA SGR_BOLD "Warning" SGR_BLINK "❗" SGR_RESET ": "
inline constexpr auto k_reescaped_warning_banner =
    WARNING_BANNER_PREFIX
    "Special characters are escaped; Ex.: \"\\n\" prints literally (not a newline) to keep the table aligned.\n"
    SGR_RESET;

// Note: The following string contains Greek characters.
// I dont remember what's going on with unicode and C++
// but in case it causes any problems just remove the Greek part
// It is just a reference to a lecture.
inline constexpr auto k_temp_reuse_warning_banner =
    WARNING_BANNER_PREFIX
    "Temporaries are context-local. Each function starts with a fresh temp frame (counter reset to zero).\n"
    "           Nested function definitions or functions assigned to tables/variables therefore allocate temps from a\n"
    "           clean slate. When the function definition ends, the previous temp counter is restored. (Lecture 9 Slide 40)\n"
    "           >>> Οι κρυφές μεταβλητές είναι κανονικές μεταβλητές και η δημιουργία\n"
    "           >>> τους απαιτεί δημιουργία νέου συμβόλου στον πίνακα συμβόλων, ενώ\n"
    "           >>> απενεργοποιούνται κανονικά εκτός της εμβέλειας δήλωσής τους\n"
    SGR_RESET;

inline constexpr auto k_cya_mode_off_warning_banner =
    WARNING_BANNER_PREFIX
    "This executable was built with CYA_MODE disabled. As a result:\n"
    "           a) Aggressive reuse of temporary variables is enabled; table literals are handled differently.\n"
    "           b) Assignment temps are no longer created except when strictly required (e.g., inside function calls).\n"
    SGR_RESET;

namespace alpha
{
[[nodiscard]] std::string to_string(const CodeAddress label)
{
    return label.is_none() ? "NONE" : FMT::to_string(label.value);
}
} // namespace alpha

namespace
{
void create_export_directory(std::string_view dirname);
void enter_export_directory(std::string_view dirname);
void exit_export_directory(const std::filesystem::path& original_path);
[[nodiscard]] std::string escape(alpha::StringSpan str);
[[nodiscard]] std::string ensure_quote_wrapped(const std::string& str);
[[nodiscard]] std::string expr_formatter(const alpha::Expr* expr, const char* missing_marker = "");
[[nodiscard]] std::string argument_formatter(
    const alpha::vm::Argument* a, const char* missing_marker = "");

template <unsigned column, unsigned column_width, typename T>
[[nodiscard]] std::string color_column(T&& value);
template <bool colorize, unsigned column, unsigned column_width, typename T>
[[nodiscard]] std::string format_column(T&& value);
template <bool colorize, typename Stream>
void print_ir(
    Stream& out,
    const std::vector<alpha::ir::Quad>& quads,
    const alpha::LocationTracker& lt,
    bool print_detailed);

void create_export_directory(const std::string_view dirname)
{
    std::filesystem::create_directories(dirname);
}

void enter_export_directory(const std::string_view dirname)
{
    std::filesystem::current_path(dirname);
}

void exit_export_directory(const std::filesystem::path& original_path)
{
    std::filesystem::current_path(original_path);
}

std::string escape(const alpha::StringSpan str)
{
    std::string out;
    for (std::size_t i = 0; i < str.size; ++i)
    {
        switch (const char ch = str.data[i]; ch)
        { // clang-format off
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        case '\v': out += "\\v"; break;
        case '\f': out += "\\f"; break;
        case '\b': out += "\\b"; break;
        // case '\\': out += "\\\\"; break;
        // case '\"': out += "\\\""; break;
        default: out += ch; break;
        } // clang-format on
    }
    return out;
}

std::string ensure_quote_wrapped(const std::string& str)
{
    if (str.size() >= 2 && str.front() == '"')
    {
        DMASSERT(str.back() == '"' && "String didn't start with \" so it shouldn't end");
        return str;
    }
    return FMT::format("\"{}\"", str); // Add quotes around str.
}

[[nodiscard]] std::string
expr_formatter(const alpha::Expr* expr, const char* const missing_marker)
{
    using namespace alpha;
    if (!expr) return missing_marker;

    using ET = Expr::Type;
    switch (expr->type)
    {
    case ET::CONST_BOOL:
        return static_cast<const ConstBoolExpr*>(expr)->value ? "true" : "false";
    case ET::CONST_INT:
        return FMT::format("{}", static_cast<const ConstIntExpr*>(expr)->value);
    case ET::CONST_FLOAT:
        return support::format_float(static_cast<const ConstFloatExpr*>(expr)->value);
    case ET::CONST_STRING:
        return ensure_quote_wrapped(escape(static_cast<const ConstStringExpr*>(expr)->value));
    case ET::CONST_NIL:
        return "nil";
    case ET::ARITHMETIC:
        return static_cast<const ArithmeticExpr*>(expr)->var_symbol->name.to_string();
    case ET::ASSIGN:
        return static_cast<const AssignExpr*>(expr)->var_symbol->name.to_string();
    case ET::BOOL:
        return static_cast<const BoolExpr*>(expr)->var_symbol->name.to_string();
    case ET::LIBRARY_FUNCTION:
        return static_cast<const LibFuncExpr*>(expr)->libfunc_symbol->name.to_string();
    case ET::NEW_TABLE:
        return static_cast<const NewTableExpr*>(expr)->var_symbol->name.to_string();
    case ET::PROGRAM_FUNCTION:
        return static_cast<const ProgFuncExpr*>(expr)->progfunc_symbol->name.to_string();
    case ET::TABLE_ITEM:
        return static_cast<const TableItemExpr*>(expr)->var_symbol->name.to_string();
    case ET::VARIABLE:
        return static_cast<const VariableExpr*>(expr)->var_symbol->name.to_string();
    default:
        UNREACHABLE(FMT::format("Unhandled Expr::Type: int({}) = {}",
            TO_STRING(expr->type), static_cast<int>(expr->type)
        ));
    }
}

std::string argument_formatter(
    const alpha::vm::Argument* const a,
    const char* const missing_marker)
{
    using namespace alpha;
    if (!a)
        return missing_marker;
    using AT = vm::Argument::Type;

    switch (a->type)
    {
    case AT::CONST_BOOL:
        return static_cast<const vm::ConstBoolArgument*>(a)->value ? "true" : "false";
    case AT::CONST_INT:
        return FMT::format("(int){}", static_cast<const vm::ConstIntArgument*>(a)->value);
    case AT::CONST_FLOAT:
        return FMT::format("(float){}", static_cast<const vm::ConstFloatArgument*>(a)->value);
    case AT::CONST_STRING:
        return FMT::format("(str){}", static_cast<const vm::ConstStringArgument*>(a)->pool_index);
    case AT::CONST_NIL:
        return "nil";
    case vm::Argument::Type::LABEL:
        return to_string(static_cast<const vm::LabelArgument*>(a)->value);
    case vm::Argument::Type::GLOBAL:
        return FMT::format("(global){}", static_cast<const vm::GlobalVariableArgument*>(a)->offset);
    case vm::Argument::Type::FORMAL:
        return FMT::format("(formal){}", static_cast<const vm::FormalVariableArgument*>(a)->offset);
    case vm::Argument::Type::LOCAL:
        return FMT::format("(local){}", static_cast<const vm::LocalVariableArgument*>(a)->offset);
    case vm::Argument::Type::PROGRAMFUNC:
        return FMT::format(
            "(progfunc){}", static_cast<const vm::ProgramFuncArgument*>(a)->func_idx
        );
    case vm::Argument::Type::LIBFUNC:
        return FMT::format(
            "(libfunc){}",
            static_cast<std::underlying_type_t<vm::LibFuncId>>(
                static_cast<const vm::LibFuncArgument*>(a)->libfunc_id
            )
        );
    case vm::Argument::Type::RETVAL:
        return "(retval)";
    default:
        UNREACHABLE(FMT::format("Unhandled vm::Argument::Type: int({}) = {}",
            TO_STRING(expr->type), static_cast<int>(a->type)
        ));
    }
}

template <unsigned Column, unsigned ColumnWidth, typename T>
std::string color_column(T&& value)
{
    constexpr unsigned column_count = 8;
    static_assert(Column < column_count, "So far we support a maximum of 8 columns");

    const char* ascii_color;
    if constexpr (Column == 0) ascii_color = COLOR_FG_ASCII_WHITE;
    else if constexpr (Column == 1) ascii_color = COLOR_FG_ASCII_RED;
    else if constexpr (Column == 2) ascii_color = COLOR_FG_ASCII_GREEN;
    else if constexpr (Column == 3) ascii_color = COLOR_FG_ASCII_BLUE;
    else if constexpr (Column == 4) ascii_color = COLOR_FG_ASCII_CYAN;
    else if constexpr (Column == 5) ascii_color = COLOR_FG_ASCII_YELLOW;
    else if constexpr (Column == 6) ascii_color = COLOR_FG_ASCII_MAGENTA;
    else if constexpr (Column == 7) ascii_color = COLOR_FG_MATRIX;
    else ascii_color = COLOR_FG_ASCII_DEFAULT;
    return FMT::format("{}{:<{}}{}", ascii_color, std::forward<T>(value), ColumnWidth, SGR_RESET);
}

template <bool Colorize, unsigned Column, unsigned ColumnWidth, typename T>
std::string format_column(T&& value)
{
    if constexpr (Colorize)
        return color_column<Column, ColumnWidth>(std::forward<T>(value));
    else
        return FMT::format("{:<{}}", std::forward<T>(value), ColumnWidth);
}

template <bool Colorize, typename Stream>
void print_ir(
    Stream& out,
    const alpha::ir::QuadStream& quads,
    const alpha::LocationTracker& lt,
    const bool print_detailed)
{
    constexpr alpha::u32 widths[] = {10, 15, 20, 20, 20, 10, 10, 10};
    constexpr alpha::u32 quad_header_width = [&widths]() constexpr
    {
        alpha::u32 width = 0;
        for (alpha::u32 i = 0; i < std::size(widths); ++i)
            width += widths[i];
        width += std::size(widths) - 1; // One space between each column
        return width;
    }();

    out << k_reescaped_warning_banner;
    out << k_temp_reuse_warning_banner;
    out << k_cya_mode_off_warning_banner;

    // Write export header.
    out << FMT::format(
        "{0} {1} {2} {3} {4} {5} {6} {7}\n",
        format_column<Colorize, 0, widths[0]>("quad#"),
        format_column<Colorize, 1, widths[1]>("opcode"),
        format_column<Colorize, 2, widths[2]>("result"),
        format_column<Colorize, 3, widths[3]>("arg1"),
        format_column<Colorize, 4, widths[4]>("arg2"),
        format_column<Colorize, 5, widths[5]>("label"),
        format_column<Colorize, 6, widths[6]>("line"),
        format_column<Colorize, 7, widths[7]>(print_detailed ? "LIVENESS" : "")
    );

    // Write separating dash line.
    out << std::string(quad_header_width, '-') << '\n';

    // Write quads.
    const auto quads_size = quads.size();
    for (alpha::u32 i = 0; i < quads_size; i++)
    {
        const alpha::ir::Quad& q = quads[i];

        const std::string quad_label_str =
            alpha::ir::info_traits::is_branching(quads[i].opcode)
            ? to_string(q.label)
            : alpha::k_not_available_pretty_marker;

        const auto [first_line, last_line] = lt.find_lines(q.loc);
        const std::string quad_line_str =
            first_line == last_line
            ? std::to_string(first_line.value)
            : FMT::format("{}-{}", first_line.value, last_line.value);

        out << FMT::format(
            "{0} {1} {2} {3} {4} {5} {6} {7}\n",
            format_column<Colorize, 0, widths[0]>(i + 1), // +1 as, 0 is indicating no-address
            format_column<Colorize, 1, widths[1]>(to_string(q.opcode)),
            format_column<Colorize, 2, widths[2]>(expr_formatter(q.result)),
            format_column<Colorize, 3, widths[3]>(expr_formatter(q.arg1)),
            format_column<Colorize, 4, widths[4]>(expr_formatter(q.arg2)),
            format_column<Colorize, 5, widths[5]>(quad_label_str),
            format_column<Colorize, 6, widths[6]>(quad_line_str),
            format_column<Colorize, 7, widths[7]>(print_detailed && q.is_dead ? "DEAD" : "")
        );
    }
    if constexpr (Colorize)
        out << SGR_RESET;
    out << std::endl;
}

template <bool Colorize, typename Stream>
void print_abc(Stream& out, const alpha::vm::Program& program, const alpha::LocationTracker& lt)
{
    constexpr alpha::u32 widths[] = {10, 15, 20, 20, 20, 10, 10, 10};

    out << FMT::format(
        "=============== PROGFUNC_TABLE ===============\n"
        "{0} {1} {2}\n",
        format_column<Colorize, 0, widths[0]>("address"),
        format_column<Colorize, 1, widths[1]>("size"),
        format_column<Colorize, 2, widths[2]>("id")
    );

    for (const auto& userfunc : program.progfuncs)
    {
        const alpha::StringSpan& userfunc_name =
            *DEBUG_REQUIRE_PTR(
                program.progfunc_name_registry.get_by_indexed_type(userfunc.name_str_id));
        out << FMT::format(
            "{0} {1} {2}\n",
            format_column<Colorize, 0, widths[0]>(FMT::to_string(userfunc.address.value)),
            format_column<Colorize, 1, widths[1]>(FMT::to_string(userfunc.local_count)),
            format_column<Colorize, 2, widths[2]>(userfunc_name.to_string_view())
        );
    }

    out << "=============== STRING_POOL ===============\n";
    const auto sorted_strs = [&program]()
    {
        std::vector<alpha::StringSpan> vec = program.str_literal_registry.from_value_view();
        std::ranges::sort(vec, [](const auto& a, const auto& b)
        {
            return a.to_string_view() < b.to_string_view();
        });
        return vec;
    }();
    for (const alpha::StringSpan str : sorted_strs)
    {
        const std::optional<unsigned> str_id = program.str_literal_registry.get_index_of(str);
        DMASSERT(str_id.has_value() && "String came from the registry itself, it must have value");
        out << FMT::format("{:>4}. {}\n", *str_id, escape(str));
    }

    out << FMT::format( // Write export header.
        "=============== BYTECODE ===============\n"
        "{0} {1} {2} {3} {4} {5}\n",
        format_column<Colorize, 0, widths[0]>("instr#"),
        format_column<Colorize, 1, widths[1]>("opcode"),
        format_column<Colorize, 2, widths[2]>("result"),
        format_column<Colorize, 3, widths[3]>("arg1"),
        format_column<Colorize, 4, widths[4]>("arg2"),
        format_column<Colorize, 5, widths[5]>("line")
    );

    for (alpha::u32 i = 0; i < program.instructions.size(); ++i)
    {
        const auto& inst = program.instructions[i];
        const auto [first_line, last_line] = lt.find_lines(inst.loc);
        const std::string quad_line_str =
            first_line == last_line
            ? std::to_string(first_line.value)
            : FMT::format("{}-{}", first_line.value, last_line.value);
        out << FMT::format(
            "{0} {1} {2} {3} {4} {5}\n",
            format_column<Colorize, 0, widths[0]>(i + 1), // +1 as, 0 is indicating no-address
            format_column<Colorize, 1, widths[1]>(to_string(inst.opcode)),
            format_column<Colorize, 2, widths[2]>(argument_formatter(inst.result.get())),
            format_column<Colorize, 3, widths[3]>(argument_formatter(inst.arg1.get())),
            format_column<Colorize, 4, widths[4]>(argument_formatter(inst.arg2.get())),
            format_column<Colorize, 5, widths[5]>(quad_line_str)
        );
    }
}
} // namespace
namespace alpha
{
inline constexpr auto k_scanner_eof_null_padding = 2; // For 2 consecutive NULL bytes.

CompilationPipeline::CompilationPipeline(
    const settings::ExprOpts& expr_opts,
    const settings::IROpts& ir_opts,
    const settings::ConfigFlags& config_flags,
    TranslationUnitBuffer& tu_buffer,
    LocationTracker& lt,
    DiagnosticEngine& diagnostic_engine,
    SymbolTable* const symbol_table)
    : lt_(lt),
      diagnostic_engine_(diagnostic_engine),
      parse_ctx_(support::require_ptr(symbol_table)),
      lexer_ctx_(),
      scanner_(std::make_unique<ScannerAdapter>(
          lexer_ctx_, lt, diagnostic_engine.reporter(), tu_buffer
          )),
      semantic_system_(
          expr_opts,
          &parse_ctx_,
          support::require_ptr(symbol_table),
          diagnostic_engine_.reporter()
      ),
      ir_validator_(std::make_unique<IRValidator>(config_flags, diagnostic_engine.reporter())),
      ir_optimizer_(std::make_unique<IROptimizer>(ir_opts)) { DMASSERT(!!ir_optimizer_); }

void
CompilationPipeline::scan_tokens()
{
    std::cout << "\n--------------------Lexical Analysis--------------------\n"
        << FMT::format( // Header
            "{0} {1} {2}",
            format_column<true, 0, 10>("#"),
            format_column<true, 1, 10>("line"),
            format_column<true, 2, 10>("token")
        )
        << std::endl;

    ALPHA_YYSTYPE yystype;
    ALPHA_YYLTYPE yyltype;

    u32 token_counter = 0;
    while (true)
    {
        const int token = scanner_->alpha_yylex(
            &yystype,
            &yyltype,
            lexer_ctx_,
            lt_,
            diagnostic_engine_.reporter()
        );

        if (token == TKN_YYEOF)
            break;


        std::cout << FMT::format(
                "{0} {1} {2}",
                format_column<true, 0, 10>(lt_.find_first_line(yyltype).value),
                format_column<true, 1, 10>(token_counter++),
                format_column<true, 2, 10>(token)
            )
            << std::endl;
    }
    std::cout << "\n----------------End of Lexical Analysis-----------------\n";
}

void
CompilationPipeline::execute()
{
    running_phase_ = Phase::FRONTEND;
    run_frontend();
    ir_quads_ = semantic_system_.gateway->extract_quads();

    ir_validator_->run(ir_quads_);

    if (diagnostic_engine_.has_errors())
        return;

    running_phase_ = Phase::IR_OPTIMIZATION;
    ir_quads_ = ir_optimizer_->run(std::move(ir_quads_));

    running_phase_ = Phase::ABC_GENERATION;

    program_ = std::make_unique<vm::Program>(ABC_Generator::run(
        ABC_Generator::Config{ir_quads_, parse_ctx_.space_handler.global_var_count()}
    ));
}

void
CompilationPipeline::run_frontend()
{
    // The following are the possible return values yyparse can return (based on bison's manual).
    // constexpr auto successful_parsing = 0;
    // constexpr auto invalid_input = 1;
    // constexpr auto memory_exhaustion = 2;

    parser_retval_ = alpha_yyparse(
        *scanner_,
        lexer_ctx_,
        lt_,
        diagnostic_engine_,
        diagnostic_engine_.reporter(),
        semantic_system_
    );
}

bool
CompilationPipeline::is_poisoned() const { return semantic_system_.good(); }

const ir::QuadStream&
CompilationPipeline::get_quads() const noexcept { return ir_quads_; }

const vm::Program&
CompilationPipeline::get_program() const noexcept
{
    DMASSERT(!!program_);
    return *program_;
}

void
CompilationPipeline::notify_hard_error()
{
    switch (running_phase_)
    {
    case Phase::FRONTEND: return semantic_system_.gateway->notify_hard_error();
    default: UNREACHABLE("Unknown `running_phase`");
    }
}

DiagnosticEngine::Policy
TranslationUnit::create_diagnostic_engine_policy()
{
    return DiagnosticEngine::Policy{
        .should_emit_diagnostic = [this]() { return compilation_pipeline_->is_poisoned(); },
        .notify_max_errors_reached = []() { notify_max_errors_reached(); },
        .notify_fatal_error = [this]()
        {
            compilation_pipeline_->notify_hard_error();
            notify_fatal_error();
        },
        .notify_hard_error = [this]() { compilation_pipeline_->notify_hard_error(); }
    };
}

TranslationUnit::TranslationUnit(
    const std::filesystem::path& source_path,
    const std::size_t max_errors,
    const settings::ExprOpts& expr_opts,
    const settings::IROpts& ir_opts,
    const settings::ConfigFlags& config_flags)
    : source_path_(source_path),
      expr_opts_(expr_opts),
      diagnostic_engine_(create_diagnostic_engine_policy(), max_errors),
      translation_unit_buffer_(
          TranslationUnitBufferLoader::load_tub(source_path, k_scanner_eof_null_padding)
      ),
      loc_tracker_(translation_unit_buffer_->size() - translation_unit_buffer_->null_padding),
      diagnostic_formatter_(
          source_path, loc_tracker_, *support::require_ptr(translation_unit_buffer_.get()), true
      ),
      symbol_table_(),
      compilation_pipeline_(std::make_unique<CompilationPipeline>(
          expr_opts,
          ir_opts,
          config_flags,
          *support::require_ptr(translation_unit_buffer_.get()),
          loc_tracker_,
          diagnostic_engine_,
          &symbol_table_
      )) {}

TranslationUnit::~TranslationUnit() = default;

void
TranslationUnit::compile()
{
    struct FreezeOnExit // A way to run code before function frame collapses (return, throw)
    {
        LocationTracker& loc_tracker;
        explicit FreezeOnExit(LocationTracker& loc_tracker) : loc_tracker(loc_tracker) {}
        ~FreezeOnExit() { loc_tracker.lines_frozen.raise(); }
    } freeze_on_exit(loc_tracker_);

    try
    {
        compilation_pipeline_->execute();
        // If we reach this point, execute() completed without throwing any exceptions.
        execution_completed_ = true;
    }
    catch (exception::SanityLimitError& e)
    {
        std::cerr << FMT::format("Sanity limit exceeded: {}", e.what()) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (exception::DiagnosticFatalError) { /* Reached a fatal diagnostic - stop compilation. */ }
    catch (exception::DiagnosticLimitError) { /* Reached diagnostics limit  - stop compilation. */ }
}

void
TranslationUnit::notify_fatal_error() { throw exception::DiagnosticFatalError{}; }

void
TranslationUnit::notify_max_errors_reached() { throw exception::DiagnosticLimitError{}; }

void
TranslationUnit::only_lex_tokens() const { compilation_pipeline_->scan_tokens(); }

template <bool show_temps>
void
TranslationUnit::show_symbol_table_impl() const
{
    std::cout << COLOR_FG_ASCII_BLUE;
    const auto& symbol_per_scope_vector = symbol_table_.symbols_per_scope();
    for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
    {
        if (symbol_per_scope_vector[scope].empty())
            continue;
        std::cout << FMT::format(
            "----------------------------------     Scope #{:<4}     ----------------------------------\n",
            scope
        );
        for (const Symbol* symbol_ptr : symbol_per_scope_vector[scope])
        {
            if constexpr (!show_temps)
                if (symbol_ptr->is_variable() && static_cast<const VarSymbol*>(symbol_ptr)->is_temp)
                    continue;

            std::cout << FMT::format(
                "{:<30} {:<20} (line {:>5}) (scope {:>4}) {}\n",
                FMT::format("\"{}\"", symbol_ptr->name.to_string_view()),
                FMT::format("[{}]", symbol_ptr->type_to_string()),
                loc_tracker_.find_symbol_line(symbol_ptr->loc).value,
                symbol_ptr->scope,
                symbol_ptr->is_variable()
                ? FMT::format("(offset {})", static_cast<const VarSymbol*>(symbol_ptr)->offset)
                : ""
            );
        }
        std::cout << '\n';
    }
    std::cout << SGR_RESET << std::endl;
}

void
TranslationUnit::show_symbol_table() const { show_symbol_table_impl<true>(); }

void
TranslationUnit::show_symbol_table_without_temps() const { show_symbol_table_impl<false>(); }

void
TranslationUnit::show_diagnostics() const
{
    const std::string source_filename = source_path_.filename().string();
    for (const auto& diagnostic : diagnostic_engine_.get_diagnostics())
        std::cerr << diagnostic_formatter_.format(*diagnostic);
    if (!diagnostic_engine_.get_diagnostics().empty())
        std::cerr << std::endl;
}

void
TranslationUnit::show_ir(const bool detailed) const
{
    print_ir<true>(std::cout, compilation_pipeline_->get_quads(), loc_tracker_, detailed);
}

void
TranslationUnit::show_abc() const
{
    print_abc<true>(std::cout, compilation_pipeline_->get_program(), loc_tracker_);
}


void
TranslationUnit::export_symbol_table() const
{
    export_within_dir(
        k_symbol_table_exports_dirname,
        [this]() { export_symbol_table_dispatch(); }
    );
}

void
TranslationUnit::export_diagnostics() const
{
    const auto impl = [this]()
    {
        const std::string outfile_name = source_path_.filename().string() + k_diagnostic_export_ext;
        std::ofstream outfile(outfile_name);
        if (!outfile)
            throw std::runtime_error(FMT::format(
                "Failed opening file {} to export compile errors", outfile_name));

        outfile << k_diagnostic_csv_export_header; // Write CSV header.
        auto write_issue_line = [&](const DiagnosticCode code, const Issue& issue)
        {
            outfile << FMT::format(
                "{0},{1},{2},{3}\n",
                to_string(code),
                issue.line(loc_tracker_).value,
                issue.column(loc_tracker_).value,
                to_string(issue.type)
            );
        };

        for (const auto& d : diagnostic_engine_.get_diagnostics())
        {
            write_issue_line(d->code, d->primary);
            for (const Issue& note : d->note_list)
                write_issue_line(d->code, note);
        }
    };

    export_within_dir(k_diagnostic_exports_dirname, [impl]() { impl(); });
}

void
TranslationUnit::export_ir() const
{
    const auto impl = [this]()
    {
        const std::string outfile_name = source_path_.filename().string() + k_ir_export_ext;
        std::ofstream outfile(outfile_name);
        if (!outfile)
            throw std::runtime_error(
                FMT::format("Failed opening file {} to export ir", outfile_name));

        outfile << k_ir_csv_export_header; // Write CSV header.

        auto write_ir_line = [&](const std::size_t quad_no, const ir::Quad& q)
        {
            const auto [first_line, last_line] = loc_tracker_.find_lines(q.loc);
            std::string quad_label_str =
                alpha::ir::info_traits::is_branching(q.opcode)
                ? std::to_string(q.label.value)
                : alpha::k_not_available_marker;

            outfile << FMT::format(
                "{0},{1},{2},{3},{4},{5},{6},{7}\n",
                quad_no,
                to_string(q.opcode),
                expr_formatter(q.result, k_not_available_marker),
                expr_formatter(q.arg1, k_not_available_marker),
                expr_formatter(q.arg2, k_not_available_marker),
                quad_label_str,
                first_line.value,
                last_line.value
            );
        };

        const auto& quads = compilation_pipeline_->get_quads();
        for (std::size_t i = 0; i < quads.size(); ++i)
            write_ir_line(i + 1, quads[i]); // +1 cause quad address 0 is indicating no-address
    };

    export_within_dir(k_ir_exports_dirname, [impl]() { impl(); });
}

void
TranslationUnit::emit_abc() const
{
    const std::vector<u8> abc = ABC_Serializer::serialize(compilation_pipeline_->get_program());

    const auto impl = [this, &abc]()
    {
        const std::string outfile_name = source_path_.stem().string() + k_abc_binary_ext;
        std::ofstream outfile{outfile_name, std::ios::binary};
        if (!outfile)
            throw std::runtime_error("Failed opening file for writing: " + outfile_name);
        outfile.write(reinterpret_cast<const char*>(abc.data()), abc.size());
    };

    export_within_dir(k_abc_binaries_dirname, impl);
}

bool TranslationUnit::compiled_ok() const noexcept
{
    return execution_completed_ && !diagnostic_engine_.has_errors();
}

template <bool export_temps>
void
TranslationUnit::export_symbol_table_impl() const
{
    const std::string outfile_name = source_path_.filename().string() + k_symbol_table_export_ext;
    std::ofstream outfile(outfile_name);
    if (!outfile)
        throw std::runtime_error(
            FMT::format("Failed opening file {} to export symbol table", outfile_name));

    outfile << k_symbol_table_csv_export_header; // Write CSV header.

    auto write_symbol_line = [&](const Symbol* symbol_ptr)
    {
        outfile << FMT::format(
            "{0},{1},{2},{3}\n",
            symbol_ptr->name.to_string_view(),
            symbol_ptr->type_to_string(),
            loc_tracker_.find_symbol_line(symbol_ptr->loc).value,
            symbol_ptr->scope
        );
    };

    const auto& symbol_per_scope_vector = symbol_table_.symbols_per_scope();
    for (u32 scope = k_global_scope; scope < symbol_per_scope_vector.size(); scope++)
        for (const Symbol* symbol_ptr : symbol_per_scope_vector[scope])
        {
            if constexpr (!export_temps)
                if (symbol_ptr->is_variable() && static_cast<const VarSymbol*>(symbol_ptr)->is_temp)
                    continue;
            write_symbol_line(symbol_ptr);
        }
}

void
TranslationUnit::export_symbol_table_without_temps_dispatch() const
{
    export_symbol_table_impl<false>();
}

void
TranslationUnit::export_symbol_table_dispatch() const { export_symbol_table_impl<true>(); }

void
TranslationUnit::export_symbol_table_without_temps() const
{
    export_within_dir(
        k_symbol_table_exports_dirname,
        [this]() { export_symbol_table_impl<false>(); }
    );
}

void
TranslationUnit::export_within_dir(
    const std::string_view dirname,
    std::function<void()> export_func) const
{
    const auto original_path = std::filesystem::current_path();
    create_export_directory(dirname);
    enter_export_directory(dirname);
    export_func();
    exit_export_directory(original_path);
}
} // namespace alpha
