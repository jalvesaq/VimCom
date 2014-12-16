# This file is part of vimcom R package
# 
# It is distributed under the GNU General Public License.
# See the file ../LICENSE for details.
# 
# (c) 2011 Jakson Aquino: jalvesaq@gmail.com
# 
###############################################################

.onLoad <- function(libname, pkgname) {
    if(Sys.getenv("VIMRPLUGIN_TMPDIR") == "" || Sys.getenv("VIMRPLUGIN_TMPDIR") == "None")
        return(invisible(NULL))
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

    if(Sys.getenv("VIMEDITOR_SVRNM") %in% c("", "MacVim", "NoClientServer", "NoServerName"))
        options(vimcom.vimpager = FALSE)
    if(is.null(getOption("vimcom.vimpager"))){
        options(vimcom.vimpager = TRUE)
    }
    if(getOption("vimcom.vimpager"))
        options(pager = vim.hmsg)
}

.onAttach <- function(libname, pkgname) {
    if(Sys.getenv("VIMRPLUGIN_TMPDIR") == "" || Sys.getenv("VIMRPLUGIN_TMPDIR") == "None")
        return(invisible(NULL))
    if(version$os == "mingw32")
        termenv <- "MinGW"
    else
        termenv <- Sys.getenv("TERM")

    if(interactive() && termenv != "" && termenv != "dumb" && Sys.getenv("VIMRPLUGIN_COMPLDIR") != ""){
        dir.create(Sys.getenv("VIMRPLUGIN_COMPLDIR"), showWarnings = FALSE)
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
        # "pager" and "editor" can't be optional because Neovim buffer isn't a
        # real terminal.
        options(pager = vim.hmsg,
                # continue = "#<#\n", # workaround for Neovim job limitation
                # prompt = "#>#\n", # https://github.com/neovim/neovim/issues/1574
                editor = vim_edit)
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
