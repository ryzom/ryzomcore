-- go to the home page of the ring window: the start page of the Ring web pages, for the character in game.
-- The window is r2ed_web_admin of help.xml (the r2ed_ring_window this file was written for is no more).

function browseRingWindowHomePage()	
	local adminWindow = getUI("ui:interface:r2ed_web_admin")
	if adminWindow then
		adminWindow:find("admin_web_page"):browse(getMainPageURL() .. "?ingame=1&charSlot=" .. getCharSlot())
		-- adminWindow:find("admin_web_page"):browse("http://r2linux03/ring/web_start.php?ingame=1")
	end
end

-- called when the window is shown (on_active of r2ed_web_admin): the first time, it gets its home page
local firstBrowseDone = false
function onRingWindowShown()
	if not firstBrowseDone then
		browseRingWindowHomePage()
		firstBrowseDone = true
	end
end
