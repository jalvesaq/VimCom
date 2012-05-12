
vim.openpdf <- function(x)
{
    pdfviewer <- getOption("pdfviewer")
    path <- sub("\\.tex$", ".pdf", x)
    if(!identical(pdfviewer, "false")){
        if(.Platform$OS.type == "windows" && identical(pdfviewer, file.path(R.home("bin"), "open.exe")))
            shell.exec(path)
        else 
            system2(pdfviewer, shQuote(path), wait = FALSE, stdout = FALSE, stderr = FALSE)
    }
}

vim.interlace <- function(rnowebfile, latexcmd = "pdflatex", bibtex = FALSE,
                          knit = FALSE, view = TRUE, quiet = TRUE, ...)
{
    if(knit)
        Sres <- knit(rnowebfile, ...)
    else
        Sres <- Sweave(rnowebfile, ...)
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
            vim.openpdf(Sres)
    }
}
