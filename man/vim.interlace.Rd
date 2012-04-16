\name{vim.interlace}
\alias{vim.interlace}
\title{Run interlace and pdflatex in sequence}
\description{
  Run the R function Sweave() or knit() and, then, the application pdflatex.
}
\usage{
vim.interlace(rnowebfile, latexcmd = "pdflatex", bibtex = FALSE,
           knit = FALSE, ...)
}
\arguments{
  \item{rnowebfile}{The path to the Rnoweb file.}
  \item{latexcmd}{The command to run on the generated .tex file.}
  \item{bibtex}{Whether to run bibtex.}
  \item{knit}{Whether to use knitr instead of Sweave.}
  \item{\dots}{Further arguments to be passed to \code{Sweave}.}
}
