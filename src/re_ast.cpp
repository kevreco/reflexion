#include "re_ast.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace Ast;
/*
void Ast::TopLevel::add(Node* n) {
    nodes.push_back(n);
}



Ast::Directive::Directive(Type t) :
    Expression(loc, t) {

}

Ast::DirectiveImport::DirectiveImport(const Location& loc, const char* p, unsigned len) :
    Directive(loc,DIRECTIVE_IMPORT) {


    filepath = 0;
    filepath_len = 0;
    basenamepath_ptr = 0;
    basenamepath_len = 0;


    if (!p || !len) {
        return;
    }

    filepath = p;
    filepath_len = len;

    const char* cursor = filepath;
    while (*cursor && *cursor != '.') {
        ++cursor;
    }

    basenamepath_ptr = filepath;
    basenamepath_len = cursor - filepath;
}


Ast::DirectiveRun::DirectiveRun(const Location& loc, FunctionCall* fc) :
    Directive(loc, DIRECTIVE_RUN) {
    assert(fc);
    function_call = fc;
}

// @TODO remove strdup
Ast::Identifier::Identifier(const Location& loc, const char* n, unsigned length) :
    Expression(loc, IDENTIFIER) {

    name = strdup(n);
    len = length;

}

//Ast::Primary::Primary(Expression* exp) : Expression(exp->loc, PRIMARY), expression(exp) {

//}


Ast::Unary::Unary(const Location& loc, UnaryOperator op_, Expression* rhs) :
    Expression(loc, UNARY),
    op(op_),
    operand(rhs) {

}

Ast::Binary::Binary(Expression* lhs_, BinaryOperator op_, Expression* rhs_) :
    Expression(lhs->location, BINARY),
    lhs(lhs_),
    op(op_),
    rhs(rhs_) {

}

Ast::TypeDeclaration::TypeDeclaration(Identifier* i, Definition* definition_, Expression* e) :
    Expression(i->location, TYPE_DECLARATION),
    identifier(i),
    definition(definition_),
    expression(e) {

    if (!definition)  {
        // deduce type
        switch(expression->type) {
        //@TODO
        //case KEYWORD:
        //break;
        case IDENTIFIER: {
            auto idd = static_cast<Identifier*>(e);

            assert("@TODO");
            //definition = new Definition(idd->location);
            break;
        }
        default:
            fprintf(stderr, "ast declaration constructor, unkown expression type %d\n", expression->type);

        }
    }

    assert(definition);
}

Ast::Definition::Definition(const Location& loc, DefKind t) :
    Statement(loc, DEFINITION),
    def_kind(t) {

}
Ast::StructDefinition::StructDefinition(const Location& loc, const std::deque<FunctionDefinition*>& f) :
    Definition(loc, DEF_STRUCT),
    functions(f) {

}

Ast::FunctionDefinition::FunctionDefinition(const Location& loc, Definition* return_type_, Block* b) :
    Definition(loc, DEF_FUNCTION),
    return_type(return_type_),
    block(b) {

}



//Ast::TypeDefinition::TypeDefinition(const Location& loc) :
//    Definition(loc, DEF_UNKNOW) {
//}

Ast::FunctionCall::FunctionCall(Expression* expr) :
    Expression(expr->location, FUNCTION_CALL), expression(expr) {
}


Ast::Literal::Literal(const Location& loc, LiteralType lt) :
    Expression(loc, LITERAL),
    literal_type(lt) {
}

Ast::Statement::Statement(const Location& loc, Type t) :
    Expression(loc, t) {
}

Ast::If::If(const Location& loc, ClauseStatementPair& c, ElseIfList& e, Statement* s) :
    Statement(loc, IF),
    firstPair(c),
    elseIfList(e),
    finalStatement(s) {
    
}

Ast::Block::Block(const Location& loc, std::deque<Statement*>& s, std::deque<DeferStatement*>& defer, std::deque<FunctionDefinition*>& fd, Flags f) :
    Statement(loc, BLOCK),
    statements(s),
    defered(defer),
    nestedFunctions(fd),
    flags(f)  {

}

Ast::Return::Return(const Location& loc, Expression* expr) :
    Statement(loc, RETURN),
    expression(expr) {
}

Ast::ExpressionStatement::ExpressionStatement(Expression* expr) :
    Statement(expr->location, EXPRESSION_STATEMENT),
    expression(expr) {
}

Ast::EmptyStatement::EmptyStatement(const Location& loc) :
    Statement(loc, EMPTY) {
}

Ast::DeferStatement::DeferStatement(const Location& loc, Statement* st) :
    Statement(loc, DEFER),
    statement(st) {
}
*/
