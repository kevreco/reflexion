#include "re_compiler.h"

#include <iostream>

#include "re_lexer.h"
#include "re_ast.h"

#include "re_context.h"
#include "re_manager.h"

//-------------------------------------------------------------------------
// Compiler
//-------------------------------------------------------------------------

bool Compiler::compile_file(const char* filepath) {
    printf("Compiler::compile_file [%s]\n", filepath);

    // Compiler only accept absolute file path, for debug reason it's more relevant to know the absolute path than the relative one

    if (!g_is_absolute_path(filepath)) {
        fprintf(stderr, "Compiler::compile_file error: filepath must be a absolute path\n");
        return false;
    }

    // [Windows] "rb" is need to avoid some translation/encoding platform dependant stuff
    // \r\n are handled in the lexer
    FILE* src_file = fopen(filepath, "rb");

    if (!src_file) {
        fprintf(stderr, "Compiler::compileFile can't open '%s'\n", filepath);
        return false;
    }

    fseek(src_file, 0, SEEK_END);
    long fsize = ftell(src_file);
    fseek(src_file, 0, SEEK_SET);

    if (!fsize) {
        fclose(src_file);
        fprintf(stdout, "Warning: Compiler::compileFile empty file '%s'\n", filepath);
        return true;
    }

    char *content = (char*)malloc(fsize + 1);
    fread(content, fsize, 1, src_file);
    fclose(src_file);
    content[fsize] = 0;

    if (!content) {
        fprintf(stderr, "Compiler::compileFile can't read file content'%s'\n", filepath);
        return false;
    }

    {

        // use a non null terminated string
        const utf8_string& str_with_len = manager.register_path_in_pool(filepath);

        std::cout << str_with_len << std::endl;

        // @TODO check if already parsed and remove/replace std::string
        
        const std::string s(filepath);
        //auto found = ctx.compiledFilePaths.find(s);
        //if (found != ctx.compiledFilePaths.end()) {
        //    printf("Compiler log, file has already been '%s' compiled!", filepath);
        //    return true;
        //}

        // Initialize parser


        Parser parser_(manager, content, std::strlen(content), str_with_len);

        TopLevel* top_level = 0;

        top_level = parser_.parse_top_level();


        // Parse top level
        if (!top_level) {


            return false;
        } else {

            top_level->set_path(&str_with_len);
            manager.register_parsed_file(&str_with_len);

        }

        // register file as compiled
        
        //ctx.compiledFilePaths.insert(std::string(filepath));
        //ctx.topLevels.push_back(&top_level);



        if (!top_level->import_items.empty()) {
            // @TODO use a names const expr value
            char fullpath[4096];

            for (DirectiveImport* import: top_level->import_items) {

                if (!compile_file(fullpath)) {
                    fprintf(stderr, "Compiler Error : Can't compile import file:'%.*s'!\n",
                            import->absolute_path.size(),
                            import->absolute_path.data());
                    return false;
                }
            }
        }


    }

    free(content);

    return true;

} // compile_file
