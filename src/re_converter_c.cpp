#include "re_converter_c.h"

#include <assert.h>

#include <string.h> // memset

#include "re_ast.h"
#include "re_manager.h"


//-------------------------------------------------------------------------
// Forward Private Function Declarations
//-------------------------------------------------------------------------

namespace {



void print_top_level(ConverterC& c);

void print_expression(ConverterC& c,const Expression* e);
void print_import(ConverterC& c,const DirectiveImport* i);
void print_literal(ConverterC& c,const Literal* l);


//void printPrimary(const Primary* p);
void print_unary(ConverterC& c,const Unary* u);
void print_binary(ConverterC& c,const Binary* b);
void print_unary_op(ConverterC& c,const UnaryOperator u);
void print_binary_op(ConverterC& c,const BinaryOperator b);
void print_function_call(ConverterC& c,const FunctionCall* fc);

void print_declaration(ConverterC& c,const Declaration* d);
//void print_top_level_definition(ConverterC& c,const Declaration* d); // Top level definition
void print_type_definition(ConverterC& c,const TypeDefinition* d, const Declaration* ident); // Local definition
void print_struct_definition(ConverterC& c,const StructDef* sd, const Identifier* ident);
void print_function_definition(ConverterC& c,const FunctionDef* fd, const Identifier* ident);
void print_top_level_function_definition(ConverterC& c,const FunctionDef* fd, const Identifier* ident);

//void print_type_definition(ConverterC& c, const TypeDefinition* type_definition);



enum ParentStatement {
    NONE,
    COMPOUND,
};


void print_nested_function(ConverterC& c,const FunctionDef* fd);
//void print_nested_protoype(ConverterC& c,const FunctionDeclaration* fd, const Identifier* ident);


void print_statement_from_block(ConverterC& c,const Statement* s, ParentStatement ps);
void print_statement(ConverterC& c,const Statement* s, ParentStatement ps);
void print_if(ConverterC& c,const If* i);
void print_block(ConverterC& c,const Block* b, ParentStatement ps);

void print_return(ConverterC& c,const Return* r);
void print_expression_statement(ConverterC& c, const ExpressionStatement* es);

void print_identifier(ConverterC& c, const Identifier* ident);
//void print_type_specifier_to_file(std::ofstream& str, const TypeDefinition* t);
void print_function_prototype(ConverterC& c, const FunctionDef* func_definition, const Identifier* ident);

// Indent stuff

void indent_inc(ConverterC& c);
void indent_dec(ConverterC& c);

void indent(ConverterC& c);

// Helper to deal easily with non null terminated string

inline void print_str(FILE* file, const utf8_string& str) {
    assert(!str.empty());
    fprintf(file, "%.*s", str.size(), str.data());
}
} // anonymous namespace

const char* ConverterC::indent_txt = "    ";

const char* boilerplate
=R"raw(// <Boilerplate>

 typedef signed long long   s64;
 typedef signed int         s32;
 typedef signed short       s16;
 typedef signed char        s8;

 typedef unsigned long long u64;
 typedef unsigned int       u32;
 typedef unsigned short     u16;
 typedef unsigned char      u8;

 // </Boilerplate>

 )raw";

//-------------------------------------------------------------------------
// Converter
//-------------------------------------------------------------------------

using namespace Ast;

ConverterC::ConverterC(Manager& mgr) :
    manager(mgr),
    is_top_level(true),
    indent_lvl(0)
{

} // ConverterC


void ConverterC::compile_to_file(const char* outpout_filename)
{
    if (!manager.errors.empty()) {
        fprintf(stderr, "ConverterC::convert: Error occured, can't continue.\n");
        return;
    }

    printf("Trying to generate: %s\n", outpout_filename);

    // file_h.open(outpout_filename);

    of = fopen(outpout_filename, "wa");

    //if (!file_h.is_open()) {
    // fprintf(stderr, "Can't create or open file: %s\n", outpout_filename);
    // }

    if (!of) {
        fprintf(stderr, "Can't create or open file: %s\n", outpout_filename);
    }

    // Print boiler plate
    {
        //file_h << boilerplate;
        fprintf(of, boilerplate);
    }


    { // Setup nested prefix

        //memset(nested_func_name_buffer, 0, MaxIdentifierNameLength);

        //sprintf(nested_func_name_buffer, "__nested_%.*s",
        //        top_level.fileinfo.basename.len,
        //        top_level.fileinfo.basename.ptr
        //        );
        //current_nested_func_id = 0;

    }

    ConverterC& self = *this;

    print_top_level(self);

    if (of) {
        fclose(of);
    }

} // convert


