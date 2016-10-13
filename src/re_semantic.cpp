#include "re_semantic.h"

#include <assert.h>
#include <iostream>

#include "re_manager.h"
#include "re_parser.h"

Semantic::Semantic(Manager& mgr) :
    manager(mgr) {

}

void Semantic::analyze() {


    printf("Top level declaration count %d\n", manager.global_scope.declarations.size());


    // Resolving identifier


    for(Declaration* decl: manager.global_scope.declarations) {
        resolve_identifiers(decl);

    }

    // Resolve constant / constant folding

    for(Declaration* decl: manager.global_scope.declarations) {

        infer_constant_from_declaration(decl);

    }

    // Check for unresolving identifier

    for(Declaration* decl: manager.global_scope.declarations) {
        printf("Unresolved count: %d\n", decl->unresolved_idents.size());
        for (Identifier* ident: decl->unresolved_idents) {
            manager.log_error(ident->location, "Unresolved declaration: %.*s\n", ident->name->size(), ident->name->data());
        }
    }


    // Check for unresolving identifier


    for(Declaration* decl: manager.global_scope.declarations) {
        infer_type_from_literal_declaration(decl->expression);
    }

}


// Private

Declaration* Semantic::find_declaration(Identifier* ident, Block* scope) {

    Declaration* result = 0;

    for (Declaration* decl: scope->declarations) {
        if (decl->identifier->name == ident->name) {
            result = decl;
            break;
        }
    }

    return result;
}

void Semantic::resolve_identifiers(Declaration* decl) {

    //printf("resolve_identifiers\n");

    // Use a while loop to be able to remove an item from the collection

    auto& ident_array = decl->unresolved_idents;
    printf("ident_array count A: %d\n", ident_array.size());
    Identifier* ident = 0;
    auto it = std::begin(ident_array);
    while (it != std::end(ident_array)) {
        ident = *it;

        // If identifier is resolved, it shouldn't be in "unresolved_idents"
        //assert(!decl->identifier->resolved_declaration);

        Declaration* found = find_declaration(ident, ident->scope);
        if (found) {
            if (found->is_resolved) {
                const utf8_string* circular_ref = ident->name;
                manager.log_error(ident->location, "Circular reference %.*s", circular_ref->size(), circular_ref->data());
                const utf8_string* prev_ref = found->identifier->name;
                manager.log_error(found->location, "Previous resolved reference %.*s", prev_ref->size(), prev_ref->data());
            }
            const utf8_string* str = found->identifier->name;
            printf("Declaration found ! %.*s\n", str->size(), str->data());
            ident->resolved_declaration = found;
            decl->is_resolved = true;
            it = ident_array.erase(it);
            continue;
        } else {
            ++it;
        }
    }
    printf("ident_array count B: %d\n", ident_array.size());




}

Literal* Semantic::infer_constant_from_declaration(Declaration* decl) {

    Expression* decl_expr = decl->expression;

    infer_constant_from_expression(decl_expr);
    assert(decl_expr->inferred_type);

    decl->type_def = decl_expr->inferred_type;
    /*
    for(Identifier* ident: decl->unresolved_idents) {
        if (ident->resolved_declaration->is_const) {
            infer_constant_from_ident(ident);

            assert(decl->expression->inferred_type);

        }
    }
    */
}

