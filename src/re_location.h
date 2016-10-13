#ifndef RE_LOCATION_H
#define RE_LOCATION_H

#include "re_global.h"

struct Location {

    const utf8_string* filepath = 0;
    int row = -1, col = -1, pos = -1;

    // filepath of AST toplevel node
    //Location();

    // increment row number
    void incRow();
    // increment col number
    void incColumn();



}; // Location

#endif // RE_LOCATION_H
