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

vim.openpdf <- function(x, quiet = FALSE)
{
    pdfviewer <- getOption("pdfviewer")
    path <- sub("\\.tex$", ".pdf", x)
    if(!identical(pdfviewer, "false")){
        if(.Platform$OS.type == "windows" && identical(pdfviewer, file.path(R.home("bin"), "open.exe")))
            shell.exec(path)
        else 
            if(quiet)
                system2(pdfviewer, shQuote(path), wait = FALSE, stdout = FALSE, stderr = FALSE)
            else
                system2(pdfviewer, shQuote(path), wait = FALSE)
    }
}

vim.interlace.rnoweb <- function(rnowebfile, latexcmd = "pdflatex", bibtex = FALSE,
                          knit = FALSE, view = TRUE, quiet = TRUE, pdfquiet = FALSE, ...)
{
    if(knit){
        if(!require(knitr))
            stop("Please, install the 'knitr' package.")
        Sres <- knit(rnowebfile, envir = globalenv())
    } else {
        Sres <- Sweave(rnowebfile, ...)
    }
    if(exists('Sres')){
        # From RStudio: Check for spaces in path (Sweave chokes on these)
        if(length(grep(" ", Sres)) > 0)
            stop(paste("Invalid filename: '", Sres, "' (TeX does not understand paths with spaces).", sep=""))
        if(.Platform$OS.type == "windows"){
            # From RStudio:
            idx = !identical(.Platform$pkgType, "source")
            tools::texi2dvi(file = Sres, pdf = TRUE, index = idx, quiet = quiet)
        } else {
            system(paste(latexcmd, Sres))
            if(bibtex){
                system(paste("bibtex", sub("\\.tex$", ".aux", Sres)))
                system(paste(latexcmd, Sres))
                system(paste(latexcmd, Sres))
            }
        }
        if(view)
            if(pdfquiet)
                vim.openpdf(Sres, TRUE)
            else
                vim.openpdf(Sres)
        if(getOption("vimcom.texerrs"))
            vim.showTexErrors(sub("\\.tex$", ".log", Sres))
    }
}

vim.interlace.rrst <- function(Rrstfile, view = TRUE, pdfquiet = FALSE,
                               compiler = "rst2pdf", ...)
{
    if(!require(knitr))
        stop("Please, install the 'knitr' package.")
    knit2pdf(Rrstfile, compiler = compiler, ...)
    if (view) {
        Sys.sleep(0.2)
        pdffile = sub('\\.Rrst$', ".pdf", Rrstfile, ignore.case = TRUE)
        if(pdfquiet)
            vim.openpdf(pdffile, TRUE)
        else
            vim.openpdf(pdffile)
    }
}

vim.interlace.rmd <- function(Rmdfile, view = TRUE, pdfquiet = FALSE,
                              pandoc_args = "",  pdfout = "latex", ...)
{
    if(!require(knitr))
        stop("Please, install the 'knitr' package.")
    knit(Rmdfile, ...)
    tex.file <- sub("[Rr]md", "tex", Rmdfile)
    pandoc.cmd <- paste("pandoc -s", pandoc_args ,"-f markdown -t", pdfout,
                        sub("[Rr]md", "md", Rmdfile), ">", tex.file)
    system(pandoc.cmd)
    system(paste("pdflatex", tex.file, {if (pdfquiet) "> /dev/null" else ""}))
    if (view) {
        Sys.sleep(.2)
        pdffile = sub('.[Rr]md$', ".pdf", Rmdfile, ignore.case=TRUE)
        if(pdfquiet) vim.openpdf(pdffile, TRUE)
        else vim.openpdf(pdffile)
    }
}
