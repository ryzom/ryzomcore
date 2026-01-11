-- This file contains code that is called each frame by the C++ framework


r2.UIMainLoop = 
{
	-- TODO nico : move this in a better place
	LeftQuotaModified = true, -- when set to true, the "left quota" field in the scenario window should be updated	
   PlotItemsModified = false
}



local firstLoop = true
local loopIndex = 0
local lastTime = 0
--------------------------------------------------------------------------------
-- Called by the C++ framework just after scene has been rendered and before ui rendering
function r2.UIMainLoop:onPostSceneRender()   
	-- if left quota has been modified then update scenario window
	if r2.Mode == "Edit" then
		if self.LeftQuotaModified then		
			r2.ScenarioWindow:updateLeftQuota()		
		end   
		r2.SelectBar:update()
	end
   -- plot items
   if self.PlotItemsModified then
      r2.PlotItemDisplayerCommon:updateAll()
      self.PlotItemsModified = false
   end
	-- if there's a forms displayed, call its update proc if it has one
	if r2.CurrentForm and r2.CurrentForm.active then
		if type(r2.CurrentForm.Env.Form.onPostRender) == "function" then
			r2.CurrentForm.Env.Form.onPostRender(r2.CurrentForm.Env.FormInstance)
		end
	end


	if firstLoop == true then
		firstLoop = false
	else
		loopIndex = loopIndex + 1
		if loopIndex > 10 then			
			loopIndex = 0
			local now =os.clock ()
			if now ~= lastTime then
				lastTime = now
				if r2.Translator then r2.Translator.updateEachSecond() end
				if ld.update then ld.update() end
				if not r2_core.UserComponentManager:isInitialized() and r2.isServerEditionModuleUp() then
					r2_core.UserComponentManager:init()
				end
			end 
		end
	end
end