//-------------------------------------------------------------------------
// Private Function Definition
//-------------------------------------------------------------------------

namespace {

void print_top_level(ConverterC& c) {

    // Print other declaration

    if (!c.manager.global_scope.declarations.empty()) {

        fprintf(c.of, "// === <resolved declarations> ===\n\n");
        // Print all prototypes
        for (auto declaration: c.manager.global_scope.declarations) {

            printf("TEST 01\n");
            print_declaration(c, declaration);
            fprintf(c.of, "\n");
        }
        fprintf(c.of, "\n// === </resolved definitions> ===\n\n");
    }
    // Print include file in the c file

/*
    if (!top_level.import_items.empty()) {

        fprintf(c.of, "// === imports ===\n\n");
        // Print all imports
        for (auto i: top_level.import_items) {
            print_import(c, i);
        }

        fprintf(c.of, "\n\n// === /imports ===\n\n");
    }
*/

    /*
    // Print struct

    if (!top_level.struct_declarations.empty()) {

        fprintf(c.of, "// === <forwards struct declarations> ===\n\n");
        // Print all prototypes
        for (auto declaration: top_level.struct_declarations) {
            const StructDef* struct_definition = static_cast<const StructDef*>(declaration->type_def);
            print_struct_definition(c, struct_definition, declaration->identifier);
            //c.file_h << ";\n";
        }
        fprintf(c.of, "\n// === </forwards struct declarations> ===\n\n");
    }

    // Print forward prototype

    if (!top_level.function_declarations.empty()) {

        fprintf(c.of, "// === <forwards function declarations> ===\n\n");
        // Print all prototypes
        for (auto declaration: top_level.function_declarations) {
            const FunctionDef* func_definition = static_cast<const FunctionDef*>(declaration->type_def);
            print_function_prototype(c, func_definition, declaration->identifier);
            fprintf(c.of, ";\n");
        }
        fprintf(c.of, "\n// === </forwards function declarations> ===\n\n");
    }


    // Print function definition

    if (!top_level.function_declarations.empty()) {

        fprintf(c.of, "// === <function definitions> ===\n\n");
        // Print all prototypes
        for (auto declaration: top_level.function_declarations) {
            const FunctionDef* func_definition = static_cast<const FunctionDef*>(declaration->type_def);
            print_function_definition(c, func_definition, declaration->identifier);
        }
        fprintf(c.of, "\n// === </function definitions> ===\n\n");
    }
*/


    /*

    for (const Node* n: top_level.nodes) {
        switch (n->type) {

        case NAMED_DECLARATION : {
            auto i = static_cast<const Declaration*>(n);
            print_named_declaration(c, i);
            break;
        }
        case LITERAL: {
            auto l = static_cast<const Literal*>(n);
            print_literal(c, l);
            break;
        }
            // @TEMP remove this
        case FUNCTION_CALL: {
            auto l = static_cast<const FunctionCall*>(n);
            print_function_call(c, l);
            break;
        }

        default: {
            fprintf(stderr, "ConverterC print_top_level, unknown type ast node type %d :: %c\n", n->type, n->type);
        }
        }
    }
*/

} // print_top_level

void print_if(ConverterC& c,const If* i) {

    // if (ps == COMPOUND) {
    
    // }

    indent(c);
    fprintf(c.of, "if ");

    // First statement
    ClauseStatementPair p(i->firstPair);
    assert(p.first);

    fprintf(c.of, "(");
    print_expression(c, p.first);
    fprintf(c.of, ")");
    /*if (p.second->type == EMPTY) {
        c.file_h << ";";
    } else if (p.second->type == BLOCK) {
        c.file_h << " ";
        auto b = static_cast<const Block*>(p.second);
        printBlock(b, NONE);
    } else {
        c.file_h << "\n";
        indent_inc();
        printStatement(p.second, NONE);
        indent_dec();
        }*/
    print_statement(c, p.second, NONE);

    //if (!i->elseIfList.empty()) {
    for(const ClauseStatementPair& s: i->elseIfList) {

        fprintf(c.of, " else if (");
        print_expression(c, s.first);
        fprintf(c.of, ")");

        print_statement(c, s.second, NONE);
    }

    if (i->finalStatement) {

        fprintf(c.of, " else");
        Statement* s = i->finalStatement;
        print_statement(c, s, NONE);
        //indent(c, c.file_h) << "}";
    }

} // print_if

void print_expression(ConverterC& c, const Expression* e) {
    printf("print_expression\n");
    switch (e->type) {
    case IDENTIFIER: {
        auto ident = static_cast<const Identifier*>(e);
        print_identifier(c, ident);
        break;
    }
    case LITERAL: {
        auto literal = static_cast<const Literal*>(e);
        print_literal(c, literal);
        break;
    }
    case UNARY: {
        auto unary = static_cast<const Unary*>(e);
        print_unary(c, unary);
        break;
    }
    case BINARY: {
        auto binary = static_cast<const Binary*>(e);
        print_binary(c, binary);
        break;
    }
    case NAMED_DECLARATION : {
        auto def = static_cast<const Declaration*>(e);
        print_declaration(c, def);
        break;
    }
    case FUNCTION_CALL: {
        auto call = static_cast<const FunctionCall*>(e);
        print_function_call(c, call);
        break;
    }

    case TYPE_DEFINITION : {
        assert("A Definition should be handled in a TypeDeclaration\n");
        break;
    }


    default:
        fprintf(stderr, "print_expression unknown type %d\n", e->type);
    } // end switch

} // print_expression

//void print_import(ConverterC& c, const DirectiveImport* import) {


//} // print_import


void print_unary(ConverterC& c, const Unary* u) {
    //printf("print unary\n");
    fprintf(c.of, "(");
    print_unary_op(c, u->op);
    print_expression(c, u->operand);
    fprintf(c.of, ")");

} // print_unary

void print_binary(ConverterC& c, const Binary* b) {
    //printf("print binary\n");
    fprintf(c.of, "(");
    print_expression(c, b->lhs);
    print_binary_op(c, b->op);
    print_expression(c, b->rhs);
    fprintf(c.of, ")");

} // print_binary

void print_function_call(ConverterC& c, const FunctionCall* fc) {
    //printf("print_function_call\n");
    print_expression(c, fc->expression);
    fprintf(c.of, "()");
    //printf("END printFunctionCall\n");

} // print_function_call

void print_unary_op(ConverterC& c, const UnaryOperator u) {
    printf("print_unary_op\n");

    switch(u) {
    case '!':
    case '+':
    case '-': {
        //c.file_h << (char)u;
        fprintf(c.of, "%c", (char)u);
        break;
    }
    default: {
        fprintf(stderr, "print_unary_op, unknown op: %c\n", u);
    }
    }

} // print_unary_op

void print_binary_op(ConverterC& c, const BinaryOperator b) {


    switch(b) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=': {
        fprintf(c.of, " %c ", (char)b);
        break;
    }
    default: {
        fprintf(stderr, "print_binary_op, unknown op: %c\n", b);
    }
    }

} // print_binary_op


