

try
(
	-- Get files in the %MaxSourceDirectory% directory
	files = getFiles "%MaxSourceDirectory%/*.max" 
	gc()
	
	-- Sort files
	sort files
	gc()
	
	-- No file ?
	if files.count != 0 then
	(
		-- For each files
		for i = 1 to files.count do
		(
			inputMaxFile = files[i]
			outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".%PreGenFileExtension%")
			
			try
			(
				-- Compare file date
				if (NeLTestFileDate outputNelFile inputMaxFile) == true then
				(
					-- Free memory and file handles
					gc()
					heapfree
					
					-- Reset 3dsmax
					resetMAXFile #noprompt
					
					-- Open the max project
					nlerror("Scanning file " + inputMaxFile + " ...")
					if (loadMaxFile inputMaxFile quiet:true) == true then
					(
						runNelMaxExport(inputMaxFile)
					)
					else
					(
						-- Error
						nlerror("ERROR exporting '%PreGenFileExtension%': can't open the file " + inputMaxFile)
					)
				)
				else
				(
					nlerror("SKIPPED " + inputMaxFile)
				)
			)
			catch 
			(
				-- Error
				nlerror("ERROR error exporting '%PreGenFileExtension%' in files " + inputMaxFile)
			)
		)
	)
	else
	(
		nlerror("WARNING no *.max file in folder %MaxSourceDirectory%")
	)
)
catch 
(
	-- Error
	nlerror("ERROR fatal error exporting '%PreGenFileExtension%' in folder %MaxSourceDirectory%")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX() #noPrompt

