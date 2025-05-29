// Note: Alpha Parser Context is implemented as a header-only class.
//       Since it consists mostly of small, frequently used functions,
//       we define all methods in the header and mark them as `inline`.
//
//       This serves two purposes:
//       1) To comply with the One Definition Rule (ODR) when the header
//          is included in multiple translation units.
//       2) To hint to the compiler to inline calls for better performance.

#ifndef ALPHA_PARSER_CONTEXT_HPP
#define ALPHA_PARSER_CONTEXT_HPP

#include <limits>
#include <list>
#include <stack>
#include <vector>
#include "_parser_common.hpp"
#include "core/alpha_konstants.hpp"
#include "core/alpha_numeric_types.hpp"
#include "parser/alpha_symbol_table.hpp"
#include "utils/misc.hpp"
#include "utils/smart_assert.h"

namespace Alpha
{
struct ParseCache;
class SpaceHandler;
class ScopeHandler;
class FunctionCtxHandler;
struct FunctionBackpatchInfo;
struct FunctionDataFrame;
class NameGenerator;
class QuadHandler;
class ExprHandler;
class ParseCtx;

/**
 * @brief Temporary semantic state used during parsing.
 *
 * ParseCache holds intermediate data needed across specific grammar rules
 * during Bison parsing. It stores complex semantic values that cannot
 * safely or cleanly be represented in the %union (e.g., std::string,
 * structs with non-trivial constructors).
 *
 * This separation exists because the parser uses Bison's C backend,
 * which relies on a raw union for token values. To maintain clean
 * memory safety and modern C++ idioms (RAII, strong typing), ParseCache
 * acts as a companion scratchpad for semantic actions that require
 * richer state tracking.
 *
 * Each substructure in ParseCache typically maps to a specific grammar rule
 * or parsing context (e.g., function headers, block spans), allowing
 * clean separation, clarity, and scalability.
 *
 * Lifetime: ParseCache is owned by ParseCtx and lives for the duration
 * of parsing a single alpha source file.
 */
struct ParseCache
{ // TODO: all these names are ugly.. maybe technique is ugly too.. REFACTOR ASAP!
    struct funcPrefixState
    {
        std::string id;
        SourceLocation location{};
    } func_prefix;

    struct methodCallIdState
    {
        std::string id;
        SourceLocation id_location;
        SourceLocation method_call_location;
    } method_call_id;

    struct ifPrefixState
    {
        std::stack<u32> quads_to_patch;
    } if_prefix;

    struct elsePrefixState
    {
        std::stack<u32> quads_to_patch;
    } else_prefix;

    struct
    {
        std::stack<u32> next_quad_stack;
    } logical_marker;

    struct whileStartState
    {
        std::stack<u32> next_quad_stack;
    } while_start;

    struct whileConditionState
    {
        std::stack<u32> quads_to_patch;
    } while_condition;

    struct
    {
        std::stack<u32> quads_to_patch_1;
        std::stack<u32> quads_to_patch_2;
        std::stack<u32> quads_to_patch_3;
    } n;

    struct
    {
        std::stack<u32> quads_to_patch;
    } m;

    struct forHeaderState
    {
        std::stack<u32> test_quads_to_patch;
        std::stack<u32> enter_quads_to_patch;
    } for_header;
};

class SpaceHandler : private Immobile
{
public:
    static constexpr u32 k_initial_space = 0;
    static_assert(k_initial_space == 0, "Space mapping logic requires 0");
    static constexpr u32 k_initial_variable_offset = 0;
    static_assert(k_initial_variable_offset == 0, "Variable offset must start at 0");

    SpaceHandler();

    ~SpaceHandler();

    void enter_space();

    void exit_space();

    [[nodiscard]] Variable::Space space() const noexcept;

    [[nodiscard("Discarding this return value breaks variable offset sequencing")]]
    u32 next_offset() noexcept;

private:
    std::stack<u32> variable_offset_stack_;
};

class ScopeHandler : private Immobile
{
public:
    ScopeHandler();

