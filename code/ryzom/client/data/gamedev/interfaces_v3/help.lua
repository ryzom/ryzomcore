-- In this file we define functions that serves for help windows

------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (help==nil) then
	help= {};
end

------------------------------------------------------------------------------------------------------------
-- 
function help:closeCSBrowserHeader()
	local ui = getUI('ui:interface:cs_browser');
	
	-- save size
	ui_cs_browser_h = ui.h;
	ui_cs_browser_w = ui.w;
	
	-- reduce window size
	ui.pop_min_h = 32;
	ui.h = 0;
	ui.w = 216;
end

------------------------------------------------------------------------------------------------------------
-- 
function help:openCSBrowserHeader()
	local ui = getUI('ui:interface:cs_browser');
	ui.pop_min_h = 96;

	-- set size from saved values
	if (ui_cs_browser_h ~= nil) then
		ui.h = ui_cs_browser_h;
	end
	
	 if (ui_cs_browser_w ~= nil) then
		ui.w = ui_cs_browser_w;
	end
end