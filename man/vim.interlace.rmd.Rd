\name{vim.interlace.rmd}
\alias{vim.interlace.rmd}
\title{Convert a rmd file to PDF}
\description{
  Run the \pkg{knitr} function \code{knit()} to convert a rmd file to 
  PDF. 
}
\usage{
vim.interlace.rmd(Rmdfile, rmddir, view = TRUE,
                  pandoc_args = "",  pdfout = "latex", ...)
}
\arguments{
  \item{Rmdfile}{The Rmd file to be processed.}
  \item{rmddir}{The directory of the Rnoweb file.}
  \item{view}{Logical value indicating whether to show the generated PDF document.}
  \item{pandoc_args}{Arguments to pandoc.}
  \item{pdfout}{Type of output.}
  \item{\dots}{Further arguments to be passed to \code{knit()}.}
}
