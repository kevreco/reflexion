#ifndef RE_CONVERTERC_H
#define RE_CONVERTERC_H

#include "re_ast.h"

#include <fstream> // std::ofstream

// @TODO remove all std::iostream stuff and replace it by standard c io

/*

=== Compiler limits: ===

- Identifier name length max:
Microsoft C++: 2048 characters [1]
Intel C++: 2048 characters [2]
g++: No limit



[1] https://msdn.microsoft.com/en-us/library/565w213d.aspx
The resultant name, including the type information, cannot be longer than 2048 characters.

[2] http://www2.units.it/divisioneisi/ci/tartaglia/intel/cce/cref_cls.pdf
Number of identifiers in a single block 2048


*/


using namespace Ast;

struct Manager;

struct ConverterC {

    static const int MaxFilenameBufferSize = 257;
    static const int MaxIdentifierNameLength = 2048;

    Manager& manager;

    FILE* of = 0;

    // Output file



    bool is_top_level;

    // Nested function/struct stuff

    char nested_func_name_buffer[MaxIdentifierNameLength];
    int current_nested_func_id;

    // Indent stuff

    static const char* indent_txt;
    int indent_lvl;


    // Functions
    
    ConverterC(Manager& mgr);

    void compile_to_file(const char* outpout_filename);



}; // ConverterC



#endif // RE_CONVERTERC_H
