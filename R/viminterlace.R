
vim.interlace <- function(rnowebfile, latexcmd = "pdflatex", bibtex = FALSE,
                          knit = FALSE, view = TRUE, ...)
{
    if(knit){
        Sres <- knit(rnowebfile, ...)
    } else {
        Sres <- Sweave(rnowebfile, ...)
    }
    if(exists('Sres')){
        if(.Platform$OS.type == "windows"){
            tools::texi2pdf(Sres, quiet = bibtex == FALSE)
        } else {
            system(paste(latexcmd, Sres))
            if(bibtex){
                system(paste("bibtex", sub("\\.tex$", ".aux", Sres)))
                system(paste(latexcmd, Sres))
                system(paste(latexcmd, Sres))
            }
        }
        if(view){
            # Copyed from RShowDoc()
            pdfviewer <- getOption("pdfviewer")
            path <- sub("\\.tex$", ".pdf", Sres)
            if (!identical(pdfviewer, "false")) {
                if (.Platform$OS.type == "windows" && identical(pdfviewer, file.path(R.home("bin"), "open.exe")))
                    shell.exec(path)
                else 
                    system2(pdfviewer, shQuote(path), wait = FALSE)
            }
        }
    }
}