Literal* Semantic::infer_constant_from_expression(Expression* expr) {

    printf("infer_constant_from_expression %d\n", expr->type);

    // @TODO use switch
    if (expr->type == IDENTIFIER) {
        Identifier* ident = static_cast<Identifier*>(expr);
        //Identifier* ident = static_cast<Identifier*>(expression);
        // At this step all identifier should have been resolved.
        assert(ident->resolved_declaration);

        // If the identifier has an inffered_type no need to need anything
        if (!ident->inferred_type) {
            if (ident->resolved_declaration->is_const) {

                if (ident->resolved_declaration->type_def) {
                    printf("Already inferred yeah! \n");
                    expr->inferred_type = ident->resolved_declaration->type_def;
                } else {
                    printf("Not inferred inferred yet! \n");
                    infer_constant_from_declaration(ident->resolved_declaration);
                     assert(ident->resolved_declaration->type_def);
                     expr->inferred_type = ident->resolved_declaration->type_def;
                }

            } else {
                manager.log_error(ident->location, "Try to infer non const expression\n");
            }
        }
    } else if (expr->type == LITERAL) {


        Literal* literal = static_cast<Literal*>(expr);

        LiteralDef* def_literal = AST_NEW(LiteralDef, expr->location);

        def_literal->literal = literal;
        expr->inferred_type = def_literal;

        if (literal->literal_type == LITERAL_NUMBER) {
            printf("Infer Literal Number\n");
        }

    } else if (expr->type == BINARY) {

        Binary* binary = static_cast<Binary*>(expr);

        infer_constant_from_expression(binary->lhs);

        infer_constant_from_expression(binary->rhs);




    } else {
        manager.log_error(expr->location, "Internal Error: infer_constant_from_expression, case not handle yet\n");
    }
}

void Semantic::infer_type_from_literal_declaration(Expression* expression) {

    if (!expression) {
        return;
    }
    if (expression->inferred_type) {
        // Expression type already inferred.
        return;
    }

    Location loc = expression->location;

    if (expression->type == LITERAL) {

        Literal* literal = static_cast<Literal*>(expression);

        LiteralDef* def_literal = AST_NEW(LiteralDef, loc);

        def_literal->literal = literal;
        expression->inferred_type = def_literal;
        /*
        if (literal->literal_type == LITERAL_NUMBER) {
            expression->inferred_type = type_def_number;
        } else if (literal->literal_type == LITERAL_STRING) {
            expression->inferred_type = type_def_string;
        } else if (literal->literal_type == LITERAL_TRUE) {
            expression->inferred_type = type_def_true;
        } else if (literal->literal_type == LITERAL_FALSE) {
            expression->inferred_type = type_def_false;
        } else if (literal->literal_type == LITERAL_NULL) {
            expression->inferred_type = type_def_null;
        }
        */

    }
}

Expression* Semantic::find_constant_substitute(Declaration* decl, Identifier* ident) {

    Expression* result = 0;
    if (!decl->is_const) {
        result = 0;
    } else {
        result = eval_expression_to_literal(decl->expression);
    }

    return result;

}
Expression* Semantic::eval_expression_to_literal(Expression* expression) {

    Expression* result = 0;
    switch (expression->type) {
    case IDENTIFIER: {
        Identifier* ident = static_cast<Identifier*>(expression);
        result = ident->inferred_type;
        break;
    }
    case BINARY:{
        Binary* binary = static_cast<Binary*>(expression);
        BinaryOperator bin_op = binary->op;
        Expression* lit_lhs = eval_expression_to_literal(binary->lhs);
        Expression* lit_rhs = eval_expression_to_literal(binary->rhs);



        //infer_type_from_expression(binary->lhs);
        //infer_type_from_expression(binary->rhs);
        break;
    }
    } // end switch
}

Expression* Semantic::eval_const_binary(Binary* binary) {

    Expression* lhs = binary->lhs;
    Expression* rhs = binary->rhs;
    // int + real => (real)int + real => real
    // real + int => real + (real)int => real
    // char + real => (real)char + real => real
    // real + char => real + (real)char => real
    // real + real => real
    // int + int => int
    // char + char => int + int => int
    // char + int => int + int => int
    // int + char => int + int => int
    // bool + bool => bool
    // default mismatch on operator %d between %s and %s
}

Expression* eval_int_binary(Literal* lhs, Literal* rhs) {
    // * int
    // / int
    // + int
    // - int
    // < bool
    // > bool
    // <= bool
    // >= bool
    // == bool
    // != bool
}

Expression* eval_float_binary(Literal* lhs, Literal* rhs) {
    // * float
    // / float
    // + float
    // - float
    // < bool
    // > bool
    // <= bool
    // >= bool
    // == bool
    // != bool

}

Expression* eval_bool_binary(Literal* lhs, Literal* rhs) {

    // "=="
    // "!="
    // "&&"
    // "||"

}