    ~ScopeHandler() = default;

    void skip_next_scope_increment() noexcept;

    void enter_scope() noexcept;

    void exit_scope() noexcept;

    [[nodiscard]] u32 scope() const noexcept { return scope_; }

private:
    ToggleSwitch skip_next_scope_increment_;
    u32 scope_;
};

class FunctionCtxHandler : private Immobile
{
public:
    struct FunctionBackpatchInfo
    {
        const std::string name;
        const u32 scope;
        const SourceLocation location;
        const u32 local_variable_count;
        const Function *function_symbol;
        const u32 label_to_jump; // label to jump over function def in runtime.. TODO:
        // please rename its ugly
    };

    FunctionCtxHandler(ParseCtx &parse_ctx);

    ~FunctionCtxHandler();

    void enter_loop() noexcept;

    void exit_loop() noexcept;

    void add_function_parameter(const std::string &name, SourceLocation loc);

    void clear_function_parameters() noexcept;

    void add_local() noexcept;

    void enter_function(const Function *function_symbol, u32 label_of_jump);

    [[nodiscard]] FunctionBackpatchInfo exit_function() noexcept;

    [[nodiscard]] u32 function_nesting_depth() const noexcept;

    [[nodiscard]] u32 current_function_scope() const noexcept;

    [[nodiscard]] const std::string &current_function_name() const noexcept;

    [[nodiscard]] SourceLocation current_function_location() const noexcept;

    [[nodiscard]] u32 loop_depth() const noexcept;

    [[nodiscard]] const std::list<Parameter> &function_parameters() const noexcept;

    [[nodiscard]] u32 next_function_address() noexcept { return next_function_address_++; }

    [[nodiscard]] const std::vector<u32> &get_breaklist() // TODO CLEANUP
    {
        DEBUG_SMART_ASSERT(!frame_stack_.empty());
        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_breaklist_stack.empty());

        return frame_stack_.top().function_breaklist_stack.top();
    }

    [[nodiscard]] const std::vector<u32> &get_continuelist() // TODO CLEANUP
    {
        DEBUG_SMART_ASSERT(!frame_stack_.empty());
        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_continuelist_stack.empty());

        return frame_stack_.top().function_continuelist_stack.top();
    }

    void add_label_to_breaklist(u32 jump_label) // Quad label of jump used to break.
    {
        DEBUG_SMART_ASSERT(!frame_stack_.empty()); // TODO replace 1 (due to global frame)
        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_breaklist_stack.empty());

        // we used Stack so each loop has its own break list
        frame_stack_.top().function_breaklist_stack.top().push_back(jump_label);
    }

    void add_label_to_continuelist(u32 jump_label) // Quad label of jump used to break.
    {
        // TODO repetitive code (same as continue.. DRY it out).
        DEBUG_SMART_ASSERT(!frame_stack_.empty()); // TODO replace 1 (due to global frame)

        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_continuelist_stack.empty());

        // we used Stack so each loop has its own continue list.
        frame_stack_.top().function_continuelist_stack.top().push_back(jump_label);
    }

    [[nodiscard]] const std::vector<u32> &get_returnlist() // TODO CLEANUP
    {
        // We must be in a function. // Note at size 1. it global dataframe
        // So calling this function while there is only 1 framestack is a logic issue.
        DEBUG_SMART_ASSERT(frame_stack_.size() > 1); // TODO replace 1 (due to global frame)
        return frame_stack_.top().function_returnlist;
    }

    void add_label_to_returnlist(const u32 jump_label) // Quad label of jump used to break.
    {
        // We must be in function
        DEBUG_SMART_ASSERT(frame_stack_.size() > 1); // TODO replace 1 (due to global frame)

        frame_stack_.top().function_returnlist.push_back(jump_label);
    }