void print_literal(ConverterC& c, const Literal* l) {
    

    switch(l->literal_type) {

    case LITERAL_NUMBER: {
        fprintf(c.of, "%lld", l->integer_value);
        break;
    }
        /*
    case LITERAL_FLOAT: {
        c.file_h << l->float_value;
        break;
    }
        */
    case LITERAL_TRUE: {
        fprintf(c.of, "true");
        break;
    }
    case LITERAL_FALSE: {
        fprintf(c.of, "false");
        break;
    }
    case LITERAL_NULL: {
        fprintf(c.of, "null");
        break;
    }
    default: {
        fprintf(stderr, "print_literal unknown type: %d\n", l->literal_type);
    }
    } // end switch

} // print_literal

void print_declaration(ConverterC& c, const Declaration* declaration) {
    printf("print_declaration\n");



    if (declaration->type_def) {
    //if (declaration->inferred_type) {

        printf("print_declaration A\n");
       print_type_definition(c, declaration->type_def, declaration);
        //print_type_definition(c, declaration->inferred_type, declaration);


        //fprintf(c.of, " ");

        
        //if (d->expression) {
        //    c.file_h << " = ";
        //    print_expression(c, d->expression);
        //}
    }

    // @TEMP
    if (declaration->expression) {
        printf("print_named_declaration B\n");
        print_expression(c, declaration->expression);


        if (declaration->type_def && declaration->type_def->def_kind == TypeDefinition::DEF_LITERAL) {
            fprintf(c.of, ";");
        }

    }
} // print_declaration

