#ifndef RE_COMPILER_H
#define RE_COMPILER_H

#include "re_parser.h"
#include "re_context.h"

struct Manager;

struct Compiler {

    Compiler(Manager& mgr) :
        manager(mgr)
    {

    }

    bool compile_file(const char* filepath);

    Manager& manager;

}; // Compiler

#endif // RE_COMPILER_H
