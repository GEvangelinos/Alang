#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

#include <string>			   // for string
#include "core/alpha_error.hpp"		   // for ErrorTracker
#include "core/alpha_location.hpp"	   // for Location
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_symbol_table.hpp"   // for Symbol, SymbolTable

#include <list> // for list, _List_const_iterator

#include "core/alpha_error.hpp"		   // for ErrorTracker, Diagnostic
#include "core/alpha_konstants.hpp"	   // for k_global_scope, k_public_...
#include "core/alpha_location.hpp"	   // for Location
#include "utils/misc.hpp"		   // for DEBUG_ALWAYS_INLINE
#include "core/alpha_types.hpp"		   // for u32
#include "parser/_parser_common.hpp"	   // for Parameter
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "utils/format_adapter.hpp"	   // for format, FMT
#include "utils/smart_assert.h"		   // for DEBUG_SMART_ASSERT
#include "parser/alpha_backpatcher.hpp"

using namespace Alpha;

namespace // (Anonymous)
{
	namespace Loop
	{
		enum class Keyword
		{
			BREAK,
			CONTINUE,
		};

		[[nodiscard]] std::string to_string(const Keyword keyword) noexcept
		{
			switch (keyword)
			{
			case Keyword::BREAK:
				return "break";
			case Keyword::CONTINUE:
				return "continue";
			default:
				[[unlikely]] SMART_ASSERT(false);
			}
		}
	}; // namespace Loop

	DEBUG_ALWAYS_INLINE void
	loopCtrlStmt__loopkeyword_impl(
	    const ParseCtx &parse_ctx,
	    const Location keyword_location,
	    ErrorTracker &et,
	    const Loop::Keyword keyword)
	{
		if (parse_ctx.function_ctx_handler.loop_depth() > 0)
			return;

		std::string keyword_name = Loop::to_string(keyword);
		std::string error = FMT::format("`{}` statement not in a loop statement", keyword_name);
		et.report_error(CTError::Type::SEMANTIC, error, keyword_location);
	}

	[[nodiscard]] DEBUG_ALWAYS_INLINE bool
	reported_parameter_name_conflict(
	    const SymbolTable &st,
	    const u32 current_scope,
	    const Parameter &parameter,
	    ErrorTracker &et)
	{
		// Library‐function conflict
		if (st.is_lib_function(parameter.name))
		{
			const std::string error = FMT::format(
			    "`{}` is a library function, can't declare it as formal", parameter.name);
			et.report_error(CTError::Type::SEMANTIC, error, parameter.location);
			return true;
		}
		const Symbol *formal_symbol = st.lookup_local(parameter.name, current_scope);
		// Parameter‐redeclared conflict
		if (formal_symbol)
		{
			// Parameter should produce name conflicts only with themselves.
			DEBUG_SMART_ASSERT(					      //
			    dynamic_cast<const Variable *>(formal_symbol) != nullptr, //
			    formal_symbol->is_variable()			      //
			);

			const std::string error = FMT::format("redefinition of parameter `{}`", parameter.name);
			const std::string note = FMT::format("previous definition of `{}` here", parameter.name);
			et.report_error(
			    CTError::Type::SEMANTIC, error, parameter.location, note, formal_symbol->location);
			return true;
		}
		return false;
	}

	[[nodiscard]] DEBUG_ALWAYS_INLINE bool
	reported_function_name_conflict(
	    SymbolTable &st,
	    const u32 current_scope,
	    const std::string function_name,
	    const Location id_location,
	    ErrorTracker &et)
	{
		if (st.is_lib_function(function_name))
		{
			const std::string error =
			    FMT::format("redefinition of library function `{}`", function_name);
			et.report_error(CTError::Type::SEMANTIC, error, id_location);
			return true;
		}

		const Symbol *found_symbol = st.lookup_local(function_name, current_scope);
		if (!found_symbol)
			return false;
		if (found_symbol->is_function())
		{
			const std::string error = FMT::format("redefinition of `function {}`", function_name);
			const std::string note =
			    FMT::format("previous definition of `function {}` here", function_name);
			et.report_error(
			    CTError::Type::SEMANTIC, error, id_location, note, found_symbol->location);
		}
		else if (found_symbol->is_variable())
		{
			const std::string error = FMT::format("`{}` redefined as a function", function_name);
			const std::string note =
			    FMT::format("`{}` previously defined as a variable here", function_name);
			et.report_error(
			    CTError::Type::SEMANTIC, error, id_location, note, found_symbol->location);
		}
		return true;
	}

