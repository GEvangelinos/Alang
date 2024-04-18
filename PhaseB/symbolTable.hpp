#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <cstdint>
#include <unordered_map>
#include <list>
#include <stack>
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
                DuplicateSymbolError = -1000,
                DuplicateArgumentError,
                SymbolNotFunction,
                SymbolNotVariable,
                SymbolNotFound,
                SymbolOutsideFunction,
                InvalidInput,
                ErrorError // This means something was fucked up... And the error was not specified by this enum class.
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

        public:
                std::string getName() const;
                uint32_t getScope() const;
                uint32_t getLine() const;
                SymbolType getType() const;
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

                uint32_t currentScope = GLOBAL_SCOPE_DEPTH;
                uint32_t maximumReachedScopeDepth = GLOBAL_SCOPE_DEPTH;
                uint32_t namelessFunctionCounter = 0;
                size_t totalSymbolCount = 0;
                std::unordered_map<std::string, std::list<SymbolTableEntry *>> symbolMap;
                std::unordered_map<uint32_t, std::vector<SymbolTableEntry *>> symbolInsertionMap;
                std::vector<ScopeType> scopeTypeVector; // Add frames with insert, remove with hide.
                std::stack<uint32_t> loopDepthCounterStack;
                std::vector<std::pair<uint32_t, std::string>> syntaxErrorVector;
                // Implement a stack of vectors of strings (or references/pointers) to hide effieciently symvols.
                OperationResult insertEntry(SymbolTableEntry *newEntryPtr);
                SymbolTableEntry *lookUpAtScopeDepth(const std::string &name, uint32_t scopeDepth);
                void loadLibraryFunctions();

        public:
                static constexpr uint32_t GLOBAL_SCOPE_DEPTH = 0;
                OperationResult insertVariable(std::string name, uint32_t line, SymbolType type);
                OperationResult insertVariable(std::string name, uint32_t line); // Inserts Variable at current scope.
                OperationResult insertFunction(std::string name, uint32_t line, SymbolType type, const std::list<std::string> &argumentNames);
                OperationResult insertNamelessFunction(uint32_t line, SymbolType type, std::list<std::string> &argumentNames);
                SymbolTableEntry *lookUpGlobalScope(const std::string &name);
                SymbolTableEntry *lookUpCurrentScope(const std::string &name);
                const SymbolTableEntry *lookUpChainScope(const std::string &name, const SymbolType type) const; // Inclusive to current and global scope.
                std::pair<OperationResult, SymbolTableEntry *> lookUpFunction(const std::string &name) const;   // Inclusive to current and global scope.
                std::pair<OperationResult, SymbolTableEntry *> lookUpVariable(const std::string &name) const;   // Inclusive to current and global scope.
                std::pair<OperationResult, SymbolTableEntry *> lookUpSymbol(const std::string &name);           // Inclusive to current and global scope.
                bool isLibraryFunction(const std::string &name);
                bool isInsideFunctionScope();
                OperationResult hideCurrentScopeSymbols();
                void incrementScope(bool isFunctionScope);
                void decrementScope();
                uint32_t getCurrentScope() const;

                void pushNewLoopDepthCounter();
                void popLoopDepthCounter();
                void incrementCurrentLoopDepthCounter();
                void decrementCurrentLoopDepthCounter();
                uint32_t getCurrentLoopDepthCounter();

                void registerSyntaxError(std::string errorMessage, uint32_t lineNumber);
                void printSyntaxErrorVector();
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
                Function(std::string name, uint32_t scope, uint32_t line, SymbolType type, const std::list<std::string> &argumentNames);
                std::list<std::string> argumentNames;

                static uint32_t lineOfLastFunction;
                friend class SymbolTable;

        public:
                static std::list<std::string> idList;
                static std::string nameOfLastFunction;
        };

}

#endif /* SYMBOL_TABLE_HPP */