# VimCom.plus

## How to install

This is the development version of the R package "VimCom".
The easiest way to install it is to use the [devtools] package.

```s
library(devtools)
install_github('jalvesaq/VimCom')
```

However, note that you only need the version of VimCom available in the
github repository if you are using the development version of Vim-R-plugin,
which is available at https://github.com/jcfaria/Vim-R-plugin.

## vimcom *versus* vimcom.plus

This package was originally developed with the name "vimcom" (Vim
Communication). However, I had to remove a few lines of code to keep the
package on CRAN, and opted to call the complete version as "vimcom.plus". The
name change was done only in the DESCRIPTION file: the repository still has
the old name.

[devtools]: http://cran.r-project.org/web/packages/devtools/index.html
