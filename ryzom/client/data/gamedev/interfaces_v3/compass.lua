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
	local win = getUI("ui:interface:app2453")
	if win ~= nil and win.active == true then
		win.active = false
	else
		AppZone:launchApp(2453)
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
RYZOM_COMPASS_VERSION = 335
