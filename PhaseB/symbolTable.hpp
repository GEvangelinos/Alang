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
                DuplicateError = -1,
                InvalidInput = -2,
                ErrorError = -3 // This means something was fucked up... And the error was not specified by this enum class.
        };

        class SymbolTableEntry
        {
        private:
                const std::string name;
                const uint32_t scope;
                const uint32_t line;
                SymbolType type;
                bool isActive;

        protected:
                void set_name(std::string name);
                void set_scope(uint32_t scope);
                void set_line(uint32_t line);
                void set_type(SymbolType type);
                void activate();
                void deactivate();

                friend class SymbolTable;
        };

        class SymbolTable
        {
        private:
                enum class ScopeType // Todo: Please rename these enums...
                {
                        GLOBAL_SCOPE,
                        FUNCTION_SCOPE,
                        OTHER_SCOPE
                };

                static constexpr uint32_t GLOBAL_SCOPE_DEPTH = 0;
                std::unordered_map<std::string, std::list<SymbolTableEntry>> symbolMap;
                std::vector<ScopeType> scopeTypeVector; // Add frames with insert, remove with hide.
                uint32_t currentScope;
                size_t totalSymbolCount;
                const SymbolTableEntry *lookUpAtScopeDepth(const std::string &name, uint32_t scopeDepth);

        public:
                OperationResult insert(std::string name, uint32_t line, uint32_t scopeDepth, SymbolType type, bool active = 1);
                OperationResult insert(const SymbolTableEntry &entry);
                const SymbolTableEntry *lookUpGlobalScope(const std::string &name);
                const SymbolTableEntry *lookUpCurrentScope(const std::string &name);
                const SymbolTableEntry *lookUpChainScope(const std::string &name); // Inclusive to current and global scope.
                OperationResult hide(std::string name, uint32_t scope);
                OperationResult hide(const SymbolTableEntry &entry);

                SymbolTable(); // = default
        };

        class Variable : public SymbolTableEntry
        {
                Variable(std::string name, uint32_t scope, uint32_t line, SymbolType type);
        };

        class Function : public SymbolTableEntry
        {
        };

}

#endif /* SYMBOL_TABLE_HPP */