void print_top_level_definition(ConverterC& c, const TypeDefinition* d, const Identifier* ident) {
    //printf("print_top_level_definition\n");
    switch(d->def_kind) {
    case TypeDefinition::DEF_FUNCTION: {
        auto f = static_cast<const FunctionDef*>(d);
        
        print_top_level_function_definition(c, f, ident);
        break;
    }
    default:
        fprintf(stderr, "print_top_level_definition, unknown definition type %d\n", d->def_kind);
        return;
    }

} // print_top_level_definition

void print_type_definition(ConverterC& c, const TypeDefinition* def, const Declaration* decl) {
    assert(def);
    printf("print_type_definition: kind:%d\n", def->def_kind);


    switch(def->def_kind) {

    case TypeDefinition::DEF_LITERAL: {

        const LiteralDef* literal_def = static_cast<const LiteralDef*>(def);
        const Literal* literal = literal_def->literal;
        if (literal->literal_type == LITERAL_NUMBER) {
        //print_literal(c, literal_def->literal);
        //print_function_definition(c, definition, name_ident);
        //print_identifier(c, declaration->identifier);
            if (decl->is_const) {
                fprintf(c.of, "const ");
            }
            fprintf(c.of, "int");

            if (decl->identifier) {
                fprintf(c.of, " ");
                print_identifier(c, decl->identifier);
            }
            fprintf(c.of, " = ");
        }

        break;
    }

    case TypeDefinition::DEF_FUNCTION: {

        const FunctionDef* definition = static_cast<const FunctionDef*>(def);

        print_function_definition(c, definition, decl->identifier);
        //print_identifier(c, declaration->identifier);

        break;
    }

    case TypeDefinition::DEF_STRUCT: {

        const StructDef* definition = static_cast<const StructDef*>(def);

        print_struct_definition(c, definition, decl->identifier);
        //print_identifier(c, declaration->identifier);

        break;
    }



    case TypeDefinition::DEF_TYPE_SPECIFIER: {

        printf("print type specifer\n");
        const TypeSpecifier* definition = static_cast<const TypeSpecifier*>(def);


        print_identifier(c, definition->type_name);


        if (decl->identifier) {
            fprintf(c.of, " ");
            print_identifier(c, decl->identifier);
        }

        break;
    }

    case TypeDefinition::DEF_POINTER: {
        printf("print ptr\n");

        const PointerDef* definition = static_cast<const PointerDef*>(def);



        print_type_definition(c, definition->pointed, 0);

        fprintf(c.of, "*");

        if (decl->identifier) {
            fprintf(c.of, " ");
            print_identifier(c, decl->identifier);
        }

        break;
    }
    case TypeDefinition::DEF_ARRAY: {


        const ArrayDef* definition = static_cast<const ArrayDef*>(def);


        print_type_definition(c, definition->type, 0);

        //print_identifier(c, ident);
        fprintf(c.of, " ");


        if (decl->identifier) {
            //fprintf(c.of, " ");
            print_identifier(c, decl->identifier);
        }

        for (const Expression* expr: definition->expression_list) {
            fprintf(c.of, "[0]");
        }


        break;
    }
    default:
        fprintf(stderr, "print_declaration, unknown definition type %d\n", def->def_kind);
        return;
    }
} // print_definition


//void print_definition(ConverterC& c, const TypeDefinition* type_definition) {

//print_expression(c, type_definition);

//} // print_type_definition


void print_top_level_function_definition(ConverterC& c, const FunctionDef* func_definition, const Identifier* ident) {
    //printf("print_top_level_function_definition\n");

    print_function_prototype(c, func_definition, ident);
    fprintf(c.of, ";\n\n");

    print_function_definition(c, func_definition, ident);
    //printf("END print_top_level_function_definition\n");
}

void print_function_definition(ConverterC& c, const FunctionDef* func_definition, const Identifier* ident) {
    printf("print_function_definition\n");

    //printf("nested function count: %d\n", fd->block->nestedFunctions.size());
    for (const auto nested: func_definition->block->nestedFunctions) {

        print_nested_function(c, nested);
        //printdFunctionDefinition(nested, Nested);
    }


    print_function_prototype(c, func_definition, ident);
    fprintf(c.of, " ");
    
    if (func_definition->block) {
        print_block(c, func_definition->block, NONE);
    }

    fprintf(c.of, "\n\n");
    //printf("END printFunctionDefinition\n");
} // print_function_definition

