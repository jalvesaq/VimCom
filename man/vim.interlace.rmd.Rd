\name{vim.interlace.rmd}
\alias{vim.interlace.rmd}
\title{Convert a rmd file to PDF}
\description{
  Run the \pkg{knitr} function \code{knit()} to convert a rmd file to 
  PDF. 
}
\usage{
vim.interlace.rmd(Rmdfile, view = TRUE, pdfquiet = FALSE, 
                  pandoc_args = "",  pdfout = "latex", ...)
}
\arguments{
  \item{Rmdfile}{The path to the Rmd file.}
  \item{view}{Logical value indicating whether to show the generated PDF document.}
  \item{pdfquiet}{Whether to suppress output of application opening the PDF.}
  \item{pandoc_args}{Arguments to pandoc.}
  \item{pdfout}{Type of output.}
  \item{\dots}{Further arguments to be passed to \code{knit()}.}
}
