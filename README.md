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

[Vim-R-plugin]: https://github.com/jcfaria/Vim-R-plugin
[Vim]: http://www.vim.org
[Neovim]: http://neovim.org
[released version of Vim-R-plugin]: http://www.vim.org/scripts/script.php?script_id=2628
[released version of vimcom]: https://github.com/jalvesaq/VimCom/releases
[devtools]: http://cran.r-project.org/web/packages/devtools/index.html
[nvimcom]: https://github.com/jalvesaq/nvimcom
