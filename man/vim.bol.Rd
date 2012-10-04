\name{vim.bol}
\alias{vim.bol}
\title{
Build omnilist for Vim.
}
\description{
Build the omnils file for Vim. The function is supposed to be
called by the user through Vim-R-plugin (command :RUpdateObjList).
}
\usage{
vim.bol(omnilist, packlist, allnames = FALSE)
}
\arguments{
  \item{omnilist}{Path to the omnilist file.}
  \item{packlist}{Character vector listing the packages whose list of objects
    should be built.}
  \item{allnames}{Logical. Whether to include objects whose names start with a dot.}
}
\author{ Jakson Alves de Aquino <jalvesaq@gmail.com> }
