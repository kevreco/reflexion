#ifndef RE_PARSER_H
#define RE_PARSER_H

#include <stack>

#include "re_global.h" // utf8_string
#include "re_lexer.h"
#include "re_ast.h"
#include "re_context.h" // used for Scope::Type @TODO try to factor file and remove this

using namespace Ast;

//static inline Node* ast_new(Node* ast_node, const Location& loc)

static inline Node* ast_new(Node* ast_node, const Location& loc) {

    ast_node->location = loc;
    return ast_node;
}


// Shortcut
#define AST_NEW(type, loc) ((type*)ast_new(new type, loc))




struct Manager;

struct Parser {
    
    enum EndTagToken {
        SEMICOLON = 1 << 0,
        PARENTHESIS = 1 << 1,
        BRACK = 1 << 2,
        COLON = 1 << 3,
        COMMA = 1 << 4,
        NEWLINE = 1 << 5,
        NONE = 1 << 6,
    };

    // handle scopes
    Manager& manager;
    Lexer lex;

    // Scope
    static const int MAX_SCOPE_DEPTH = 256;
    Block* current_scope;
    Block* scope_stack[MAX_SCOPE_DEPTH];
    int scope_index = -1;

    // Declarations
    darray<Declaration*> declaration_stack;

    std::stack<EndTagToken> end_tag_tokens;

    
    Parser(Manager& mgr, const char* src, int len, const utf8_string& path);
    
    TopLevel* parse_top_level();

    // Private functions

    bool current_scope_is_global();

    void push_scope(Block* scope);
    void pop_scope();

    void add_to_scope(Declaration* decl, Block* scope);

    void push_declaration(Declaration* decl);
    void pop_declaration();

    Declaration* get_current_declaration();

    void add_to_unresolved_idents(Identifier* ident);

    bool check_redeclaration(Declaration* decl, Block* scope);

}; // Parser

#endif // Parser
