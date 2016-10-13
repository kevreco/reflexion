#include "re_location.h"

//Location::Location() : row(0), col(0), pos(0) {
    //printf("Location(), %d, %d\n", row, col);
//}


void Location::incRow() {
    ++row;
    col = 0;
    ++pos;
}

void Location::incColumn() {
    ++col;
    ++pos;
}

