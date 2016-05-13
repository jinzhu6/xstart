#include "strtrim.h"

char *strcpytrim(char *d, const char *s, int mode, char *delim)
{
    char *o = d; // save orig
    char *e = 0; // end space ptr.
    char dtab[256] = {0};

    if (!s || !d) return 0;

    if (!delim) delim = " \t\r\n\f";
    while (*delim)
        dtab[*delim++] = 1;

    while ( (*d = *s++) != 0 ) {
        if (!dtab[*d]) { // Not a match char
            e = 0;       // Reset end pointer
        } else {
            if (!e) e = d;  // Found first match.

            if ( mode == STRLIB_MODE_ALL || ((mode != STRLIB_MODE_RIGHT) && (d == o)) )
                continue;
        }
        d++;
    }
    if (mode != STRLIB_MODE_LEFT && e) { // for everything but trim_left, delete trailing matches.
        *e = 0;
    }
    return o;
}



#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// trim from start
std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}
