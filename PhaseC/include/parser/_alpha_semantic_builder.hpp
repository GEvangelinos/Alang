inline ExprList *SemanticBuilder::make_empty_expr_list() { return new ExprList(); }

inline void SemanticBuilder::delete_expr_list(ExprList *&elist)
{
        delete elist;
        DEBUG_NULLIFY(elist);
}

inline void SemanticBuilder::delete_dict_list(DictList *&dlist)
{
        for(ExprPair *pair : *dlist)
                delete pair;
        delete dlist;
        DEBUG_NULLIFY(dlist);
}

inline ExprList *SemanticBuilder::make_expr_list_with(Expr *expr, const SourceLocation new_expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr);
        ExprList *new_expr_list = make_empty_expr_list();
        return extend_expr_list_with(new_expr_list, expr, new_expr_loc);
}

inline ExprList *SemanticBuilder::extend_expr_list_with(ExprList *expr_list, Expr *expr,
                                                        const SourceLocation new_expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr_list, !!expr);
        update_expr_location(expr, new_expr_loc);
        expr_list->push_back(expr);
        return expr_list;
}

inline Expr *CLASS_NAME::make_call(Expr *lvalue, ExprList *&elist, SourceLocation call_loc)
{
        Expr *func_expr = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        for(Expr *e : *elist)
                parse_ctx_.quad_handler.emit_quad(IOPCode::PARAM, e, nullptr, nullptr,
                                                  e->location //
                );

        DEBUG_SMART_ASSERT(!!func_expr->symbol);

        parse_ctx_.quad_handler.emit_quad(IOPCode::CALL, func_expr, nullptr, nullptr, call_loc);

        Expr *getretval_expr = parse_ctx_.expr_handler.make_expr_variable(parse_ctx_.new_temp(),
                k_no_location //
        );

        parse_ctx_.quad_handler.emit_quad(
                IOPCode::GETRETVAL, nullptr, nullptr, getretval_expr,
                k_no_location);
        // We could pass call_location, but we follow strict policy: temps have
        // no loc

        delete_expr_list(elist);
        return getretval_expr;
}

inline Expr *SemanticBuilder::resolve_lvalue_to_primary(Expr *lvalue)
{
        return parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
}

inline Expr *SemanticBuilder::resolve_call_to_primary(Expr *call)
{
        return parse_ctx_.expr_handler.emit_quad_if_table_item(call);
}


inline Expr *SemanticBuilder::make_table_list(ExprList *&elist, SourceLocation table_list_loc)
{
        DEBUG_SMART_ASSERT(!!elist);
        Expr *new_table_expr = parse_ctx_.expr_handler.make_expr_new_table(table_list_loc);
        parse_ctx_.quad_handler.emit_quad(IOPCode::TABLECREATE, nullptr, nullptr, new_table_expr,
                                          table_list_loc);

        // Emit list's items.
        u32 list_index = 0;
        for(auto expr_it = elist->crbegin(); expr_it != elist->crend(); ++expr_it)
        {
                Expr *index_expr = parse_ctx_.expr_handler.make_expr_const_int(
                        list_index++,
                        (*expr_it)->location
                        // //TODO: you could remove as index its unseen is source
                        // code, and locations is ment to point to source code,
                        // Except if we think of it, as "cause-of-existance"
                        // Like I exist dude to this thing there...
                );
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, index_expr, *expr_it,
                                                  new_table_expr,
                                                  (*expr_it)->location //
                );
        }
        delete_expr_list(elist);
        return new_table_expr;
}

inline Expr *SemanticBuilder::make_table_dict(DictList *&dlist, SourceLocation table_dict_loc)
{
        Expr *new_table_expr = parse_ctx_.expr_handler.make_expr_new_table(table_dict_loc);
        parse_ctx_.quad_handler.emit_quad(IOPCode::TABLECREATE, nullptr, nullptr, new_table_expr,
                                          table_dict_loc //
        );

        for(auto it = dlist->crbegin(); it != dlist->crend(); ++it)
        {
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, (*it)->first,
                                                  (*it)->second, new_table_expr,
                                                  k_no_location //
                );
        }
        delete_dict_list(dlist);
        return new_table_expr;
}

