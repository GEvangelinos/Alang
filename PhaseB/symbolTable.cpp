#include "./symbolTable.hpp"
#include <algorithm>
#include <string>
#include <unordered_map>
// Todo: add prints on "verbose" mode.

namespace Alpha
{

        /* Entries in list are sorted based on the scope, ascending order. */
        OperationResult SymbolTable::insert(std::string name, uint32_t line, uint32_t scopeDepth, SymbolType type, bool active)
        {
        }

        const SymbolTableEntry *SymbolTable::lookUpAtScopeDepth(const std::string &name, uint32_t scopeDepth)
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name of symbol not found.
                        return nullptr;
                for (const SymbolTableEntry &entry : mapIterator->second)
                {
                        if (entry.isActive && entry.scope == scopeDepth)
                                return &entry;
                        else if (entry.isActive && entry.scope > scopeDepth)
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

        /* FIXME: TODO : UNDONE */
        const SymbolTableEntry *SymbolTable::lookUpChainScope(const std::string &name)
        {
                auto mapIterator = this->symbolMap.find(name);
                if (mapIterator == this->symbolMap.end()) // Name not found.
                        return nullptr;
                const auto &scopeList = mapIterator->second;
                size_t scopeDepth = this->currentScope;
                bool outsideLastFunction = false;
                for (auto listIterator = scopeList.crbegin(); listIterator != scopeList.crend(); listIterator++)
                {
                        if (!listIterator->isActive || listIterator->scope > this->currentScope) // Verbose condition as greater scope depths than current are always deactivated.
                                continue;
                        if (listIterator->scope == this->currentScope)
                                return &(*listIterator);
                        if (this->scopeTypeVector[scopeDepth] == ScopeType::FUNCTION_SCOPE)
                                outsideLastFunction = true;
                        else
                                scopeDepth--;
                        if (outsideLastFunction && listIterator->type != SymbolType::GLOBAL)
                                return nullptr;
                        return &(*listIterator);
                }
        }
}