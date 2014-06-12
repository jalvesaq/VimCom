# VimCom.plus

This is the development version of the R package "VimCom", which runs a server
in R to receive messages from the [Vim-R-plugin]. This package has support for
both [Vim] and [Neovim].

Note that you only need this version of VimCom if you are using the
development version of [Vim-R-plugin]. If you are using a [released version of
Vim-R-plugin], you will also need a [released version of vimcom.plus].

## How to install

To build the support for Vim's 'clientserver' on Unix systems (such as Linux
and Mac OS X), the X11 header libraries must be installed. Please access the
[official vimcom.plus webpage] for details.

The easiest way to install the package is to use the [devtools] package.

```s
library(devtools)
install_github('jalvesaq/VimCom')
```

To manually download and install VimCom, do the following in a terminal
emulator:

```sh
git clone https://github.com/jalvesaq/VimCom.git
R CMD INSTALL VimCom
```

The communication with Neovim requires neither X11 nor Windows. Hence, if you
will use the package only with Neovim, you do not need the support for Vim's
'clientserver' feature, which is disabled if the package is installed with the
command below:

```sh
R CMD INSTALL --configure-args='--disable-clientserver' VimCom
```

Of course, without support for the 'clientserver' feature you also do not need
to install the X11 headers on Unix.


## vimcom *versus* vimcom.plus

This package was originally developed with the name "vimcom" (Vim
Communication). However, I had to remove a few lines of code to keep the
package on CRAN, and opted to call the complete version "vimcom.plus". The
name change was done only in the DESCRIPTION file: the repository still has
the old name.

[Vim-R-plugin]: https://github.com/jcfaria/Vim-R-plugin
[Vim]: http://www.vim.org
[Neovim]: http://neovim.org
[official vimcom.plus webpage]: http://www.lepem.ufc.br/jaa/vimcom.plus.html
[released version of Vim-R-plugin]: http://www.vim.org/scripts/script.php?script_id=2628
[released version of vimcom.plus]: http://www.lepem.ufc.br/jaa/vimcom.plus.html
[devtools]: http://cran.r-project.org/web/packages/devtools/index.html
