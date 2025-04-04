#include "parser/alpha_symbol_table.hpp"
#include "misc/smart_assert.h"
#include "core/alpha_location.hpp"

#include <stdexcept>
#include <unordered_set>
#include <format>

namespace /* (Anonymous) */
{
        using namespace Alpha;
        SymbolType determine_symbol_type(std::optional<SymbolType> symbol_type_opt, u32 active_scope_depth)
        {
                if (symbol_type_opt.has_value())
                        return symbol_type_opt.value();
                if (active_scope_depth == GLOBAL_SCOPE_DEPTH)
                        return SymbolType::GLOBAL;
                return SymbolType::LOCAL;
        }

        std::string generate_anonymous_function_name(u32 &anonymous_function_counter)
        {
                const std::string function_name = ANONYMOUS_FUNCTION_NAME_PREFIX + std::to_string(anonymous_function_counter);
                anonymous_function_counter++;
                return function_name;
        }

} /* Anonymous namespace */

namespace Alpha
{
        const std::string &SymbolTableEntry::name() const noexcept { return name_; }
        u32 SymbolTableEntry::scope() const noexcept { return scope_; }
        SourceRange SymbolTableEntry::source_range() const noexcept { return source_range_; }
        SymbolType SymbolTableEntry::type() const noexcept { return type_; }
        bool SymbolTableEntry::is_active() const noexcept { return is_active_; }
        void SymbolTableEntry::activate() noexcept { is_active_ = true; }
        void SymbolTableEntry::deactivate() noexcept { is_active_ = false; }

        SymbolTableEntry::SymbolTableEntry(const std::string &name, u32 scope, SourceRange source_range, SymbolType type) noexcept
            : name_(name),
              scope_(scope),
              source_range_(source_range),
              type_(type),
              is_active_(true) {}

