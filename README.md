# VimCom

This is the development version of the R package "VimCom", which runs a server
in R to receive messages from the [Vim-R-plugin]. This package has support
only for [Vim]. If you use [Neovim], please, look at [nvimcom].

Note that you only need this version of VimCom if you are using the
development version of [Vim-R-plugin]. If you are using a [released version of
Vim-R-plugin], you will also need a [released version of vimcom].

## How to install

The easiest way to install vimcom is to use the [devtools] package.

```s
devtools::install_github("jalvesaq/VimCom")
```

To manually download and install VimCom, do the following in a terminal
emulator:

```sh
git clone https://github.com/jalvesaq/VimCom.git
```

And, then, do in R:

```s
install.packages("path/to/VimCom", type = "source", repos = NULL)
```

On Mac OS X, by default, vimcom will be compiled without support for Vim's
clientserver feature, which depends on the X Server. MacVim's clientserver
feature depends on Cocoa. On any other Unix system, the package is built with
support for X Server and, consequently, the X11 headers must be installed.
Please access the [official vimcom webpage] for details.

To enable the support for Vim's 'clientserver' based on the X Server on Mac OS
X, do the following:

```s
install.packages("path/to/VimCom", type = "source", repos = NULL,
                  configure.args = "--enable-clientserver")
```


## vimcom *versus* vimcom.plus

This package was originally developed with the name "vimcom" (Vim
Communication). However, I had to remove a few lines of code to keep the
package on CRAN, and opted to call the complete version "vimcom.plus". The
name change was done only in the DESCRIPTION file: the repository had still
kept the old name. The decision to keep two versions of the package proved to
be wrong because it caused confusion among users. Finally, on July 2014, I
decided to request the removal of vimcom package from CRAN, and changed its
name back to "vimcom".

[Vim-R-plugin]: https://github.com/jcfaria/Vim-R-plugin
[Vim]: http://www.vim.org
[Neovim]: http://neovim.org
[official vimcom webpage]: http://www.lepem.ufc.br/jaa/vimcom.html
[released version of Vim-R-plugin]: http://www.vim.org/scripts/script.php?script_id=2628
[released version of vimcom]: http://www.lepem.ufc.br/jaa/vimcom.html
[devtools]: http://cran.r-project.org/web/packages/devtools/index.html
[nvimcom]: https://github.com/jalvesaq/nvimcom
