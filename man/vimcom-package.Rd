\name{vimcom-package}
\alias{vimcom-package}
\alias{vimcom}
\docType{package}
\title{Allow the communication between Vim and R }
\description{This package provides a TCP/IP server to allow the communication
  between R and Vim-R-plugin. It also has some functions called by the
  plugin.

  The \samp{vimcom.verbose} option controls the amount of debugging
  information printed on R Console. If the value is 1, the package version is
  output on startup. Higher values, up to 4, will make the package print
  information verbosely which is useful only if you want either to fix a bug
  or to understand how vimcom works. If you run Vim in an terminal emulator
  without the \samp{--servername} argument, you have to set the value of
  vimcom.verbose to -1 to suppress the message about Vim-R-plugin missing
  features. Neovim does not require the \samp{--servername} argument.

  Below is an example of how to load \pkg{vimcom} in your \samp{~/.Rprofile}:

  \preformatted{
    if(interactive()){
      options(vimcom.verbose = 1)
      library(vimcom)
    }
}

}
\seealso{\link[base]{Startup}}
\author{
Jakson Alves de Aquino \email{jalvesaq@gmail.com}
}
\keyword{ package }
