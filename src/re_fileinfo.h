#ifndef RE_FILEINFO_H
#define RE_FILEINFO_H



#include "stdio.h" // printf // debug

struct FileInfo {

    struct string {
        const char * ptr;
        unsigned int len;
    };

    //const char* file_full_path; // /my/dir/name.ext
    FileInfo(const char* full_path);
    ~FileInfo();

    const char* filepath; // /my/dir/
    string absolute_path; // C:/username/my/dirname
    string fullpath;      // /my/dir/name.ext
    string relative_dir;  // /my/dir/
    string filename;      // name.ext
    string basename;      // name


    // Debug function

    void print_path() const;

}; // FileInfo

#endif // RE_FILEINFO_H
