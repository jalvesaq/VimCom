SyncTeX_readconc <- function(basenm)
{
    texidx <- 1
    rnwidx <- 1
    ntexln <- length(readLines(paste0(basenm, ".tex")))
    lstexln <- 1:ntexln
    lsrnwf <- lsrnwl <- rep(NA, ntexln)
    conc <- readLines(paste0(basenm, "-concordance.tex"))
    idx <- 1
    maxidx <- length(conc) + 1
    while(idx < maxidx && texidx < ntexln && length(grep("Sconcordance", conc[idx])) > 0){
        curline <- sub("\\\\Sconcordance\\{concordance:", "", conc[idx])
        texf <- sub('([^:]*):.*', '\\1', curline)
        rnwf <- sub('[^:]*:([^:]*):.*', '\\1', curline)
        idx <- idx + 1
        concnum <- ""
        while(idx < maxidx && length(grep("Sconcordance", conc[idx])) == 0){
            concnum <- paste0(concnum, conc[idx])
            idx <- idx + 1
        }
        concnum <- gsub('%', '', concnum)
        concnum <- sub('\\}', '', concnum)
        concl <- strsplit(concnum, " ")
        concl <- as.numeric(concl[[1]])
        ii <- 1
        maxii <- length(concl) - 1
        rnwl <- concl[1]
        lsrnwl[texidx] <- rnwl
        lsrnwf[texidx] <- rnwf
        texidx <- texidx + 1
        while(ii < maxii && texidx < ntexln){
            ii <- ii + 1
            lnrange <- 1:concl[ii]
            ii <- ii + 1
            for(iii in lnrange){
                if(texidx >= ntexln)
                    break
                rnwl <- rnwl + concl[ii]
                lsrnwl[texidx] <- rnwl
                lsrnwf[texidx] <- rnwf
                texidx <- texidx + 1
            }
        }
    }
    return(data.frame(texlnum = lstexln, rnwfile = lsrnwf, rnwline = lsrnwl, stringsAsFactors = FALSE))
}

GetRnwLines <- function(x, l)
{
    if(file.exists(sub(".log$", "-concordance.tex", x))){
        conc <- SyncTeX_readconc(sub(".log$", "", x))
        for(ii in 1:length(l)){
            if(length(grep("line [0-9]", l[ii])) > 0){
                texln <- as.numeric(sub(".*line ([0-9]*)", "\\1", l[ii]))
                idx <- 1
                while(idx < nrow(conc) && texln > conc$texlnum[idx]){
                    idx <- idx + 1
                    if(conc$texlnum[idx] >= texln){
                        l[ii] <- sub("(.*) line ([0-9]*)",
                                     paste0("\\1 line \\2 [",
                                            conc$rnwfile[idx], ": ",
                                            conc$rnwline[idx], "]"), l[ii])
                        break
                    }
                }
            } else if(length(grep("lines [0-9]*--[0-9]*", l[ii])) > 0){
                texln1 <- as.numeric(sub(".*lines ([0-9]*)--.*", "\\1", l[ii]))
                texln2 <- as.numeric(sub(".*lines [0-9]*--([0-9]*).*", "\\1", l[ii]))
                rnwIdx1 <- NA
                rnwIdx2 <- NA
                idx <- 1
                while(idx < nrow(conc) && texln1 > conc$texlnum[idx]){
                    idx <- idx + 1
                    if(conc$texlnum[idx] >= texln1){
                        rnwIdx1 <- idx
                        break
                    }
                }
                idx <- 1
                while(idx < nrow(conc) && texln2 > conc$texlnum[idx]){
                    idx <- idx + 1
                    if(conc$texlnum[idx] >= texln2){
                        rnwIdx2 <- idx
                        break
                    }
                }
                if(!is.na(rnwIdx1) && !is.na(rnwIdx2)){
                        l[ii] <- sub("(.*) lines ([0-9]*)--([0-9]*)",
                                     paste0("\\1 lines \\2--\\3 [",
                                            conc$rnwfile[rnwIdx1], ": ",
                                            conc$rnwline[rnwIdx1], "--",
                                            conc$rnwline[rnwIdx2], "]"), l[ii])
                }
            }
        }
    }
    l
}

ShowTexErrors <- function(x)
{
    l <- readLines(x)
    idx <- rep(FALSE, length(l))
    idx[grepl("^(Over|Under)full \\\\(h|v)box ", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^Class \\w+ (Error|Warning):", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^LaTeX (Error|Warning):", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^Package \\w+ (Error|Warning):", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^No pages of output", l, useBytes = TRUE)] <- TRUE
    if(sum(idx) > 0){
        l <- l[idx]
        l <- GetRnwLines(x, l)
        msg <- paste0("\nLaTeX errors and warnings:\n\n", paste(l, collapse = "\n"), "\n")
        cat(msg)
    }
}

