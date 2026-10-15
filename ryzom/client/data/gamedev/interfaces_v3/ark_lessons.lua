if game == nil then
	game= {}
end

if ArkLessons == nil then
	ArkLessons = {}

	ArkLessons.Callbacks = {}
	ArkLessons.Callbacks["accept"] = {}
	ArkLessons.Callbacks["reward"] = {}
	ArkLessons.Callbacks["step"] = {}
end

if ArkLessons.CurrentStep == nil then ArkLessons.CurrentStep = {} end
if ArkLessons.UsedWindow == nil then ArkLessons.UsedWindow = {} end
if ArkLessons.RevealStep == nil then ArkLessons.RevealStep = {} end
if ArkLessons.OnDraw == nil then ArkLessons.OnDraw = {} end

if ArkLessons.RevealMoves == nil then
	ArkLessons.RevealMoves = {}
	ArkLessons.RevealTooltips = {}
end

function ArkLessons:Callback(event, id, args)

	if ArkLessons.Callbacks and ArkLessons.Callbacks[event] and ArkLessons.Callbacks[event][id] then
		ArkLessons.Callbacks[event][id](args)
	end
end

game.ArkLessonUsedWindowUrl = "https://app.ryzom.com/app_arcc/index.php?action=mLesson_Run&script="

webig.urls_to_check = {}


function ArkOpenLesson(id, require_rpitem)
	debug(require_rpitem)
	if id ~= 0 and id ~= nil then
		local win = getUI("ui:interface:web_transaction_lessons")
		if win then
			win:find("html"):browse(game.ArkLessonUsedWindowUrl..id)
		else
			getUI("ui:interface:web_transactions"):find("html"):browse(game.ArkLessonUsedWindowUrl..id)
		end
	end
end

function ArkRevealLesson(id, i, total)
	if i == game.ArkLessonRevealStep[id] then
		game.ArkLessonRevealStep[id] = game.ArkLessonRevealStep[id] + 1
		game:ArkLessonCallback("step", id, i)
		game:ArkRevealLessonInfos(id, i, total)
		if i == total then
			game:ArkAcceptLesson()
		end
	end
end

function game:ArkRevealLessonInfos(id, i, total)
	local ui = getUI("ui:interface:ArkLessonWin"..tostring(id))
	if ui ~= nil  then
		local html = ui:find("html")
		html:showDiv("enabled_"..tostring(i), false)
		html:showDiv("disabled_"..tostring(i), false)
		html:showDiv("current_"..tostring(i), true)
		if i > 1 then
			if i ~= total+1 then
				html:showDiv("current_"..tostring(i-1), false)
				html:showDiv("enabled_"..tostring(i-1), true)
			end
		end
		if game.ArkLessonRevealCaps and game.ArkLessonRevealCaps[id] then
			if total > i then
				setCap(game.ArkLessonRevealCaps[id], "p", math.floor((100*i)/total), tostring(i).." / "..tostring(total))
			else
				setCap(game.ArkLessonRevealCaps[id], "p", 100, "")
			end
		end
	end
end


function ArkLessonUpdateHtml(win, scriptid, title, progression, started, finished, requirement, reward)
	win = getUI(win)
	win = win:find("div_lesson_"..scriptid..":html")
	local top = ""
	if requirement ~= [[]] then
		requirement = [[<tr><td height="20px" style="font-size: 11px; color: pink">]]..ArkLessons.NeedRequirement..[[</td>]]
	else
		requirement = ""
		top = [[<tr><td height="5px"></td></tr>]]
	end

	local progressionHtml = "<td><table><tr><td><table style=\'background-color: black;\'><tr><td></td></tr></table></td></tr></table></td>"
	local height = "50"
	if progression then
		if requirement ~= "" then
			height = "12"
		else
			height = "25"
		end
		pogressionHtml = "<tr><td height=\'12px\' align=\'left\' >"..progression.."</td></tr>"
	end

	local color = "AAA"
	if started then
		if finished then
			color = "FFFFFF"
		else
			color = "FFDD4AFF"
		end
	end

	win:renderHtml([[
		<td height="60px">
		<table cellspacing="0" cellpadding="0">
		<tr>
			<td width="2px"></td>
			<td width="407px" align="left" valign="top">
				<table width="407px" cellspacing="0" cellpadding="0">
					]]..top..[[
					<tr>
						<td height="]]..height..[[px" valign="top"><strong style="color: #]]..color..[[">]]..title..[[</strong></td>
					</tr>
					]]..requirement..pogressionHtml..[[
				</table>
			</td>
			<td width="6px"></td>
			<td align="left" valign="top" height="80px" width="46px">]]..reward..[[</td>
		</tr>
		</table>
	</td>
	]])
