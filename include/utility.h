#ifndef UTILITY_H
#define UTILITY_H

#include <cstdio>

// color printing

#define RED             "\033[0;31m" 
#define LIGHT_RED       "\033[1;31m" 
#define GREEN           "\033[0;32m" 
#define LIGHT_GREEN     "\033[1;32m" 
#define YELLOW          "\033[0;33m" 
#define LIGHT_YELLOW    "\033[1;33m" 
#define BLUE            "\033[0;34m" 
#define LIGHT_BLUE      "\033[1;34m" 
#define PURPLE          "\033[0;35m" 
#define LIGHT_PURPLE    "\033[1;35m" 
#define CYAN            "\033[0;36m" 
#define LIGHT_CYAN      "\033[1;36m" 
#define NO_COLOR        "\033[m"


#ifdef NDEBUG

#define assertm(cond, ..)
#define debug(..)

#else

#define assert(cond) do { \
    if (!(cond)) {\
        fprintf (stderr, "\033[31m assertion failed in < %s > %s:%d \033[m\n", __FUNCTION__, __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)

#define assertm(cond, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "\033[31m assertion failed in < %s > %s:%d, \033[m", __FUNCTION__, __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
        abort(); \
    } \
} while(0)


#define debug(...) do { \
    fprintf(stderr, "\033[31m < %s > %s:%d, \033[m", __FUNCTION__, __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
} while(0)


#endif

#define NOT_IMPLEMENTED do { \
    fprintf(stderr, "\033[31m %s() in %s is not implemented \033[m \n", __FUNCTION__, __FILE__); \
    abort(); \
} while (0)

#endif

