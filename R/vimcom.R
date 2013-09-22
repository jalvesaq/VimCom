# This file is part of vimcom R package
# 
# It is distributed under the GNU General Public License.
# See the file ../LICENSE for details.
# 
# (c) 2011 Jakson Aquino: jalvesaq@gmail.com
# 
###############################################################

.onLoad <- function(libname, pkgname) {
    library.dynam("vimcom.plus", pkgname, libname, local = FALSE)

    if(is.null(getOption("vimcom.verbose")))
        options(vimcom.verbose = 0)

    if(is.null(getOption("vimcom.opendf")))
        options(vimcom.opendf = TRUE)

    if(is.null(getOption("vimcom.openlist")))
        options(vimcom.openlist = FALSE)

    if(is.null(getOption("vimcom.allnames")))
        options(vimcom.allnames = FALSE)

    if(is.null(getOption("vimcom.texerrs")))
        options(vimcom.texerrs = FALSE)

    if(version$os == "mingw32")
        termenv <- "MinGW"
    else
        termenv <- Sys.getenv("TERM")
    if(interactive() && termenv != "" && termenv != "dumb"){
        .C("vimcom_Start",
           as.integer(getOption("vimcom.verbose")),
           as.integer(getOption("vimcom.opendf")),
           as.integer(getOption("vimcom.openlist")),
           as.integer(getOption("vimcom.allnames")),
           PACKAGE="vimcom.plus")
    }
}

.onUnload <- function(libpath) {
    .C("vimcom_Stop", PACKAGE="vimcom.plus")
    library.dynam.unload("vimcom.plus", libpath)
}