end

function ArkLessons:Show(id, w, h, content, scriptid)
	local ui = getUI("ui:interface:"..id)
	-- deleteUI(framewin)
	-- local framewin = getUI("ui:interface:"..id)
	local center = false
	if ui == nil then
		createRootGroupInstance("webig_browser_no_header", id, {h=h, w=1024, global_color="false", bg_color="0 0 0 255", movable="0"})
		WebBrowser:addWindow(id, "", getUI("ui:interface:"..id))
		ui = getUI("ui:interface:"..id)
		center = true
	end

	ui.opened = true
	ui.openable = false
	ui.active = true
	ui.w = w
	ui.h = h
	ui.pop_min_w = w
	ui.pop_max_w = w
	ui.pop_min_h = h
	ui.pop_max_h = h

	local html = ui:find("html")
	html:renderHtml(content)

	if center then
		ui.x = math.floor((getUI("ui:interface").w - ui.w) / 2)
		ui.y = math.floor((getUI("ui:interface").h + ui.h) / 2)
	end

	setOnDraw(ui, "ArkLessons:onDraw("..tostring(scriptid)..")")
end

function ArkLessons:Minimize(id, h, w)
	local ui = getUI("ui:interface:ArkLessonWin"..tostring(id))
	if ui then
		ui.w = w
		ui.h = h
		ui.pop_min_w = w
		ui.pop_max_w = w
		ui.pop_min_h = h
		ui.pop_max_h = h
		webig:displayWait("ui:interface:ArkLessonWin"..tostring(id))
	end
	webig:checkUrl(ArkLessons.NextStepUrl[id]["mini"])
end


function ArkLessons:MiniShowSteps(id, i)
	getUI("ui:interface:web_transactions_lessons"):find("html"):browse(ArkLessons.NextStepUrl[id]["url"].."&mini_step="..tostring(i))
end


function ArkRevealLesson(id, i, total)
	if i == ArkLessons.CurrentStep[id] then
		ArkLessons.RevealStep[id] = ArkLessons.RevealStep[id] + 1
		ArkLessons:Callback("step", id, i)
		ArkLessons:ArkRevealLessonInfos(id, i, total)
		WebQueue:push(ArkLessons.NextStepUrl[id][i])
	end
end

function ArkLessons:ArkRevealLessonInfos(scriptid, i, total)

	local ui = getUI("ui:interface:ArkLessonWin"..tostring(scriptid))
	if ui ~= nil  then
		local html = ui:find("html")

		html:showDiv("disabled_"..tostring(i), false)
		html:showDiv("current2_"..tostring(i), false)
		html:showDiv("enabled_"..tostring(i), false)
		html:showDiv("current_"..tostring(i), true)

		if ArkLessons.OnDraw[scriptid] == nil then ArkLessons.OnDraw[scriptid] = {}	end
		if ArkLessons.OnDraw[scriptid][i] == nil or ArkLessons.OnDraw[scriptid][i][4] == nil or ArkLessons.OnDraw[scriptid][i][4] < 1 then
			ArkLessons.OnDraw[scriptid][i] = {"blink", nltime.getLocalTime(), 1, 4}
		end
		if i > 1 and i ~= total+1 then
			html:showDiv("current_"..tostring(i-1), false)
			html:showDiv("enabled_"..tostring(i-1), true)
			ArkLessons.OnDraw[scriptid][i-1] = nil
		end

		if ArkLessons.RevealCaps and ArkLessons.RevealCaps[scriptid] then
			if total > i then
				setCap(ArkLessons.RevealCaps[scriptid], "p", math.floor((100*i)/total), tostring(i).." / "..tostring(total))
			else
				setCap(ArkLessons.RevealCaps[scriptid], "p", 100, "")
			end
		end
	end
