vim.showTexErrors <- function(x)
{
    l <- readLines(x)
    idx <- rep(FALSE, length(l))
    idx[grepl("^(Over|Under)full \\\\(h|v)box ", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^Class \\w+ (Error|Warning):", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^LaTeX (Error|Warning):", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^Package \\w+ (Error|Warning):", l, useBytes = TRUE)] <- TRUE
    idx[grepl("^No pages of output", l, useBytes = TRUE)] <- TRUE
    if(sum(idx) > 0){
        msg <- paste0("\nLaTeX errors and warnings:\n\n", paste(l[idx], collapse = "\n"), "\n")
        cat(msg)
    }
}

vim.openpdf <- function(x)
{
    path <- sub("\\.tex$", ".pdf", x)
    if(.Platform$OS.type == "windows" && identical(getOption("pdfviewer"), file.path(R.home("bin"), "open.exe"))){
        shell.exec(path)
    } else {
        .C("vimcom_open_pdf", paste0("ROpenPDF('", path, "')"), PACKAGE="vimcom")
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
            vim.openpdf(Sres)
        if(getOption("vimcom.texerrs"))
            vim.showTexErrors(sub("\\.tex$", ".log", Sres))
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
        vim.openpdf(pdffile)
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
        vim.openpdf(pdffile)
    }
}
