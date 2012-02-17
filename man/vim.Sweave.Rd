\name{vim.Sweave}
\alias{vim.Sweave}
\title{Run Sweave and pdflatex in sequence}
\description{
  Run the R function Sweave and, then, the application pdflatex.
}
\usage{
vim.Sweave(rnowebfile, latexcmd = "pdflatex", bibtex = FALSE, ...)
}
\arguments{
  \item{rnowebfile}{The path to the Rnoweb file.}
  \item{latexcmd}{The command to run on the generated .tex file.}
  \item{bibtex}{Whether to run bibtex.}
  \item{\dots}{Further arguments to be passed to \code{Sweave}.}
}
