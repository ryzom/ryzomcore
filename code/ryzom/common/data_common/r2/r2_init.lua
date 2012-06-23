-- main init function

r2.init = function ()
	local function protected()

		debugInfo("r2.init begin")		
		profileFunction(r2.registerHighLevel, "r2.registerHighLevel")
		profileFunction(r2.registerBasicBricks, "r2.registerBasicBricks")
		profileFunction(r2.loadFeatures, "r2.loadFeatures")
		profileFunction(r2.loadPalette, "r2.loadPalette")

		profileFunction(r2.setupClasses, "r2.setupClasses")

		-- tmp
		if r2.InClient == true then
			--r2:testPropertySheet()
			profileMethod(r2, "buildAllPropertySheetsAndForms", "r2:buildAllPropertySheetsAndForms") 
		end
		-- IMPORTANT : should be called after all parameters of classes (displayers ...) have been initialized
		r2.TextMgr = r2.newComponent("TextManager")
		
		
		debugInfo("r2.init end")
	end
	

	local ok, errMsg = pcall(protected)

	if not ok then
		debugInfo("Error while initialization:'".. errMsg.."'")
	end


end

