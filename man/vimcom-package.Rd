\name{vimcom-package}
\alias{vimcom-package}
\alias{vimcom}
\docType{package}
\title{Allow the communication between Vim and R }
\description{This package provides a TCP/IP server to allow the communication
  between R and Vim-R-plugin. It also has some functions called by the
  plugin. By default, the elements of data.frames are shown in the object
  browser, but not the elements of other types of lists, and, by default,
  names of objects which begin with a dot are omitted. The following options
  control the vimcom behavior:

  \tabular{lll}{
    Option          \tab Description                     \tab Default \cr
    vimcom.verbose  \tab Print information.              \tab 0       \cr
    vimcom.opendf   \tab Show data.frames elements.      \tab TRUE    \cr
    vimcom.openlist \tab Show lists elements.            \tab FALSE   \cr
    vimcom.allnames \tab Show .GlobalEnv hidden objects. \tab FALSE   \cr
    vimcom.texerrs  \tab Output TeX errors and warnings. \tab FALSE   \cr
  }

  You may want to put the options above in your Rprofile rather than setting
  them and loading the vimcom package manually. Below is an example of
  ~/.Rprofile

  \preformatted{
    if(interactive()){
      options(vimcom.verbose = 1, vimcom.allnames = TRUE,
              vimcom.texerrs = TRUE)
      library(vimcom)
    }
}

}
\seealso{\link[base]{Startup}}
\author{
Jakson Alves de Aquino <jalvesaq@gmail.com>
}
\keyword{ package }
