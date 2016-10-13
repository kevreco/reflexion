#ifndef TOKEN_H
#define TOKEN_H

#include "stdio.h"
#include <cstring>
#include <string.h> // strncmp

#include "re_global.h"

struct Token {

    static const int MaxCapacity = 1 << 16; // 1 << 16 = 65536
    static const int MaxSize = MaxCapacity-1;

/*
    union {
        char buffer[MaxCapacity];
        char text[MaxCapacity];
    };
*/

    union {
        const char* buffer;
        const char* text;
        double float_value;
        s64 integer_value;
    };

    unsigned int len;

    void set_str(const char* s, int l) {
        if (l >= MaxCapacity) {
            fprintf(stderr, "Token set_str error, max capacity reach.");
        }

        //strncpy(buffer, s, l);
        buffer = s;
        len = l;
    }

    void set_char(const char* s) {
        //buffer[0] = c;
        //buffer[1] = 0;
        buffer = s;
        len = 1;

    }

    void clear() {
        //buffer[0] = 0;
        buffer = 0;
        len = 0;
    }

    bool text_equal(const char* str) {
        size_t str_len = strlen(str);
        if (len != str_len) {
            return false;
        } else {
            return strncmp(buffer, str, str_len) == 0;
        }
    }


    static void print(const Token& t);
    

    enum Type {

        Eof = '\0',

        BraceL = '{',
        BraceR = '}',

        ParenL = '(',
        ParenR = ')',

        SquareL = '[',
        SquareR = ']',

        Exlam =       '!', // 33
        DoubleQuote = '"', // 34
        Hash =        '#', // 35
        Percent =     '%', // 37
        Amp =         '&', // 38
        Star =        '*', // 42
        Plus =        '+', // 43
        Comma =       ',', // 44
        Minus =       '-', // 45
        Period =      '.', // 46
        Slash =       '/', // 47
        Colon =       ':', // 58
        SemiColon =   ';', // 59
        Less =        '<', // 60
        Equal =       '=', // 61
        Greater =     '>', // 62
        Question =    '?', // 63

        Caret =       '^', // 94
        Pipe =        '|', // 124
        Tilde =       '~', // 126


        // Keywords

        Auto,
        #define TokentrAuto      "auto"
        Break,
        #define TokenStrBreak    "break"
        Case,
        #define TokenStrCase     "case"
        Char,
        #define TokenStrChar     "char"
        Const,
        #define TokenStrConst    "const"
        Continue,
        #define TokenStrContinue "continue"
        Default,
        #define TokenStrDefault  "default"
        Defer,
        #define TokenStrDefer    "defer"
        Do,
        #define TokenStrDo       "do"
        Double,
        #define TokenStrDouble   "double"
        Else,
        #define TokenStrElse     "else"
        Enum,
        #define TokenStrEnum     "enum"
        Extern,
        #define TokenStrExtern   "extern"
        Float,
        #define TokenStrFloat    "float"
        For,
        #define TokenStrFor      "for"
        Goto,
        #define TokenStrGoto     "goto"
        If,
        #define TokenStrIf       "if"
        Int,
        #define TokenStrInt      "int"
        Long,
        #define TokenStrLong     "long"
        Register,
        #define TokenStrRegister "register"
        Return,
        #define TokenStrReturn   "return"
        Signed,
        #define TokenStrSigned   "signed"
        Sizeof,
        #define TokenStrSizeof   "sizeof"
        Short, // 25
        #define TokenStrShort    "short"
        Static,
        #define TokenStrStatic   "static"
        Struct,
        #define TokenStrStruct   "struct"
        Switch,
        #define TokenStrSwitch   "switch"
        Typedef,
        #define TokenStrTypedef  "typedef"
        Union,
        #define TokenStrUnion    "union"
        Unsigned,
        #define TokenStrUnsigned "unsigned"
        Void,
        #define TokenStrVoid     "void"
        Volatile,
        #define TokenStrVolatile "volatile"
        While, // 35
        #define TokenStrWhile    "while"
        
        // Cpp keywords
        
        Class,     // Cpp only
        #define TokenStrClass     "class"
        Constexpr, // Cpp only ?
        #define TokenStrConstexpr "constexpr"
        Template,  // Cpp only ?
        #define TokenStrTemplate  "template"
        Using,     // Cpp only ?
        #define TokenStrUsing     "using"
        Typename,  // Cpp only ?
        #define TokenStrTypename  "typename"

        // Reflexion keywords

        Import,
        #define TokenStrImport  "import"
        Run,
        #define TokenStrRun  "run"
        // OPERATOR

        // Comparison
        EqualEqual,
        #define TokenStrEqualEqual   "=="
        NotEqual,     // "!="
        #define TokenStrNotEqual     "!="
        LessEqual,    // "<="
        #define TokenStrLessEqual    "<="
        GreaterEqual, // ">="
        #define TokenStrGreaterEqual ">="

        // Logical
        AmpAmp,     // "&&"
        #define TokenStrAmpAmp   "&&"
        PipePipe,   // "||"
        #define TokenStrPipePipe "||"

        // Shift
        LessLess,  // "<<"
        #define TokenStrLessLess       "<<"
        GreaterGreater, // ">>"
        #define TokenStrGreaterGreater ">>"

        LessLessEqual,  // "<<="
        #define TokenStrLessLessEqual       "<<="
        GreaterGreaterEqual, // ">>="
        #define TokenStrGreaterGreaterEqual ">>="


        // Increment
        PlusPlus,   // "++"
        #define TokenStrPlusPlus        "++"
        MinusMinus, // "--"
        #define TokenStrMinusMinus      "--"
        MinusMinusMinus, // "---"
        #define TokenStrMinusMinusMinus "---"


        Arrow,      // "->"
        #define TokenStrArrow     "->"
        
        // ArithEqual
        PlusEqual,  // "+="
        #define TokenStrPlusEqual  "+="
        MinusEqual, // "-="
        #define TokenStrMinusEqual "-="
        MulEqual,   // "*="
        #define TokenStrMulEqual   "*="
        DivEqual,   // "/="
        #define TokenStrDivEqual   "/="
        ModEqual,   // "%="
        #define TokenStrModEqual   "%="

        // BitwiseEqual
        AmpEqual, // "&="
        #define TokenStrAmpEqual   "&="
        PipeEqual,  // "|="
        #define TokenStrPipeEqual  "|="
        CaretEqual, // "^="
        #define TokenStrCaretEqual "^="
        
        // Cpp only ?
        ColonColon, // "::"
        #define TokenStrDoubleColon "::"
        
        // Not in C or Cpp
        ColonEqual, // ":="
        #define TokenStrColonEqual ":="
        
        // Not in C or Cpp
        EqualArrow, // "=>
        #define TokenStrEqualArrow "=>"

        DotDot, // ".."
        #define TokenStrDotDot ".."
        DotDotDot, // "..."
        #define TokenStrDotDotDot "..."
  
        // Literal
        
        Identifier,
        AsciiString, // "my string" // 364
        AsciiChar, // '\n'
        True,
        #define TokenStrTrue  "true"
        False,
        #define TokenStrFalse "false"
        Null,
        #define TokenStrNull  "null"
        LiteralInteger,
        LiteralFloat,
  
        // Misc.
        Error,
    };

    Type type;

    static Type identifier_or_keyword(const char* str);
};

#endif // TOKEN_H