end

function openArkLessonScript(i, script_id, url)
	if i == ArkLessons.RevealStep[script_id] then
		getUI("ui:interface:ArkLessonWin"..tostring(script_id)).active = false
		WebQueue:push(url)
	end
end

function readArkLesson(i, script_id, url)
	if ArkLessons.LockedArkReadlesson then
		return
	end

	if i == ArkLessons.RevealStep[script_id] then
		ArkLessons.LockedArkReadlesson = true
		ArkLessons:ArkRevealLessonInfos(script_id, i, 8)
		WebQueue:push(url)
	end
end

function ArkLessons:onDraw(scriptid)
	if ArkLessons.OnDraw[scriptid] ~= nil then
		for id, command in pairs(ArkLessons.OnDraw[scriptid]) do
			if command == "rotate" then
				doRotateLessonQuestion(scriptid, id)
			elseif command[1] == "blink" then
				if command[4] > 0 and (nltime.getLocalTime() - command[2]) > 300 then
					ui = getUI("ui:interface:ArkLessonWin"..tostring(scriptid))
					local html = ui:find("html")
					if command[3] == 1 then
						ArkLessons.OnDraw[scriptid][id][3] = 2
						html:showDiv("current_"..tostring(id), false)
						html:showDiv("current2_"..tostring(id), true)
					else
						ArkLessons.OnDraw[scriptid][id][3] = 1
						html:showDiv("current_"..tostring(id), true)
						html:showDiv("current2_"..tostring(id), false)
					end
					ArkLessons.OnDraw[scriptid][id][2] = nltime.getLocalTime()
					ArkLessons.OnDraw[scriptid][id][4] = ArkLessons.OnDraw[scriptid][id][4] - 1
					if ArkLessons.OnDraw[scriptid][id][4] <= 0 then
						ArkLessons.LockedArkReadlesson = nil
					end
				end
			end
		end
	end
end


function ArkLessons:openLessonBox(continent, name, text)
	if game.spawnShapesByZone == nil then return end

	local box =  game.spawnShapesByZone[continent]
	if box then
		box = box[name]
		if box then
			if box[9] then
				deleteShape(box[9])
			end
			if box[8] and box[8]["action"] then
				box[8]["action"]= text
				game.spawnShapesByZone[continent][name][9] = SceneEditor:spawnShape(box[12]..".shape", box[2], box[3], box[4], box[5], box[6], box[7], box[8])
			end
		end
	end
end

function ArkLessons:finishLessonBox(continent, name, text)
	print_r(game.spawnShapesByZone)

	if game.spawnShapesByZone == nil then return end

	local box =  game.spawnShapesByZone[continent]
	if box then
		box = box[name]
		if box then
			if box[9] then
				deleteShape(box[9])
			end
			if box[10] then
				deleteShape(box[10])
			end

			if box[8] and box[8]["action"] then
				box[8]["action"] = text
				game.spawnShapesByZone[continent][name][9] = SceneEditor:spawnShape(box[12]..".shape", box[2], box[3], box[4], box[5], box[6], box[7], box[8])
				game.spawnShapesByZone[continent][name][10] = nil
			end
		end
	end
end

function ArkLessons:init()
	if webig.check_started == nil then
		webig.urls_to_check = {}
		webig.check_started = true
		addOnDbChange(getUI("ui:interface:web_transactions_lessons"), "@UI:VARIABLES:CURRENT_SERVER_TICK", "webig:checkUrlLoop()")
	end
