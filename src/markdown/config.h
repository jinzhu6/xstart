#ifndef __AC_MARKDOWN_D
#define __AC_MARKDOWN_D 1

#define OS_DARWIN 0
#define USE_DISCOUNT_DL 1
#define while(x) while( (x) != 0 )
#define if(x) if( (x) != 0 )
#define DWORD unsigned int
#define WORD unsigned short
#define BYTE unsigned char
#define HAVE_PWD_H 0
#define HAVE_GETPWUID 0
#define HAVE_SRANDOM 0
#define INITRNG(x) srand((unsigned int)x)
#define HAVE_BZERO 1
#define HAVE_RANDOM 0
#define COINTOSS() (rand()&1)
#define HAVE_STRCASECMP 0
#define HAVE_STRNCASECMP 0
#define HAVE_FCHDIR 1
#define TABSTOP 4
//#define PATH_FIND "/usr/bin/find"
//#define PATH_SED "/usr/bin/sed"

#ifdef _WIN32
#define strnicmp _strnicmp
#define strncasecmp strnicmp
#define inline _inline
#else
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
#endif

#endif/* __AC_MARKDOWN_D */
