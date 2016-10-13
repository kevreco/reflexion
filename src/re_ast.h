#ifndef RE_AST_H
#define RE_AST_H

#include <vector>
#include <deque>
#include <utility>

#include "re_global.h"
//#include "re_manager.h"
#include "re_location.h"
#include "re_fileinfo.h"

// Forward declarations




namespace Ast {


// Directives

struct DirectiveImport;
struct DirectiveRun;

struct Expression;
//struct Block;
struct Statement;
struct FunctionCall;
//struct Type;
//struct Identifier;

//using TypeSpecifier = Identifier;
// clause statment if/while (condition) statement
using ClauseStatementPair = std::pair<Expression*, Statement*>;
using ElseIfList = std::deque<ClauseStatementPair>;
} // namespace Ast

namespace Ast {
enum Type {
    TOP_LEVEL,
    // Directives
    DIRECTIVE_IMPORT,
    DIRECTIVE_RUN,

    IDENTIFIER,
    UNARY,
    BINARY,
    BINARY_OPERATOR, // 5

    ASSIGNMENT,
    LITERAL,

    NAMED_DECLARATION,
    TYPE_DEFINITION, // 9
    TYPE_SPECIFIER,
    ELLIPSIS,

    FUNCTION_CALL,
    INDEX,
    SELECTOR,
    // Statements

    IF,
    BLOCK,
    RETURN,
    EXPRESSION_STATEMENT,
    EMPTY,
    DEFER,
};

enum BinaryOperator {
    BinaryUnknow = 0,
    Add = '+', // +
    Sub = '-', // -
    Mul = '*', // *
    Div = '/', // /
    Mod = '%', // %
    Assign = '=', // =
};

enum UnaryOperator {
    UnaryUnknow = 0,

    LogicalNotOp = '!',
    PositiveOp = '+',
    NegativeOp = '-',
    //DereferenceOp = '*',
    //AdressOp = '&',
    BitwiseNot = '~',
    IncrementOp,
    DecrementOp,
};


/*
    enum Precedence {
        Sequence,
        Throw,
        Assignment,
        LogicalOR,
        LogicalAND,
        BitwiseOR,
        BitwiseXOR,
        BitwiseAND,
        Equality,
        Relational,
        BitwiseSHIFT,
        Additive,
        Multiplicative,
        PointerToMember,
        Unary,
        Postfix,
        Scope,
    }
    */
}

namespace Ast {

struct Expression;
struct Identifier;
struct Declaration;
struct TypeDefinition;
struct Literal;

struct Import;
struct Block;
struct Statement;
struct DeferStatement;


struct TypeDefinition;

struct FunctionDef;

struct Node {

    Location location;
    Type type;
}; // end struct Node

/*
struct Scope : Node {

    // Can be TopLevel or Block
    Scope(Type t, Scope* parent_scope);

    // If null this current scope is the global scope
    Scope* parent_scope;
};
*/

// Base class
struct Expression : Node {

    TypeDefinition* inferred_type = 0;
};

// Base class
struct Statement : Expression {
};

struct Block : Statement {

    enum Flags {
        Unknown, // @TODO remove this
        BLOCK_FUNCTION,
        Compound,
    };

    Block() {
        type = BLOCK;
    }


    std::deque<Declaration*> declarations;

    std::deque<Statement*> statements;
    std::deque<DeferStatement*> defered;
    std::deque<FunctionDef*> nestedFunctions;


    Flags flags = Unknown;
};

// @Dirty TopLevel Derived from Block
struct TopLevel : Node {

    TopLevel() {
        type = TOP_LEVEL;
    }

    void set_path(const utf8_string* path_) {
        path = path_;
    }

    void add(Node* n);


    const utf8_string* path = 0;
    //FileInfo fileinfo;

    std::vector<DirectiveImport*> import_items;
    std::vector<DirectiveRun*> run_items;
    std::vector<Node*> nodes;

    darray<Declaration*> function_declarations;
    darray<Declaration*> struct_declarations;
};



/*
      can be:
      - a
      - a.type
      - a.type[0]
      - a.type[a.index]
      - a.type[a.index]()
      - a.type[a.index]()[0]
      - ...
     */



// Base class
struct Directive : Expression {
};

struct DirectiveImport : Directive {

