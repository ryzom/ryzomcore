-- The Ring window: r2ed_web_admin of help.xml (the r2ed_ring_window this file was written for is no more).

local ringWindowId = "ui:interface:r2ed_web_admin"

-- go to the home page of the ring window: the start page of the Ring web pages, for the character in game
-- (the address /openRingWindow browses)
function browseRingWindowHomePage()
	local adminWindow = getUI(ringWindowId)
	if adminWindow then
		adminWindow:find("admin_web_page"):browse(getMainPageURL() .. "?charSlot=" .. getCharSlot())
		-- adminWindow:find("admin_web_page"):browse("http://r2linux03/ring/web_start.php?ingame=1")
	end
end

-- true when the window has been asked for no page yet
local function ringWindowHasNoPage()
	local adminWindow = getUI(ringWindowId)
	return adminWindow ~= nil and adminWindow:find("admin_web_page").url == ""
end

-- Called when the window is made active (on_active of r2ed_web_admin). That is not yet the user who
-- opens it: the interface makes the windows of a saved desktop active, and hides them again, each time it
-- loads or saves the desktops. A page asked for then goes out although nobody sees the window.
-- So nothing is asked for here. A window that has no page gets its start page when it is drawn, which
-- is when the user sees it (onRingWindowDraw).
function onRingWindowShown()
	if ringWindowHasNoPage() then
		setOnDraw(getUI(ringWindowId), "onRingWindowDraw()")
	end
end

-- Called each time the window is drawn after it was made active without a page, until it has asked for
-- its start page: once.
-- The first frames of the game are drawn before the shard has sent its id, and with it the name of its
-- web server (getShardId is 0 until then). The login cookie is sent to that server only: a start page
-- asked for earlier answers that the authentication failed, and that answer stays in the window. The
-- page is asked for at the first draw after the id, also in a window that a saved desktop brings back
-- open at the start of the game.
function onRingWindowDraw()
	if getShardId() == 0 then
		return
	end
	setOnDraw(getUI(ringWindowId), "")
	-- a page it was asked to browse meanwhile (/openRingWindow, /browseRingAdmin) stays
	if ringWindowHasNoPage() then
		browseRingWindowHomePage()
	end
end
