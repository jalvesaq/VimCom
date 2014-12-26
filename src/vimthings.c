/* The code in this file was adapted from version 1.1 of vim-remote, written
 * by Yukihiro Nakadaira based on Vim source code. The vim-remote library is
 * available at http://www.vim.org/scripts/script.php?script_id=3482 */


#ifndef NO_X_CLIENTSERVER
#include "vimthings.h"

/************************************************************************
 * Functions for handling growing arrays.
 */

/*
 * Initialize a growing array.	Don't forget to set ga_itemsize and
 * ga_growsize!  Or use ga_init2().
 */
    void
ga_init(gap)
    garray_T *gap;
{
    gap->ga_data = NULL;
    gap->ga_maxlen = 0;
    gap->ga_len = 0;
}

    void
ga_init2(gap, itemsize, growsize)
    garray_T	*gap;
    int		itemsize;
    int		growsize;
{
    ga_init(gap);
    gap->ga_itemsize = itemsize;
    gap->ga_growsize = growsize;
}

/*
 * Clear an allocated growing array.
 */

    void
ga_clear(gap)
    garray_T *gap;
{
    free(gap->ga_data);
    ga_init(gap);
}


/*
 * Make room in growing array "gap" for at least "n" items.
 * Return FAIL for failure, OK otherwise.
 */
    int
ga_grow(gap, n)
    garray_T	*gap;
    int		n;
{
    size_t	len;
    char_u	*pp;

    if (gap->ga_maxlen - gap->ga_len < n)
    {
	if (n < gap->ga_growsize)
	    n = gap->ga_growsize;
	len = gap->ga_itemsize * (gap->ga_len + n);
	pp = alloc_clear((unsigned)len);
	if (pp == NULL)
	    return FAIL;
	gap->ga_maxlen = gap->ga_len + n;
	if (gap->ga_data != NULL)
	{
	    mch_memmove(pp, gap->ga_data,
				      (size_t)(gap->ga_itemsize * gap->ga_len));
	    free(gap->ga_data);
	}
	gap->ga_data = pp;
    }
    return OK;
}


/*
 * Concatenate a string to a growarray which contains characters.
 * Note: Does NOT copy the NUL at the end!
 */
    void
ga_concat(gap, s)
    garray_T	*gap;
    char_u	*s;
{
    int    len = (int)STRLEN(s);

    if (ga_grow(gap, len) == OK)
    {
	mch_memmove((char *)gap->ga_data + gap->ga_len, s, (size_t)len);
	gap->ga_len += len;
    }
}

/*
 * Append one byte to a growarray which contains bytes.
 */
    void
ga_append(gap, c)
    garray_T	*gap;
    int		c;
{
    if (ga_grow(gap, 1) == OK)
    {
	*((char *)gap->ga_data + gap->ga_len) = c;
	++gap->ga_len;
    }
}

/*
 * skipwhite: skip over ' ' and '\t'.
 */
    char_u *
skipwhite(q)
    char_u	*q;
{
    char_u	*p = q;

    while (vim_iswhite(*p)) /* skip to next non-white */
	++p;
    return p;
}


/*
 * Variant of isxdigit() that can handle characters > 0x100.
 * We don't use isxdigit() here, because on some systems it also considers
 * superscript 1 to be a digit.
 */
    int
vim_isxdigit(c)
    int		c;
{
    return (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'f')
	|| (c >= 'A' && c <= 'F');
}


/*
 * Return the value of a single hex character.
 * Only valid when the argument is '0' - '9', 'A' - 'F' or 'a' - 'f'.
 */
    int
hex2nr(c)
    int		c;
{
    if (c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
	return c - 'A' + 10;
    return c - '0';
}

#endif
