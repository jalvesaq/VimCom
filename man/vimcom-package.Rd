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
    vimcom.alwaysls \tab Update the Object Browser more. \tab TRUE    \cr
    vimcom.texerrs  \tab Output TeX errors and warnings. \tab FALSE   \cr
  }

  The \samp{vimcom.verbose} option controls the amount of debugging
  information printed on R Console. If the value is 1, the package version is
  output on startup. Higher values, up to 4, will make the package print
  information verbosely which is useful only if you want either to fix a bug
  or to understand how vimcom works.

  By default, when a \samp{data.frame} appears in the Object Browser for the
  first time, its elements are immediately displayed, but the elements of a
  \samp{list} are displayed only if it is explicitly opened (see the
  Vim-R-plugin documentation for details). The options \samp{vimcom.opendf}
  and \samp{vimcom.openlist} controls the initial status (either opened or
  closed) of, respectively, data.frames and lists.

  If the value of \samp{vimcom.alwaysls} is \samp{FALSE}, the Object Browser
  will update the R's workspace list of objects only when the number of
  objects changes.

  If you set the value of \samp{vimcom.texerrs} to \samp{TRUE}, LaTeX errors
  and warnings produced by the compilation of the .tex document into .pdf file
  will be output to R Console at the end of the compilation. So, you do not
  have to scroll the R Console seeking for these messages.

  You may want to put the some of the options above in your Rprofile rather
  than setting them and loading the vimcom package manually. Below is an
  example of ~/.Rprofile

  \preformatted{
    if(interactive()){
      options(vimcom.verbose = 1,
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
