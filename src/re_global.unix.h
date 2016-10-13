#ifndef RE_GENERAL_UNIX_H
#define RE_GENERAL_UNIX_H

bool g_is_absolute_path(const char* path)
{

    int len = strlen(path);

    if (len == 0) {
        return false;
    }

    return (path[0] == '/');
}


#endif // RE_GENERAL_H
