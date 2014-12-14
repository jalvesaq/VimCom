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
    Option          \tab Description                            \tab Default \cr
    vimcom.verbose  \tab Print information.                     \tab 0       \cr
    vimcom.opendf   \tab Show data.frames elements.             \tab TRUE    \cr
    vimcom.openlist \tab Show lists elements.                   \tab FALSE   \cr
    vimcom.allnames \tab Show .GlobalEnv hidden objects.        \tab FALSE   \cr
    vimcom.alwaysls \tab Update the Object Browser more.        \tab TRUE    \cr
    vimcom.labelerr \tab Warn if label is not \samp{character}. \tab TRUE    \cr
    vimcom.texerrs  \tab Output TeX errors and warnings.        \tab TRUE    \cr
    vimcom.vimpager \tab Show R documentation in Vim.           \tab TRUE    \cr
  }

  The \samp{vimcom.verbose} option controls the amount of debugging
  information printed on R Console. If the value is 1, the package version is
  output on startup. Higher values, up to 4, will make the package print
  information verbosely which is useful only if you want either to fix a bug
  or to understand how vimcom works. If you run Vim in an terminal emulator
  without the \samp{--servername} argument, you have to set the value of
  vimcom.verbose to -1 to suppress the message about Vim-R-plugin missing
  features. Neovim does not require the \samp{--servername} argument.

  By default, when a \samp{data.frame} appears in the Object Browser for the
  first time, its elements are immediately displayed, but the elements of a
  \samp{list} are displayed only if it is explicitly opened (see the
  Vim-R-plugin documentation for details). The options \samp{vimcom.opendf}
  and \samp{vimcom.openlist} controls the initial status (either opened or
  closed) of, respectively, data.frames and lists. The options are ignored for
  data.frames and lists of libraries which are always started closed.

  If the value of \samp{vimcom.alwaysls} is \samp{FALSE}, the Object Browser
  will update the R's workspace list of objects only when the number of
  objects changes.

  If an object R's workspace has the attribute \samp{label}, it is displayed
  in Vim's Object Browser. If the \samp{label} attribute is not of class
  \samp{"character"}, and if  \samp{vimcom.labelerr} is \samp{TRUE}, an error
  message is printed in the Object Browser.

  If the value of \samp{vimcom.texerrs} is \samp{TRUE}, LaTeX errors and
  warnings produced by the compilation of the .tex document into .pdf file
  will be output to R Console at the end of the compilation. So, you do not
  have to scroll the R Console seeking for these messages.

  If the value of \samp{vimcom.vimpager} is true, the pager becomes a function
  that will open R documentation in Vim (If it does not work for you, see the
  Vim-R-plugin section \samp{r-plugin-nox}).

  You should put some of the options above in your Rprofile rather than
  setting them and loading the vimcom package manually. Below is an
  example of \samp{~/.Rprofile}

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
Jakson Alves de Aquino \email{jalvesaq@gmail.com}
}
\keyword{ package }
