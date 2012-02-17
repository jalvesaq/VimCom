\name{vim.help}
\alias{vim.help}
\title{Write R help content into a file to be read by Vim}
\description{This function is supposed to be called by the Vim-R-plugin.}
\usage{
vim.help(topic, w, classfor, package)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{topic}{The topic whose help is desired.}
  \item{w}{The width of the text. }
  \item{classfor}{The R expression following a parenthesis. This expression
    will be used to call the help for a function method (if available) instead
    of the default documentation help.}
  \item{package}{The package where the function is.}
}
\author{Jakson Alves de Aquino <jalvesaq@gmail.com>}
