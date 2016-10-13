#include "re_fileinfo.h"

#include "stdio.h" // printf
#include "string.h" // strlen


//-------------------------------------------------------------------------
// Forward Private Function Declarations
//-------------------------------------------------------------------------

namespace {

char * get_end_basename(const char* filename);

char * get_filename(const char* fullpath);

} // anonymous namespace


//-------------------------------------------------------------------------
// FileInfo
//-------------------------------------------------------------------------


FileInfo::FileInfo(const char* fpath) {

    filepath = 0;

    fullpath.ptr = 0;
    fullpath.len = 0;

    filename.ptr = 0;
    filename.len = 0;

    basename.ptr = 0;
    basename.len = 0;

    relative_dir.ptr = 0;
    relative_dir.len = 0;

    if (!fpath) {
        return;
    }


    filepath = strdup(fpath);

    fullpath.ptr = filepath;
    fullpath.len = strlen(filepath);


    filename.ptr = get_filename(filepath);
    filename.len = strlen(filename.ptr);

    const char* cursor = filename.ptr;

    while (*cursor && *cursor != '.') {
        cursor++;
    }

    basename.ptr = filename.ptr;
    basename.len = cursor - filename.ptr;

    relative_dir.ptr = fullpath.ptr;
    relative_dir.len = (fullpath.len - filename.len);

    printf("======= FileInfo =======\n");
    printf("fullpath:      %.*s\n", fullpath.len, fullpath.ptr);
    printf("relative_dir:  %.*s\n", relative_dir.len, relative_dir.ptr);
    printf("filename:      %.*s\n", filename.len, filename.ptr);
    printf("basename:      %.*s\n", basename.len, basename.ptr);
    printf("======= /FileInfo =======\n");
}

FileInfo::~FileInfo() {

}


void FileInfo::print_path() const {
    printf("%.*s\n", fullpath.len, fullpath.ptr);
}

//-------------------------------------------------------------------------
// Private Function Definition
//-------------------------------------------------------------------------

namespace {

inline char * get_end_basename(const char* filename)
{
    const char *base = filename;

    while (*filename)
    {
        filename++;
        if (*filename == '.')
        {
            base = filename;
        }
    }
    return (char *) base;
}

inline char * get_filename(const char* fullpath)
{
    const char *base = fullpath;

    while (*fullpath)
    {
        if (*fullpath++ == '/')
        {
            base = fullpath;
        }
    }
    return (char *) base;
}

} // anonymous namespace
