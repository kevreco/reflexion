#ifndef RE_SYMBOL_TABLE_H
#define RE_SYMBOL_TABLE_H

#include <string>
#include <unordered_map>

namespace Ast {
    struct Expression;
}
using namespace Ast;

struct SymEntry {

    int sym_type;
    Expression* expr;
};

struct SymTable
{


    std::unordered_map<std::string, SymEntry> table;


};

#endif // RE_SYMBOL_TABLE_H
