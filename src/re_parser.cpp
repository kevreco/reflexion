#include "re_parser.h"

#include <assert.h>
#include <cstdlib>
#include <iostream>

#include "re_location.h"
#include "re_context.h"
#include "re_manager.h"


//-------------------------------------------------------------------------
// Forward Private Function Declarations
//-------------------------------------------------------------------------

// Helpers


namespace {



bool is_binary_operator(Token::Type t);
int operator_precedence(Token::Type t);

bool priority_is_higher(int precedenceA, int precedenceB);
bool priority_is_lower(int precedenceA, int precedenceB);
bool priority_is_equal(int precedenceA, int precedenceB);

// @TODO remove this
bool is_right_left_associative(Token::Type t);
bool is_right_left_associative(BinaryOperator b);
BinaryOperator get_binary_operator(Token::Type t);

//bool is_operand(Token::Type t);
//@TODO create another function more efficient
//bool is_end_token(Token::Type t, Parser::EndTagToken e);
//void log_err(const char* msg);

// Directive
Directive* parse_directive(Parser& p);
Directive* parse_directive_import(Parser& p);
Directive* parse_directive_run(Parser& p);

Expression* parse_expression(Parser& p, Expression* previousLhs = 0);
Expression* parse_const_expression(Parser& p);

Expression* parse_parenthese(Parser& p);
Expression* parse_unary(Parser& p);
Expression* parse_primary(Parser& p);
Expression* parse_operance_sequence(Parser& p, Expression* exp);
//Expression* parsePostFix(Expression* lhs);
Expression* parse_rhs(Parser& p, Expression* lhs, int priority);
//BinaryOperator* parseBinaryOperator();

// Statement
enum ParentScope {
    Global,
    Local
    //Function,
    //Namespace, // Not used
};

Statement* parse_statement(Parser& p);
Identifier* parse_identifier(Parser& p);

// Literals
Literal* parse_integer(Parser& p);

enum DefScope {
    DS_TOP_LEVEL,
    DS_NESTED,
};

Declaration*  parse_declaration(Parser& p, Identifier* i, ParentScope parent_scope = Local);
TypeDefinition*       parse_declaration(Parser& p);
TypeDefinition*       parse_function_declaration(Parser& p);
ArrayDef*  parse_array_declaration(Parser& p);
StructDef* parse_struct_declaration(Parser& p);
TypeDefinition*       parse_type_specifier(Parser& p);

FunctionCall* parse_function_call(Parser& p, Expression* exp);


enum PushScope {
    Yes,
    No
};

If* parse_if(Parser& p);
Block* parse_block(Parser& p, PushScope ps, Scope::Type t);
Statement* parse_return(Parser& p);


// Helper

void expect_and_consume(Parser& p, Token::Type type);

void expect_and_consume(Parser& p, char c) {
    expect_and_consume(p, (Token::Type)c);
}

Identifier* make_identifier(Parser& p, const utf8_string* unique_id) {

}


} // anonymous namespace


//-------------------------------------------------------------------------
// Parser
//-------------------------------------------------------------------------


using namespace Ast;

Parser::Parser(Manager& mgr, const char* src, int len, const utf8_string& current_path) :
    manager(mgr) {

    assert(src);
    assert(len);

    lex = Lexer();
    // Set source

    lex.set_source(src, len, &current_path);
    //mgr.register_parsed_file();

    declaration_stack.reserve(256);
}