private:
    struct FunctionDataFrame
    {
        const std::string name;
        const u32 scope;
        const SourceLocation location;
        const Function *function_symbol; // Valid function ONLY IF NOT nullptr;

        u32 loop_nesting_count = 0;

        // This is labels of breaks per loop in function
        std::stack<std::vector<u32>> function_breaklist_stack;
        // This is labels of continue of loops per loop in function
        std::stack<std::vector<u32>> function_continuelist_stack;

        // This is labels returns per function (in this FunctionDataFrame).
        std::vector<u32> function_returnlist;

        const u32 label_of_jump; // used to go over function definition in runtime.

        u32 local_variable_count = 0;

        FunctionDataFrame(std::string name, const u32 scope, const SourceLocation loc,
                          const Function *f,
                          const u32 label_of_jump)
            : name(std::move(name)),
              scope(scope),
              location(loc),
              function_symbol(f),
              label_of_jump(label_of_jump)
        {}
    };

    std::stack<FunctionDataFrame> frame_stack_;
    std::list<Parameter> function_parameters_;
    u32 next_function_address_ = 0;

    ParseCtx &parse_ctx_;
};

class NameGenerator : private Immobile
{
public:
    [[nodiscard]] std::string new_temp_name();

    void reset_temp_names() { temp_name_counter_ = 0; }

    [[nodiscard]] std::string new_anonymous();

private:
    u32 temp_name_counter_ = 0;
    u32 anonymous_counter_ = 0;
};

// class QuadHandler : private Immobile
{
public:
        void emit_quad(IOPCode iopc, const Expr *arg1, const Expr *arg2, const Expr *result,
                       SourceLocation loc);

        void emit_quad_w_jump_step(IOPCode iopc, const Expr *arg1, const Expr *arg2, u32 jump_step,
                                   SourceLocation loc);

        void emit_quad_labelless(IOPCode iopc, const Expr *arg1, const Expr *arg2,
                                 const Expr *result, SourceLocation loc);

        void emit_quad_w_label(IOPCode iopc, const Expr *arg1, const Expr *arg2,
                               const Expr *result, u32 label, SourceLocation loc);

        void patch_quad(u32 target_quad_label, u32 destination_label);

        void patch_list(const std::vector<u32> &patch_list, u32 destination_label);

        [[nodiscard]] const std::vector<Quad> &quads() const { return quads_; }
        [[nodiscard]] u32 next_quad_label() const { return next_quad_label_; }

private:
        [[nodiscard]] static bool requires_label(IOPCode iopc) noexcept;

        void emit_quad_impl(IOPCode iopc, const Expr *arg1, const Expr *arg2, const Expr *result,
                            u32 label, SourceLocation loc);

        std::vector<Quad> quads_;
        u32 next_quad_label_ = 1; // First quad_label is always 1, (0 for backpatching)
};

class ExprHandler : private Immobile
{
public:
    explicit ExprHandler(ParseCtx &parse_ctx);

    ~ExprHandler() noexcept;

    [[nodiscard]] Expr *emit_quad_if_table_item(Expr *expr);

private:
    std::vector<const Expr *> expr_sink_;
    ParseCtx &parse_ctx_;
};

class ParseCtx : private Immobile
{
public:
    ParseCache cache;
    SpaceHandler space_handler;
    ScopeHandler scope_handler;
    FunctionCtxHandler function_ctx_handler;
    QuadHandler quad_handler;
    ExprHandler expr_handler;
    NameGenerator name_generator;

    ParseCtx(SymbolTable &st, CTIssueTracker &et);

    ~ParseCtx() = default;

    [[nodiscard]] const Variable *new_temp();

private:
    SymbolTable &st_;
    [[maybe_unused]] CTIssueTracker &et_; // TODO: remove if unused
};

inline SpaceHandler::SpaceHandler()
{
    enter_space(); // We push the first scope space frame (PROGRAM_VAR)
};

inline SpaceHandler::~SpaceHandler()
{
    std::cerr << "VARIABLE OFFSET_STACK_SIZE = " << variable_offset_stack_.size() << std::endl;
    DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1);
}

