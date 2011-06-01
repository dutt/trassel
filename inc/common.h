#ifndef COMMON_H
#define COMMON_H

#include "typedefs.h"

#include <string>

#if defined(WIN32) || defined(_WIN32)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

# ifdef _DEBUG
#  include <assert.h>
# else
#  define assert(__x) /* __x */
#endif

#else
# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif
# include <cstdlib> // for rand()
namespace std
{
    typedef std::basic_string<wchar_t> wstring;
}
# ifdef DEBUG
#  include <cassert>
# else
#  define assert(__x) /* __x */
# endif
# ifndef min
template<typename T>
static inline T min(T x, T y) { return x < y ? x : y; }
# endif
# ifndef max
template<typename T>
static inline T max(T x, T y) { return x > y ? x : y; }
# endif
#endif

#endif /* COMMON_H */
