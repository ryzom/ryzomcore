
try
(
	undo off
	(
		NelToolLoggerInitError %LogErrorFile%
		NelToolLoggerInitDepend %LogDependFiles%
		
		-- Unhide, unselect
		unhidecategory()
		max unhide all
		max select none
		
		-- Setup
		lowercasenodes()
		dependxref undefined
		
		-- Go!
		runNelMaxExport()
		
		NelToolLoggerReleaseError
		NelToolLoggerReleaseDepend
		
		gc()
	)
)
catch 
(
	-- Error
	nlerror("Fatal error occured")
	NelForceQuitRightNow()
)