inline void SpaceHandler::enter_space() { variable_offset_stack_.push(k_initial_variable_offset); }

inline void SpaceHandler::exit_space()
{
    constexpr auto spaces_for_closure = 2; // 1 formalArg + 1 functionLocal

    DEBUG_SMART_ASSERT(                                         //
        variable_offset_stack_.size() > spaces_for_closure, //
        is_odd(variable_offset_stack_.size())               //
    );

        #pragma unroll
    for (auto i = 0; i < spaces_for_closure; ++i)
        variable_offset_stack_.pop();
}

inline Variable::Space SpaceHandler::space() const noexcept
{
    DEBUG_SMART_ASSERT(!variable_offset_stack_.empty()); // A stack frame must always exist
    const auto frame_index = variable_offset_stack_.size() - 1; // -1 for size to index

    if (frame_index == k_initial_space)
        return Variable::Space::PROGRAM_VAR;
    if (is_odd(frame_index))
        return Variable::Space::FORMAL_ARGUMENT;
    return Variable::Space::FUNCTION_LOCAL;
}

inline u32 SpaceHandler::next_offset() noexcept
{
    DEBUG_SMART_ASSERT(!variable_offset_stack_.empty());
    return variable_offset_stack_.top()++;
}

inline ScopeHandler::ScopeHandler() : scope_(k_global_scope)
{
    // A ToggleSwitch must always be initialized as disabled.
    SMART_ASSERT(skip_next_scope_increment_.is_disabled());
}

inline void ScopeHandler::skip_next_scope_increment() noexcept
{
    skip_next_scope_increment_.enable();
}

inline void ScopeHandler::enter_scope() noexcept
{
    if (skip_next_scope_increment_.is_enabled())
    {
        skip_next_scope_increment_.disable();
        return;
    }
    DEBUG_SMART_ASSERT(scope_ < k_max_scope);
    ++scope_;
}

inline void ScopeHandler::exit_scope() noexcept
{
    // We expect `skip_next_scope_increment` ToggleSwitch to be
    // disabled. That is because if you exit a block, it means you first
    // entered it. So if you exit a block and the
    // `skip_next_scope_increment` switch is enabled, there is a logic
    // issue.
    DEBUG_SMART_ASSERT(                              //
        scope_ > k_global_scope,                 //
        skip_next_scope_increment_.is_disabled() //
    );
    --scope_;
}

inline FunctionCtxHandler::FunctionCtxHandler(ParseCtx &parse_ctx) : parse_ctx_(parse_ctx)
{
    // We push a stack-frame, for loops that might occur outside functions.
    // So every frame corresponds to a function except the first.
    frame_stack_.emplace(k_global_data_frame_name, k_global_scope, k_no_location, nullptr,
                         k_no_label);
}

inline FunctionCtxHandler::~FunctionCtxHandler()
{
    DEBUG_SMART_ASSERT(                                       //
        frame_stack_.size() == k_global_data_frame_count, //
        function_parameters_.empty()                  // All parameters must be used.
    );
}

// Label of jump is for jumping over the function is runtime...
inline void FunctionCtxHandler::enter_function(const Function *function_symbol,
                                               const u32 label_of_jump)
{
#ifdef DEBUG_MODE
    DEBUG_SMART_ASSERT(frame_stack_.size() < k_max_function_nesting);
    if (!!function_symbol)
        DEBUG_SMART_ASSERT(
        function_symbol->name == parse_ctx_.cache.func_prefix.id,
        function_symbol->scope == parse_ctx_.scope_handler.scope(),
        function_symbol->loc == parse_ctx_.cache.func_prefix.location,
        function_symbol->is_function(),
        function_symbol->type == Symbol::Type::PROGRAM_FUNCTION
        // Only library functions are defined in source code.
    );
#endif // DEBUG_MODE

    frame_stack_.emplace(FunctionDataFrame(
        parse_ctx_.cache.func_prefix.id, parse_ctx_.scope_handler.scope(),
        parse_ctx_.cache.func_prefix.location, function_symbol, label_of_jump));

    // Function scope is entered here.
    // We skip the next `{` block’s scope to avoid double scoping.
    parse_ctx_.scope_handler.enter_scope();
    parse_ctx_.scope_handler.skip_next_scope_increment();
}

