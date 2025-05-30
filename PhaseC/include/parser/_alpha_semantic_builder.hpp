inline Expr *SemanticBuilder::convert_to_boolean(Expr *expr, SourceLocation expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr);
        if(expr->type == Expr::Type::BOOLEAN_EXPR)
                return expr;

        auto &eh = parse_ctx_.expr_handler;
        auto &qh = parse_ctx_.quad_handler;

        Expr *bool_expr = eh.make_expr_boolean(expr_loc);
        Expr *true_expr = make_const_true(expr_loc); // TODO : Dude.. having so many make function
        // is confusing and pointless;

        bool_expr->backpatch_info->true_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::IF_EQ, expr, true_expr, nullptr, expr_loc);
        // TODO: this would be a good place to free initial expr* as its now
        // useless.. Also you could reuse old expr.. why make new all the time??
        // Like all expressions get a "face-lift"
        bool_expr->backpatch_info->false_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, expr_loc);
        return bool_expr;
}




inline Expr *SemanticBuilder::make_logical_and(
        Expr *left, Expr *right, SourceLocation result_loc,
        [[maybe_unused]] SourceLocation left_loc,  // TODO: If you dont do constant folding remove
        [[maybe_unused]] SourceLocation right_loc) // TODO: If you dont do constant folding remove
{
        auto &qh = parse_ctx_.quad_handler;
        auto &eh = parse_ctx_.expr_handler;
        Expr *bool_result_expr = eh.make_expr_boolean(result_loc);

        DEBUG_SMART_ASSERT(!!left->backpatch_info);

        // TODO: Semantic Manager has a patch function.. this fucks with DRY,
        // So make a common backpatcher class or namespace that does this function for you
        // Maybe put it in parseCTX or in QUAD_HANDLER.
        for(u32 quad_label : left->backpatch_info->true_list)
        {
                DEBUG_SMART_ASSERT(parse_ctx_.cache.logical_marker.next_quad_stack.size() > 0);
                qh.patch_quad(quad_label, parse_ctx_.cache.logical_marker.next_quad_stack.top());
        }
        parse_ctx_.cache.logical_marker.next_quad_stack.pop();
        left->backpatch_info->true_list.clear();

        // TODO: MAKE A CUSTOM MERGE FUNCTION this FUCKs with DRY, as in logical or we do
        // the same fucking thing... just in reverse... FOR FUCK SAKE
        left->backpatch_info->false_list.insert(left->backpatch_info->false_list.end(),
                                                right->backpatch_info->false_list.begin(),
                                                right->backpatch_info->false_list.end());
        bool_result_expr->backpatch_info = left->backpatch_info;

        // TODO: FUCKING DO SOMETHING MORE EFFIECIENT that COPYING VECTOR. (maybe std::move or
        // std::swap. move sounds more effiecient!!)
        bool_result_expr->backpatch_info->true_list = right->backpatch_info->true_list;

        return bool_result_expr;
}

inline Expr *SemanticBuilder::make_logical_not(Expr *expr, SourceLocation result_loc)
{
        // TODO: Can we check if already boolexpr and reuse that? instead of making new?
        DEBUG_SMART_ASSERT(!!expr);
        auto &eh = parse_ctx_.expr_handler;
        auto &qh = parse_ctx_.quad_handler;

        Expr *bool_result_expr = eh.make_expr_boolean(result_loc);
        Expr *true_expr = make_const_true(result_loc);

        bool_result_expr->backpatch_info->true_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::IF_EQ, expr, true_expr, nullptr, result_loc);
        bool_result_expr->backpatch_info->false_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, result_loc);

        std::swap(bool_result_expr->backpatch_info->true_list,
                  bool_result_expr->backpatch_info->false_list);
        return bool_result_expr;
}

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

inline Expr *SemanticBuilder::resolve_assign_expr(Expr *lvalue, Expr *expr,
                                                  const SourceLocation assign_loc)
{
        DEBUG_SMART_ASSERT(!!lvalue, !!expr);

        validate_lvalue_for_assignment(lvalue->symbol, assign_loc);
        if(lvalue->type == Expr::Type::TABLE_ITEM)
                return handle_table_item_assignment(lvalue, expr, assign_loc);
        return handle_direct_assignment(lvalue, expr, assign_loc);
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

inline Expr *SemanticBuilder::handle_table_item_assignment(Expr *lvalue, Expr *expr,
                                                           SourceLocation assign_loc)
{
        parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, lvalue, lvalue->index, expr,
                                          assign_loc);

        Expr *rvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        return parse_ctx_.expr_handler.make_expr_assign(rvalue, assign_loc);
}

inline Expr *SemanticBuilder::handle_direct_assignment(Expr *lvalue, Expr *expr,
                                                       SourceLocation assign_loc)
{
        parse_ctx_.quad_handler.emit_quad(
                IOPCode::ASSIGN, expr, nullptr, lvalue,
                assign_loc);
        // TODO (NOT IMPORTANT): loc (can we construct it from expr (to catch
        // whole assignment expression?))

        Expr *assignExpr = parse_ctx_.expr_handler.make_expr_assign(parse_ctx_.new_temp(),
                assign_loc //
        );

        parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, lvalue, nullptr, assignExpr,
                                          k_no_location);

        return assignExpr;
}

inline void SemanticBuilder::update_expr_location(Expr *expr, SourceLocation new_expr_loc)
{
        expr->location = new_expr_loc;
}



} // namespace Alpha

#endif // ALPHA_SEMANTIC_BUILDER_HPP
