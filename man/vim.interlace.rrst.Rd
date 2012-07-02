\name{vim.interlace.rrst}
\alias{vim.interlace.rrst}
\title{Run knit2pdf to convert a Rrst file to to PDF using pdflatex or rst2pdf}
\description{
  Run the \link{knitr} function \link[knitr]{knit2pdf}() to convert a Rrst (reStructuredText) file to 
  PDF. If desired, the user can specify to use rst2pdf to compile to PDF if a 
  LaTeX installation is not present.
}
\usage{
vim.interlace.rrst(Rrstfile, compiler = NULL, view = TRUE,
                   pdfquiet = FALSE, ...)
}
\arguments{
  \item{Rrstfile}{The path to the Rrst file.}
  \item{compiler}{The compiler program to use for the rst to PDF conversion.}
  \item{view}{Logical value indicating whether to show the generated PDF document.}
  \item{pdfquiet}{Whether to suppress output of application opening the PDF.}
  \item{\dots}{Further arguments to be passed to compiler (usually \code{rst2pdf} or \code{pdflatex}).}
}