inline FunctionCtxHandler::FunctionBackpatchInfo FunctionCtxHandler::exit_function() noexcept
{
    // A frame always exist for loops outside functions.
    DEBUG_SMART_ASSERT(frame_stack_.size() > k_global_data_frame_count);
    // All loops must be closed before exiting function.
    DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count == 0);

    const FunctionDataFrame top_frame = std::move(frame_stack_.top());
    frame_stack_.pop();

    return {
        .name = top_frame.name,
        .scope = top_frame.scope,
        .location = top_frame.location,
        .local_variable_count = top_frame.local_variable_count,
        .function_symbol = top_frame.function_symbol,
        .label_to_jump = top_frame.label_of_jump
    };
}

inline u32 FunctionCtxHandler::function_nesting_depth() const noexcept
{
    return frame_stack_.size() - k_global_data_frame_count;
}

inline u32 FunctionCtxHandler::current_function_scope() const noexcept
{
    DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
    return frame_stack_.top().scope;
}

inline const std::string &FunctionCtxHandler::current_function_name() const noexcept
{
    DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
    return frame_stack_.top().name;
}

inline SourceLocation FunctionCtxHandler::current_function_location() const noexcept
{
    DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
    return frame_stack_.top().location;
}

inline void FunctionCtxHandler::enter_loop() noexcept
{
    DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
    DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count < k_max_loop_nesting);

    ++frame_stack_.top().loop_nesting_count;

    // Emplace empty breaklist (vector)
    frame_stack_.top().function_breaklist_stack.emplace();

    // Emplace empty continuelist (vector)
    frame_stack_.top().function_continuelist_stack.emplace();

    std::cout << "At enter_loop function_continuelist_stack_size == "
        << frame_stack_.top().function_continuelist_stack.size() << std::endl;
}

inline void FunctionCtxHandler::exit_loop() noexcept
{
    DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
    DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count > 0);
    --frame_stack_.top().loop_nesting_count;

    DEBUG_SMART_ASSERT(frame_stack_.top().function_breaklist_stack.size() > 0);
    DEBUG_SMART_ASSERT(frame_stack_.top().function_continuelist_stack.size() > 0);
    // Emplace empty breaklist (vector)
    frame_stack_.top().function_breaklist_stack.pop();

    // Emplace empty continuelist (vector)
    frame_stack_.top().function_continuelist_stack.pop();
}

inline u32 FunctionCtxHandler::loop_depth() const noexcept
{
    DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
    return frame_stack_.top().loop_nesting_count;
}

inline void FunctionCtxHandler::add_function_parameter(const std::string &name, SourceLocation loc)
{
    function_parameters_.emplace_back(name, loc);
}

inline const std::list<Parameter> &FunctionCtxHandler::function_parameters() const noexcept
{
    return function_parameters_;
}

inline void FunctionCtxHandler::clear_function_parameters() noexcept
{
    function_parameters_.clear();
}

inline void FunctionCtxHandler::add_local() noexcept { ++frame_stack_.top().local_variable_count; }

inline std::string NameGenerator::new_temp_name()
{
    return k_temp_variable_prefix + std::to_string(temp_name_counter_++);
}

inline std::string NameGenerator::new_anonymous()
{
    return k_private_anonymous_prefix + std::to_string(anonymous_counter_++);
}

inline void QuadHandler::emit_quad(const IOPCode iopc, const Expr *arg1, const Expr *arg2,
                                   const Expr *result, const SourceLocation loc)
{
    // TODO. IF only used by non required-IOPCs, emit requires_label check and just put
    // k_no_label.
    emit_quad_impl(iopc, arg1, arg2, result,
                   requires_label(iopc) ? next_quad_label_ : k_no_label, loc);
}