    DirectiveImport() {

        type = DIRECTIVE_IMPORT;
    }
    /*
    void set_info() {
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
*/
    utf8_string absolute_path;
    //const char* filepath;
    //unsigned filepath_len;

    //const char* basenamepath_ptr; // filepath up to the last '.' (not included).
    //int basenamepath_len;
};

struct DirectiveRun : Directive {
    DirectiveRun() {
        type = DIRECTIVE_RUN;
    }

    FunctionCall* function_call = 0;
};

/*
    struct VariableDeclaration : Node {
        VariableDeclaration() : Node(AstVariableDeclaration) {}
        Type type;
        Identifier* identifier;
        Expression* expression;
    };
    */

// SPECIAL_EXPRESSION





struct Identifier : Expression {
    Identifier()  {
        type = IDENTIFIER;
    }

    const utf8_string* name = 0;

    // Unless at top level, an identifier always reference a declaration
    Declaration* resolved_declaration = 0;
    // Unless at top level, an identifier belong to a declaration.
    Declaration* parent_declaration = 0;
    Block* scope = 0;

    bool is_builtin_type = false;

}; // end struct Identifier

// EXPRESSION

/*
    struct Primary : Node {
        Primary(Expression* exp);
        Expression* expression;
    };
    */
struct Unary : Expression {

    Unary() {
        type = UNARY;
    }

    UnaryOperator op = UnaryUnknow;
    // Primary
    Expression* operand = 0;
}; // end struct Binary

struct Binary : Expression {

    Binary() {
        type = BINARY;
    }


    Expression* lhs = 0;
    BinaryOperator op = BinaryOperator::BinaryUnknow;
    Expression* rhs = 0;
}; // end struct Binary

/*
    struct BinaryOperator : Expression {
        BinaryOperator(BinaryOperatorType o);

        BinaryOperatorType op;
    }; // end struct BinaryOperator

    struct AssignmentOperator : BinaryOperator {
        AssignmentOperator(BinaryOperatorType o);

        BinaryOperatorType op;
    }; // end struct BinaryOperator
    */

// NamedDeclaration are built-in type, struct, enum, array, pointer and function
// I use "TypeDeclaration" instead of "Declaration",
// I still don't know if 'using' 'import' etc. will be considered as declarations

// Rename this "DeclarationHead" or something ?

// ident : Vector[4];         // in this case the data_type is the same as the expression
// ident : make_vector_4();   // in this case the data_type will be 'Vector[4]' and the expression 'make_vector_4()'
struct Declaration : Expression {
    //Declaration(Identifier* i, TypeSpecifier* t);
    Declaration() {
        type = NAMED_DECLARATION;
    }

    // null parent scope is global scope


    // Sometime a named identifier doesn't have name, yes.
    Identifier* identifier = nullptr;
    TypeDefinition* type_def = nullptr;

    Expression* expression = nullptr;

    Block* parent_scope = 0;

    bool is_const = false;

    // variable used only for c convertion
    struct C {
        mutable char name[512];
    } c;


    // Unique identifier withi the declaration
    darray<Identifier*> unresolved_idents;
    bool is_resolved = false;


}; // end struct Declaration

/*
    struct Initialization : Declaration {


        Identifier* identifier = nullptr;
        TypeSpecifier* typeSpecifier = nullptr;
        Expression* expression = nullptr;
    }; // end struct Initialization
    */

// function definition or struct definition
struct TypeDefinition : Statement {

    // Definition type
    enum DefKind {
        DEF_UNKNOW,
        DEF_FUNCTION,
        DEF_STRUCT,
        DEF_ENUM,
        DEF_POINTER,
        DEF_ARRAY,
        DEF_LITERAL,
        DEF_TYPE_SPECIFIER, // MyType, *Vector, [4] float
    };

    //Declaration(Identifier* i, TypeSpecifier* t);
    TypeDefinition() {
        type = TYPE_DEFINITION;
    }

    //Identifier* identifier = nullptr;
    DefKind def_kind = DEF_UNKNOW;


}; // end struct Declaration


enum FunctionDefinitionFlags {
    FD_NONE = 0,
    FD_IS_NESTED = 1 << 1,
    //B = 1 << 2,
};

struct FunctionDef : TypeDefinition {
    FunctionDef() {
        def_kind = DEF_FUNCTION;
    }

