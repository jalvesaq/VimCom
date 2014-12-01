# This file is part of vimcom R package
# 
# It is distributed under the GNU General Public License.
# See the file ../LICENSE for details.
# 
# (c) 2011 Jakson Aquino: jalvesaq@gmail.com
# 
###############################################################

.onLoad <- function(libname, pkgname) {
    library.dynam("vimcom", pkgname, libname, local = FALSE)

    if(is.null(getOption("vimcom.verbose")))
        options(vimcom.verbose = 0)

    if(is.null(getOption("vimcom.opendf")))
        options(vimcom.opendf = TRUE)

    if(is.null(getOption("vimcom.openlist")))
        options(vimcom.openlist = FALSE)

    if(is.null(getOption("vimcom.allnames")))
        options(vimcom.allnames = FALSE)

    if(is.null(getOption("vimcom.texerrs")))
        options(vimcom.texerrs = TRUE)

    if(is.null(getOption("vimcom.alwaysls")))
        options(vimcom.alwaysls = TRUE)

    if(is.null(getOption("vimcom.labelerr")))
        options(vimcom.labelwarn = TRUE)

}

.onAttach <- function(libname, pkgname) {
    if(version$os == "mingw32")
        termenv <- "MinGW"
    else
        termenv <- Sys.getenv("TERM")

    if(interactive() && termenv != "" && termenv != "dumb" && Sys.getenv("VIMRPLUGIN_HOME") != ""){
        dir.create(paste0(Sys.getenv("VIMRPLUGIN_HOME"), "/r-plugin/objlist/"),
                   showWarnings = FALSE)
        .C("vimcom_Start",
           as.integer(getOption("vimcom.verbose")),
           as.integer(getOption("vimcom.opendf")),
           as.integer(getOption("vimcom.openlist")),
           as.integer(getOption("vimcom.allnames")),
           as.integer(getOption("vimcom.alwaysls")),
           as.integer(getOption("vimcom.labelerr")),
           path.package("vimcom"),
           as.character(utils::packageVersion("vimcom")),
           PACKAGE="vimcom")
    }
    if(termenv == "NeovimTerm"){
        Sys.setenv(TERM="dumb")
        options(continue = "#<#\n",
                prompt = "#>#\n",
                editor = vimcom:::vimcom_edit)
    }
}

.onUnload <- function(libpath) {
    .C("vimcom_Stop", PACKAGE="vimcom")
    unlink(paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/vimcom_running_", Sys.getenv("VIMINSTANCEID")))
    Sys.sleep(1)
    library.dynam.unload("vimcom", libpath)
}


vimcom_edit <- function(name, file, title)
{
    if(file != "")
        stop("Feature not implemented. Use nvim to edit files.")
    if(is.null(name))
        stop("Feature not implemented. Use nvim to create R objects from scratch.")

    finalA <- paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/vimcom_edit_", Sys.getenv("VIMINSTANCEID"), "_A")
    finalB <- paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/vimcom_edit_", Sys.getenv("VIMINSTANCEID"), "_B")
    unlink(finalB)
    writeLines(text = "Waiting...", con = finalA)

    initial = paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/vimcom_edit_", round(runif(1, min = 100, max = 999)))
    sink(initial)
    dput(name)
    sink()

    .C("vimcom_msg_to_vim",
       paste0("ShowRObject('", initial, "')"),
       PACKAGE="vimcom")

    while(file.exists(finalA))
        Sys.sleep(1)
    x <- eval(parse(finalB))
    unlink(initial)
    unlink(finalB)
    return(invisible(x))
}
