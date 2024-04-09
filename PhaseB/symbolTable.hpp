#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <cstdint>
#include <unordered_map>
#include <list>
#include <vector>

namespace Alpha
{
        enum class SymbolType
        {
                GLOBAL,
                LOCAL,
                FORMAL,
                USERFUNC,
                LIBFUNC
        };

        enum class OperationResult : int
        {
                Success = 0,
                DuplicateSymbolError = -1,
                DuplicateArgumentError = -2,
                InvalidInput = -3,
                ErrorError = -4 // This means something was fucked up... And the error was not specified by this enum class.
        };

        class SymbolTableEntry
        {
        private:
                const std::string name;
                const uint32_t scope;
                const uint32_t line;
                const SymbolType type;
                bool isActive;
                SymbolTableEntry() = delete;
                friend class SymbolTable;

        protected:
                SymbolTableEntry(const std::string &name, uint32_t scope, uint32_t line, SymbolType type);
        };

        class SymbolTable
        {
        private:
                enum class ScopeType // Todo: Please rename these enums...
                {
                        GLOBAL_SCOPE,
                        FUNCTION_SCOPE,
                        PLAIN_SCOPE
                };

                static constexpr uint32_t GLOBAL_SCOPE_DEPTH = 0;
                uint32_t currentScope;
                uint32_t maximumReachedScopeDepth;
                size_t totalSymbolCount;
                std::unordered_map<std::string, std::list<SymbolTableEntry *>> symbolMap;
                std::unordered_map<uint32_t, std::vector<SymbolTableEntry *>> symbolInsertionMap;
                std::vector<ScopeType> scopeTypeVector; // Add frames with insert, remove with hide.
                // Implement a stack of vectors of strings (or references/pointers) to hide effieciently symvols.
                OperationResult insertEntry(SymbolTableEntry *newEntryPtr);
                const SymbolTableEntry *lookUpAtScopeDepth(const std::string &name, uint32_t scopeDepth);
                OperationResult incrementScope(ScopeType scopeType);

        public:
                OperationResult insertVariable(std::string name, uint32_t line, SymbolType type);
                OperationResult insertFunction(std::string name, uint32_t line, SymbolType type, std::list<std::string> &argumentNames);
                const SymbolTableEntry *lookUpGlobalScope(const std::string &name);
                const SymbolTableEntry *lookUpCurrentScope(const std::string &name);
                const SymbolTableEntry *lookUpChainScope(const std::string &name, const SymbolType type); // Inclusive to current and global scope.
                // OperationResult hide(std::string name, uint32_t scope);
                OperationResult hideCurrentScopeSymbols();
                OperationResult incrementScopePlainBlock();
                OperationResult incrementScopeFunctionBlock();
                OperationResult decrementScope();
                void printSymbolInsertionVector();

                SymbolTable();
                ~SymbolTable();
        };

        class Variable : public SymbolTableEntry
        {
        private:
                Variable(std::string name, uint32_t scope, uint32_t line, SymbolType type);
                friend class SymbolTable;
        };

        class Function : public SymbolTableEntry
        {
        private:
                Function(std::string name, uint32_t scope, uint32_t line, SymbolType type, std::list<std::string> &argumentNames);
                std::list<std::string> argumentNames;
                friend class SymbolTable;
        };

}

#endif /* SYMBOL_TABLE_HPP */