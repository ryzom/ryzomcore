

printLog(log, ">>> Export skel directly <<<")
mkPath(log, ExportBuildDirectory + "/" + SkelExportDirectory)
for dir in SkelSourceDirectories:
	mkPath(log, DatabaseDirectory + "/" + dir)
	copyFilesExtNoSubdirIfNeeded(log, DatabaseDirectory + "/" + dir, ExportBuildDirectory + "/" + SkelExportDirectory, ".skel")