void print_nested_function(ConverterC& c, const FunctionDef* fd) {
    //printf("printNestedFunction\n");


    for (auto nested: fd->block->nestedFunctions) {
        print_nested_function(c, nested);
    }



    char current_nested_name[ConverterC::MaxIdentifierNameLength] = {};
    int count = sprintf(current_nested_name,
                        "inline static %s_%d",
                        c.nested_func_name_buffer,
                        c.current_nested_func_id);

    if (count > ConverterC::MaxIdentifierNameLength) {
        fprintf(stderr, "Nested functione name too long\n");
    }

    fprintf(c.of, current_nested_name);

    fprintf(c.of, "(");
    const auto& param = fd->arguments;
    if (!param.empty()) {
        auto it = fd->arguments.begin();
        auto itEnd = fd->arguments.end();
        //printParam(*it); // @TODO
        ++it;
        while(it != itEnd) {
            fprintf(c.of, ",");
            //printParam(*it); // @TODO
            ++it;
        }
    }
    fprintf(c.of, ") ");
    
    //printPrototype(c, fd);

    if (fd->block) {
        print_block(c, fd->block, NONE);
    }
    
    fprintf(c.of, "\n\n");

    // Increment the id for nested function
    c.current_nested_func_id++;
} // print_nested_function

void print_nested_prototype(ConverterC& c, const FunctionDef* func_definition, const Identifier* ident) {

    indent(c);
    print_function_prototype(c, func_definition, ident);
    fprintf(c.of, ";\n");

} // print_nested_prototype


void print_struct_definition(ConverterC& c, const StructDef* struct_definition, const Identifier* ident) {
    printf("print_struct_definition\n");
    assert(struct_definition);

    fprintf(c.of, "struct ");
    print_identifier(c, ident);
    //print_str(c.of, *ident->name);
    fprintf(c.of, " {");
    if (!struct_definition->members.empty()) {
        //@TODO fill struct
        fprintf(c.of, "\n");
        indent_inc(c);
        for (Declaration* decl: struct_definition->members) {
            indent(c);
            print_declaration(c, decl);
            fprintf(c.of, ";\n");
        }
        indent_dec(c);
    }
    fprintf(c.of, "};\n");


} // print_struct_definition


void print_statement_from_block(ConverterC& c, const Statement* s, ParentStatement ps) {
    //printf("print_statement_from_block\n");
    switch(s->type) {
    
    case EMPTY: {
        fprintf(c.of, ";");
    }
    case BLOCK: {
        auto b = static_cast<const Block*>(s);
        print_block(c, b, ps);
        break;
    }
    case IF: {
        auto i = static_cast<const If*>(s);
        print_if(c, i);
        break;
    }

    case RETURN: {
        auto r = static_cast<const Return*>(s);
        print_return(c, r);
        break;
    }
    case EXPRESSION_STATEMENT: {
        auto es = static_cast<const ExpressionStatement*>(s);
        print_expression_statement(c, es);

        break;
    }
        /*case FUNCTION_DEFINITION: {
        auto d = static_cast<const FunctionDefinition*>(s);

        //auto r = static_cast<const Return*>(s);

        break;
        }*/
        /*case DEFINITION: {
        auto d = static_cast<const Definition*>(s);
        print_definition(c, d);

        break;

    }
    case DEFINITION: {
        assert("A Definition should be handled in a TypeDeclaration\n");
        break;
    }
*/
    default:
        fprintf(stderr, "print_statement_from_block, unknown statement %d\n", s->type);
        return;
    }

} // print_statement_from_block

void print_statement(ConverterC& c, const Statement* s, ParentStatement ps) {
    //printf("print_statement\n");

    if (s->type == EMPTY) {
        fprintf(c.of, ";");
        return;
    } else if (s->type == BLOCK) {
        fprintf(c.of, " ");
        auto b = static_cast<const Block*>(s);
        print_block(c, b, ps);
        return;
    }

    fprintf(c.of, "\n");
    indent_inc(c);

    switch(s->type) {

    case IF: {
        //indent_inc();
        //indent(c, c.file_h);
        auto i = static_cast<const If*>(s);
        print_if(c, i);
        //indent_dec();
        break;
    }

    case RETURN: {
        auto r = static_cast<const Return*>(s);
        print_return(c, r);
        break;
    }
    case EXPRESSION_STATEMENT: {
        auto es = static_cast<const ExpressionStatement*>(s);
        print_expression_statement(c, es);

        break;
    }
        /*case FUNCTION_DEFINITION: {
        auto d = static_cast<const FunctionDefinition*>(s);

        //auto r = static_cast<const Return*>(s);

        break;
        }*/
        /*
    case DEFINITION: {
        auto d = static_cast<const Definition*>(s);
        print_definition(c, d);
        break;
    }

    case DEFINITION: {
        auto d = static_cast<const Definition*>(s);
        print_definition(c, d);
        break;
    }
*/
    default:
        fprintf(stderr, "print_statement, unknown statement %d\n", s->type);
        return;
    }

    indent_dec(c);

} // print_statement

