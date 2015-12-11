
vim.print <- function(object, objclass)
{
    if(!exists(object))
        stop("object '", object, "' not found")
    if(!missing(objclass) & length(grep(object, names(.knownS3Generics))) > 0){
        for(cls in objclass){
            if(exists(paste(object, ".", objclass, sep = ""))){
                .newobj <- get(paste(object, ".", objclass, sep = ""))
                warning("Printing ", object, ".", objclass, "\n", sep = "")
                break
            }
        }
    }
    if(!exists(".newobj"))
        .newobj <- get(object)
    print(.newobj)
}

