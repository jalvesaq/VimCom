
vim.pager <- function(files, header, title, delete.file)
{
    if(Sys.getenv("VIMRPLUGIN_TMPDIR") == "")
        stop("VIMRPLUGIN_TMPDIR not set.")
    file.copy(files[1],
              paste(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/Rdoc", sep = ""))
}

vim.help <- function(topic, w, classfor, package)
{
    if(version$major < "2" || (version$major == "2" && version$minor < "12.0"))
        return("The use of Vim as pager for R requires R >= 2.12.0. Please, put in your vimrc: let vimrplugin_vimpager = \"no\"")

    if(!missing(classfor) & length(grep(topic, names(.knownS3Generics))) > 0){
        curwarn <- getOption("warn")
        options(warn = -1)
        try(classfor <- classfor, silent = TRUE)  # classfor may be a function
        try(.theclass <- class(classfor), silent = TRUE)
        options(warn = curwarn)
        if(exists(".theclass")){
            for(i in 1:length(.theclass)){
                newtopic <- paste(topic, ".", .theclass[i], sep = "")
                if(length(help(newtopic))){
                    topic <- newtopic
                    break
                }
            }
        }
    }

    # Requires at least R 2.12
    oldRdOp <- tools::Rd2txt_options()
    on.exit(tools::Rd2txt_options(oldRdOp))
    tools::Rd2txt_options(width = w)

    oldpager <- getOption("pager")
    on.exit(options(pager = oldpager), add = TRUE)
    options(pager = vim.pager)

    if(missing(package))
        h <- help(topic, help_type = "text")
    else
        h <- help(topic, package = as.character(package), help_type = "text")

    if(length(h) == 0){
        msg <- paste('No documentation for "', topic, '" in loaded packages and libraries.', sep = "")
        return(msg)
    }
    if(length(h) > 1){
        if(missing(package)){
            h <- sub("/help/.*", "", h)
            h <- sub(".*/", "", h)
            msg <- "MULTILIB"
            for(l in h)
                msg <- paste(msg, l)
            return(msg)
        } else {
            h <- h[grep(paste("/", package, "/", sep = ""), h)]
            if(length(h) == 0)
                return(paste("Package '", package, "' has no documentation for '", topic, "'", sep = ""))
        }
    }
    print(h)

    return("VIMHELP")
}

