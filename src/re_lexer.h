#ifndef RE_LEXER_H
#define RE_LEXER_H

#include <cstring>

#include "stdio.h"


#include "re_token.h"
#include "re_location.h"

#include <iostream>
#include <assert.h>


// This lexer has been inspired by stb_c_lexer.h
// https://github.com/nothings/stb

// @TODO
// get_byte_literal();
// @TODO
// get_utf8_str_literal();

struct Lexer {
    
    const char* src;
    const char* end;
    const char* cur;

    int len;
    Token token;
    Location location;

    Lexer() {
        src = 0;
        end = 0;
        cur = 0;
        len = 0;
    }
    
    void set_source(const char* s, unsigned int l, const utf8_string* path) {

        assert(path);

        // @TODO make a function location.clear() ?
        location.row = 1;// = Location();
        location.col = 0;
        location.pos = 0;
        location.filepath = path;

        if (s && l) {
            src = s;
            end = s + l;
            cur = s;
            len = l;
        }

    }

    const Token& get_next_token();

    void goto_next_token();

    void expect_and_consume(char c);
};

#endif // RE_LEXER_H