TopLevel* Parser::parse_top_level() {

    printf("parse_top_level ============= \n");

    // Reset lexer
    Parser& self = *this;

    TopLevel* top_level = new TopLevel;

    push_scope(&manager.global_scope);


    lex.get_next_token();


    // Loop parsing one top level item
    while(lex.token.type != Token::Eof) {

        switch (lex.token.type) {
        case '#': {
            Directive* dir = parse_directive(self);
            // @TODO error if null ?
            if (dir->type == DIRECTIVE_IMPORT) {
                auto directive_import = static_cast<DirectiveImport*>(dir);
                top_level->import_items.push_back(directive_import);
            } else if (dir->type == DIRECTIVE_RUN) {
                auto directive_run = static_cast<DirectiveRun*>(dir);
                top_level->run_items.push_back(directive_run);
            } else {
                fprintf(stderr, "Unkown directive parsed\n");
            }
            break;
        }

        case Token::Identifier: {

            Identifier* expr = parse_identifier(self);
            auto ident =  static_cast<Identifier*>(expr);

            if (lex.token.type == ':'
                    || lex.token.type == Token::ColonEqual
                    || lex.token.type == Token::ColonColon) { // Declaration
                // Parse Declaration
                Declaration* declaration = parse_declaration(self, ident, Global);

                if (!declaration) {
                    return 0;
                }

                if (declaration && declaration->type_def) {
                    switch(declaration->type_def->def_kind) {
                    case TypeDefinition::DEF_FUNCTION: {
                        top_level->function_declarations.push_back(declaration);
                        break;
                    }
                    case TypeDefinition::DEF_STRUCT: {
                        top_level->struct_declarations.push_back(declaration);
                        break;
                    }

                    default: {
                        fprintf(stderr, "Warning: Unknown definition type %d\n", declaration->type_def->def_kind);
                    }
                    } // end switch

                } else {
                    manager.unresolved_declarations.push_back(declaration);

                    add_to_scope(declaration, current_scope);

                    expect_and_consume(self, ';');
                }


            }
            break;
        }

        case Token::Eof: {
            printf("End of file reached!\n");
            return 0;
        }
        default:

            fprintf(stderr, "parse top level, unknown token type %d :: %c\n", lex.token.type, lex.token.type);
            return 0;
        } // end switch
    } // end while

    return top_level;
} // parse_top_level

bool Parser::current_scope_is_global() {
    return scope_index == 0;
}

void Parser::push_scope(Block* scope) {
    scope_index++;
    scope_stack[scope_index] = scope;

    current_scope = scope_stack[scope_index];
}

void Parser::pop_scope() {
    // assert current scope is not global scope
    assert(scope_index != 0);
    assert(scope_index > 0);

    --scope_index;
    current_scope = scope_stack[scope_index];
}


void Parser::add_to_scope(Declaration* decl, Block* scope) {

    assert(scope);

    bool redeclared = check_redeclaration(decl,  scope);

    if (!redeclared) {
        scope->declarations.push_front(decl);

    }
}

void Parser::push_declaration(Declaration* decl) {
    declaration_stack.push_back(decl);
}

void Parser::pop_declaration() {
    declaration_stack.pop_back();
}

Declaration* Parser::get_current_declaration() {
    Declaration* result = 0;
    if (!declaration_stack.empty()) {
        result = declaration_stack.back();
    }
    return result;
}

bool Parser::check_redeclaration(Declaration* decl, Block* scope) {

    bool redeclared = false;
    for (Declaration* it: scope->declarations) {
        if (it->identifier->name == decl->identifier->name) {
            manager.log_error(decl->location, "Redeclaration of %.*s", decl->identifier->name->size(), decl->identifier->name->data());
            manager.log_error(it->location, "Previous declaration here.");
            redeclared = true;
            break;
        }
    }

    return redeclared;
}
//-------------------------------------------------------------------------
// Private Function Definition
//-------------------------------------------------------------------------

