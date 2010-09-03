

printLog(log, ">>> List %PreGenFileExtension% <<<")
outDirPacsPrim =  ExportBuildDirectory + "/" + %PreGenExportDirectoryVariable%
mkPath(log, outDirPacsPrim)
listPath = ExportBuildDirectory + "/" + %PreGenExportDirectoryVariable% + "/landscape_col_prim_pacs_list.txt"
if os.path.isfile(listPath):
	os.remove(listPath)
if WantLandscapeColPrimPacsList:
	exportedPacsPrims = findFiles(log, outDirPacsPrim, "", ".%PreGenFileExtension%")
	printLog(log, "WRITE " + listPath)
	listFile = open(listPath, "w")
	for exported in exportedPacsPrims:
		listFile.write(exported + "\n")
	listFile.close()

