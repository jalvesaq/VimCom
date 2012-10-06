#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  A copy of the GNU General Public License is available at
#  http://www.r-project.org/Licenses/

### Jakson Alves de Aquino
### Tue, January 18, 2011

# This function writes two files: one with the names of all functions in all
# packages (either loaded or installed); the other file lists all objects,
# including function arguments. These files are used by Vim to highlight
# functions and to complete the names of objects and the arguments of
# functions.

vim.grepl <- function(pattern, x) {
    res <- grep(pattern, x)
    if(length(res) == 0){
        return(FALSE)
    } else {
        return(TRUE)
    }
}

vim.omni.line <- function(x, envir, printenv, curlevel) {
    if(curlevel == 0){
        xx <- get(x, envir)
    } else {
        x.clean <- gsub("$", "", x, fixed = TRUE)
        x.clean <- gsub("_", "", x.clean, fixed = TRUE)
        haspunct <- vim.grepl("[[:punct:]]", x.clean)
        if(haspunct[1]){
            ok <- vim.grepl("[[:alnum:]]\\.[[:alnum:]]", x.clean)
            if(ok[1]){
                haspunct  <- FALSE
                haspp <- vim.grepl("[[:punct:]][[:punct:]]", x.clean)
                if(haspp[1]) haspunct <- TRUE
            }
        }

        # No support for names with spaces
        if(vim.grepl(" ", x)){
            haspunct <- TRUE
        }

        if(haspunct[1]){
            xx <- NULL
        } else {
            xx <- try(eval(parse(text=x)), silent = TRUE)
            if(class(xx)[1] == "try-error"){
                xx <- NULL
            }
        }
    }

    if(is.null(xx)){
        x.group <- " "
        x.class <- "unknown"
    } else {
        if(x == "break" || x == "next" || x == "for" || x == "if" || x == "repeat" || x == "while"){
            x.group <- "flow-control"
            x.class <- "flow-control"
        } else {
            if(is.function(xx)) x.group <- "function"
            else if(is.numeric(xx)) x.group <- "numeric"
            else if(is.factor(xx)) x.group <- "factor"
            else if(is.character(xx)) x.group <- "character"
            else if(is.logical(xx)) x.group <- "logical"
            else if(is.data.frame(xx)) x.group <- "data.frame"
            else if(is.list(xx)) x.group <- "list"
            else x.group <- " "
            x.class <- class(xx)[1]
        }
    }

    if(x.group == "function"){
        if(curlevel == 0){
            if(vim.grepl("GlobalEnv", printenv)){
                cat(x, "\x06function\x06function\x06", printenv, "\x06", vim.args(x, txt = ""), "\n", sep="")
            } else {
                cat(x, "\x06function\x06function\x06", printenv, "\x06", vim.args(x, txt = "", pkg = printenv), "\n", sep="")
            }
        } else {
            # some libraries have functions as list elements
            cat(x, "\x06function\x06function\x06", printenv, "\x06Unknown arguments", "\n", sep="")
        }
    } else {
        if(is.list(xx)){
            if(curlevel == 0){
                cat(x, "\x06", x.class, "\x06", x.group, "\x06", printenv, "\x06Not a function", "\n", sep="")
            } else {
                cat(x, "\x06", x.class, "\x06", " ", "\x06", printenv, "\x06Not a function", "\n", sep="")
            }
        } else {
            cat(x, "\x06", x.class, "\x06", x.group, "\x06", printenv, "\x06Not a function", "\n", sep="")
        }
    }

    if(is.list(xx) && curlevel == 0){
        obj.names <- names(xx)
        curlevel <- curlevel + 1
        if(length(xx) > 0){
            for(k in obj.names){
                vim.omni.line(paste(x, "$", k, sep=""), envir, printenv, curlevel)
            }
        }
    }
}

# Build Omni List
vim.bol <- function(omnilist, packlist, allnames = FALSE) {
    vim.OutDec <- options("OutDec")[[1]]
    options(OutDec = ".")

    if(vim.grepl("/r-plugin/objlist", omnilist) == FALSE){
        sink(omnilist, append = FALSE)
        obj.list <- objects(".GlobalEnv", all.names = allnames)
        l <- length(obj.list)
        if(l > 0)
            for(obj in obj.list) vim.omni.line(obj, ".GlobalEnv", ".GlobalEnv", 0)
        sink()
        writeLines(text = "Finished",
                   con = paste(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/vimbol_finished", sep = ""))
        return(invisible(NULL))
    }

    cat("Building files with lists of objects in loaded packages for omni completion and Object Browser...\n")

    dir.create(omnilist, showWarnings = FALSE)

    loadpack <- search()
    if(missing(packlist))
        listpack <- loadpack[grep("^package:", loadpack)]
    else
        listpack <- paste("package:", packlist, sep = "")

    needunload <- FALSE
    for(curpack in listpack){
        curlib <- sub("^package:", "", curpack)
        if(vim.grepl(curlib, loadpack) == FALSE){
            cat("Loading   '", curlib, "'...\n", sep = "")
            needunload <- try(require(curlib, character.only = TRUE))
            if(needunload != TRUE){
                needunload <- FALSE
                next
            }
        }
        obj.list <- objects(curpack, all.names = allnames)
        l <- length(obj.list)
        if(l > 0){
            sink(paste(omnilist, "omnils_", curlib, sep = ""), append = TRUE)
            for(obj in obj.list) vim.omni.line(obj, curpack, curlib, 0)
            sink()
        }
        if(needunload){
            cat("Detaching '", curlib, "'...\n", sep = "")
            try(detach(curpack, unload = TRUE, character.only = TRUE), silent = TRUE)
            needunload <- FALSE
        }
    }
    options(OutDec = vim.OutDec)
    writeLines(text = "Finished",
               con = paste(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/vimbol_finished", sep = ""))
    cat("Finished.\n")
    return(invisible(NULL))
}

