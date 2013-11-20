\name{vim.pager}
\alias{vim.pager}
\title{Use Tmux to open R documentation in Vim}
\description{If R was started by the Vim-R-plugin and both Vim and R are
  inside the same Tmux session, then this function can be used as pager for R
  documentation. If you set \code{options(pager = vim.pager)}, R documentation
  will be shown in Vim.}
\usage{
vim.pager(files, header, title, delete.file)
}
\arguments{
  \item{files}{Ignored.}
  \item{header}{Ignored.}
  \item{title}{The name of topic to seek help for.}
  \item{delete.file}{Ignored.}
}
\author{Jakson Alves de Aquino <jalvesaq@gmail.com>}
