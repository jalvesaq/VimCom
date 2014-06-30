# VimCom.plus

This is the development version of the R package "VimCom", which runs a server
in R to receive messages from the [Vim-R-plugin]. This package has support for
both [Vim] and [Neovim].

Note that you only need this version of VimCom if you are using the
development version of [Vim-R-plugin]. If you are using a [released version of
Vim-R-plugin], you will also need a [released version of vimcom].

## How to install

To build the support for Vim's 'clientserver' on Unix systems (such as Linux
and Mac OS X), the X11 header libraries must be installed. Please access the
[official vimcom webpage] for details.

The easiest way to install vimcom is to use the [devtools] package.

```s
library(devtools)
install_github('jalvesaq/VimCom')
```

To manually download and install VimCom, do the following in a terminal
emulator:

```sh
git clone https://github.com/jalvesaq/VimCom.git
R CMD build VimCom
R CMD INSTALL vimcom_1.0-0.tar.gz
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
name change was done only in the DESCRIPTION file: the repository had still
kept the old name. The decision to keep two version of the package proved to
be wrong because it caused confusion among users. Finally, on July 2014, I
decided to request the removal of vimcom package from CRAN, and rename the
"vimcom.plus" package to its original name: vimcom.

[Vim-R-plugin]: https://github.com/jcfaria/Vim-R-plugin
[Vim]: http://www.vim.org
[Neovim]: http://neovim.org
[official vimcom webpage]: http://www.lepem.ufc.br/jaa/vimcom.html
[released version of Vim-R-plugin]: http://www.vim.org/scripts/script.php?script_id=2628
[released version of vimcom]: http://www.lepem.ufc.br/jaa/vimcom.html
[devtools]: http://cran.r-project.org/web/packages/devtools/index.html
