files <- Sys.glob(paste0("*", SHLIB_EXT))
dest <- file.path(R_PACKAGE_DIR, paste0('libs', R_ARCH))
dir.create(dest, recursive = TRUE, showWarnings = FALSE)
file.copy(files, dest, overwrite = TRUE)
if(file.exists("symbols.rds"))
    file.copy("symbols.rds", dest, overwrite = TRUE)

execs <- "apps/vclientserver"
if(WINDOWS)
    execs <- c("apps/vclientserver.exe", "apps/libVimR.dll")
if(any(file.exists(execs))){
    dest <- file.path(R_PACKAGE_DIR,  paste0('bin', R_ARCH))
    dir.create(dest, recursive = TRUE, showWarnings = FALSE)
    file.copy(execs, dest, overwrite = TRUE)
}

