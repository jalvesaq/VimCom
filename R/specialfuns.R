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

# Adapted from: https://stat.ethz.ch/pipermail/ess-help/2011-March/006791.html
vim.args <- function(funcname, txt, pkg = NULL)
{
    if(is.null(pkg)){
        deffun <- paste(funcname, ".default", sep = "")
        if (existsFunction(deffun)) {
            funcname <- deffun
        } else if(!existsFunction(funcname)) {
            return("NOT_EXISTS")
        }
        frm <- formals(funcname)
    } else {
        idx <- grep(paste(":", pkg, sep = ""), search())
        ff <- "NULL"
        tr <- try(ff <- get(paste(funcname, ".default", sep = ""), pos = idx), silent = TRUE)
        if(class(tr)[1] == "try-error")
            ff <- get(funcname, pos = idx)
        frm <- formals(ff)
    }
    res <- NULL
    for (field in names(frm)) {
        type <- typeof(frm[[field]])
        if (type == 'symbol') {
            res <- append(res, paste('\x09', field, sep = ''))
        } else if (type == 'character') {
            res <- append(res, paste('\x09', field, '\x07"', frm[[field]], '"', sep = ''))
        } else if (type == 'logical') {
            res <- append(res, paste('\x09', field, '\x07', as.character(frm[[field]]), sep = ''))
        } else if (type == 'double') {
            res <- append(res, paste('\x09', field, '\x07', as.character(frm[[field]]), sep = ''))
        } else if (type == 'NULL') {
            res <- append(res, paste('\x09', field, '\x07', 'NULL', sep = ''))
        } else if (type == 'language') {
            res <- append(res, paste('\x09', field, '\x07', deparse(frm[[field]]), sep = ''))
        }
    }
    idx <- grep(paste("^\x09", txt, sep = ""), res)
    res <- res[idx]
    res <- paste(res, sep = '', collapse='')
    res <- sub("^\x09", "", res)
    res <- gsub("\n", "\\\\n", res)

    if(length(res) == 0)
        res <- "NO_ARGS"
    return(res)
}


vim.list.args <- function(ff){
    knownGenerics <- c(names(.knownS3Generics),
                       tools:::.get_internal_S3_generics()) # from methods()
    ff <- deparse(substitute(ff))
    keyf <- paste("^", ff, "$", sep="")
    is.generic <- (length(grep(keyf, knownGenerics)) > 0)
    if(is.generic){
        mm <- methods(ff)
        l <- length(mm)
        if(l > 0){
            for(i in 1:l){
                if(exists(mm[i])){
                    cat(ff, "[method ", mm[i], "]:\n", sep="")
                    print(args(mm[i]))
                    cat("\n")
                }
            }
            return(invisible(NULL))
        }
    }
    print(args(ff))
}


vim.plot <- function(x)
{
    xname <- deparse(substitute(x))
    if(length(grep("numeric", class(x))) > 0 || length(grep("integer", class(x))) > 0){
        oldpar <- par(no.readonly = TRUE)
        par(mfrow = c(2, 1))
        hist(x, col = "lightgray", main = paste("Histogram of", xname), xlab = xname)
        boxplot(x, main = paste("Boxplot of", xname),
                col = "lightgray", horizontal = TRUE)
        par(oldpar)
    } else {
        plot(x)
    }
}

