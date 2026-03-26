-- In this file we define functions that serves for compass window

if (game==nil) then
	game= {};
end

if DynE == nil then
	DynE = {}
	DynE.lastWinUpdate = 0
end

if DynE.otherMapPoint == nil then
	DynE.otherMapPoints = {}
end

function DynE:AddOtherMapPoints()
	if DynE.otherMapPoints ~= nil then
		for k, v in pairs(DynE.otherMapPoints) do
			for _, point in pairs(v) do
				addLandMark(point[1], point[2], point[3], point[4],"","","","","","")
			end
		end
	end
end


function game:areInSilan()
	polygons = {{8128,-10208}, {11368,-10208}, {11392,-12392}, {8096,-12368}}
	local x,y = getPlayerPos()
	for i = 1, 4 do
		local z = point_inside_poly(x, y, polygons)
		if z == true then
			return true
		end
	end
	return false
end

function game:updateCompass()
	if game.InGameDbInitialized == false then
		return
	end

	local calendar_win = getUI("ui:interface:compass:calendar")
	local format_date = "%d %m %Y %H:%M"
	format_date = findReplaceAll(format_date, "%m", i18n.get("uiMonth"..getTimestampHuman("%m")):toUtf8())
	calendar_win.tooltip = getUCtf8(getTimestampHuman(format_date))
end


function game:displayDynE()
	local win = getUI("ui:interface:ark_dyne_window")
	if win ~= nil and win.active == true then
		win.active = false
	else
		if not win then
			win = createRootGroupInstance("webig_browser", "ark_dyne_window", {x = 0, y = 0, w = 370, h = 600})
			local ui = getUI("ui:interface")
			win.x = math.floor((ui.w - 400) / 2)
			win.y = math.floor((ui.h + 600) / 2)
			win:find("html"):renderHtml("<i>uiLoading</i>")
		end
		win.active = true
		win.opened = true
		win:find("browse_redo").active = false
		win:find("browse_redo").active = false
		win:find("browse_undo").active = false
		win:find("browse_home").active = false
		getUI("ui:interface:web_lua_action"):find("html"):browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=12517&command=reset_all")
	end
end

function game:displayCalendar()
	local win = getUI("ui:interface:webig")
	if win ~= nil and win.active == true then
		win.active = false
	else
		win.active = true
	end
end

setOnDraw(getUI("ui:interface:compass"), "game:updateCompass()")

-- VERSION --
FILE_COMPASS_VERSION = 183
