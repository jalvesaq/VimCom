/* The code in this file was adapted from version 1.1 of vim-remote, written
 * by Yukihiro Nakadaira based on Vim source code. The vim-remote library is
 * available at http://www.vim.org/scripts/script.php?script_id=3482 */

#ifndef VIMREMOTE_H_INCLUDED
#define VIMREMOTE_H_INCLUDED

#include <stddef.h>

int vimremote_init();
int vimremote_uninit();
int vimremote_remoteexpr(const char *servername, const char *expr, char **result);

#endif
