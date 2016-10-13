#ifndef RE_UNICODE_H
#define RE_UNICODE_H


#include "re_global.h"

typedef u32 unicode_codepoint;

void encode_utf8(const unicode_codepoint uc, std::string& dest);


void encode_utf8(const unicode_codepoint uc, std::string& dest)
{
    if (uc <= 0x7f)
        dest.push_back(static_cast<s8>(uc));
    else if (uc <= 0x7FF)
    {
        dest.push_back(0xC0| ((uc >> 6)& 0x1F));
        dest.push_back(0x80| (uc & 0x3F));
    }
    else if (uc <= 0xFFFF)
    {
        dest.push_back(0xE0| ((uc >> 12)& 0x0F));
        dest.push_back(0x80| ((uc >> 6) & 0x3F));
        dest.push_back(0x80| (uc & 0x3F));
    }
    else if (uc <= 0x10FFFF)
    {
        dest.push_back(0xF0| ((uc >> 18)& 0x07));
        dest.push_back(0x80| ((uc >> 12) & 0x3F));
        dest.push_back(0x80| ((uc >> 6) & 0x3F));
        dest.push_back(0x80| (uc & 0x3F));
    }
    else
        dest.push_back(static_cast<s8>(ErrorChar));

}

utf8_string decode_utf8() {



}

#endif // RE_UNICODE_H