        SymbolTable::SymbolTable(ParserContext &parser_context, ErrorTracker &error_tracker_ref)
            : active_scope_depth_(GLOBAL_SCOPE_DEPTH),
              maximum_reached_scope_depth_(GLOBAL_SCOPE_DEPTH),
              anonymous_function_counter_(0),
              total_symbol_count_(0),
              error_tracker_(error_tracker_ref),
              parser_context_(parser_context)
        {
                scope_type_vector_.push_back(ScopeType::GLOBAL_SCOPE);
                this->push_new_loop_depth_counter();

                /* Loading library functions, to symbolTable. */
                this->insert_function("print", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("input", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("objectmemberkeys", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("objecttotalmembers", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("objectcopy", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("totalarguments", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("argument", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("typeof", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("strtonum", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("sqrt", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("cos", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
                this->insert_function("sin", SymbolType::LIBFUNC, LIBFUNC_ARGUMENT_INFO, LIBFUNC_SOURCE_RANGE);
        }

        SymbolTable::~SymbolTable()
        {
                for (auto map_pair : symbol_map_)
                        for (auto entry_ptr : map_pair.second)
                                delete entry_ptr;
        }

        const SymbolTableEntry *SymbolTable::insert_entry(SymbolTableEntry *entry_ptr)
        {
                if (this->lookup_local(entry_ptr->name()) != nullptr)
                        return nullptr;
                auto &symbol_name_list = symbol_map_[entry_ptr->name()];
                auto list_iterator = symbol_name_list.begin();
                while (list_iterator != symbol_name_list.end() && (*list_iterator)->scope() < active_scope_depth_)
                        ++list_iterator;
                symbol_name_list.insert(list_iterator, entry_ptr);
                symbol_insertion_map_[entry_ptr->scope()].push_back(entry_ptr);

                return entry_ptr;
        }

        const SymbolTableEntry *SymbolTable::insert_variable(
            const std::string &name,
            std::optional<SymbolType> symbol_type_opt,
            const SourceRange &source_range)
        {
                SymbolType symbol_type = determine_symbol_type(symbol_type_opt, active_scope_depth_);
                if (symbol_type == SymbolType::GLOBAL && active_scope_depth_ != 0)
                        return nullptr;
                VariableEntry *new_variable_ptr = new VariableEntry(name, active_scope_depth_, source_range, symbol_type);
                const SymbolTableEntry *return_value = this->insert_entry(new_variable_ptr);
                if (return_value == nullptr)
                        delete new_variable_ptr;
                return return_value;
        }

        const SymbolTableEntry *SymbolTable::insert_function(
            std::optional<std::string> name,
            SymbolType symbol_type,
            const std::list<std::pair<std::string, Location>> &arguments_info,
            const SourceRange &source_range)
        {
                if (!name.has_value())
                        name = generate_anonymous_function_name(anonymous_function_counter_);

                if (symbol_type != SymbolType::LIBFUNC && symbol_type != SymbolType::USERFUNC)
                        throw std::invalid_argument(std::string(__func__) + "(): Symbol type not a function.");
                if (symbol_type == SymbolType::LIBFUNC && active_scope_depth_ != 0 && source_range.last_index_ == 0)
                        throw std::invalid_argument(std::string(__func__) + "(): LIBFUNCs are declared only in scope 0, source_range[0,0].");
                std::unordered_set<std::string> unique_checker;
                for (const auto &arg_info : arguments_info)
                {
                        std::string error_message;
                        if (!unique_checker.insert(arg_info.first).second)
                                error_message = std::format(
                                    "In the definition of the function {}, argument {} is already declared.", name.value(), arg_info.first);
                        else if (this->is_library_function(arg_info.first))
                                error_message = std::format(
                                    "In the definition of the function {}, argument {} is library function.", name.value(), arg_info.first);
                        if (!error_message.empty())
                        {
                                error_tracker_.register_compile_time_error(new SyntaxError(source_range, error_message));
                                return nullptr;
                        }
                }

                std::list<Location> argument_names;
                std::transform(arguments_info.begin(), arguments_info.end(),
                               std::back_inserter(argument_names),
                               [](const auto &pair)
                               { return pair.second; });

                FunctionEntry *new_function_ptr = new FunctionEntry(
                    name.value(), active_scope_depth_, source_range, symbol_type, argument_names);
                const SymbolTableEntry *insertion_result = this->insert_entry(new_function_ptr);
                if (insertion_result == nullptr)
                        delete new_function_ptr;
                return insertion_result;
        }
        // else
        //         // TODO: FIXME: FIX: lineOfLastFunction Function::lineOfLastFunction = line;
        //         ;
        // We create the function and pass those names to the function's argument list
        // But yet the arguments of the function are not yet instantiated... Though we checked for duplicates.
        // At this point, we return... The think is that we havent declared the function's arguments.
        // It is the job of the incrementScope()'s function to check the Function:argumentNames list
        // and if it is not empty to declare them as FORMAL variables, and reset the list.

        const SymbolTableEntry *SymbolTable::lookup_at_scope_depth(const std::string &name, uint32_t scope_depth) const
        {
                auto map_iterator = symbol_map_.find(name);
                if (map_iterator == symbol_map_.end()) // Name of symbol not found.
                        return nullptr;

                for (SymbolTableEntry *entry_ptr : map_iterator->second)
                {
                        if (entry_ptr->is_active() && entry_ptr->scope() == scope_depth)
                                return entry_ptr;
                        else if (entry_ptr->is_active() && entry_ptr->scope() > scope_depth)
                                break;
                }
                return nullptr; /* Name was found, but no entry at scope Depth. */
        }

        const SymbolTableEntry *SymbolTable::lookup_global(const std::string &name) const
        {
                return this->lookup_at_scope_depth(name, GLOBAL_SCOPE_DEPTH);
        }

        const SymbolTableEntry *SymbolTable::lookup_local(const std::string &name) const
        {
                return this->lookup_at_scope_depth(name, active_scope_depth_);
        }

        bool SymbolTable::is_library_function(const std::string &name)
        {
                const SymbolTableEntry *entry = this->lookup_global(name);
                return entry && entry->type() == Alpha::SymbolType::LIBFUNC;
        }

        bool SymbolTable::is_in_function_scope()
        {
                for (auto scope_iterator = scope_type_vector_.rbegin(); scope_iterator != scope_type_vector_.rend(); ++scope_iterator)
                        if (*scope_iterator == ScopeType::FORMAL_SCOPE)
                                return true;
                return false;
        }

        /* Performs a scoped lookup for a symbol name.
         * If the symbol is not found, or is inactive, returns nullptr.
         * - Functions (USERFUNC) and global symbols can be accessed across scopes.
         * - Variables cannot be accessed across function boundaries — if a FORMAL_SCOPE
         *   is encountered between the current and defining scope, access is blocked.
         */
        const SymbolTableEntry *SymbolTable::lookup_chain(const std::string &name, const SymbolType type) const
        {
                auto map_iterator = symbol_map_.find(name);
                if (map_iterator == symbol_map_.end()) /* Name not found. */
                        return nullptr;
                const auto &scope_list = map_iterator->second;
                auto list_iterator = scope_list.crbegin();
                while (list_iterator != scope_list.crend() && !(*list_iterator)->is_active())
                        ++list_iterator;
                if (list_iterator == scope_list.crend()) /* There was no active symbol. */
                        return nullptr;
                if ((*list_iterator)->type() == SymbolType::GLOBAL || type == SymbolType::USERFUNC)
                        return *list_iterator;

                /* Looking for local variable in outer scope, (non-function scope).*/
                for (auto vector_index = active_scope_depth_; vector_index > (*list_iterator)->scope(); vector_index--)
                        if (scope_type_vector_[vector_index] == ScopeType::FORMAL_SCOPE)
                                return nullptr;
                return *list_iterator;
        }

        const SymbolTableEntry *SymbolTable::lookup_function(const std::string &name) const
        {
                auto map_iterator = symbol_map_.find(name);
                if (map_iterator == symbol_map_.end()) // Name not found.
                        return nullptr;
                const auto &scope_list = map_iterator->second;
                auto list_iterator = scope_list.crbegin();
                while (list_iterator != scope_list.crend() && !(*list_iterator)->is_active())
                        ++list_iterator;
                if (list_iterator == scope_list.crend()) // There was no active symbol.
                        return nullptr;
                if ((*list_iterator)->type() == SymbolType::LIBFUNC || (*list_iterator)->type() == SymbolType::USERFUNC)
                        return *list_iterator;
                return nullptr;
        }

        const SymbolTableEntry *SymbolTable::lookup_variable(const std::string &name) const
        {
                auto map_iterator = symbol_map_.find(name);
                if (map_iterator == symbol_map_.end()) // Name not found.
                        return nullptr;
                const auto &scope_list = map_iterator->second;
                auto list_iterator = scope_list.crbegin();
                while (list_iterator != scope_list.crend() && !(*list_iterator)->is_active())
                        ++list_iterator;
                if (list_iterator == scope_list.crend()) // There was no active symbol.
                        return nullptr;
                if ((*list_iterator)->type() == SymbolType::LIBFUNC || (*list_iterator)->type() == SymbolType::USERFUNC)
                        return nullptr;
                if ((*list_iterator)->type() == SymbolType::GLOBAL)
                        return *list_iterator;

                /* Looking for local variable in outer scope, (non-function scope).*/
                for (uint32_t vector_index = active_scope_depth_; vector_index > (*list_iterator)->scope(); vector_index--)
                        if (scope_type_vector_[vector_index] == ScopeType::FORMAL_SCOPE)
                                return nullptr;
                return *list_iterator;
        }

        const SymbolTableEntry *SymbolTable::lookup_symbol(const std::string &name) const
        {
                auto map_iterator = symbol_map_.find(name);
                if (map_iterator == symbol_map_.end()) // Name not found.
                        return nullptr;
                const auto &scope_list = map_iterator->second;
                auto list_iterator = scope_list.crbegin();
                while (list_iterator != scope_list.crend() && !(*list_iterator)->is_active())
                        ++list_iterator;
                if (list_iterator == scope_list.crend()) // There was no active symbol.
                        return nullptr;
                if ((*list_iterator)->type() != SymbolType::LOCAL && (*list_iterator)->type() != SymbolType::FORMAL)
                        return *list_iterator;

                /* Looking for local variable in outer scope, (non-function scope).*/
                for (uint32_t vector_index = active_scope_depth_; vector_index > (*list_iterator)->scope(); vector_index--)
                        if (scope_type_vector_[vector_index] == ScopeType::FORMAL_SCOPE)
                                return nullptr;
                return *list_iterator;
        }

        void SymbolTable::hide_current_scope_symbols()
        {
                // TODO: Ineffiecient as fuck... FIXME: stack of vectors of references.
                for (auto &map_pair : symbol_map_)
                        for (auto *entry_ptr : map_pair.second)
                                if (entry_ptr->is_active() && entry_ptr->scope() == active_scope_depth_)
                                        entry_ptr->deactivate();
        }

        void SymbolTable::enter_scope(bool is_function_scope)
        {
                active_scope_depth_++;
                if (active_scope_depth_ > maximum_reached_scope_depth_)
                        maximum_reached_scope_depth_ = active_scope_depth_;
                scope_type_vector_.push_back(is_function_scope ? ScopeType::FORMAL_SCOPE : ScopeType::LOCAL_SCOPE);

                if (is_function_scope)
                {
                        // New function, new Counter for while loops...
                        this->push_new_loop_depth_counter();

                        // Check if you increment this scope due to a function declaration
                        // and if the function owns any FORMAL variables (function arguments).
                        // If there are and FORMAL argument to declare, do so.
                        for (const auto &arg_info : parser_context_.function_argument_list()) // No need to check if idList is empty */
                        {
                                const std::string &argument_name = arg_info.first;
                                const Location &argument_location = arg_info.second;
                                if (insert_variable(argument_name, SymbolType::FORMAL, argument_location))
                                        throw std::runtime_error(std::string(__func__) + "(): Insertion of function's arguments to new scope failed.");
                        }
                        parser_context_.clear_function_argument_list();
                }
        }

        void SymbolTable::exit_scope()
        {
                this->hide_current_scope_symbols();
                if (active_scope_depth_ == GLOBAL_SCOPE_DEPTH)
                        throw std::runtime_error(std::string(__func__) + "(): Tried to decrement scope, when being in global scope.");

                if (scope_type_vector_.back() == ScopeType::FORMAL_SCOPE)
                        this->pop_loop_depth_counter();
                scope_type_vector_.pop_back();
                active_scope_depth_--;
        }

        uint32_t SymbolTable::active_scope_depth() const
        {
                return active_scope_depth_;
        }

        void SymbolTable::push_new_loop_depth_counter()
        {
                loop_depth_counter_stack_.push(0);
        }

        void SymbolTable::pop_loop_depth_counter()
        {
                if (loop_depth_counter_stack_.top() != 0)
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while currentLoopDepthCounter was not 0.");
                loop_depth_counter_stack_.pop();
        }

        void SymbolTable::enter_loop()
        {
                if (loop_depth_counter_stack_.empty())
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while stack was empty.");
                // The above exception should not be thrown under correct usage of the stack, as the first stack frame is created in the constructor.
                loop_depth_counter_stack_.top()++;
        }

        void SymbolTable::exit_loop()
        {
                if (loop_depth_counter_stack_.empty())
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while stack was empty.");
                if (loop_depth_counter_stack_.top() == 0)
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while current counter is already 0.");
                loop_depth_counter_stack_.top()--;
        }

        u32 SymbolTable::active_loop_depth() const
        {
                return loop_depth_counter_stack_.top();
        }

        // FIXME: using vector is insufficient as you go many times over the wrong elements.
        void SymbolTable::printSymbolInsertionVector()
        {
                std::cout << COLOR_ASCII_FG_BLUE;
                for (uint32_t scopeDepth = this->GLOBAL_SCOPE_DEPTH; scopeDepth <= this->maximumReachedScopeDepth; scopeDepth++)
                {
                        std::cout << "--------------------     Scope #" << scopeDepth << "     --------------------" << std::endl;
                        for (uint32_t vector_index = 0; vector_index < this->symbolInsertionMap[scopeDepth].size(); vector_index++)
                        {
                                auto *entry = this->symbolInsertionMap[scopeDepth][vector_index];
                                std::cout << "\"" << entry->name << "\"" << " ";
                                std::cout << "[";
                                switch (entry->type)
                                {
                                case SymbolType::LIBFUNC:
                                        std::cout << "LIBRARY FUNCTION";
                                        break;
                                case SymbolType::GLOBAL:
                                        std::cout << "GLOBAL VARIABLE";
                                        break;
                                case SymbolType::USERFUNC:
                                        std::cout << "USER FUNCTION";
                                        break;
                                case SymbolType::FORMAL:
                                        std::cout << "FORMAL ARGUMENT";
                                        break;
                                case SymbolType::LOCAL:
                                        std::cout << "LOCAL VARIABLE";
                                        break;
                                }
                                std::cout << "]" << " ";
                                std::cout << "(line " << entry->line << ")" << " "
                                          << "(scope " << entry->scope << ")" << std::endl;
                        }
                        std::cout << std::endl;
                }
                std::cout << SGR_RESET;
        }

        VariableEntry::VariableEntry(const std::string &name, u32 scope, SourceRange source_range, SymbolType type)
            : SymbolTableEntry(name, scope, source_range, type) {}

        FunctionEntry::FunctionEntry(const std::string &name,
                                     u32 scope,
                                     SourceRange source_range,
                                     SymbolType type,
                                     const std::list<std::string> &argument_names);
            : SymbolTableEntry(name, scope, source_range, type), argumentNames(argumentNames)
            {
            }

            std::list<std::string> Function::idList;
            uint32_t Function::lineOfLastFunction = -1;

}