end


function rotateLessonQuestion(scriptid, id)
	local framewin = getUI("ui:interface:ArkLessonWin"..scriptid)
	local empty = framewin:find("empty_"..id)
	if not empty then return end
	local sel = empty:find("selection")
	local scene = empty:find("scene")

	if ArkLessons.RevealStep[scriptid] == id then
		local moves={-1, 0.5, 0, 0.5, 1}
		local moves2={-1, 1}
		if ArkLessons.RevealMoves[scriptid] == nil then
			ArkLessons.RevealMoves[scriptid] = {}
		end
		ArkLessons.RevealMoves[scriptid][id] = {moves[math.random(#moves)], moves[math.random(#moves)], moves2[math.random(#moves2)]}
		if ArkLessons.OnDraw[scriptid] == nil then ArkLessons.OnDraw[scriptid] = {}	end
		ArkLessons.OnDraw[scriptid][id] = "rotate"
		sel.tooltip = getUCtf8(ArkLessons.RevealTooltips[scriptid][id])
		scene:getElement("camera#0").posy = 24.5
	else
		sel.col_over = "0 0 0 0"
		sel.col_pushed = "0 0 0 0"
		sel.tooltip = ""
		scene:getElement("camera#0").posy = 23.5
	end
end


function doRotateLessonQuestion(scriptid, id)
	local framewin = getUI("ui:interface:ArkLessonWin"..scriptid)
	local empty = framewin:find("empty_"..id)
	if not empty then return end
	local x,y = getMousePos()
	empty:find("selection").col_normal = "200 200 255 50"
	if empty ~= nil and x > empty.x_real and x < empty.x_real + empty.w and  y > empty.y_real and y < empty.y_real + empty.h then
		local scene = empty:find("scene")
		empty:find("selection").col_over = "255 200 255 50"
		scene:getElement("shape#0").rotx = scene:getElement("shape#0").rotx + ArkLessons.RevealMoves[scriptid][id][1]
		scene:getElement("shape#0").roty = scene:getElement("shape#0").roty + ArkLessons.RevealMoves[scriptid][id][2]
		scene:getElement("shape#0").rotz = scene:getElement("shape#0").rotz + ArkLessons.RevealMoves[scriptid][id][3]
	end
end

function webig:checkUrlLoop()
	if webig.urls_to_check[1] ~= nil then
		if webig.urls_to_check[1][2] == "done" then
			table.remove(self.urls_to_check, 1)
			return
		end

		if webig.urls_to_check[1][2] == "wait" then
			local url = webig.urls_to_check[1][1]
			webig.urls_to_check[1][2] = "run"
			getUI("ui:interface:web_transactions_lessons"):find("html"):browse(url)
		end
	end
end


function webig:checkUrl(url)
	table.insert(webig.urls_to_check, {url, "wait"})
end

function webig:luaWebLoaded()
	if webig.urls_to_check[1] then
		webig.urls_to_check[1][2] = "done"
	end
end

function webig:displayWait(ui_name)
	local ui = getUI(ui_name)
	if ui then
		ui.active = true
		ui:find("html"):renderHtml([[
		<style>
			* { color: white; background-color: #000000F0 }
		</style>
		<table><tr>
			<td>
				<div class="ryzom-ui-grouptemplate" id="ark_lesson_please_wait" style="template:webig_inv_item_shape;id:ark_lesson_please_wait;w:]]..tostring(ui.w)..[[;h:]]..tostring(ui.h)..[[;shape:ge_mission_chrono.ps;shape_pos:0 0 -1;rotz:0;roty:0;dist:0;fontsize:16;text:uiPleaseWait;text_y:]]..tostring(-(ui.h/5.7))..[[;text_color:255 200 150 255;color_over:0 165 255 0;fx:env_aqua2.ps;"></div>
			</td>
		</tr></table>
		<body>]])
	end
end

function webig:openWin(ui_name)
	local ui = getUI(ui_name)
	if ui then
		ui.active = true
	end
end

