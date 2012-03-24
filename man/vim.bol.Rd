\name{vim.bol}
\alias{vim.bol}
\title{
Build omnilist for Vim.
}
\description{
Build the omniList file for Vim. The function is supposed to be
called by the user through Vim-R-plugin (command :RUpdateObjList).
}
\usage{
vim.bol(omnilist, what = "loaded", allnames = FALSE)
}
\arguments{
  \item{omnilist}{Path to the omnilist file.}
  \item{what}{Character. Whether to build only loaded packages or all installed packages.}
  \item{allnames}{Logical. Whether to include objects whose names start with a dot.}
}
\author{ Jakson Alves de Aquino <jalvesaq@gmail.com> }
