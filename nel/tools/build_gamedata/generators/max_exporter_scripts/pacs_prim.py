

# Remove bad file from previous script version
listPath = ExportBuildDirectory + "/" + %PreGenExportDirectoryVariable% + "/landscape_col_prim_pacs_list.txt"
if os.path.isfile(listPath):
	os.remove(listPath)

