#include "re_token.h"

struct keyword_item {
    Token::Type type;
    const char* keyword;
    size_t size;
};

static keyword_item keywords[] = {
    {Token::Auto,"auto",4},
    {Token::Break,"break",5},
    {Token::Case,"case",4},
    {Token::Char,"char",4},
    {Token::Const,"const",5},
    {Token::Continue,"continue",8},
    {Token::Default,"default",7},
    {Token::Defer,"defer",5},
    {Token::Do,"do",7},
    {Token::Double,"double",6},
    {Token::Else,"else",4},
    {Token::Enum,"enum",4},
    {Token::Extern,"extern",6},
    {Token::Float,"float",5},
    {Token::For,"for",3},
    {Token::Goto,"goto",4},
    {Token::If,"if",2},
    {Token::Int,"int",3},
    {Token::Long,"long",4},
    {Token::Register,"register",8},
    {Token::Return,"return",6},
    {Token::Signed,"signed",6},
    {Token::Sizeof,"sizeof",6},
    {Token::Short,"short",5},
    {Token::Static,"static",6},
    {Token::Struct,"struct",6},
    {Token::Switch,"switch",6},
    {Token::Typedef,"typedef",7},
    {Token::Union,"union",5},
    {Token::Unsigned,"unsigned",8},
    {Token::Void,"void",4},
    {Token::Volatile,"volatile",8},
    {Token::While,"while",5},

    // Cpp token

    {Token::Class,"class",5},
    {Token::Constexpr,"constexpr",9},
    {Token::Template,"template",8},
    {Token::Typename,"typename",8},
    {Token::Using,"using",5},

    // Reflexion keywords

    {Token::Import,"import",6},
    {Token::Run,"run",3},
};

// Currently used only for debuging

static keyword_item literals[] = {
    {Token::AsciiString,"asciistring",11},
    {Token::AsciiChar,"asciichar",10},
    {Token::AsciiChar,"asciichar",10},
    {Token::False,"false",5},
    //{Token::Identifier,"identifier",10},
    {Token::LiteralInteger,"literal_integer",15},
    {Token::LiteralFloat,"literal_float",13},
    {Token::Null,"null",4},
    {Token::True,"true",4},
};

// Currently used only for debuging

static keyword_item operators[] = {
    {Token::EqualEqual,"==",2},
    {Token::NotEqual,"!=",2},
    {Token::LessEqual,"<=",2},
    {Token::GreaterEqual,">=",2},
    {Token::AmpAmp,"&&",2},
    {Token::PipePipe,"||",2},
    {Token::LessLess,"<<",2},
    {Token::GreaterGreater,">>",2},

    {Token::LessLessEqual,"<<=",3},
    {Token::GreaterGreaterEqual,">>=",3},

    {Token::PlusPlus,"++",2},
    {Token::MinusMinus,"--",2},
    {Token::MinusMinusMinus,"---",3},

    {Token::PlusEqual,"+=",2},
    {Token::MinusEqual,"-=",2},
    {Token::MulEqual,"*=",2},
    {Token::DivEqual,"/=",2},
    {Token::ModEqual,"%=",2},

    {Token::AmpEqual,"&=",2},
    {Token::PipeEqual,"|=",2},
    {Token::CaretEqual,"^=",2},

     {Token::ColonColon,"::",2},
     {Token::ColonEqual,":=",2},
     {Token::EqualArrow,"=>",2},

    {Token::DotDot,"..",2},
    {Token::DotDotDot,"...",3},
};


Token::Type Token::identifier_or_keyword(const char* str) {

    Token::Type result = Token::Identifier;

    for (const keyword_item& i: keywords) {
        if (strncmp(i.keyword, str, i.size) == 0) {
            result = i.type;
            break;
        }
    }


    return result;
}

void Token::print(const Token& t) {

    for (const keyword_item& i: keywords) {
        if (i.type == t.type) {
            printf("Token - keyword: %s\n", i.keyword);
            return;
        }
    }

    for (const keyword_item& i: operators) {
        if (i.type == t.type) {
            printf("Token - op: %s\n", i.keyword);
            return;
        }
    }


    if (t.type == Token::Identifier) {
        printf("Token - identifier : %.*s\n", t.len, t.text);
        return;
    }

    if (t.type == Token::LiteralInteger) {
        printf("Token - integer: %lld\n", t.integer_value);
        return;
    } else if (t.type == Token::LiteralFloat) {
        printf("Token - float: %g\n", t.float_value);
        return;
    }


    // other literal
    for (const keyword_item& i: literals) {
        if (i.type == t.type) {
            printf("Token - Literal %s: %.*s\n", i.keyword, t.len, t.text);
            return;
        }
    }


    if (t.type == Error) {
        printf("Token - Error\n");
        return;
    } else if (t.type == Eof) {
        printf("Token - Eof\n");
        return;
    } else {
        if (t.type < 256) { // single char token
            printf("Token - single char: %c\n", t.type);
            return;
        }
        // Print unhandled token here
    }
    // @TODO print operators

    fprintf(stderr, "print_token error, unkown token: %d\n", t.type);
    return;
}
