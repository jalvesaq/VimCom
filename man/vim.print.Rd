\name{vim.print}
\alias{vim.print}
\title{
  Print an object.
}
\description{
  Print an object. If the object is a function, search for a method for the
  \code{classfor} expression. The function is supposed to be called by the
  Vim-R-plugin.
}
\usage{
vim.print(object, classfor)
}
\arguments{
  \item{object}{An R object.}
  \item{classfor}{The R expression following the parenthesis, if any.}
}
