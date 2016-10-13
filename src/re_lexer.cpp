#include "re_lexer.h"

#include "assert.h"

//-------------------------------------------------------------------------
// Forward Private Function Declarations
//-------------------------------------------------------------------------

namespace {

bool is_end_line(char c);
bool is_whitespace(char c);
bool is_alpha(char c);
bool is_alphanum(char c);
bool is_utf8(char c);
bool is_eof(Lexer& l);

// Go to next char
void consume_one(Lexer& l);

// Equal to next char
bool next_equal(Lexer& l, char c);

// Equal to next next char
bool next_next_equal(Lexer& l, char c);

// Get next char
char get_next(Lexer& l);


// Skip n char
void skipn(Lexer& l, unsigned n);

const Token& get_number(Lexer& l);

void skip_whitespace(Lexer& l);

void skip_whitespaces_and_comments(Lexer& l);

const Token& get_ascii_char_literal(Lexer& l);

} // anonymous namespace


//-------------------------------------------------------------------------
// Lexer
//-------------------------------------------------------------------------


const Token& Lexer::get_next_token() {
    
    Lexer& l = *this;

    skip_whitespaces_and_comments(l);

    if (is_eof(l)) {
        token.type = Token::Eof;
        return token;
    }

    switch(*cur) {



    // We handle identifier by default because they are many
    default: {

        if (is_alpha(*cur) || *cur == '_' || is_utf8(*cur)) {



            //token.reset();
            const char* start = cur;
            int n = 0;
            do {
                ++n;
                consume_one(l);

            } while((is_alphanum(*cur) || *cur == '_' || is_utf8(*cur)));

            token.set_str(start, n);

            //if ((t = KeywordCheck(start, n)) != Token::None) {
            token.type = Token::identifier_or_keyword(start);

            return token;
        } // end if IsAlpha(c()) || c() == '_' || IsUtf8(c())
        else {
            fprintf(stderr, "Lexer::get_next_token @TODO handle illegal char for identifier %d, %c\n", *cur, *cur);
        }
        break;
    } // end default case
    case ' ':
    case '\n':
    case '\r':
    case '\t':
    case '\f':
    case '\v': {
        assert(0); // this case should be handled by skiping whitespace
    }
    case '#':
    case '[':
    case ']':
    case '(':
    case ')':
    case '{':
    case '}':
    case ';':
    case ',': {
        goto single_char_token;
    }

        // Comparison or Assign
    case '=' : {

        if (next_equal(l,'=')) {
            token.type = Token::EqualEqual;
            token.set_str(TokenStrEqualEqual, sizeof(TokenStrEqualEqual));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'>')) {
            token.type = Token::Equal;
            token.set_str(TokenStrEqualArrow, sizeof(TokenStrEqualArrow));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }
    case '!' : {

        if (next_equal(l,'=')) {
            token.type = Token::NotEqual;
            token.set_str(TokenStrNotEqual, sizeof(TokenStrNotEqual));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '<' : {

        if (next_equal(l,'<')) {
            if (next_next_equal(l,'=')) {
                token.type = Token::LessLessEqual;
                token.set_str(TokenStrLessLessEqual, sizeof(TokenStrLessLessEqual));
                skipn(l,3);
                return token;
            }
            token.type = Token::LessLess;
            token.set_str(TokenStrLessLess, sizeof(TokenStrLessLess));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'=')) {
            token.type = Token::LessEqual;
            token.set_str(TokenStrLessEqual, sizeof(TokenStrLessEqual));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '>' : {

        if (next_equal(l,'>')) {
            if (next_next_equal(l,'=')) {
                token.type = Token::GreaterGreaterEqual;
                token.set_str(TokenStrGreaterGreaterEqual, sizeof(TokenStrGreaterGreaterEqual));
                skipn(l,3);
                return token;
            }
            token.type = Token::GreaterGreater;
            token.set_str(TokenStrGreaterGreater, sizeof(TokenStrGreaterGreater));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'=')) {
            token.type = Token::GreaterEqual;
            token.set_str(TokenStrGreaterEqual, sizeof(TokenStrGreaterEqual));
            skipn(l,2);
            return token;
        }

        goto single_char_token;
    }

        // Logical and bitwise
    case '&' : {

        if (next_equal(l,'&')) {
            token.type = Token::AmpAmp;
            token.set_str(TokenStrAmpAmp, sizeof(TokenStrAmpAmp));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'=')) {
            token.type = Token::AmpEqual;
            token.set_str(TokenStrAmpEqual, sizeof(TokenStrAmpEqual));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '|' : {

        if (next_equal(l,'|')) {
            token.type = Token::PipePipe;
            token.set_str(TokenStrPipePipe, sizeof(TokenStrPipePipe));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'=')) {
            token.type = Token::PipeEqual;
            token.set_str(TokenStrPipeEqual, sizeof(TokenStrPipeEqual));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '+' : {
        if (next_equal(l,'+')) {
            token.type = Token::PlusPlus;
            token.set_str(TokenStrPlusPlus, sizeof(TokenStrPlusPlus));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'=')) {
            token.type = Token::PlusEqual;
            token.set_str(TokenStrPlusEqual, sizeof(TokenStrPlusEqual));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '-' : {
        if (next_equal(l,'-')) {
            token.type = Token::MinusMinus;
            token.set_str(TokenStrMinusMinus, sizeof(TokenStrMinusMinus));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'=')) {
            token.type = Token::MinusEqual;
            token.set_str(TokenStrMinusEqual, sizeof(TokenStrMinusEqual));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'>')) {
            token.type = Token::Arrow;
            token.set_str(TokenStrArrow, sizeof(TokenStrArrow));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '*' : {
        if (next_equal(l,'=')) {
            token.type = Token::MulEqual;
            token.set_str(TokenStrMulEqual, sizeof(TokenStrMulEqual));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '/' : {
        if (next_equal(l,'=')) {
            token.type = Token::DivEqual;
            token.set_str(TokenStrDivEqual, sizeof(TokenStrDivEqual));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'/')) {
            assert(0); // this case should be handeled at the begining of the loop
        } else if (next_equal(l,'*')) {
            assert(0); // this case should be handeled at the begining of the loop
        } else {

            goto single_char_token;
        }
    }

    case '%' : {
        if (next_equal(l,'=')) {
            token.type = Token::ModEqual;
            token.set_str(TokenStrModEqual, sizeof(TokenStrModEqual));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case '^' : {
        if (next_equal(l,'=')) {
            token.type = Token::CaretEqual;
            token.set_str(TokenStrCaretEqual, sizeof(TokenStrCaretEqual));
            skipn(l,2);
            return token;
        }

        goto single_char_token;
    }

    case '.' : {
        if (next_equal(l,'.')) {
            if (next_next_equal(l,'.'))
                token.type = Token::DotDotDot;
            token.set_str(TokenStrDotDotDot, sizeof(TokenStrDotDotDot));
            skipn(l,3);
            return token;
        } else {
            token.type = Token::DotDot;
            token.set_str(TokenStrDotDot, sizeof(TokenStrDotDot));
            skipn(l,2);
            return token;
        }
        goto single_char_token;
    }

    case ':' : {
        if (next_equal(l,':')) {
            token.type = Token::ColonColon;
            token.set_str(TokenStrDoubleColon, sizeof(TokenStrDoubleColon));
            skipn(l,2);
            return token;
        } else if (next_equal(l,'=')) {
            token.type = Token::ColonEqual;
            token.set_str(TokenStrColonEqual, sizeof(TokenStrColonEqual));
            skipn(l,2);
            return token;
        }

        goto single_char_token;
    }

    case '"' : {
        return get_ascii_char_literal(l);
    }

    case '\0': {
        token.type = Token::Eof;
        return token;

    } // end case '\0'

        // Label to handle single char token, ugly to put that here ? :(
single_char_token: {
        token.type = Token::Type(*cur);
        consume_one(l);
        return token;
    }

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
        // @TODO parse octal number
        return get_number(l);

        break;
    }

    } // end switch


    token.type = Token::Eof;
    return token;

} // get_next_token


void Lexer::goto_next_token() {
    get_next_token();
} // goto_next_token()

//-------------------------------------------------------------------------
// Private Function Definition
//-------------------------------------------------------------------------

namespace {

inline bool is_end_line(char c) {
    return (c == '\n' || c == '\r');
}

inline bool is_whitespace(char c) {
    return (c == ' ' || c == '\n'
            || c == '\t' || c == '\r'
            || c == '\f' || c == '\v');
}

inline bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z');
}

inline bool is_alphanum(char c) {
    return (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9');
}

inline bool is_utf8(char c) {
    // @TODO do it in a better way
    return (unsigned char) c >= 128;
}

inline bool is_eof(Lexer& l) {
    return l.cur == l.end || *l.cur == '\0';
} // eof

// Go to next char
inline void consume_one(Lexer& l) {

    if (*l.cur == '\n' || *l.cur == '\r') {

        // \r\n is considered as "one" but two characters are skipped
        // thx to stb_c_lexer for this nice 'trick'
        l.cur += (l.cur[0]+l.cur[1] == '\r'+'\n' ? 2 : 1); // skip newline
        l.location.incRow(); // row counting
    } else {

        l.cur++;
        l.location.incColumn(); // column couting
    }

}

// Equal to next char
inline bool next_equal(Lexer& l, char c) {
    return *(l.cur + 1) == c;
}
// Equal to next next char
inline bool next_next_equal(Lexer& l, char c) {
    return *(l.cur + 2) == c;
}


// Get next char
inline char get_next(Lexer& l) {

    consume_one(l);

    return *l.cur;
}

// Skip n char
inline void skipn(Lexer& l, unsigned n) {

    for (unsigned i = 0; i < n; ++i) {
        consume_one(l);
    }

    if (l.cur >= l.end) {
        fprintf(stderr, "skipn, buffer overflow\n");
    }
}

inline const Token& get_number(Lexer& l) {

    const char* start = l.cur;

    do {
        switch (*l.cur) {
        case '0': {
            // try parse hex integer
            if (next_equal(l, 'x') || next_equal(l, 'X')) {
                skipn(l, 2); // skip 0 and 'x'
                const char* end = 0;//l.cur;
                long integer = strtol((char *) l.cur, (char **) &end, 16);

                if (l.cur == end) {
                    // @TODO handle better error;
                    fprintf(stderr, "Error hexadecimal value\n");

                    l.token.type = Token::Eof;

                    return l.token;
                }
                l.token.type = Token::LiteralInteger;
                l.token.integer_value = integer;

                // @FIXME is it safe to do this ?
                l.cur = end;

                return l.token;
            }
        }



        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':

            // Decimal floats
        {
            //const char* cursor = l.cur;
            while (!is_eof(l) && is_alpha(*l.cur)) {
                consume_one(l);
            }

            if (!is_eof(l)) {
                if (*l.cur == '.' || *l.cur == 'e' || *l.cur == 'E') {

                    const char* end = 0;

                    l.token.type = Token::LiteralInteger;
                    l.token.float_value = strtod(start, (char**) &end);


                    // @FIXME is it safe to do this ?
                    l.cur = end;

                    return l.token;

                }
            }


        }

            // Decimal Ints
        {

            const char *end = 0;
            l.token.type = Token::LiteralInteger;
            l.token.integer_value = strtol((char *) start, (char **) &end, 10);

            // @FIXME is it safe to do this ?
            l.cur = end;

            return l.token;
        }

        } // end switch

    } while(is_alphanum(*l.cur));

    // Error
    fprintf(stderr, "Internal error: this kind of number is not handled yet ?\n");

    l.token.type = Token::Eof;


    Token::print(l.token);

    return l.token;
} // get_number

inline void skip_whitespace(Lexer& l) {

    consume_one(l);
    while(is_whitespace(*l.cur)) {
        consume_one(l);
    }

} // skip_whitespace

inline void skip_whitespaces_and_comments(Lexer& l) {

    for(;;) {

        // Skip Whitespaces
        if (is_whitespace(*l.cur)) {

            skip_whitespace(l);
            continue;
        }

        // Skip comments
        if (*l.cur == '/') {
            // Skip c comment
            if (next_equal(l, '*')) {
                consume_one(l); // Skip '/'
                consume_one(l); // Skip '*'
                while (!(*l.cur == '*' && next_equal(l, '/')) && !is_eof(l)) {
                    consume_one(l);
                }


                if (*l.cur == '*' && next_equal(l, '/')) {
                    consume_one(l); // Skip '*' and set the cursor on the char after
                    consume_one(l); // Skip '/' and set the cursor on the char after

                }

                continue;

            } else if (next_equal(l, '/')) { // Skip cpp comment

                while (!is_eof(l) && !is_end_line(*l.cur)) {
                    consume_one(l);
                }
                // We are on the the end of line char, continue will consume this char and
                // potential whitespaces could be skipped
                continue;
            }
        }
        // If we reach here there is no more space or comment
        break;
    } // end for
}

inline const Token& get_ascii_char_literal(Lexer& l) {

    assert(*l.cur == '"');

    consume_one(l); // ignore the quote char


    const char* start = l.cur;
    int n = 0;
    while(*l.cur && *l.cur != '"' && *l.cur != '\n') {
        consume_one(l);
        ++n;
    }

    if (*l.cur != '"') {
        fprintf(stderr, "Lexer Error: literal string is not well terminated with '\"'\n");
        l.token.type = Token::Error;
        return l.token;
    } else {
        consume_one(l);
    }

    l.token.type = Token::AsciiString;
    if (n == 0) {
        l.token.clear();
    } else {
        l.token.set_str(start, n);
    }
    return l.token;
} // get_ascii_char_literal

} // anonymous namespace


