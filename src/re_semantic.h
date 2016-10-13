#ifndef RE_SEMANTIC_H
#define RE_SEMANTIC_H

#include "re_ast.h"

using namespace Ast;

#include <map>
#include <vector>

struct Manager;

struct Semantic {

    Manager& manager;
    
    Semantic(Manager& mgr);

    void analyze();

private:

    Declaration* find_declaration(Identifier* ident, Block* scope);
    void resolve_identifiers(Declaration* declaration);

    Literal* infer_constant_from_declaration(Declaration* decl);
    Literal* infer_constant_from_expression(Expression* expr);

    void infer_type_from_literal_declaration(Expression* expression);


    // Constant propagation
    Expression* find_constant_substitute(Declaration* decl, Identifier* ident);
    Expression* eval_expression_to_literal(Expression* expression);


    Expression* eval_const_binary(Binary* binary);
    Expression* eval_int_binary(Literal* lhs, Literal* rhs);
    Expression* eval_float_binary(Literal* lhs, Literal* rhs);
    Expression* eval_bool_binary(Literal* lhs, Literal* rhs);
};


#endif // RE_SEMANTIC_H
