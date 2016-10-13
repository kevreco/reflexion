#include "re_manager.h"

#include <cstdarg>
#include <cstdio>
#include <assert.h>

#include <iostream> // @TEMP Remove this

#include "re_parser.h" // AST_NEW


//-------------------------------------------------------------------------
// Forward Private Function Declarations
//-------------------------------------------------------------------------




Manager::Manager()
{

    //type_def_number = new LiteralDef;
    //type_def_string = new LiteralDef;
    //type_def_true   = new LiteralDef;;
    //type_def_false  = new LiteralDef;
    //type_def_null   = new LiteralDef;
    // type_def_bool   = 0;
    //type_def_void   = 0;
}

const utf8_string& Manager::register_path_in_pool(const char* path) {

    utf8_string absolute_path(path);

    auto i = paths_pool.find(absolute_path);

    std::pair<std::set<utf8_string>::iterator,bool> result(paths_pool.end(), false);

    if (i == paths_pool.end()) {

        result = paths_pool.insert(absolute_path);
    }

    return *result.first;
}

void Manager::register_parsed_file(const utf8_string* str) {

    auto i = register_parsed_files.find(str);

    if (i == register_parsed_files.end()) {

        register_parsed_files.insert(str);
    } else {
        fprintf(stderr, "parsed file has been register twice... something is wrong.\n");
    }
}



/*
void Manager::log_error_parsing(Ast::Expression* expr, const char* format, ...)
{
    va_list args;

    va_start(args, format);
    fprintf(stderr, "Parsing: ");
    vfprintf(stderr, format, args);
    va_end(args);

    // update error count

    ++error_count;
}
*/

void Manager::log_error(const Location& loc, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    // Calculate the needed size
    // From http://en.cppreference.com/w/cpp/io/c/fprintf
    // If buf_size is zero, nothing is written and buffer may be a null pointer
    // however the return value (number of bytes that would be written) is still calculated and returned.
    int size = std::vsnprintf(nullptr, 0, fmt, args);

    if (size > Message::MaxLength) {
        fprintf(stderr, "Manager::log_error: max length reached...\n");
        return;
    }

    // Build message
    Message msg;
    msg.location = loc;
    std::vsnprintf(&msg.str[0], size + 1, fmt, args);

    va_end(args);

    errors.push_back(msg);

    //std::cout <<  msg.str << std::endl;
}

void Manager::log_warning(const Location& loc, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    // Calculate the needed size
    // From http://en.cppreference.com/w/cpp/io/c/fprintf
    // If buf_size is zero, nothing is written and buffer may be a null pointer
    // however the return value (number of bytes that would be written) is still calculated and returned.
    int size = std::snprintf(nullptr, 0, fmt, args);

    if (size > Message::MaxLength) {
        fprintf(stderr, "Manager::log_warning: max length reached...\n");
        return;
    }

    // Build message
    Message msg;
    msg.location = loc;
    std::snprintf(&msg.str[0], size + 1, fmt, args);

    va_end(args);

    warnings.push_back(msg);

    //std::cout <<  msg.str << std::endl;
}

void Manager::display_messages() {

    if (!warnings.empty() || !errors.empty()) {

        fprintf(stderr, "\n");
        for( const Message& msg: warnings) {
            fprintf(stderr, "\twarning: %s", msg.str);
        }

        for( const Message& msg: errors) {
            fprintf(stderr, "%.*s\n(%d:%d) error : %s\n",
                    msg.location.filepath->size(), msg.location.filepath->data(),
                    msg.location.row, msg.location.col,
                    msg.str);
        }
        fprintf(stderr, "\n");

    }
}



const utf8_string* Manager::get_unique_ident_str_ptr(const utf8_string& ident_str) {

    // @TODO create custom hash_table, I don't want to deal with std::map/set iterator

    const utf8_string* result = 0;
    for(const utf8_string* it: ident_string_pool) {
        if (*it == ident_str) {
            result = it;
            break;
        }
    }

    if (!result) {
        result = new utf8_string(ident_str);
        ident_string_pool.insert(result);
    }


    return result;

}


