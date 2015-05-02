# This file is part of vimcom R package
# 
# It is distributed under the GNU General Public License.
# See the file ../LICENSE for details.
# 
# (c) 2011 Jakson Aquino: jalvesaq@gmail.com
# 
###############################################################

.onLoad <- function(libname, pkgname) {
    if(Sys.getenv("VIMRPLUGIN_TMPDIR") == "")
        return(invisible(NULL))
    library.dynam("vimcom", pkgname, libname, local = FALSE)

    if(is.null(getOption("vimcom.verbose")))
        options(vimcom.verbose = 0)

    if(Sys.getenv("VIMEDITOR_SVRNM") %in% c("", "NoClientServer", "NoServerName"))
        options(vimcom.vimpager = FALSE)

    # The remaining options are set by Neovim. Don't try to set them in your
    # ~/.Rprofile because they will be overridden here:
    if(file.exists(paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/start_options.R"))){
        source(paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/start_options.R"))
    } else {
        options(vimcom.opendf = TRUE)
        options(vimcom.openlist = FALSE)
        options(vimcom.allnames = FALSE)
        options(vimcom.texerrs = TRUE)
        options(vimcom.labelerr = TRUE)
        if(is.null(getOption("vimcom.vimpager")))
            options(vimcom.vimpager = TRUE)
    }
    if(getOption("vimcom.vimpager"))
        options(pager = vim.hmsg)
}

.onAttach <- function(libname, pkgname) {
    if(Sys.getenv("VIMRPLUGIN_TMPDIR") == "")
        return(invisible(NULL))
    if(version$os == "mingw32")
        termenv <- "MinGW"
    else
        termenv <- Sys.getenv("TERM")

    if(interactive() && termenv != "NeovimTerm" && termenv != "dumb" && Sys.getenv("VIMRPLUGIN_COMPLDIR") != ""){
        dir.create(Sys.getenv("VIMRPLUGIN_COMPLDIR"), showWarnings = FALSE)
        .C("vimcom_Start",
           as.integer(getOption("vimcom.verbose")),
           as.integer(getOption("vimcom.opendf")),
           as.integer(getOption("vimcom.openlist")),
           as.integer(getOption("vimcom.allnames")),
           as.integer(getOption("vimcom.labelerr")),
           path.package("vimcom"),
           as.character(utils::packageVersion("vimcom")),
           PACKAGE="vimcom")
    }
}

.onUnload <- function(libpath) {
    if(is.loaded("vimcom_Stop", PACKAGE = "vimcom")){
        .C("vimcom_Stop", PACKAGE="vimcom")
        if(Sys.getenv("VIMRPLUGIN_TMPDIR") != ""){
            unlink(paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/vimcom_running_",
                          Sys.getenv("VIMINSTANCEID")))
            if(.Platform$OS.type == "windows")
                unlink(paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/rconsole_hwnd_",
                              Sys.getenv("VIMRPLUGIN_SECRET")))
        }
        Sys.sleep(0.2)
        library.dynam.unload("vimcom", libpath)
    }
}


vim_edit <- function(name, file, title)
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

vim_capture_source_output <- function(s, o)
{
    capture.output(base::source(s, echo = TRUE), file = o)
    .C("vimcom_msg_to_vim", paste0("GetROutput('", o, "')"), PACKAGE="vimcom")
}

vim_viewdf <- function(oname)
{
    ok <- try(o <- get(oname, envir = .GlobalEnv), silent = TRUE)
    if(inherits(ok, "try-error")){
        .C("vimcom_msg_to_vim",
           paste0("RWarningMsg('", '"', oname, '"', " not found in .GlobalEnv')"),
           PACKAGE="vimcom")
        return(invisible(NULL))
    }
    if(is.data.frame(o) || is.matrix(o)){
        write.table(o, sep = "\t", row.names = FALSE, quote = FALSE,
                    file = paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/Rinsert"))
        .C("vimcom_msg_to_vim", paste0("RViewDF('", oname, "')"), PACKAGE="vimcom")
    } else {
        .C("vimcom_msg_to_vim",
           paste0("RWarningMsg('", '"', oname, '"', " is not a data.frame or matrix')"),
           PACKAGE="vimcom")
    }
    return(invisible(NULL))
}
