
vim.hmsg <- function(files, header, title, delete.file)
{
    if(Sys.getenv("VIMRPLUGIN_TMPDIR") == "")
        stop("VIMRPLUGIN_TMPDIR not set.")
    dest <- paste0(Sys.getenv("VIMRPLUGIN_TMPDIR"), "/Rdoc")
    file.copy(files[1], dest, overwrite = TRUE)
    keyword <- sub(".* '", "", title)
    keyword <- sub(".* \u2018", "", keyword)
    keyword <- sub("'", "", keyword)
    keyword <- sub("\u2019", "", keyword)
    .C("vimcom_msg_to_vim", paste0("ShowRDoc('", keyword, "')"), PACKAGE="vimcom")
    return(invisible(NULL))
}

vim.help <- function(topic, w, objclass, package)
{
    if(!missing(objclass) & length(grep(topic, names(.knownS3Generics))) > 0){
        for(i in 1:length(objclass)){
            newtopic <- paste(topic, ".", objclass[i], sep = "")
            if(length(utils::help(newtopic))){
                topic <- newtopic
                break
            }
        }
    }

    # Requires at least R 2.12
    oldRdOp <- tools::Rd2txt_options()
    on.exit(tools::Rd2txt_options(oldRdOp))
    tools::Rd2txt_options(width = w)

    oldpager <- getOption("pager")
    on.exit(options(pager = oldpager), add = TRUE)
    options(pager = vim.hmsg)

    # try devtools first (if loaded)
    if ("devtools" %in% loadedNamespaces()) {
        if (missing(package)) {
            if (!is.null(devtools:::find_topic(topic))) {
                devtools::dev_help(topic)
                return(invisible(NULL))
            }
        } else {
            if (package %in% devtools::dev_packages()) {
                ret = try(devtools::dev_help(topic), silent = TRUE)
                if (inherits(ret, "try-error"))
                    .C("vimcom_msg_to_vim", paste0("RWarningMsg('", as.character(ret), "')"), PACKAGE="vimcom")
                return(invisible(NULL))
            }
        }
    }

    if(missing(package))
        h <- utils::help(topic, help_type = "text")
    else
        h <- utils::help(topic, package = as.character(package), help_type = "text")

    if(length(h) == 0){
        msg <- paste('No documentation for "', topic, '" in loaded packages and libraries.', sep = "")
        .C("vimcom_msg_to_vim", paste0("RWarningMsg('", msg, "')"), PACKAGE="vimcom")
        return(invisible(NULL))
    }
    if(length(h) > 1){
        if(missing(package)){
            h <- sub("/help/.*", "", h)
            h <- sub(".*/", "", h)
            msg <- paste("MULTILIB", paste(h, collapse = " "), topic)
            .C("vimcom_msg_to_vim", paste0("ShowRDoc('", msg, "')"), PACKAGE="vimcom")
            return(invisible(NULL))
        } else {
            h <- h[grep(paste("/", package, "/", sep = ""), h)]
            if(length(h) == 0){
                msg <- paste("Package '", package, "' has no documentation for '", topic, "'", sep = "")
                .C("vimcom_msg_to_vim", paste0("RWarningMsg('", msg, "')"), PACKAGE="vimcom")
                return(invisible(NULL))
            }
        }
    }
    print(h)

    return(invisible(NULL))
}