	void report_out_of_scope_variable(
	    const char *id_name,
	    const Location id_location,
	    const std::string &current_function_name,
	    const Location current_function_location,
	    const Symbol *found_symbol,
	    ErrorTracker &et)
	{
		using DT = Diagnostic::Type;
		DEBUG_SMART_ASSERT(found_symbol != nullptr);
		const std::string error = FMT::format("variable `{}` is not accessible in function `{}`",
						      id_name, current_function_name);
		const std::string note1 = FMT::format("function `{}` declared here", current_function_name);
		const std::string note2 = FMT::format("variable `{}` declared here", id_name);
		et.report_error(CTError::Type::SEMANTIC, error, id_location,
				std::list<Diagnostic>{{DT::NOTE, note1, current_function_location},
						      {DT::NOTE, note2, found_symbol->location}});
	}

	DEBUG_ALWAYS_INLINE void
	insert_function_parameters(SymbolTable &st, ParseCtx &parse_ctx, ErrorTracker &et)
	{

		auto scope = parse_ctx.scope_handler.scope();
		constexpr auto space = Variable::Space::FORMAL_ARGUMENT;
		DEBUG_SMART_ASSERT(parse_ctx.space_handler.space() == Variable::Space::FORMAL_ARGUMENT);

		for (const Parameter &param : parse_ctx.function_ctx_handler.function_parameters())
			if (!reported_parameter_name_conflict(st, scope, param, et))
				st.insert_variable(
				    param.name,
				    scope,
				    space,
				    parse_ctx.space_handler.next_offset(),
				    param.location);
	}

	[[nodiscard]] DEBUG_ALWAYS_INLINE bool
	is_modifiable_lvalue(const Symbol *const lvalue)
	{
		if (lvalue == nullptr) // nullptr implies runtime-evaluated lvalue (e.g. member access)
			return true;
		return lvalue->is_variable();
	}

	DEBUG_ALWAYS_INLINE void
	term__lvalue_op(
	    const std::string &op_name,
	    const Symbol *lvalue,
	    const Location term_location,
	    ErrorTracker &et)
	{
		// lvalue is valid to be nullptr (runtime evaluation).
		DEBUG_SMART_ASSERT(op_name == "increment" || op_name == "decrement");
		if (is_modifiable_lvalue(lvalue))
			return;
		std::string error = FMT::format("{} operator can not be used on function", op_name);
		et.report_error(CTError::Type::SEMANTIC, error, term_location);
	}

} // namespace

// +-----------------------------------------------------------------+
// |---------------- SEMANTIC_ACTION_FUNCTIONS_BELOW ----------------|
// +-----------------------------------------------------------------+
inline void loopCtrlStmt__break(const ParseCtx &parse_ctx, const Location break_location, ErrorTracker &et)
{
	loopCtrlStmt__loopkeyword_impl(parse_ctx, break_location, et, Loop::Keyword::BREAK);
}

inline void loopCtrlStmt__continue(const ParseCtx &parse_ctx, const Location continue_location, ErrorTracker &et)
{
	loopCtrlStmt__loopkeyword_impl(parse_ctx, continue_location, et, Loop::Keyword::CONTINUE);
}

inline void term__inc_lvalue(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
	term__lvalue_op("increment", lvalue, term_location, et);
}

inline void term__lvalue_inc(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
	term__lvalue_op("increment", lvalue, term_location, et);
}

inline void term__dec_lvalue(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
	term__lvalue_op("decrement", lvalue, term_location, et);
}

inline void term__lvalue_dec(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
	term__lvalue_op("decrement", lvalue, term_location, et);
}