namespace {



bool is_binary_operator(Token::Type t) {

    switch(t) {
    case '*':
    case '/':
    case '.':
    case '+':
    case '=':
    case ':':
        return true;
    case ',':
    case Token::NotEqual:
    case '%':
    case Token::ModEqual:
    case '&':
    case Token::AmpAmp:
    case Token::AmpEqual:
    case Token::MulEqual:
    case Token::PlusEqual:
    case Token::MinusEqual:
    case Token::DivEqual:
    case Token::Arrow:
    case '<':
    case Token::LessLess:
    case Token::LessEqual:
    case Token::LessLessEqual:
    case Token::EqualEqual:
    case '>':
    case Token::GreaterGreater:
    case Token::GreaterEqual:
    case Token::GreaterGreaterEqual:
    case '^':
    case Token::CaretEqual:
    case '|':
    case Token::PipePipe:
    case Token::PipeEqual:
        fprintf(stderr, "Internal Error: this binary operator is not active yet: %d\n", t);
        return true;
    default:
        return false;
    } // end switch
}

// Inspired by: http://en.cppreference.com/w/cpp/language/operator_precedence
int operator_precedence(Token::Type t) {

    // The more is the value, the less is the priority
    switch(t) {

    //case Token::ColonColon: // Scope resolution/namespace
    //    return 10;
    case '*':
    case '/':
        return 50;
    case '.':
    case '+':
        return 60;
    case '=':
    case Token::ColonColon:
        return 150;

    default:
        fprintf(stderr, "Internal Error: operator precedence unknown: %d('%c') \n", t, t);
        return 0;
    } // end switch
}

bool priority_is_higher(int precedenceA, int precedenceB) {
    return (precedenceA < precedenceB);
}

bool priority_is_lower(int precedenceA, int precedenceB) {
    return precedenceA > precedenceB;
}

bool priority_is_equal(int precedenceA, int precedenceB) {
    return precedenceA == precedenceB;
}

// Inspired by: http://en.cppreference.com/w/cpp/language/operator_precedence
bool is_right_left_associative(BinaryOperator b) {


    switch(b) {

/*
++   -- 	Prefix increment and decrement 	Right-to-left
+   - 	Unary plus and minus
!   ~ 	Logical NOT and bitwise NOT
(type) 	C-style type cast
* 	Indirection (dereference)
& 	Address-of
sizeof 	Size-of[note 1]
new, new[] 	Dynamic memory allocation
delete, delete[] 	Dynamic memory deallocation 
*/

    case '=':

        return true;
    default:
        return false;
    } // end switch

    return false;
}

bool is_right_left_associative(Token::Type t) {


    switch(t) {

/*
++   -- 	Prefix increment and decrement 	Right-to-left
+   - 	Unary plus and minus
!   ~ 	Logical NOT and bitwise NOT
(type) 	C-style type cast
* 	Indirection (dereference)
& 	Address-of
sizeof 	Size-of[note 1]
new, new[] 	Dynamic memory allocation
delete, delete[] 	Dynamic memory deallocation 
*/

    case '=':
        return true;
    default:
        return false;
    } // end switch

    return false;
}

BinaryOperator get_binary_operator(Token::Type t) {
    switch(t) {
    case '+':
        return Add;
    case '-':
        return Sub;
    case '*':
        return Mul;
    case '/':
        return Div;
    case '%':
        return Mod;
    case '=':
        return Assign;
    default:
        fprintf(stderr, "@warn, parser, get binary operator, unknow operator\n");
        return BinaryUnknow;
    }
}

bool is_end_token(Parser& p, Token::Type t, Parser::EndTagToken e) {
    return (t == ')' && !p.end_tag_tokens.empty() && p.end_tag_tokens.top() == e);
}


Directive* parse_directive(Parser& p) {
    //printf("parse_directive\n");

    expect_and_consume(p, '#');

    if (p.lex.token.type == Token::Identifier) {
        if (p.lex.token.text_equal("import")) {
            return parse_directive_import(p);
        } else if (p.lex.token.text_equal("run")) {
            return parse_directive_run(p);
        } else {
            fprintf(stderr, "unkown directive %.*s\n", p.lex.token.len, p.lex.token.buffer);
        }
    }

    return nullptr;
}

Directive* parse_directive_import(Parser& p) {
    printf("parse_import\n");

    Location loc = p.lex.location;

    expect_and_consume(p, Token::Import);

    if (p.lex.token.type == Token::AsciiString) {

        DirectiveImport* import = AST_NEW(DirectiveImport, loc);
        import->absolute_path = utf8_string(p.lex.token.text);

        p.lex.goto_next_token(); // Skip path string
        return import;
    }


    // @TODO manage error
    return nullptr;
}

Directive* parse_directive_run(Parser& p) {
    //printf("parse_run\n");

    Location loc = p.lex.location;

    expect_and_consume(p, Token::Run);

    if (p.lex.token.type == Token::Identifier) {

        Identifier* ident = parse_identifier(p);
        FunctionCall* function_call = parse_function_call(p, ident);
        if (function_call) {
            DirectiveRun* run = AST_NEW(DirectiveRun, loc);
            run->function_call = function_call;
            return run;
        } else {
            //p.manager.log_error_parsing("A function must be called after the #run directive\n");
        }
    }


    // @TODO manage error
    return nullptr;
}

Ast::Expression* parse_expression(Parser& p, Expression* previousLhs) {
    printf("parse_expression\n");

    Expression* lhs = previousLhs ? previousLhs :  parse_primary(p);


    if (!lhs) {
        return 0;
    }

    Expression* rhs = parse_rhs(p, lhs, 0);
    if (!rhs) {
        return lhs;
    }
    return rhs;

}

Expression* parse_const_expression(Parser& p) {

    Expression* result = 0;

    switch(p.lex.token.type) {

    case Token::Struct: {
        result = parse_struct_declaration(p);
        break;
    }
    case Token::Enum: {
        fprintf(stderr, "parse_definition enum error, unimplemented type %d\n", p.lex.token.type);
        break;
    }
    default: {
        result = parse_expression(p);
    }

    }
    return result;
}

Ast::Expression* parse_parenthese(Parser& p) {

    //printf("parse_parenthese\n");

    expect_and_consume(p, '(');

    p.end_tag_tokens.push(Parser::PARENTHESIS);

    Expression* expr = parse_expression(p);

    expect_and_consume(p, ')');

    p.end_tag_tokens.pop();

    return expr;
}

Expression* parse_unary(Parser& p) {
    printf("parse_unary\n");

    Expression* result = 0;

    Location loc = p.lex.location;

    switch(p.lex.token.type){

    case '!': {
        p.lex.goto_next_token(); // Skip '!'
        Unary* unary = AST_NEW(Unary, loc);
        unary->op = LogicalNotOp;
        unary->operand = parse_primary(p);
        result = unary;
        break;
    }
    case '+': {
        p.lex.goto_next_token(); // Skip '+'

        Unary* unary = AST_NEW(Unary, loc);
        unary->op = PositiveOp;
        unary->operand = parse_primary(p);
        result = unary;
        break;
    }
    case '-': {
        p.lex.goto_next_token(); // Skip '-'
        Unary* unary = AST_NEW(Unary, loc);
        unary->op = NegativeOp;
        unary->operand = parse_primary(p);
        result = unary;
        break;
    }
        //case '*': {
        //    p.lex.goto_next_token(); // Skip '*'
        //    Unary* unary = AST_NEW(Unary, loc);
        //    unary->op = DereferenceOp;
        //    unary->operand = parse_primary(p);
        //    result = unary;
        //    break;
        //}
    case '~': {
        p.lex.goto_next_token(); // Skip '~'

        Unary* unary = AST_NEW(Unary, loc);
        unary->op = BitwiseNot;
        unary->operand = parse_primary(p);
        result = unary;
        break;
    }
    case Token::PlusPlus: {
        p.lex.goto_next_token(); // Skip '++'

        Unary* unary = AST_NEW(Unary, loc);
        unary->op = IncrementOp;
        unary->operand = parse_primary(p);
        result = unary;
        break;
    }
    case Token::MinusMinus: {
        p.lex.goto_next_token(); // Skip '--'

        Unary* unary = AST_NEW(Unary, loc);
        unary->op = DecrementOp;
        unary->operand = parse_primary(p);
        result = unary;
        break;
    }
        /// Literals
        ///
    case Token::Identifier: {

        Identifier* ident = parse_identifier(p);

        if (p.lex.token.type == '(') {

            FunctionCall* fc = parse_function_call(p, ident);
            assert(fc);
            return fc;
        }
        return ident;
    }
    case Token::LiteralInteger: {
        printf("TEST 0002\n");
        Literal* num = parse_integer(p);

        result = num;
        break;
    }
        // @TODO other literals

    case Token::Struct: {

        p.manager.log_error(loc, "Struct must be declared as constant.\n");
        break;
    }
    case Token::Enum: {

        p.manager.log_error(loc, "Enum must be declared as constant.\n");
        break;
    }
    default: {

        fprintf(stderr,
                "parse_unary error, unknow primary expression (not handle yet) %d|%c"
                "(@TODO is this a real error ?)\n",
                p.lex.token.type, p.lex.token.type);
        return nullptr;
    }

    } // end switch

    return result;
}

Expression* parse_primary(Parser& p) {
    printf("parse_primary\n");

    Expression* result = 0;
    //@TODO rename 'un' as 'result'
    Expression* un = parse_unary(p);
    if (un) {
        return un;
    }

    if (p.lex.token.type == Token::Identifier) {
        Identifier* ident = parse_identifier(p);
        return parse_operance_sequence(p, ident);

    } // End if

    const Token& t = p.lex.token;
    Location loc = p.lex.location;

    switch (t.type)  {
    case '(': {
        Expression* exp = parse_parenthese(p);
        return exp;
    }
    case Token::Identifier: {
        fprintf(stderr, "parse_primary ImplementationError, Identifier should be parse before switch entry (due to selector expr, index expr and function call !\n");

        break;
    }
    case Token::True: {
        p.lex.goto_next_token();
        Literal* lit = AST_NEW(Literal, loc);
        lit->literal_type = LITERAL_TRUE;
        result = lit;
        break;
    }
    case Token::False: {
        p.lex.goto_next_token();
        Literal* lit = AST_NEW(Literal, loc);
        lit->literal_type = LITERAL_FALSE;
        result = lit;
        break;
    }
    case Token::Null: {
        p.lex.goto_next_token();
        Literal* lit = AST_NEW(Literal, loc);
        lit->literal_type = LITERAL_NULL;
        result = lit;
        break;
    }
    default: {
        fprintf(stderr, "parse_primary, case note handled %d\n", t.type);

    }
    } // end switch

    return result;
}

Expression* parse_operance_sequence(Parser& p, Expression* exp) {

    while (p.lex.token.type == '(' ||
           p.lex.token.type == '[' ||
           p.lex.token.type == '.'
           ) {
        switch(p.lex.token.type) {
        case (char)'(': {
            Expression* fc = parse_function_call(p, exp);
            exp = parse_operance_sequence(p, fc);
            break;
        }

        case '[': {
            fprintf(stderr, "parseOperandSequence @TODO parseIndex\n");
            return nullptr;
        }

        case '.': {
            fprintf(stderr, "parseOperandSequence @TODO parseSelector\n");
            return nullptr;
        }
        default: {
            fprintf(stderr, "parseOperandSequence we shouldn't get here !\n");
            return exp;
        }
        }
    }

    return exp;
}

// See http://en.wikipedia.org/wiki/Operator-precedence_parser
Expression* parse_rhs(Parser& p, Expression* lhs, int lhsPrecedence) {
    printf("parse_rhs\n");

    if (is_end_token(p, p.lex.token.type, Parser::PARENTHESIS)) {
        //printf("FINAL end parseRhs\n");
        return nullptr;
    }

    // Just an alias
    const Token& t = p.lex.token;

    while(is_binary_operator(t.type)) {

        int opPrec = operator_precedence(t.type);
        if (opPrec < lhsPrecedence) {
            break;
        }

        BinaryOperator bop = get_binary_operator(t.type);
        p.lex.goto_next_token(); // skip operator

        Expression* rhs = parse_primary(p);

        if (!rhs) {
            return 0;
        }

        while(is_binary_operator(t.type)) {

            int opPrec2 = operator_precedence(t.type);
            if (!((opPrec > opPrec2) || (is_right_left_associative(t.type) && opPrec2 == opPrec))) {
                break;
            }

            Expression* expr = parse_rhs(p, rhs, opPrec2);
            if (expr) {
                rhs = expr;
            } else {
                fprintf(stderr, "parse_rhs, we shouldn't get here!\n");
            }
        }

        assert(lhs);
        assert(bop);
        assert(rhs);

        Binary* binary_lhs = AST_NEW(Binary, lhs->location);
        binary_lhs->lhs = lhs;
        binary_lhs->op = bop;
        binary_lhs->rhs = rhs;

        lhs = binary_lhs;

    }

    return lhs;
}

Statement* parse_statement(Parser& p) {
    printf("parse_statement\n");

    Statement* result = 0;

    Statement* statement = nullptr;
    Expression* lhs = nullptr;
    if (p.lex.token.type == Token::Identifier) {


        Expression* expr = parse_expression(p, lhs);
        assert(expr);

        ExpressionStatement* exp_stat = AST_NEW(ExpressionStatement, expr->location);
        exp_stat->expression = expr;

        statement = exp_stat;

        p.lex.goto_next_token(); // Skip ';'
        return statement;

    } else {

        switch(p.lex.token.type) {
        case ';': {
            Location loc = p.lex.location;
            p.lex.goto_next_token(); // Skip ';'
            EmptyStatement* empty_stat = AST_NEW(EmptyStatement, loc);
            result = empty_stat;
            break;
        }
        case '{': {

            result = parse_block(p, PushScope::Yes, Scope::Compound);
            break;
        }
        case Token::If: {

            result = parse_if(p);
            break;
        }
        case Token::Return: {

            result = parse_return(p);
            break;
        }
        case Token::Defer: {
            Location loc = p.lex.location;
            p.lex.goto_next_token(); // Skip defer keyword
            Statement* st = parse_statement(p);
            /* @TODO handle error of statement is :
               - a return statement.
               - another defer statement.
               - a throw statemen.
               -...
             */
            DeferStatement* defer_stat = AST_NEW(DeferStatement, loc);
            defer_stat->statement = st;
            result = defer_stat;

            break;
        }
        default: {
            fprintf(stderr, "parseStatement, unhandle case %d/%c.\n", p.lex.token.type, p.lex.token.type);

        }
        }

    } // end if/else

    return result;
}

Identifier* parse_identifier(Parser& p) {

    assert(p.lex.token.type == Token::Identifier);

    const Token& ident_tok = p.lex.token;

    Identifier* result = AST_NEW(Identifier, p.lex.location);


    utf8_string tmp = utf8_string(ident_tok.text, ident_tok.len);

    const utf8_string* ident_str = p.manager.get_unique_ident_str_ptr(tmp);
    result->name = ident_str;
    result->scope = p.current_scope;

    assert(result);
    assert(result->name);
    assert(result->scope);

    Declaration* decl = p.get_current_declaration();
    // if there is no declaration, it means we are at top level
    if (decl) {
        decl->unresolved_idents.push_back(result);
        result->parent_declaration = decl;
    }

    p.lex.goto_next_token(); // Skip identifier

    return result;
}

Declaration* parse_declaration(Parser& p, Identifier* ident, ParentScope parent_scope) {

    printf("parse_declaration\n");

    assert(p.lex.token.type == ':'
           || p.lex.token.type == Token::ColonEqual
           || p.lex.token.type == Token::ColonColon);

    Declaration* result = 0;
    result = AST_NEW(Declaration, ident->location);

    p.push_declaration(result);

    TypeDefinition* definition = 0;

    Expression* expression = 0;

    bool is_const_expr = false;

    if (p.lex.token.type == ':') {
        p.lex.goto_next_token(); // Skip ':'

        expression = parse_expression(p);

        assert("Internal Error: @TODO Implementation");

    } else if (p.lex.token.type == Token::ColonEqual) { // token equal ':='
        p.lex.goto_next_token(); // Skip ':='


        assert("Internal Error: @TODO Implementation");

    }  else if (p.lex.token.type == Token::ColonColon) { // token equal '::'
        p.lex.goto_next_token(); // Skip '::'

        expression = parse_const_expression(p);
        is_const_expr = true;
    }

    if (!expression) {
        return 0;
    }



    result->identifier = ident;
    result->type_def = 0;
    result->is_const = is_const_expr;
    result->expression = expression;

    p.pop_declaration();

    return result;
} // parse_declaration


Literal* parse_integer(Parser& p) {
    assert(p.lex.token.type == Token::LiteralInteger);

    Literal* lit = AST_NEW(Literal, p.lex.location);
    lit->literal_type = LITERAL_NUMBER;

    Token::print(p.lex.token);

    lit->integer_value = p.lex.token.integer_value;
    p.lex.goto_next_token();
    return lit;

} // parse_number

TypeDefinition* parse_declaration(Parser& p) {
    printf("parse_declaration\n");

    TypeDefinition* result = 0;

    Location loc = p.lex.location;

    switch(p.lex.token.type) {

    case '*': {
        p.lex.goto_next_token(); // Skip '*'

        PointerDef* ptr_decl = AST_NEW(PointerDef, loc);

        TypeDefinition* pointed = parse_declaration(p);

        ptr_decl->pointed = pointed;

        result = ptr_decl;
        break;
    }

    case Token::LiteralInteger: {
        
    }
    case Token::Identifier: {

        result = parse_type_specifier(p);

        break;
    }
    case '(': {
        result = parse_function_declaration(p);

        break;
    }
    case '[': {
        //result

        ArrayDef* array_decl = parse_array_declaration(p);

        TypeDefinition* type = parse_declaration(p);

        array_decl->type = type;

        result = array_decl;

        break;
    }
    case Token::Struct: {
        result = parse_struct_declaration(p);
        break;
    }
    case Token::Enum: {
        fprintf(stderr, "parse_definition enum error, unimplemented type %d\n", p.lex.token.type);
        break;
    }
    default:
        p.manager.log_error(loc, "Internal Error: parse_declaration, unknow token %d\n", p.lex.token.type);
        Token::print(p.lex.token);
        break;
    }

    return result;
}

TypeDefinition* parse_function_declaration(Parser& p) {
    printf("parse_function_definition\n");

    Location loc = p.lex.location;

    expect_and_consume(p, '(');

    FunctionDef* result = AST_NEW(FunctionDef, loc);

    // @TODO add some end conditions
    do {
        if (p.lex.token.type == Token::Identifier) {


            Identifier* ident = parse_identifier(p);
            Declaration* decl = parse_declaration(p, ident);
            result->arguments.push_back(decl);
        }

        if (p.lex.token.type == ',') {
            p.lex.goto_next_token();
        }

    } while(p.lex.token.type != ')' && p.lex.token.type != Token::Eof);

    if (p.lex.token.type != ')') {
        fprintf(stderr, "function parameters should be finished with a ')'\n");
        return nullptr;
    }

    expect_and_consume(p, ')');

    // @TODO clean up this case
    // An external function can be declared but not def (and don't have a block) in K file (the function is linked via a dynamic or static library)
    TypeDefinition* return_type = nullptr;

    if (p.lex.token.type == Token::Arrow) {
        p.lex.goto_next_token(); // Skip '->'
        return_type = parse_declaration(p);
    }

    Block* block = nullptr;
    if (p.lex.token.type == '{') {
        block = parse_block(p, PushScope::Yes, Scope::Function);
    }

    if (!block) {
        //p.manager.log_error_parsing("Malformed function definition\n");
        return result;
    }


    result->return_type = return_type;
    result->block = block;

    return result;
}

ArrayDef* parse_array_declaration(Parser& p) {
    printf("parse_array_declaration\n");

    Location loc = p.lex.location;

    assert(p.lex.token.type == '[');

    ArrayDef* result = AST_NEW(ArrayDef, loc);

    Expression* expr = 0;

    while(p.lex.token.type == '[') {

        p.lex.goto_next_token(); // Skip '['

        if (p.lex.token.type == ']') {

            assert("Internal Error: @TODO Unknown bound array not yet implemented\n");

        } else if (p.lex.token.type == Token::Identifier) {

            expr = parse_expression(p);
            result->expression_list.push_back(expr);

        } else if (p.lex.token.type == Token::LiteralInteger) {

            expr = parse_integer(p);
            result->expression_list.push_back(expr);

        } else if (p.lex.token.type == Token::DotDotDot) {

            assert("Internal Error: @TODO Dynamic array not yet implemented\n");

        } else {
            p.manager.log_error(loc, "unexpected expression inside []\n");
        }

        expect_and_consume(p, ']');

    }

    return result;
}

StructDef* parse_struct_declaration(Parser& p) {
    // @TODO handle error
    printf("parse_struct_definition\n");

    Location loc = p.lex.location;

    expect_and_consume(p, Token::Struct);

    // @TODO implement struct parameters

    expect_and_consume(p, '{');

    StructDef* result = AST_NEW(StructDef, loc);
    // Parse members
    // If struct is not empty

    if (p.lex.token.type != '}') {
        do {

            if (p.lex.token.type == Token::Identifier) {
                Identifier* ident = parse_identifier(p);
                Token::print(p.lex.token);

                Declaration* decl = parse_declaration(p, ident);

                assert(decl);
                if (decl->type_def->def_kind == TypeDefinition::DEF_ARRAY
                        || decl->type_def->def_kind == TypeDefinition::DEF_POINTER
                        || decl->type_def->def_kind == TypeDefinition::DEF_LITERAL
                        || decl->type_def->def_kind == TypeDefinition::DEF_TYPE_SPECIFIER) {

                    result->members.push_back(decl);

                    Token::print(p.lex.token);
                    if (p.lex.token.type == ';') {
                        p.lex.goto_next_token();
                    }
                }
            } else {
                assert("@TODO handle unamed declaration anonymous struct/enum and more ? \n");
            }
            //

        } while(p.lex.token.type != '}' && p.lex.token.type != Token::Eof);

    }

    expect_and_consume(p, '}');

    return result;
}

TypeDefinition* parse_type_specifier(Parser& p) {

    TypeSpecifier* result = 0;
    //@TODO replace this with if-else statement ?
    assert(p.lex.token.type == Token::Identifier ||
           p.lex.token.type == '[' ||
           p.lex.token.type == '*'
           );
    Location loc = p.lex.location;
    std::deque<unsigned int> arrays;
    if (p.lex.token.type == '[') {
        p.lex.goto_next_token(); // Skip '['
        if (p.lex.token.type == ']') {
            //@TODO replace this with better error handler
            fprintf(stderr, "array in type specifier must have size or double dot\n");
            return nullptr;
        }
        if (p.lex.token.type == Token::DotDot) { // is dynamic array
            fprintf(stderr, "dynamic array [..] not yet implemented\n");

        } else if (p.lex.token.type == Token::LiteralInteger) {
            //@TODO check if is a positive integer
            int i = std::atoi(p.lex.token.text);
            if (i <= 0)  {
                fprintf(stderr, "array size can't be 0 or negative\n");
            } else {
                arrays.push_back(i);
            }

        }
    }

    if (p.lex.token.type == Token::Identifier) {
        Identifier* ident = parse_identifier(p);
        result = AST_NEW(TypeSpecifier, loc);
        result->type_name = ident;
    }

    return result;//new Definition(loc, arrays);
}

FunctionCall* parse_function_call(Parser& p, Expression* exp) {
    //printf("new FunctionCall\n");
    expect_and_consume(p, '(');
    //@TODO handle parameters here
    expect_and_consume(p, ')');

    FunctionCall* result = AST_NEW(FunctionCall, exp->location);
    result->expression = exp;
    return result;
}


If* parse_if(Parser& p) {

    printf("parse_if\n");

    Location loc = p.lex.location;

    expect_and_consume(p, Token::If);

    Expression* expr = (p.lex.token.type == '(') ? parse_parenthese(p) : parse_expression(p);

    assert(expr);

    Statement* statement = parse_statement(p);

    ClauseStatementPair pair(expr, statement);

    ElseIfList list;
    Statement* finalStatement = nullptr;

    // current token alias
    const Token& t = p.lex.token;
    while(t.type == Token::Else) {
        p.lex.goto_next_token(); // skip 'else'
        if (t.type == Token::If) {
            p.lex.goto_next_token(); // skip 'if'

            Expression* expr = (p.lex.token.type == '(') ? parse_parenthese(p) : parse_expression(p);

            Statement* statement = parse_statement(p);
            ClauseStatementPair csp(expr, statement);
            list.push_back(csp);
        } else {
            // parse 'else' final statement
            finalStatement = parse_statement(p);

        }
    }

    If* result = AST_NEW(If, expr->location);
    result->firstPair = pair;
    result->elseIfList = list;
    result->finalStatement = finalStatement;

    return result;
}

Block* parse_block(Parser& p, PushScope ps, Scope::Type t) {
    printf("parse_block\n");

    Location loc = p.lex.location;

    expect_and_consume(p, '{');

    std::deque<Statement*> statementList;
    std::deque<DeferStatement*> deferList;
    std::deque<FunctionDef*> nestedList;

    while (p.lex.token.type != '}' && p.lex.token.type != Token::Eof) {

        Statement* s = parse_statement(p);
        // assert(s);

        if (s) {
            if (s->type == TYPE_DEFINITION) {
                TypeDefinition* definition = static_cast<FunctionDef*>(s);
                if (definition->def_kind == TypeDefinition::DEF_FUNCTION) {
                    FunctionDef* fd = static_cast<FunctionDef*>(definition);

                    nestedList.push_back(fd);
                }
            } else if (s->type == DEFER) {
                DeferStatement* st = static_cast<DeferStatement*>(s);
                deferList.push_front(st);
            } else {
                printf("statement added !\n");
                statementList.push_back(s);
            }
        }
    }

    if (p.lex.token.type != '}') {

        p.manager.log_error(loc, "Missing parenthesis '}'\n");
        return 0;
    }

    expect_and_consume(p, '}');


    Block* result = 0;

    result = AST_NEW(Block, loc);
    result->statements = statementList;
    result->defered = deferList;
    result->nestedFunctions = nestedList;

    return result;
}

Statement* parse_return(Parser& p) {
    printf("parse_return\n");

    Location loc = p.lex.location;

    expect_and_consume(p, Token::Return);

    Expression* expr = parse_expression(p);

    Return* result = 0;
    result = AST_NEW(Return, loc);
    result->expression = expr;

    expect_and_consume(p, ';');

    return result;
}

void expect_and_consume(Parser& p, Token::Type type) {

    if (p.lex.token.type == type) {
        p.lex.goto_next_token();
    } else {

        p.manager.log_error(p.lex.location, "Internal Error: Unexpected token\n");

    }
}

} // anonymous namespace