inline void QuadHandler::emit_quad_w_label(const IOPCode iopc, const Expr *arg1, const Expr *arg2,
                                           const Expr *result, u32 label, const SourceLocation loc)
{
    emit_quad_impl(iopc, arg1, arg2, result, label, loc);
}

inline void QuadHandler::emit_quad_w_jump_step(const IOPCode iopc, const Expr *arg1,
                                               const Expr *arg2, const u32 jump_step,
                                               const SourceLocation loc)
{
    DEBUG_SMART_ASSERT(requires_label(iopc)); // This emit_quad overload is used for JUMP IOPCs
    emit_quad_impl(iopc, arg1, arg2, nullptr, next_quad_label_ + jump_step, loc);
}

inline void QuadHandler::emit_quad_labelless(const IOPCode iopc, const Expr *arg1, const Expr *arg2,
                                             const Expr *result, SourceLocation loc)
{
    emit_quad_impl(iopc, arg1, arg2, result, k_no_label, loc);
}

inline void QuadHandler::patch_quad(u32 target_quad_label, u32 destination_label)
{
    const u32 quad_index =
        target_quad_label - 1; // First quad at index 0, has quad with label 1.

    DEBUG_SMART_ASSERT(target_quad_label > 0,                  //
                       quad_index < quads_.size(),             //
                       quads_[quad_index].label == k_no_label, //
                       destination_label != k_no_label         //
    );
    quads_[quad_index].label = destination_label;
}

inline void QuadHandler::patch_list(const std::vector<u32> &patch_list, u32 destination_label)
{
    DEBUG_SMART_ASSERT(destination_label != k_no_label);
    for (u32 target_quad_label : patch_list)
        patch_quad(target_quad_label, destination_label);
}

inline void QuadHandler::emit_quad_impl(IOPCode iopc, const Expr *arg1, const Expr *arg2,
                                        const Expr *result, u32 label, SourceLocation loc)
{
    DEBUG_SMART_ASSERT(quads_.size() + 1 == next_quad_label_);
    // TODO. IF only used by non required-IOPCs, emit requires_label check and just put 0.
    quads_.emplace_back(Quad{
        .iopcode = iopc,
        .arg1 = arg1,
        .arg2 = arg2,
        .result = result,
        .label = label,
        .location = loc,
    });

    ++next_quad_label_;
}

inline Expr *ExprHandler::emit_quad_if_table_item(Expr *expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::TABLE_ITEM)
        return expr;

    Expr *expr_temp_var = make_expr_variable(parse_ctx_.new_temp(), k_no_location);

    parse_ctx_.quad_handler.emit_quad(
        IOPCode::TABLEGETELEM, expr, expr->index, expr_temp_var,
        k_no_location //

        //  expr_location_founder(expr) // TODO: REMOVE (if you dont want loc here)
    );

    return expr_temp_var;
}

inline ParseCtx::ParseCtx(SymbolTable &st, CTIssueTracker &et)
    : function_ctx_handler(*this), expr_handler(*this), st_(st), et_(et)
{}

inline const Variable *ParseCtx::new_temp()
{
    const std::string temp_name = name_generator.new_temp_name();
    const Symbol *symbol = st_.lookup_local(temp_name, scope_handler.scope());

    // We register new temp, only if current scope doesn't have that temp.
    if (!symbol)
    {
        const Variable::Type var_type =
            scope_handler.scope() == k_global_scope
            ? Variable::Type::GLOBAL_VARIABLE
            : Variable::Type::LOCAL_VARIABLE;

        symbol = st_.insert_variable(
            temp_name,
            scope_handler.scope(),
            var_type,
            space_handler.space(),
            space_handler.next_offset(),
            k_no_location
        );
    }
    // variables and values. known the line a temp was generated is useless...
    DEBUG_SMART_ASSERT(symbol->is_variable());
    return static_cast<const Variable *>(symbol);
}
} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP
