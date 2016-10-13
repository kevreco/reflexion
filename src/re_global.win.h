#ifndef RE_GENERAL_WIN_H
#define RE_GENERAL_WIN_H




#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include <iostream>

//namespace {

static void convert_str_to_wstr(const char* str, WCHAR* wstr)
{
    int size_needed;

    size_needed = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);

    if (size_needed > 0) {
        MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, size_needed);
    }
}

// Not used
/*static void convert_wchar_to_utf8_string(WCHAR* w_string, utf8_string& utf8_str)
{
    int size_needed;

    size_needed = WideCharToMultiByte(CP_ACP, 0, w_string, -1, NULL, 0, NULL, NULL);

    if (size_needed > 0) {
        utf8_str = utf8_string(size_needed, 0);

        WideCharToMultiByte(CP_ACP,0, w_string, -1, &utf8_str[0], size_needed, NULL, NULL);
    }

}
*/
static void convert_wstr_to_str(WCHAR* w_string, char* str)
{
    int size_needed;

    size_needed = WideCharToMultiByte(CP_ACP, 0, w_string, -1, NULL, 0, NULL, NULL);

    if (size_needed > 0) {

        WideCharToMultiByte(CP_ACP,0, w_string, -1, &str[0], size_needed, NULL, NULL);
    }

}


static void g_get_absolute_file_path(const char* filepath, char* abs_filepath) {

    static const int MAX_CHAR_SIZE = _MAX_DIR;

    DWORD  retval = 0;
    TCHAR  w_buffer[MAX_CHAR_SIZE] = TEXT("");

    LPWSTR* lppPart = {NULL};

    TCHAR w_returned_path[MAX_CHAR_SIZE + 1];

    convert_str_to_wstr(filepath, w_buffer);

    retval = GetFullPathNameW(w_buffer,
                              _MAX_DIR,
                              w_returned_path,
                              lppPart);

    if (retval == 0) {

        fprintf(stderr, "g_get_absolute_file_path error: GetFullPathName failed (%ld)\n", GetLastError());
        return;

    } else if (retval > PATH_MAX) {

        fprintf(stderr, "g_get_absolute_file_path error: relative name is too long\n");
        return;

    } else {
        convert_wstr_to_str(w_returned_path, abs_filepath);
    }
}

/*
static void g_get_absolute_file_path2(const char* filepath, utf8_string& absolute_path) {

    static const int MAX_CHAR_SIZE = _MAX_DIR;

    DWORD  retval = 0;
    TCHAR  w_buffer[MAX_CHAR_SIZE] = TEXT("");

    LPWSTR* lppPart = {NULL};

    TCHAR w_returned_path[MAX_CHAR_SIZE + 1];

    convert_str_to_wstr(filepath, w_buffer);

    retval = GetFullPathNameW(w_buffer,
                              _MAX_DIR,
                              w_returned_path,
                              lppPart);

    if (retval == 0) {

        fprintf(stderr, "GetFullPathName failed (%ld)\n", GetLastError());
        return;

    } else {

        convert_wchar_to_utf8_string(w_returned_path, absolute_path);

    }

}

*/

static bool g_is_absolute_path(const char* path) {

    int len = strlen(path);

    if (len == 0) {
        return false;
    }

    // Check drive C:
    int i = 0;
    if (isalpha(path[0]) && path[1] == ':') {
        i = 2;
    }
    //return path[i] != '/' && path[i] != '\\';

    return (path[i] == '/' || path[i] == '\\');

}

//} // end anonymous namespace

#endif // RE_GENERAL_WIN_H
