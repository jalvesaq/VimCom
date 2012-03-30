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
### Sat, July 17, 2010

vim.args <- function(ff, txt)
{
    res <- "NOT_EXISTS"
    ffdef <- paste(ff, ".default", sep = "")
    if(exists(ffdef, mode = "function"))
        res <- capture.output(args(ffdef, txt))
    else
        if(exists(ff, mode = "function"))
            res <- capture.output(args(ff))
    if(res != "NOT_EXISTS"){
        res <- sub("^\\s*", "", res)
        res <- paste(res, collapse = "")
        res <- sub("^function \\((.*)\\).*", "\\1", res)
        res <- gsub(", ", ",", res)
        res <- strsplit(res, ",")[[1]]
        idx <- grep(paste("^", txt, sep = ""), res)
        res <- res[idx]
        res <- paste(res, collapse = "\t")
    }
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

