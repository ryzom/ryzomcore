-- go to the home page of the ring window

function browseRingWindowHomePage()
	local adminWindow = getUI("ui:interface:r2ed_ring_window")
	if adminWindow then
		adminWindow:find("ring_page"):browse(getMainPageURL() .. "?ingame=1&charSlot=" .. getCharSlot())
	end
end

local firstBrowseDone = false
function onRingWindowShown()
	if not firstBrowseDone then
		browseRingWindowHomePage()
		firstBrowseDone = true
	end
end

-- VERSION --
RYZOM_RING_WINDOW_VERSION = 324
