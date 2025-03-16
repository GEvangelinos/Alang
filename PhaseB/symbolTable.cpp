#include "./symbolTable.hpp"
#include "./alphaDefs.hpp"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <format>

static const std::string anonymousFunctionNamePrefix = "#f_";

namespace Alpha
{
        SymbolTable::SymbolTable()
        {
                this->scopeTypeVector.push_back(ScopeType::GLOBAL_SCOPE);
                this->pushNewLoopDepthCounter();

                // Loading library functions, to symbolTable.
                this->insertFunction("print", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("input", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("objectmemberkeys", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("objecttotalmembers", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("objectcopy", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("totalarguments", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("argument", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("typeof", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("strtonum", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("sqrt", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("cos", 0, SymbolType::LIBFUNC, std::list<std::string>{});
                this->insertFunction("sin", 0, SymbolType::LIBFUNC, std::list<std::string>{});
        }

        SymbolTable::~SymbolTable()
        {
                for (auto mapPair : this->symbolMap)
                        for (auto entryPtr : mapPair.second)
                                delete entryPtr;
        }

        SymbolTableEntry::SymbolTableEntry(const std::string &name, uint32_t scope, uint32_t line, SymbolType type)
            : name(name), scope(scope), line(line), type(type), isActive(true)
        {
                if (line < 0)
                        throw std::invalid_argument(std::string(__func__) + "(): Line argument < 0");
                if (scope < 0)
                        throw std::invalid_argument(std::string(__func__) + "(): Scope argument < 0");
        }

        std::string SymbolTableEntry::getName() const
        {
                return this->name;
        }

        uint32_t SymbolTableEntry::getScope() const
        {
                return this->scope;
        }

        uint32_t SymbolTableEntry::getLine() const
        {
                return this->line;
        }

        SymbolType SymbolTableEntry::getType() const
        {
                return this->type;
        }

        OperationResult SymbolTable::insertEntry(SymbolTableEntry *entryPtr)
        {
                if (this->lookUpCurrentScope(entryPtr->name) != nullptr)
                        return OperationResult::DuplicateSymbolError;
                auto &symbolNameList = this->symbolMap[entryPtr->name];
                auto listIterator = symbolNameList.begin();
                while (listIterator != symbolNameList.end() && (*listIterator)->scope < this->currentScope)
                        listIterator++;
                symbolNameList.insert(listIterator, entryPtr);
                this->symbolInsertionMap[entryPtr->scope].push_back(entryPtr);
                return OperationResult::Success;
        }

        OperationResult SymbolTable::insertVariable(std::string varName, uint32_t line)
        {
                SymbolType typeArgument = (this->currentScope == this->GLOBAL_SCOPE_DEPTH) ? SymbolType::GLOBAL : SymbolType::LOCAL;
                return this->insertVariable(varName, line, typeArgument);
        }

        OperationResult SymbolTable::insertVariable(std::string varName, uint32_t line, SymbolType type)
        {
                if (type == SymbolType::GLOBAL && this->currentScope != 0)
                        return OperationResult::InvalidInput;
                Variable *newVariablePtr = new Variable(varName, this->currentScope, line, type);
                OperationResult returnValue = this->insertEntry(newVariablePtr);
                if (returnValue != OperationResult::Success)
                        delete newVariablePtr;
                return returnValue;
        }

        OperationResult SymbolTable::insertFunction(std::string funcName, uint32_t line, SymbolType type, const std::list<std::string> &argumentNames)
        {
                if (type != SymbolType::LIBFUNC && type != SymbolType::USERFUNC)
                        throw std::invalid_argument(std::string(__func__) + "(): Symbol type not a function.");
                if (type == SymbolType::LIBFUNC && this->currentScope != 0 && line == 0)
                        throw std::invalid_argument(std::string(__func__) + "(): LIBFUNCs are declared only in scope 0, line 0.");
                std::unordered_set<std::string> uniqueChecker;
                for (const auto &argName : argumentNames)
                {
                        if (!uniqueChecker.insert(argName).second)
                        {
                                this->errorTracker.registerCompileTimeError(new SyntaxError(line, 0, "In the definition of the function " + funcName + ", argument " + argName + " is already declared."));
                                return OperationResult::DuplicateArgumentError;
                        }
                        else if (this->isLibraryFunction(argName))
                                this->errorTracker.registerCompileTimeError(new SyntaxError(line, 0, "In the definition of the function " + funcName + ", argument " + argName + " is library function."));
                }

                // We create the function and pass those names to the function's argument list
                // But yet the arguments of the function are not yet instantiated... Though we checked for duplicates.
                Function *newFunctionPtr = new Function(funcName, this->getCurrentScope(), line, type, argumentNames);
                OperationResult returnValue = this->insertEntry(newFunctionPtr);
                if (returnValue != OperationResult::Success)
                        delete newFunctionPtr;
                else
                        Function::lineOfLastFunction = line;
                return returnValue;
                // At this point, we return... The think is that we havent declared the function's arguments.
                // It is the job of the incrementScope()'s function to check the Function:argumentNames list
                // and if it is not empty to declare them as FORMAL variables, and reset the list.
        }

        OperationResult SymbolTable::insertNamelessFunction(uint32_t line, SymbolType type, std::list<std::string> &argumentNames)
        {
                std::string internalReferenceName = anonymousFunctionNamePrefix + std::to_string(this->namelessFunctionCounter++);
                return this->insertFunction(internalReferenceName, line, type, argumentNames);
        }

        SymbolTableEntry *SymbolTable::lookUpAtScopeDepth(const std::string &name, uint32_t scopeDepth)
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name of symbol not found.
                        return nullptr;

                for (SymbolTableEntry *entryPtr : mapIterator->second)
                {
                        if (entryPtr->isActive && entryPtr->scope == scopeDepth)
                                return entryPtr;
                        else if (entryPtr->isActive && entryPtr->scope > scopeDepth)
                                break;
                }
                return nullptr; // Name was found, but no entry at scope Depth.
        }

        SymbolTableEntry *SymbolTable::lookUpGlobalScope(const std::string &name)
        {
                return this->lookUpAtScopeDepth(name, this->GLOBAL_SCOPE_DEPTH);
        }

        SymbolTableEntry *SymbolTable::lookUpCurrentScope(const std::string &name)
        {
                return this->lookUpAtScopeDepth(name, this->currentScope);
        }

        bool SymbolTable::isLibraryFunction(const std::string &name)
        {
                SymbolTableEntry *entry = this->lookUpGlobalScope(name);
                return entry && entry->getType() == Alpha::SymbolType::LIBFUNC;
        }

        bool SymbolTable::isInsideFunctionScope()
        {
                for (auto scopeIterator = this->scopeTypeVector.rbegin(); scopeIterator != this->scopeTypeVector.rend(); scopeIterator++)
                        if (*scopeIterator == ScopeType::FUNCTION_SCOPE)
                                return true;
                return false;
        }

        /* Checks from current up until global scope, if not function definition is not the middle. */
        const SymbolTableEntry *SymbolTable::lookUpChainScope(const std::string &name, const SymbolType type) const
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name not found.
                        return nullptr;
                const auto &scopeList = mapIterator->second;
                auto listIterator = scopeList.crbegin();
                while (listIterator != scopeList.crend() && !(*listIterator)->isActive)
                        listIterator++;
                if (listIterator == scopeList.crend()) // There was no active symbol.
                        return nullptr;
                if ((*listIterator)->type == SymbolType::GLOBAL || type == SymbolType::USERFUNC)
                        return *listIterator;

                /* Looking for local variable in outer scope, (non-function scope).*/
                for (int vectorIndex = this->currentScope; vectorIndex > (*listIterator)->scope; vectorIndex--)
                        if (this->scopeTypeVector[vectorIndex] == ScopeType::FUNCTION_SCOPE)
                                return nullptr;
                return *listIterator;
        }

        std::pair<OperationResult, SymbolTableEntry *> SymbolTable::lookUpFunction(const std::string &name) const
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name not found.
                        return std::make_pair(OperationResult::SymbolNotFound, nullptr);
                const auto &scopeList = mapIterator->second;
                auto listIterator = scopeList.crbegin();
                while (listIterator != scopeList.crend() && !(*listIterator)->isActive)
                        listIterator++;
                if (listIterator == scopeList.crend()) // There was no active symbol.
                        return std::make_pair(OperationResult::SymbolNotFound, nullptr);
                if ((*listIterator)->type == SymbolType::LIBFUNC || (*listIterator)->type == SymbolType::USERFUNC)
                        return std::make_pair(OperationResult::Success, *listIterator);
                return std::make_pair(OperationResult::SymbolNotFunction, nullptr);
        }

        std::pair<OperationResult, SymbolTableEntry *> SymbolTable::lookUpVariable(const std::string &name) const
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name not found.
                        return std::make_pair(OperationResult::SymbolNotFound, nullptr);
                const auto &scopeList = mapIterator->second;
                auto listIterator = scopeList.crbegin();
                while (listIterator != scopeList.crend() && !(*listIterator)->isActive)
                        listIterator++;
                if (listIterator == scopeList.crend()) // There was no active symbol.
                        return std::make_pair(OperationResult::SymbolNotFound, nullptr);
                if ((*listIterator)->type == SymbolType::LIBFUNC || (*listIterator)->type == SymbolType::USERFUNC)
                        return std::make_pair(OperationResult::SymbolNotVariable, nullptr);
                if ((*listIterator)->type == SymbolType::GLOBAL)
                        return std::make_pair(OperationResult::Success, *listIterator);

                /* Looking for local variable in outer scope, (non-function scope).*/
                for (int vectorIndex = this->currentScope; vectorIndex > (*listIterator)->scope; vectorIndex--)
                        if (this->scopeTypeVector[vectorIndex] == ScopeType::FUNCTION_SCOPE)
                                return std::make_pair(OperationResult::SymbolNotFound, nullptr);
                return std::make_pair(OperationResult::Success, *listIterator);
        }

        std::pair<OperationResult, SymbolTableEntry *> SymbolTable::lookUpSymbol(const std::string &name)
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name not found.
                        return std::make_pair(OperationResult::SymbolNotFound, nullptr);
                const auto &scopeList = mapIterator->second;
                auto listIterator = scopeList.crbegin();
                while (listIterator != scopeList.crend() && !(*listIterator)->isActive)
                        listIterator++;
                if (listIterator == scopeList.crend()) // There was no active symbol.
                        return std::make_pair(OperationResult::SymbolNotFound, nullptr);
                if ((*listIterator)->type != SymbolType::LOCAL && (*listIterator)->type != SymbolType::FORMAL)
                        return std::make_pair(OperationResult::Success, *listIterator);

                /* Looking for local variable in outer scope, (non-function scope).*/
                for (int vectorIndex = this->currentScope; vectorIndex > (*listIterator)->scope; vectorIndex--)
                        if (this->scopeTypeVector[vectorIndex] == ScopeType::FUNCTION_SCOPE)
                                return std::make_pair(OperationResult::SymbolOutsideFunction, nullptr);
                return std::make_pair(OperationResult::Success, *listIterator);
        }

        OperationResult SymbolTable::hideCurrentScopeSymbols()
        {
                // TODO: Ineffiecient as fuck... FIXME: stack of vectors of references.
                for (auto &mapPair : this->symbolMap)
                        for (auto *entryPtr : mapPair.second)
                                if (entryPtr->isActive && entryPtr->scope == this->currentScope)
                                        entryPtr->isActive = false;
                return OperationResult::Success;
        }

        void SymbolTable::incrementScope(bool isFunctionScope)
        {
                this->currentScope++;
                if (this->currentScope > this->maximumReachedScopeDepth)
                        this->maximumReachedScopeDepth = this->currentScope;
                this->scopeTypeVector.push_back(isFunctionScope ? ScopeType::FUNCTION_SCOPE : ScopeType::PLAIN_SCOPE);

                if (isFunctionScope)
                {
                        this->pushNewLoopDepthCounter();

                        // Check if you increment this scope due to a function declaration
                        // and if the function owns any FORMAL variables (function arguments).
                        // If there are and FORMAL argument to declare, do so.
                        for (const auto &argName : Function::idList) // No need to check if idList is empty, range-based loop does it.
                                if (insertVariable(argName, Function::lineOfLastFunction, SymbolType::FORMAL) != OperationResult::Success)
                                        throw std::runtime_error(std::string(__func__) + "(): Insertion of function's arguments to new scope failed.");
                        Function::idList.clear();
                        // New function, new Counter for while loops...
                }
        }

        void SymbolTable::decrementScope()
        {
                if (this->hideCurrentScopeSymbols() != OperationResult::Success)
                        throw std::runtime_error(std::string(__func__) + "(): An error occured during hiding symbols of current scope.");
                if (this->currentScope == GLOBAL_SCOPE_DEPTH)
                        throw std::runtime_error(std::string(__func__) + "(): Tried to decrement scope, when being in global scope.");

                if (this->scopeTypeVector.back() == ScopeType::FUNCTION_SCOPE)
                        this->popLoopDepthCounter();
                this->scopeTypeVector.pop_back();
                this->currentScope--;
        }

        uint32_t SymbolTable::getCurrentScope() const
        {
                return this->currentScope;
        }

        void SymbolTable::pushNewLoopDepthCounter()
        {
                this->loopDepthCounterStack.push(0);
        }

        void SymbolTable::popLoopDepthCounter()
        {
                if (this->loopDepthCounterStack.top() != 0)
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while currentLoopDepthCounter was not 0.");
                this->loopDepthCounterStack.pop();
        }

        void SymbolTable::incrementCurrentLoopDepthCounter()
        {

                if (this->loopDepthCounterStack.empty())
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while stack was empty.");
                // The above exception should not be thrown under correct usage of the stack, as the first stack frame is created in the constructor.
                this->loopDepthCounterStack.top()++;
        }

        void SymbolTable::decrementCurrentLoopDepthCounter()
        {
                if (this->loopDepthCounterStack.empty())
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while stack was empty.");
                if (this->loopDepthCounterStack.top() == 0)
                        throw std::runtime_error(std::string("Function ") + __func__ + "(): was called while current counter is already 0.");
                this->loopDepthCounterStack.top()--;
        }

        uint32_t SymbolTable::getCurrentLoopDepthCounter()
        {
                return this->loopDepthCounterStack.top();
        }

        void SymbolTable::printErrorVector()
        {
                std::cout << UNIX_COLOR_RED;
                for (const CompileTimeError *error : this->errorTracker.gerErrorVector())
                        std::cerr << error->toString() << std::endl;
                std::cout << UNIX_COLOR_RESET;
        }

        // FIXME: using vector is insufficient as you go many times over the wrong elements.
        void SymbolTable::printSymbolInsertionVector()
        {
                std::cout << UNIX_COLOR_BLUE;
                for (uint32_t scopeDepth = this->GLOBAL_SCOPE_DEPTH; scopeDepth <= this->maximumReachedScopeDepth; scopeDepth++)
                {
                        std::cout << "--------------------     Scope #" << scopeDepth << "     --------------------" << std::endl;
                        for (uint32_t vectorIndex = 0; vectorIndex < this->symbolInsertionMap[scopeDepth].size(); vectorIndex++)
                        {
                                auto *entry = this->symbolInsertionMap[scopeDepth][vectorIndex];
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
                std::cout << UNIX_COLOR_RESET;
        }

        Variable::Variable(std::string name, uint32_t scope, uint32_t line, SymbolType type)
            : SymbolTableEntry(name, scope, line, type)
        {
        }

        Function::Function(std::string name, uint32_t scope, uint32_t line, SymbolType type, const std::list<std::string> &argumentNames)
            : SymbolTableEntry(name, scope, line, type), argumentNames(argumentNames)
        {
        }

        std::list<std::string> Function::idList;
        uint32_t Function::lineOfLastFunction = -1;
        std::string Function::nameOfLastFunction;
}