    //Identifier* identifier;
    // @TODO a declaration like " a := 0" can't be a function parameter
    darray<Declaration*> arguments;
    TypeDefinition* return_type = 0;
    Block* block = 0;


    int flags = FD_NONE;

}; // end struct FunctionDeclaration



struct StructDef : TypeDefinition {
    StructDef() {
        def_kind = DEF_STRUCT;
    }

    std::deque<Declaration*> functions;
    std::deque<Declaration*> members;
}; // end struct StructDefinition


struct EnumDef : TypeDefinition {
    EnumDef() {
        def_kind = DEF_ENUM;
    }

    s64 highest_value = -1;
    s64 lowest_value = -1;

    TypeDefinition* type_definition = 0;

    darray<Declaration*> members;

    int num_members_completed = 0;
    int num_members_count = 0;
}; // end struct EnumDeclaration



/*
// Other definition which are not Struct, Enum or Function
struct TypeDefinition : Definition {
    //Declaration(Identifier* i, TypeSpecifier* t);
    TypeDefinition(const Location& l);


    TypeSpecifier* type_specifier = 0;

    Expression* expression;

}; // end struct TypeDefinition
*/

// can be  int, int[1024], MyStruct, MyStruct[2], or ... (varargs)

struct TypeSpecifier : TypeDefinition {

    TypeSpecifier() {
        def_kind = DEF_TYPE_SPECIFIER;
    }

    //TypeDefinition;//(const Location& l, const std::deque<unsigned int>& a);

    Identifier* type_name;
    darray<Expression*> expression_list;

    bool is_varargs = false;

};

struct PointerDef : TypeDefinition {

    PointerDef() {
        def_kind = DEF_POINTER;
    }

    TypeDefinition* pointed = 0;

}; // end struct PointerDefinition


struct ArrayDef : TypeDefinition {

    ArrayDef() {
        def_kind = DEF_ARRAY;
    }

    TypeDefinition* type = 0;

    darray<Expression*> expression_list;
    //darray<int> interger_list;


}; // end struct PointerDefinition


struct LiteralDef : TypeDefinition {

    LiteralDef() {
        def_kind = DEF_LITERAL;
    }

    Literal* literal = 0;

}; // end struct PointerDefinition

/* Expression in parameter should be:
        - member '.'
        - IndexExpression, '[]';
        - FunctionCallExpression, '()'
        - Identifier
     */


enum LiteralType {
    LITERAL_UNDEFINED,
    LITERAL_NUMBER,
    LITERAL_STRING,
    LITERAL_TRUE,
    LITERAL_FALSE,
    LITERAL_NULL,
};

// Literal Expression
struct Literal : Expression {
    Literal() {
        type = LITERAL;
    }

    LiteralType literal_type = LITERAL_UNDEFINED;


    union {
        utf8_string string_value;
        double      float_value;
        s64         integer_value;
        bool        bool_value; // @TODO remove this ? LITERAL_TRUE/LITERAL_FALSE is not enough ?
    };

};

struct FunctionCall : Expression {
    FunctionCall() {
        type = FUNCTION_CALL;
    }

    /*
          union {
           Special* special;
           Identifier* identifier;
          }
         */
    // Primary exp
    Expression* expression = 0;
    // @TODO take arguments/parameters
};
// STATEMENT

struct If : Statement {

    If() {
        type = IF;
    }

    ClauseStatementPair firstPair;
    ElseIfList elseIfList;
    Statement* finalStatement = 0;
};


struct Return : Statement {
    Return() {
        type = RETURN;
    }

    Expression* expression = 0;
};

/* like:
      a = 1; <- ExpressionStatement
      if (a == 1)
        a = 2; <- ExpressionStatement
    */
struct ExpressionStatement : Statement {
    ExpressionStatement() {
        type = EXPRESSION_STATEMENT;
    }

    Expression* expression;
}; // end struct ExpressionStatement


// like if (a);
struct EmptyStatement : Statement {
    EmptyStatement() {
        type = EMPTY;
    }
}; // end struct EmptyStatement

// @ TODO a defer statement can be just a statement, do we need this struct
struct DeferStatement : Statement {
    DeferStatement() {
        type = DEFER;
    }

    Statement* statement;
}; // end struct DeferStatement


} // namespace Ast



#endif // RE_AST_H
