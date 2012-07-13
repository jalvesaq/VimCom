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
    if(knit)
        Sres <- knit(rnowebfile)
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
            if(pdfquiet)
                vim.openpdf(Sres, TRUE)
            else
                vim.openpdf(Sres)
    }
}

vim.interlace.rrst <- function(Rrstfile, view = TRUE, pdfquiet = FALSE, ...)
{
   knit2pdf(Rrstfile, ...)
   if (view) {
       Sys.sleep(.2)
       pdffile = sub('.Rrst$', ".pdf", Rrstfile, ignore.case=TRUE)
       if(pdfquiet) vim.openpdf(pdffile, TRUE)
       else vim.openpdf(pdffile)
   }
}

vim.interlace.rmd <- 
    function(Rmdfile, view = TRUE, 
             pdfquiet = FALSE, pandoc_args = "",  beamer = FALSE, ...)
{
   require('knitr', quietly = TRUE)
   knit(Rmdfile, ...)
   tex.file <- sub("[Rr]md", "tex", Rmdfile)
   pandoc.cmd <- 
       paste("pandoc -s", pandoc_args ,"-f markdown -t", 
             {if (beamer) "beamer" else "latex"}, 
             sub("[Rr]md", "md", Rmdfile),
             ">", tex.file)
   system(pandoc.cmd)
   system(paste("pdflatex", tex.file, {if (pdfquiet) "> /dev/null" else ""}))
   if (view) {
       Sys.sleep(.2)
       pdffile = sub('.[Rr]md$', ".pdf", Rmdfile, ignore.case=TRUE)
       if(pdfquiet) vim.openpdf(pdffile, TRUE)
       else vim.openpdf(pdffile)
   }
}