OpenPDF <- function(x)
{
    path <- sub("\\.tex$", ".pdf", x)
    if(.Platform$OS.type == "windows" && identical(getOption("pdfviewer"), file.path(R.home("bin"), "open.exe"))){
        shell.exec(path)
    } else {
        .C("vimcom_msg_to_vim", paste0("ROpenPDF('", getwd(), "/", path, "')"), PACKAGE="vimcom")
    }
}

vim.interlace.rnoweb <- function(rnowebfile, rnwdir, latexcmd, latexmk = TRUE, synctex = TRUE, bibtex = FALSE,
                          knit = TRUE, buildpdf = TRUE, view = TRUE, quiet = TRUE, ...)
{
    oldwd <- getwd()
    on.exit(setwd(oldwd))
    setwd(rnwdir)

    Sres <- NA

    # Check whether the .tex was already compiled
    twofiles <- c(rnowebfile, sub("\\....$", ".tex", rnowebfile))
    if(sum(file.exists(twofiles)) == 2){
        fi <- file.info(twofiles)$mtime
        if(fi[1] < fi[2])
            Sres <- twofiles[2]
    }

    # Compile the .tex file
    if(is.na(Sres) || !buildpdf){
        if(knit){
            if(!require(knitr))
                stop("Please, install the 'knitr' package.")
            if(synctex)
                opts_knit$set(concordance = TRUE)
            Sres <- knit(rnowebfile, envir = globalenv())
        } else {
            Sres <- Sweave(rnowebfile, ...)
        }
    }

    if(!buildpdf)
        return(invisible(NULL))

    # Compile the .pdf
    if(exists('Sres')){
        # From RStudio: Check for spaces in path (Sweave chokes on these)
        if(length(grep(" ", Sres)) > 0)
            stop(paste("Invalid filename: '", Sres, "' (TeX does not understand paths with spaces).", sep=""))
        if(.Platform$OS.type == "windows"){
            # From RStudio:
            idx = !identical(.Platform$pkgType, "source")
            tools::texi2dvi(file = Sres, pdf = TRUE, index = idx, quiet = quiet)
        } else {
            if(missing(latexcmd)){
                if(latexmk){
                    if(synctex)
                        latexcmd = 'latexmk -pdflatex="pdflatex -file-line-error -synctex=1" -pdf'
                    else
                        latexcmd = 'latexmk -pdflatex="pdflatex -file-line-error" -pdf'
                } else {
                    if(synctex)
                        latexcmd = "pdflatex -file-line-error -synctex=1"
                    else
                        latexcmd = "pdflatex -file-line-error"
                }
            }
            system(paste(latexcmd, Sres))
            if(bibtex){
                system(paste("bibtex", sub("\\.tex$", ".aux", Sres)))
                system(paste(latexcmd, Sres))
                system(paste(latexcmd, Sres))
            }
        }
        if(view)
            OpenPDF(Sres)
        if(getOption("vimcom.texerrs"))
            ShowTexErrors(sub("\\.tex$", ".log", Sres))
    }
    return(invisible(NULL))
}

vim.interlace.rrst <- function(Rrstfile, rrstdir, view = TRUE,
                               compiler = "rst2pdf", ...)
{
    if(!require(knitr))
        stop("Please, install the 'knitr' package.")

    oldwd <- getwd()
    on.exit(setwd(oldwd))
    setwd(rrstdir)

    knit2pdf(Rrstfile, compiler = compiler, ...)
    if (view) {
        Sys.sleep(0.2)
        pdffile = sub('\\.Rrst$', ".pdf", Rrstfile, ignore.case = TRUE)
        OpenPDF(pdffile)
    }
}

vim.interlace.rmd <- function(Rmdfile, rmddir, view = TRUE,
                              pandoc_args = "",  pdfout = "latex", ...)
{
    if(!require(knitr))
        stop("Please, install the 'knitr' package.")

    oldwd <- getwd()
    on.exit(setwd(oldwd))
    setwd(rmddir)

    knit(Rmdfile, ...)
    tex.file <- sub("[Rr]md", "tex", Rmdfile)
    pandoc.cmd <- paste("pandoc -s", pandoc_args ,"-f markdown -t", pdfout,
                        sub("[Rr]md", "md", Rmdfile), ">", tex.file)
    system(pandoc.cmd)
    system(paste("pdflatex", tex.file))
    if (view) {
        Sys.sleep(.2)
        pdffile = sub('.[Rr]md$', ".pdf", Rmdfile, ignore.case=TRUE)
        OpenPDF(pdffile)
    }
}
