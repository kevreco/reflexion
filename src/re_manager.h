#ifndef RE_MANAGER_H
#define RE_MANAGER_H

//#include <unordered_map> // hash_table
#include <set>

#include <re_global.h>
#include "re_location.h"

#include "re_ast.h"

using namespace Ast;


// To handle errors and warnings
struct Message {

    static const int MaxLength = 1024;
    Location location;
    char str[MaxLength] = {0};

};


struct Manager
{

    // Global scope
    Block global_scope;

    std::set<utf8_string> paths_pool;
    std::set<const utf8_string*> register_parsed_files;

    // @TODO use stack ?
    darray<Message> warnings;
    darray<Message> errors;

    std::set<const utf8_string*> ident_string_pool;

    // Sementic Analysis

    LiteralDef* type_def_int    = 0;
    LiteralDef* type_def_float  = 0;
    LiteralDef* type_def_number = 0;
    LiteralDef* type_def_string = 0;
    LiteralDef* type_def_true   = 0;
    LiteralDef* type_def_false  = 0;
    LiteralDef* type_def_null   = 0;
    //LiteralDef* type_def_bool   = 0;
    //TypeDefinition* type_def_void   = 0;

    darray<Declaration*> unresolved_declarations;
    darray<Declaration*> resolved_declarations;



    Manager();

    const utf8_string& register_path_in_pool(const char* path);
    void register_parsed_file(const utf8_string* str);
    //void log_error(const char* tag, const char* format, ...);
    //void log_error(Ast::Expression* expr, const char* format, ...);
    void log_error(const Location& loc, const char* format, ...);
    void log_warning(const Location& loc, const char* format, ...);


    void display_messages();

   //void infer_type_from_binary(Binary* binary);


    const utf8_string* get_unique_ident_str_ptr(const utf8_string& ident);

private:


}; // struct Manager


#endif // RE_MANAGER_H
