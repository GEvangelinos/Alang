for (const auto &synonym_symbols : symbol_map_)
{
        std::cerr << std::left << std::setw(19) << synonym_symbols.first << ":";
        for (auto &symbol : synonym_symbols.second)
        {
                std::cerr << "|" << symbol.scope();
                if (symbol.is_active())
                        std::cerr << "A";
                else
                        std::cerr << "D";
                std::cerr << "|";
        }
        std::cerr << std::endl;
}

std::cerr << "___________BEGIN___________" << std::endl;
for (auto i = k_global_scope; i < symbols_per_scope_.size(); i++)
{
        std::cerr << "symbols_per_scope: " << i << std::endl;
        for (const auto &symbol : symbols_per_scope_[i])
                std::cerr << symbol->name() << std::endl;
}
std::cerr << "____________END____________DOFF" << std::endl;