inline Expr *SemanticBuilder::make_program_function(const Function *function_symbol)
{
        return parse_ctx_.expr_handler.make_expr_program_function(function_symbol);
}

inline ExprPair *SemanticBuilder::make_expr_pair(Expr *first, Expr *second)
{
        return new ExprPair(first, second);
}

inline DictList *SemanticBuilder::make_empty_dict_list() { return new DictList{}; }

inline DictList *SemanticBuilder::make_dict_list_with(ExprPair *first_element)
{
        DEBUG_SMART_ASSERT(!!first_element);
        DictList *new_dict_list = make_empty_dict_list();
        new_dict_list->push_back(first_element);
        return new_dict_list;
}

inline DictList *SemanticBuilder::extend_dict_list_with(DictList *dict_list, ExprPair *new_pair)
{
        DEBUG_SMART_ASSERT(!!dict_list, !!new_pair);
        dict_list->push_back(new_pair);
        return dict_list;
}

inline Expr *SemanticBuilder::make_table_item(Expr *&lvalue, const char *id,
                                              SourceLocation table_item_loc, SourceLocation id_loc)
{
        return parse_ctx_.expr_handler.make_expr_table_item(lvalue, id, id_loc, table_item_loc);
}

inline Expr *SemanticBuilder::make_table_item(Expr *&lvalue, Expr *expr, SourceLocation table_item_loc)
{
        return parse_ctx_.expr_handler.make_expr_table_item(lvalue, expr, table_item_loc);
}

inline Expr *SemanticBuilder::make_normal_call(Expr *&lvalue, ExprList *&elist, SourceLocation call_loc)
{
        lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        return make_call(lvalue, elist, call_loc);
}

inline Expr *SemanticBuilder::make_method_call(Expr *&lvalue, ExprList *&elist, SourceLocation call_loc)
{
        lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        elist->push_back(lvalue);

        // TODO: Understand what this name should be... And also understand first emit_quad_if...
        Expr *temp_var = parse_ctx_.expr_handler.make_expr_table_item(
                lvalue, parse_ctx_.cache.method_call_id.id,
                parse_ctx_.cache.method_call_id.id_location,
                k_no_location //
        );

        lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(temp_var);
        return make_call(lvalue, elist, call_loc);
}

inline Expr *SemanticBuilder::make_iife_call(const Function *func_symbol, ExprList *&elist,
                                             const SourceLocation call_loc)
{
        Expr *func_expr = parse_ctx_.expr_handler.make_expr_program_function(func_symbol);
        return make_call(func_expr, elist, call_loc);
}

inline void SemanticBuilder::validate_lvalue_for_assignment(const Symbol *lvalue_symbol,
                                                            const SourceLocation assign_loc)
{
        DEBUG_SMART_ASSERT(!!lvalue_symbol);
        if(Symbol::is_modifiable_symbol(lvalue_symbol))
                return;
        if(lvalue_symbol->type == Symbol::Type::LIBRARY_FUNCTION)
        {
                const std::string error =
                        FMT::format("assignment of library function `{}`", lvalue_symbol->name);
                et_.report_error(CTError::Type::SEMANTIC, error, assign_loc);
        }
        else if(lvalue_symbol->type == Symbol::Type::PROGRAM_FUNCTION)
        {
                std::string error = FMT::format("assignment of function `{}`", lvalue_symbol->name);
                std::string note = FMT::format("function {} declared here", lvalue_symbol->name);
                et_.report_error(CTError::Type::SEMANTIC, error, assign_loc, note,
                                 lvalue_symbol->location);
        }
}

inline BlockLocation SemanticBuilder::make_block_location(SourceLocation begin, SourceLocation end) noexcept
{
        return {
                .begin = begin,
                .end = end,
        };
}

inline void SemanticBuilder::update_expr_location(Expr *expr, SourceLocation new_expr_loc)
{
        expr->location = new_expr_loc;
}



} // namespace Alpha

#endif // ALPHA_SEMANTIC_BUILDER_HPP