inline void assignExpr__lvalue_assign_expr(
    const Symbol *lvalue,
    const Location assign_location,
    ErrorTracker &et)
{
	if (is_modifiable_lvalue(lvalue))
		return;

	DEBUG_SMART_ASSERT(lvalue != nullptr);
	DEBUG_SMART_ASSERT(lvalue->is_function());

	if (lvalue->type == Symbol::Type::LIBRARY_FUNCTION)
	{
		std::string error = FMT::format("assignment of library function `{}`", lvalue->name);
		et.report_error(CTError::Type::SEMANTIC, error, assign_location);
	}
	else
	{
		std::string error = FMT::format("assignment of function `{}`", lvalue->name);
		std::string note = FMT::format("function {} declared here", lvalue->name);
		et.report_error(CTError::Type::SEMANTIC, error, assign_location, note, lvalue->location);
	}
}

ALWAYS_INLINE void lvalue__id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    const Location id_location,
    const Expr *&lvalue,
    ErrorTracker &et)
{
	const Symbol *current_symbol = st.lookup_chain(id_name, parse_ctx.scope_handler.scope());
	if (!current_symbol)
		current_symbol = st.insert_variable(
		    id_name,
		    parse_ctx.scope_handler.scope(),
		    parse_ctx.space_handler.space(),
		    parse_ctx.space_handler.next_offset(),
		    id_location);
	else if (current_symbol->is_variable() &&
		 current_symbol->scope > k_global_scope &&
		 current_symbol->scope <= parse_ctx.function_ctx_handler.current_function_scope())
		report_out_of_scope_variable(
		    id_name,
		    id_location, parse_ctx.function_ctx_handler.current_function_name(),
		    parse_ctx.function_ctx_handler.current_function_location(),
		    current_symbol,
		    et);
	lvalue = parse_ctx.expr_handler.make_expr_lvalue(current_symbol);
}

ALWAYS_INLINE void lvalue__local_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    const Location id_location,
    const Expr *&lvalue,
    ErrorTracker &et)
{
	const Symbol *current_symbol = nullptr;
	if (st.is_lib_function(id_name))
	{
		std::string error = FMT::format("shadowing library function `{}`", id_name);
		et.report_error(CTError::Type::SEMANTIC, error, id_location);
		current_symbol = st.lookup_global(id_name);
		DEBUG_SMART_ASSERT(current_symbol != nullptr); // a library function is always resolved at global scope.
	}
	else
	{
		current_symbol = st.lookup_local(id_name, parse_ctx.scope_handler.scope());
		if (!current_symbol)
			current_symbol = st.insert_variable(
			    id_name,
			    parse_ctx.scope_handler.scope(),
			    parse_ctx.space_handler.space(),
			    parse_ctx.space_handler.next_offset(),
			    id_location);
	}

	lvalue = parse_ctx.expr_handler.make_expr_lvalue(current_symbol);
}

ALWAYS_INLINE void lvalue__global_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    const Location id_location,
    const Expr *&lvalue,
    ErrorTracker &et)
{
	const Symbol *current_symbol = st.lookup_global(id_name);
	if (current_symbol)
	{
		lvalue = parse_ctx.expr_handler.make_expr_lvalue(current_symbol);
		return;
	}
	std::string error = FMT::format("variable `::{}` not found in global scope", id_name);
	et.report_error(CTError::Type::SEMANTIC, error, id_location);
}

inline void tableItem__lvalue_dot_id(
    const ExprTableItem *&table_item,
    const ExprLvalue *lvalue,
    const char *id)
{
}

inline void blockOpen__lbrace(ParseCtx &parse_ctx) noexcept
{
	parse_ctx.scope_handler.enter_scope();
}

inline void blockClose__rbrace(SymbolTable &st, ParseCtx &parse_ctx) noexcept
{
	st.hide_scope_symbols(parse_ctx.scope_handler.scope());
	parse_ctx.scope_handler.exit_scope();
}

inline void funcPrefix__function(ParseCtx &parse_ctx, const Location anonymous_location)
{
	// Update ParseCache:
	parse_ctx.cache.func_prefix.id = parse_ctx.name_generator.new_anonymous();
	parse_ctx.cache.func_prefix.location = anonymous_location;

	parse_ctx.space_handler.enter_space();
}

