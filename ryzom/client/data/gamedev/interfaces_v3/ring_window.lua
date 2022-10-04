-- go to the home page of the ring window

function browseRingWindowHomePage()	
	local adminWindow = getUI("ui:interface:r2ed_ring_window")
	if adminWindow then
		adminWindow:find("ring_page"):browse(getMainPageURL() .. "?ingame=1&charSlot=" .. getCharSlot())
		-- adminWindow:find("admin_web_page"):browse("http://r2linux03/ring/web_start.php?ingame=1")
	end
end

local firstBrowseDone = false
function onRingWindowShown()
	if not firstBrowseDone then
		browseRingWindowHomePage()
		firstBrowseDone = true
	end
end
