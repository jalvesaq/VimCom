\name{vim.interlace.rnoweb}
\alias{vim.interlace.rnoweb}
\title{Run either Sweave or knit and, then, pdflatex on a Rnoweb file}
\description{
  Run the R function Sweave() or knit() and, then, the application pdflatex.
}
\usage{
vim.interlace.rnoweb(rnowebfile, latexcmd = "pdflatex", bibtex = FALSE,
           knit = FALSE, view = TRUE, quiet = TRUE,
           pdfquiet = FALSE, ...)
}
\arguments{
  \item{rnowebfile}{The path to the Rnoweb file.}
  \item{latexcmd}{The command to run on the generated .tex file.}
  \item{bibtex}{Whether to run bibtex.}
  \item{knit}{Whether to use knitr instead of Sweave.}
  \item{view}{Logical value indicating whether to show the generated PDF document.}
  \item{quiet}{Whether to pass \code{quiet = TRUE} to \link[tools]{texi2pdf} (Windows only).}
  \item{pdfquiet}{Whether to suppress output of application opening the PDF.}
  \item{\dots}{Further arguments to be passed to \code{Sweave}.}
}