void print_block(ConverterC& c, const Block* block, ParentStatement ps) {
    //printf("print_block\n");

    if (ps == COMPOUND)  {
        indent(c);
    }


    fprintf(c.of, "{\n");
    indent_inc(c);

    bool first = true;
    for (Statement* child_statement: block->statements) {
        if (first) {
            first = false;
        } else {
            fprintf(c.of, "\n");
        }
        //++counter;
        
        print_statement_from_block(c, child_statement, COMPOUND);
    }

    if (!block->defered.empty()) {
        //c.file_h << "\n";
        indent(c);
        fprintf(c.of, "\n");

        // Print defer statements
        for (int i = block->defered.size() -1; i >= 0; --i) {
            //for (int i = 0; i < b->defered.size(); ++i) {
            DeferStatement* defer = block->defered[i];
            print_statement_from_block(c, defer->statement, COMPOUND);
            fprintf(c.of, "\n");
        }
        indent(c);
    }

    indent_dec(c);
    if (first == false) {
        fprintf(c.of, "\n");
    }

    indent(c);
    fprintf(c.of, "}");

    if (ps == COMPOUND)  {
        //file_h << "\n";
        //indent(c, c.file_h);
    }

    //popNestedList();
    //indent(c, c.file_h) << "<block>\n";
    //printf("END printBlock\n");

} // print_block

void print_return(ConverterC& c, const Return* r) {
    //printf("print_return\n");

    // @TODO finish this (a return should be return always void :o)
    indent(c);
    fprintf(c.of, "return");

    if (r->expression) {

        fprintf(c.of, " ");
        print_expression(c, r->expression);
        fprintf(c.of, ";");
    }
    fprintf(c.of, "\n");

} // print_return

void print_expression_statement(ConverterC& c, const ExpressionStatement* es) {
    printf("print_expression_statement\n");
    indent(c);
    print_expression(c, es->expression);

    fprintf(c.of, ";");
    //printf("END print_expression_statement\n");

} // print_expression_statement


void print_identifier(ConverterC& c, const Identifier* ident) {
    //printf("print_identifier_to_file: %p,%p \n", i, i->name);
    assert(ident);
    assert(ident->name);

    if (ident->resolved_declaration) {
        print_expression(c, ident->resolved_declaration->expression);
    } else {
        // @TODO remove this
        print_str(c.of, *ident->name);
    }
} // print_identifier_to_file



void print_function_prototype(ConverterC& c, const FunctionDef* func_definition, const Identifier* ident) {
    // @TODO print the real return type (TypeSpecifier)
    //printf("print_prototype_to_file\n");
    if (func_definition->return_type) {
        print_type_definition(c, func_definition->return_type, NULL);
        //print_definition(c.file_h, func_definition->return_type);
    } else {
        fprintf(c.of, "void");
    }
    fprintf(c.of, " ");

    //    if (fd->isNested()) {
    //    str << fd->c.name;
    //} else {
    print_identifier(c, ident);
    //}

    //printIdentifier(str, fd->identifier);


    fprintf(c.of, "(");
    const darray<Declaration*>& args = func_definition->arguments;
    if (!args.empty()) {

        print_declaration(c, args[0]);

        for (auto it = args.begin() + 1; it != args.end(); ++it) {
            fprintf(c.of, ", ");
            print_declaration(c, *it);
        }
    }

    fprintf(c.of, ")");

} // print_prototype_to_file

// Indent stuff

inline void indent(ConverterC& c) {
    for (int i = 0; i < c.indent_lvl; ++i) {
        fprintf(c.of, c.indent_txt);
    }
} // indent

inline void indent_inc(ConverterC& c) {
    c.indent_lvl++;
} // indent_inc

inline void indent_dec(ConverterC& c) {
    c.indent_lvl--;
} // indent_dec


} // anonymous namespace
