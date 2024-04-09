#include "./symbolTable.hpp"
#include "./alphaDefs.hpp"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <cstring>
#include <iostream>
// Todo: add prints on "verbose" mode.

namespace Alpha
{
        SymbolTable::SymbolTable()
            : currentScope(this->GLOBAL_SCOPE_DEPTH), maximumReachedScopeDepth(this->GLOBAL_SCOPE_DEPTH), totalSymbolCount(0)
        {
        }

        SymbolTable::~SymbolTable()
        {
                for (auto mapPair : this->symbolMap)
                        for (auto entryPtr : mapPair.second)
                                delete entryPtr;
        }

        SymbolTableEntry::SymbolTableEntry(const std::string &name, uint32_t scope, uint32_t line, SymbolType type)
            : name(name), scope(scope), line(line), type(type)
        {
                if (line < 0)
                        throw std::invalid_argument(std::string(__func__) + "(): Line argument < 0");
                if (scope < 0)
                        throw std::invalid_argument(std::string(__func__) + "(): Scope argument < 0");
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

        OperationResult SymbolTable::insertVariable(std::string varName, uint32_t line, SymbolType type)
        {
                if (type == SymbolType::GLOBAL && this->currentScope != 0)
                        return OperationResult::InvalidInput;
                Variable *newVariablePtr = new Variable(varName, this->currentScope, line, type);
                return this->insertEntry(newVariablePtr);
        }

        OperationResult SymbolTable::insertFunction(std::string funcName, uint32_t line, SymbolType type, std::list<std::string> &argumentNames)
        {
                if (type != SymbolType::LIBFUNC && type != SymbolType::USERFUNC)
                        throw std::invalid_argument(std::string(__func__) + "(): Symbol type not a function.");
                if (type == SymbolType::LIBFUNC && this->currentScope != 0 && line == 0)
                        throw std::invalid_argument(std::string(__func__) + "(): LIBFUNCs are declared only in scope 0, line 0.");

                this->incrementScope();
                for (const auto &argName : argumentNames)
                        if (this->insertVariable(argName, line, SymbolType::FORMAL) == OperationResult::DuplicateSymbolError)
                                return OperationResult::DuplicateArgumentError;
                Function *newFunctionPtr = new Function(funcName, this->currentScope - 1, line, type, argumentNames);
                this->decrementScope();
                return this->insertEntry(newFunctionPtr);
        }

        const SymbolTableEntry *SymbolTable::lookUpAtScopeDepth(const std::string &name, uint32_t scopeDepth)
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name of symbol not found.
                        return nullptr;

                for (const SymbolTableEntry *entryPtr : mapIterator->second)
                {
                        if (entryPtr->isActive && entryPtr->scope == scopeDepth)
                                return entryPtr;
                        else if (entryPtr->isActive && entryPtr->scope > scopeDepth)
                                break;
                }
                return nullptr; // Name was found, but no entry at scope Depth.
        }

        const SymbolTableEntry *SymbolTable::lookUpGlobalScope(const std::string &name)
        {
                return this->lookUpAtScopeDepth(name, this->GLOBAL_SCOPE_DEPTH);
        }

        const SymbolTableEntry *SymbolTable::lookUpCurrentScope(const std::string &name)
        {
                return this->lookUpAtScopeDepth(name, this->currentScope);
        }

        /* Checks from current up until global scope, if not function definition is not the middle. */
        const SymbolTableEntry *SymbolTable::lookUpChainScope(const std::string &name, const SymbolType type)
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

                for (int vectorIndex = this->currentScope; vectorIndex > (*listIterator)->scope; vectorIndex--)
                        if (this->scopeTypeVector[vectorIndex] == ScopeType::FUNCTION_SCOPE)
                                return nullptr;
                return *listIterator;
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

        OperationResult SymbolTable::incrementScope()
        {
                this->currentScope++;
                if (this->currentScope > this->maximumReachedScopeDepth)
                        this->maximumReachedScopeDepth = this->currentScope;
                return OperationResult::Success;
        }

        OperationResult SymbolTable::decrementScope()
        {
                if (this->hideCurrentScopeSymbols() != OperationResult::Success)
                        throw std::runtime_error(std::string(__func__) + "(): An error occured during hiding symbols of current scope.");
                if (this->currentScope == GLOBAL_SCOPE_DEPTH)
                        throw std::runtime_error(std::string(__func__) + "(): Tried to decrement scope, when being in global scope.");
                this->currentScope--;
                return OperationResult::Success;
        }

        // FIXME: using vector is insufficient as you go many times over the wrong elements.
        void SymbolTable::printSymbolInsertionVector()
        {
                std::cout << UNIX_COLOR_BLUE;
                for (uint32_t scopeDepth = this->GLOBAL_SCOPE_DEPTH; scopeDepth <= this->maximumReachedScopeDepth; scopeDepth++)
                {
                        std::cout << "----------     Scope #"  << scopeDepth  << "     ----------" << std::endl;
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
                }
                std::cout << UNIX_COLOR_RESET;
        }

        Variable::Variable(std::string name, uint32_t scope, uint32_t line, SymbolType type)
            : SymbolTableEntry(name, scope, line, type)
        {
        }

        Function::Function(std::string name, uint32_t scope, uint32_t line, SymbolType type, std::list<std::string> &argumentNames)
            : SymbolTableEntry(name, scope, line, type), argumentNames(argumentNames)
        {
        }
}