inline void funcPrefix__function_id(
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location)
{
	// Update ParseCache
	parse_ctx.cache.func_prefix.id = id_name;
	parse_ctx.cache.func_prefix.location = id_location;

	parse_ctx.space_handler.enter_space();
}

/// Handles a function signature’s prefix + argument list.
///
/// If a name conflict is detected, we still need to call
/// enter_function() (to keep our frame‐stack balanced), but
/// we must *not* back-patch the local-variable count or we
/// ’ll end up polluting the original function’s frame with
/// local_variable_count from the redefinition.
inline void funcSignature__funcPrefix_funcArgList(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    ErrorTracker &et)
{

	bool conflicting_name = reported_function_name_conflict(
	    st,
	    parse_ctx.scope_handler.scope(),
	    parse_ctx.cache.func_prefix.id,
	    parse_ctx.cache.func_prefix.location,
	    et);

	const Function *function_symbol = nullptr;
	if (!conflicting_name)
	{
		function_symbol = st.insert_function(
		    parse_ctx.cache.func_prefix.id,
		    parse_ctx.scope_handler.scope(),
		    parse_ctx.function_ctx_handler.next_function_address(),
		    parse_ctx.function_ctx_handler.function_parameters(),
		    parse_ctx.cache.func_prefix.location);

		parse_ctx.quad_handler.emit_quad(
		    IOPCode::FUNCSTART,
		    nullptr,
		    nullptr,
		    parse_ctx.expr_handler.make_expr_lvalue(function_symbol),
		    parse_ctx.cache.func_prefix.location);
	}
	parse_ctx.function_ctx_handler.enter_function(function_symbol);
	insert_function_parameters(st, parse_ctx, et);
	parse_ctx.function_ctx_handler.clear_function_parameters();
	parse_ctx.space_handler.enter_space(); // IMPORTANT: This line is after parameter insertion!
}

inline void funcDef__funcSignature_block(
    ParseCtx &parse_ctx,
    const BlockLocation &block_location) noexcept
{
	auto fbi = parse_ctx.function_ctx_handler.exit_function();
	if (fbi.function_symbol != nullptr)
	{
		Backpatcher::set_function_local_variable_count(
		    fbi.function_symbol,
		    fbi.local_variable_count);

		parse_ctx.quad_handler.emit_quad(
		    IOPCode::FUNCEND,
		    nullptr,
		    nullptr,
		    parse_ctx.expr_handler.make_expr_lvalue(fbi.function_symbol),
		    block_location.end);
	}

	parse_ctx.space_handler.exit_space();
}

inline void const__stringliteral(char *&string_literal)
{
	delete[] string_literal;
	string_literal = nullptr;
}

inline void funcArgs__id(ParseCtx &parse_ctx, const char *id_name, const Location id_location)
{
	parse_ctx.function_ctx_handler.add_function_parameter(id_name, id_location);
}

inline void whileStmt__whileHeader(ParseCtx &parse_ctx) noexcept
{
	parse_ctx.function_ctx_handler.enter_loop();
}

inline void whileStmt__whileHeader_stmt(ParseCtx &parse_ctx) noexcept
{
	parse_ctx.function_ctx_handler.exit_loop();
}

inline void forStmt__forHeader(ParseCtx &parse_ctx) noexcept
{
	parse_ctx.function_ctx_handler.enter_loop();
}

inline void forStmt__forHeader_stmt(ParseCtx &parse_ctx) noexcept
{
	parse_ctx.function_ctx_handler.exit_loop();
}

inline void funcCtrlStmt__return(
    const ParseCtx &parse_ctx,
    const Location return_location,
    ErrorTracker &et)
{
	if (parse_ctx.function_ctx_handler.function_nesting_depth() > 0)
		return;
	std::string error = "`return` statement not in a function statement";
	et.report_error(CTError::Type::SEMANTIC, error, return_location);
}

#endif /* SEMANTIC_ACTIONS_HPP */