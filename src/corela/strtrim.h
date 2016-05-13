#ifndef _STRTRIM_H_
#define _STRTRIM_H_

#include <string>

enum strtrim_mode_t {
    STRLIB_MODE_ALL       = 0, 
    STRLIB_MODE_RIGHT     = 0x01, 
    STRLIB_MODE_LEFT      = 0x02, 
    STRLIB_MODE_BOTH      = 0x03
};

char *strcpytrim(char *d, const char *s, int mode, char *delim);

inline char *strtriml(char *d, const char *s) { return strcpytrim(d, s, STRLIB_MODE_LEFT, 0); }
inline char *strtrimr(char *d, const char *s) { return strcpytrim(d, s, STRLIB_MODE_RIGHT, 0); }
inline char *strtrim(char *d, const char *s) { return strcpytrim(d, s, STRLIB_MODE_BOTH, 0); }
inline char *strkill(char *d, const char *s) { return strcpytrim(d, s, STRLIB_MODE_ALL, 0); }

inline char *triml(char *s) { return strcpytrim(s, s, STRLIB_MODE_LEFT, 0); }
inline char *trimr(char *s) { return strcpytrim(s, s, STRLIB_MODE_RIGHT, 0); }
inline char *trim(char *s) { return strcpytrim(s, s, STRLIB_MODE_BOTH, 0); }
inline char *kill(char *s) { return strcpytrim(s, s, STRLIB_MODE_ALL, 0); }

std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);

#endif
