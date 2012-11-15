/* The code in this file was adapted from version 1.1 of vim-remote, written
 * by Yukihiro Nakadaira based on Vim source code. The vim-remote library is
 * available at http://www.vim.org/scripts/script.php?script_id=3482 */

#ifndef VIMTHINGS_H_INCLUDED
#define VIMTHINGS_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#define alloc_clear(x) (char_u *)calloc((x), 1)
#define alloc(x) (char_u *)malloc((x))
#define mch_memmove(x, y, z) memmove((void *)(x), (void *)(y), (z))
#define vim_strsave(x) (char_u *)strdup((char *)(x))
#define STRLEN(x) strlen((char *)(x))
#define STRCAT(x, y) strcat((char *)(x), (char *)(y))
#define STRCPY(x, y) strcpy((char *)(x), (char *)(y))
#define STRCMP(x, y) strcmp((char *)(x), (char *)(y))

#if defined(_WIN32) || defined(_WIN64)

#define STRICMP(x, y) stricmp((char *)(x), (char *)(y))
#define STRNICMP(x, y, z) strnicmp((char *)(x), (char *)(y), (z))

#ifdef _WIN64
typedef unsigned __int64	long_u;
# define SCANF_HEX_LONG_U       "%Ix"
#else
# if !defined(_MSC_VER)	|| (_MSC_VER < 1300)
#  define __w64
# endif
typedef unsigned long __w64	long_u;
# define SCANF_HEX_LONG_U       "%lx"
#endif

#else

#define STRICMP(x, y) strcasecmp((char *)(x), (char *)(y))
#define STRNICMP(x, y, z) strncasecmp((char *)(x), (char *)(y), (z))

typedef unsigned long long_u;

#endif

#define OK 1
#define FAIL 0

#ifndef TRUE
# define FALSE 0
# define TRUE 1
#endif

#define NUL '\0'

typedef unsigned char char_u;
typedef unsigned short short_u;
typedef unsigned int int_u;

/*
 * Structure used for growing arrays.
 * This is used to store information that only grows, is deleted all at
 * once, and needs to be accessed by index.  See ga_clear() and ga_grow().
 */
typedef struct growarray
{
    int	    ga_len;		    /* current number of items used */
    int	    ga_maxlen;		    /* maximum number of items possible */
    int	    ga_itemsize;	    /* sizeof(item) */
    int	    ga_growsize;	    /* number of items to grow each time */
    void    *ga_data;		    /* pointer to the first item */
} garray_T;

#define GA_EMPTY    {0, 0, 0, 0, NULL}

void ga_clear(garray_T *gap);
void ga_init2(garray_T *gap, int itemsize, int growsize);
int ga_grow(garray_T *gap, int n);
void ga_concat(garray_T *gap, char_u *s);
void ga_append(garray_T *gap, int c);


#define vim_iswhite(x)	((x) == ' ' || (x) == '\t')

char_u *skipwhite(char_u *q);
int vim_isxdigit(int c);
int hex2nr(int c);

#endif
