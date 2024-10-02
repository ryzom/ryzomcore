if Ark == nil then
	Ark = {}
end

function print_r(arr, indentLevel)
	local str = ""
	local indentStr = "#"

	if(indentLevel == nil) then
		debug(print_r(arr, 0))
		return
	end

	for i = 0, indentLevel do
		indentStr = indentStr.."    "
	end

	for index,value in pairs(arr) do
		if type(value) == "table" then
			str = str..indentStr..index..": \n"..print_r(value, (indentLevel + 1))
		else
			str = str..indentStr..index..": "..value.."\n"
		end
	end
	return str
end

function tablelength(T)
	if T == nil then
		return 0
	end
	local count = 0
	for _ in pairs(T) do count = count + 1 end
	return count
end

function table_compare(t1, t2)
	local ty1 = type(t1)
	local ty2 = type(t2)
	if ty1 ~= ty2 then return false end
	-- non-table types can be directly compared
	if ty1 ~= 'table' and ty2 ~= 'table' then return t1 == t2 end
	-- as well as tables which have the metamethod __eq
	local mt = getmetatable(t1)
	if not ignore_mt and mt and mt.__eq then return t1 == t2 end

	for k1, v1 in pairs(t1) do
		local v2 = t2[k1]
		if v2 == nil or v1 ~= v2 then return false end
	end
	for k2,v2 in pairs(t2) do
		local v1 = t1[k2]
		if v1 == nil or v2 ~= v1 then return false end
	end
	return true
end

function openPageInWebIg(url)
	getUI("ui:interface:webig:content:html"):browse(url)
	getUI("ui:interface:webig").active = true
	getUI("ui:interface:webig").opened = true
end

function broadcast(text, t)
	if t == nil then
		t = "AMB"
	end
	local message  = ucstring()
	text = message:fromUtf8(tostring(text))
	displaySystemInfo(message, t)
end

function fixUrl(url)
	s, n = string.gsub(url, "{amp}", "&")
	return s
end

function openArkScript(id, winid, extra)
	if winid == nil then
		winid = "ui:interface:web_transactions"
	end
	if extra == nil then
		extra = ""
	end
	getUI(winid):find("html"):browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script="..id.."&command=reset_all&"..extra)
end

function openMissionArkScript(winid, id)
	WebBrowser:openWindow(winid, "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script="..id.."&command=reset_all")
end

function getArkScript(id, selected)
	if selected ~= nil then
		url = "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script="..id.."&_hideWindow=1&command=reset_all&selected="..tostring(selected)
	else
		url = "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script="..id.."&_hideWindow=1&command=reset_all"
	end
	return url
end

function ArkUpdateItemVariant(db, id, vp, val, skin)
	local item
	if db then
		runAH(nil, "set", "dblink=UI:TEMP:ARK_RYWARD_CHAR3D_"..db..":"..vp.."|value="..val)
		item = getUI("ui:interface:encyclopedia:content:htmlC"):find("div_item_"..db)
	else
		item = getUI("ui:interface:encyclopedia:content:htmlC"):find(vp)
		local scene = getUI("ui:interface:encyclopedia:content:htmlC"):find(vp):find("scene")
		local shape = scene:getElement("shape#0")
		if skin and skin ~= "" then
			shape.name = skin
		end
		shape.textures = val
		for token in string.gmatch(vp, "[^_]*") do
			db = tonumber(token)
		end
	end

	item:find("resell").active = false
	item:find("apply").active = false
	item:find("v").hardtext = id
	item:find("infos").hardtext = i18n.get("uiPhraseTotal"):toUtf8().." = 0"

	if db ~= nil then
		if ArkItemVariantQuantities[db] ~= nil and ArkItemVariantQuantities[db][id] ~= nil then
			ArkSelectedItemVariant[db] = id
			if ArkItemVariantQuantities[db][id] > 0 then
				if item:find("apply") ~= nil then
					item:find("apply").active = true
				end
				if item:find("infos") ~= nil then
					item:find("infos").hardtext = i18n.get("uiPhraseTotal"):toUtf8().." = "..tostring(ArkItemVariantQuantities[db][id])
				end
			end
			if ArkItemVariantQuantities[db][id] >= ArkItemVariantMinToSell[db] then
				if item:find("resell") ~= nil then
					item:find("resell").active = true
				end
			end
		end
	end
end



--------------------------------------------------------------------------------
--- ARK DYNE ---
--------------------------------------------------------------------------------

if DynE == nil then
	DynE = {}
	DynE.lastWinUpdate = 0
	DynE.otherMapPoints = {}
end

function DynE:OpenStoryline(season, episode)
	ArkMissionCatalog:OpenCat("storyline", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=8840&season="..season.."&episode="..tostring(episode+1))
	getUI("ui:interface:encyclopedia").opened = true
	getUI("ui:interface:encyclopedia").active = true
end

function DynE:SetMap(name, x, y, desc)
	local link = getUI(DynEWindowId):find(name)
	if link ~= nil then
		link = link:find(":b")
		link.onover = "lua"
		link.params_over = "DynE:OpenMap("..x..","..y..")"
	end
end

function DynE:OpenMap(x, y)
	local html = [[<img src="https://api.bmsite.net/maps/static?flags=0&language=]]..tostring(getClientCfg("LanguageCode"))..[[&markers=icon:lm_marker|]]..x..[[,]]..y..[[&maptype=atys&size=380x90"/>]]..tostring(i18n.get("uiMovingTargetWarning"))
	runAH(nil, "enter_modal", "group=ui:interface:webig_html_modal")
	local whm = getUI("ui:interface:webig_html_modal")
	whm.child_resize_h=false
	whm_html = getUI("ui:interface:webig_html_modal:html")
	if whm_html ~= nil then
		whm.w = 410
		whm.h = 150
		whm_html:renderHtml(html)
	end
end

function DynE:RunMission(script, posX, posY)
	local html = getUI("ui:interface:web_transactions"):find("html")
	html:browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script="..script.."&command=reset_all&PosX="..posX.."&PosY="..posY.."&FlagName=Go+to".."&MissionText=Go")
end

function DynE:OpenHelp(img)
	local html = getUI("ui:interface:web_transactions"):find("html")
	html:browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9600&command=reset_all&img="..img)
end

function DynE:UpdateMapWindow()
	if (nltime.getLocalTime() - DynE.lastWinUpdate) > 10000 then
		DynE.lastWinUpdate = nltime.getLocalTime()
		openUrlInBg("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=12517&command=reset_all")
	end
end


--------------------------------------------------------------------------------
--- ARK MISSION CATALOG ---
--------------------------------------------------------------------------------
if ArkMissionCatalog == nil then
	ArkMissionCatalog = {
		window = nil,
		window_id = "ui:interface:encyclopedia"
	}
end

function openRyward(folder, event)
	folder = "f"..tostring(folder)
	event = tostring(event)
	if not (rykea_selected_path_B == folder and rykea_selected_path_C == event and getUI("ui:interface:encyclopedia").active == true) then
		ArkMissionCatalog:OpenCat("rykea","https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=10741&command=reset_all")
	end
	getUI(ArkMissionCatalog.window_id..":content:htmlB"):browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=10741&command=reset_all&pathB="..folder.."&pathC="..event)
	getUI(ArkMissionCatalog.window_id).active = true
end

function openRykea(folder, event)
	folder = "f"..tostring(folder)
	event = tostring(event)
	if not (rykea_selected_path_B == folder and rykea_selected_path_C == event and getUI("ui:interface:encyclopedia").active == true) then
		ArkMissionCatalog:OpenCat("rykea","https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=11938&command=reset_all")
	end
	getUI(ArkMissionCatalog.window_id..":content:htmlB"):browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=11938&command=reset_all&pathB="..folder.."&pathC="..event)
	getUI(ArkMissionCatalog.window_id).active = true
end


function ArkMissionCatalog:OpenWindow(urlA, urlB, dont_active)
	local winframe = getUI(ArkMissionCatalog.window_id)
	winframe.opened=true
	if dont_active ~= true then
		winframe.active=true
	end

	ArkMissionCatalog.window = winframe

	getUI("ui:interface:encyclopedia:content:htmlA"):browse(urlA)

	local htmlb = getUI("ui:interface:encyclopedia:content:htmlB")
	htmlb.home = urlB
	htmlb:browse("home")
end

function ArkMissionCatalog:OpenCat(cat, url)
	local ency = getUI("ui:interface:encyclopedia")
	setOnDraw(ency, "")
	ArkMissionCatalog.cat = cat
	local htmlA = getUI(ArkMissionCatalog.window_id..":content:htmlA")
	local htmlB = getUI(ArkMissionCatalog.window_id..":content:htmlB")
	local htmlC = getUI(ArkMissionCatalog.window_id..":content:htmlC")
	if cat ~= "storyline" and cat ~= "daily" and cat ~= "achv" and cat ~= "puzzle" then
		ArkMissionCatalog.posxB = 180
		ArkMissionCatalog.widthB = 240
		ArkMissionCatalog.widthC = 530
		ArkMissionCatalog.posxC = 405

		htmlB.x = ArkMissionCatalog.posxB
		htmlB.w = ArkMissionCatalog.widthB
		htmlC.w = ArkMissionCatalog.widthC
		htmlC.x = ArkMissionCatalog.posxC
		htmlC.active = true
	else
		ArkMissionCatalog.widthB = 740
		htmlB.w = ArkMissionCatalog.widthB
		if htmlC then
			htmlC.active = false
		end
	end
	htmlB.home = url.."&continent="..getContinentSheet()
	htmlB:browse("home")
	setOnDraw(ency, "ArkMissionCatalog:autoResize()")
	if ArkMissionCatalog.continent ~= getContinentSheet() then
		ArkMissionCatalog.continent = getContinentSheet()
		htmlA:browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=8746&command=reset_all&no_html_header=1&ig=1")
	end
end

function ArkMissionCatalog:UpdateMissionTexts(win, id, text1, text2)
	local w = win:find("ark_mission_"..id)
	local text = ucstring()
	text:fromUtf8(text1)
	w:find("text1").uc_hardtext = text
	text:fromUtf8(text2)
	w:find("text2").uc_hardtext = text
end

function ArkMissionCatalog:startResize()
	local ency = getUI("ui:interface:encyclopedia")
	ency.w = 950
	ency.h = 700
	setOnDraw(ency, "ArkMissionCatalog:autoResize()")
end

function ArkMissionCatalog:autoResize()
	if ArkMissionCatalog.bypass_resize then
		ArkMissionCatalog.bypass_resize = false
		return
	end

	local ui = getUI(ArkMissionCatalog.window_id)
	local htmlA = getUI(ArkMissionCatalog.window_id..":content:htmlA")
	local htmlB = getUI(ArkMissionCatalog.window_id..":content:htmlB")
	local htmlC = getUI(ArkMissionCatalog.window_id..":content:htmlC")

	if htmlC == nil or htmlC.active == false then
		if ArkMissionCatalog.cat == "storyline" then
			if ui.w < 784 then
				local td30 = htmlB:find("storyline_content")
				if td30 ~= nil then
					td30.x = math.max(0, 200-784+ui.w)
					ArkMissionCatalog.need_restore_td30 = true
				end
			else
				if ArkMissionCatalog.need_restore_td30 then
					local td30 = htmlB:find("storyline_content")
					if td30 ~= nil then
						td30.x = 200
						ArkMissionCatalog.need_restore_td30 = false
					end
				end
			end
		end

		if ui.w < 950 then
			htmlA.w = math.max(68, 220-950+ui.w)
			htmlB.x = math.max(35, 190-950+ui.w)
			ArkMissionCatalog.need_restore = true
		else
			if ArkMissionCatalog.need_restore then
				htmlA.w = 220
				htmlB.x = 180
				ArkMissionCatalog.need_restore = false
			end
		end
	else
		htmlA.w = math.max(68, 220-950+ui.w)
		htmlB.x = math.max(35, ArkMissionCatalog.posxB-950+ui.w)
		htmlC.x = math.max(75, ui.w-htmlC.w-16)
		htmlB.w = math.max(55, htmlC.x-htmlB.x+16)
	end
end

function ArkMissionCatalog:showLegacyEncyclopedia(state)
	if state == 1 then
		getUI("ui:interface:legacy_encyclopedia").active=1
	else
		getUI("ui:interface:legacy_encyclopedia").active=0
	end
end


function ArkMissionCatalog:setup()
	debug("Define Mission Catalag url")
	if ArkMissionCatalog ~= nil then
		ArkMissionCatalog.posxB = 0
	end

	local urlA = "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=8746&command=reset_all&no_html_header=1&ig=1"
	getUI("ui:interface:encyclopedia:content:htmlA"):browse(urlA)

	ArkMissionCatalog:startResize()
	local continent = getContinentSheet()
	if continent == "newbieland.continent" then
		ArkMissionCatalog:OpenCat("academic","https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=10700&command=reset_all&no_html_header=1")
	else
		ArkMissionCatalog:OpenCat("storyline","https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=8840&command=reset_all&no_html_header=1&show_latest=1")
	end

	debug("Open ency")
	getUI("ui:interface:encyclopedia").opened = true;
end

function translateText(id, script, event, dst_lang)
	framewin = getUI("ui:interface:ark_translate_lesson", false)
	if framewin == nil then
		createRootGroupInstance("webig_browser", "ark_translate_lesson", {h=480, w=980})
		framewin = getUI("ui:interface:ark_translate_lesson", false)
	end

	framewin.opened = true
	framewin.active = true
	framewin.x = math.floor((getUI("ui:interface").w - framewin.w) / 2)
	framewin.y = math.floor((getUI("ui:interface").h + framewin.h) / 2)
	setTopWindow(framewin)
	if dst_lang then
		dst_lang = "&dst_lang="..dst_lang
	else
		dst_lang = ""
	end
	framewin:find("html"):browse("https://app.ryzom.com/app_arcc/index.php?action=mTrads_Edit&event="..tostring(event).."&trad_name="..tostring(id).."&reload="..script..dst_lang)
end


function setupArkUrls()
	debug("Setup Lm Events")
	local ui = getUI("ui:interface:map:content:map_content:lm_events:html")
	ui.home = "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=12517&command=reset_all&no_html_header=1&continent=&posx=$posx$&posy=$posy$"
	ui:browse("home")

	debug("Setup Lm Icons")
	ui = getUI("ui:interface:map:content:map_content:lm_dynicons:html")
	ui.home = "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=11158&command=reset_all&no_html_header=1"
	ui:browse("home")
	game.updateRpItemsUrl = "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=11488&command=reset_all"

	local ui = getUI("ui:interface:map")
	addOnDbChange(ui, "@UI:VARIABLES:CURRENT_SERVER_TICK", "DynE:UpdateMapWindow()")
end


--------------------------------------------------------------------------------
--- ARK EDITOR ---
--------------------------------------------------------------------------------

function ArkSwitchAdvEdition(prefix)
	if ArkSwitchAdvEditionSwitch then
		ArkSwitchAdvEditionSwitch = false
		getUICaller():showDiv("advEditionDiv"..prefix, false)
	else
		ArkSwitchAdvEditionSwitch = true
		getUICaller():showDiv("advEditionDiv"..prefix, true)
	end
end

function ArkShowStageDiv(name, state)
	getUICaller():showDiv(name, state)
end

function ArkSelectRyform(curwin, id, mod)
	local curwin = getUICaller().id
	local e = ArkGetStageEdit(curwin):find(id..":eb")
	e.input_string = mod
	ArkGetStageEdit(curwin):find("send:b"):runLeftClickAction()
end

function ArkSendForm(name)
	local ui = getUICaller().id
	ArkGetStageEdit(ui):find(name.."__command:eb").input_string = "reset"
	ArkGetStageEdit(ui):find("send:b"):runLeftClickAction()
end


function ArkGetStageEdit(curwin)
	local sid = string.split(curwin, ":")
	local eid = sid[#sid]
	table.remove(sid, #sid)
	local id = sid[1]
	for i = 2, #sid do
		id = id .. ":" .. sid[i]
	end
	return getUI(id):find(eid)
end

function ArkFindUI(name)
	local i = 0
	local ui = getUICaller()
	while true do
		if ui then
			local found = ui:find(name)
			if found ~= nil then
				return found
			else
				ui = ui.parent
			end
			i = i +1
			if i >= 100 then
				return nil
			end
		else
			return nil
		end
	end
end

function ArkOnSelectChanged(name)
	local text = ArkRyformV5[name][getUICaller().selection+1]
	ArkFindUI(name..":eb").input_string = text
end

function ArkV5GetSelectEntity(prefix, input)
	name = getTargetName()
	if name == "" then
		name = getTargetTitle()
	end
	if name ~= "" and name ~= nil then
		webig:openUrlInBg("https://app.ryzom.com/app_arcc/scripts/get_bot.php?action=ark_editor&ui="..encodeURLParam(getUICaller().id).."&prefix="..tostring(prefix).."&input="..tostring(input).."&bot="..encodeURLParam(base64.encode(name)))
	end
end

function getVpx(vpx)
	while string.len(vpx) < 16 do
		vpx = "0"..vpx
	end

	local vpx1 = string.format("%06d", tonumber(vpx:sub(1, string.len(vpx)-2), 16)*256)
	local vpx2 = string.format("%06d", tonumber(vpx:sub(string.len(vpx)-1, string.len(vpx)), 16)+tonumber(vpx1:sub(string.len(vpx1)-5, string.len(vpx1))))
	local nvpx = vpx1:sub(1, string.len(vpx1)-6)..vpx2:sub(string.len(vpx2)-5, string.len(vpx2))
	return nvpx
end

function openUrlInBg(url)
	getUI("ui:interface:web_transactions"):find("html"):browse(fixUrl(url))
end

function arkWindowCloseMe(url)
	local framewin = getUICaller()
	framewin.parent.active = false
	if url then
		framewin:browse(fixUrl(url))
	end
end

function game:displayWhatsUp()
	getUI("ui:interface:npc_web_browser:content:html"):browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=11154&command=reset_all")
end

-- S2E1 STUFF --


if S2E1 == nil then
	S2E1 = {}
end

runAH(nil, "stop_event_music", "")

S2E1.WindowOpened = false
S2E1.texts = {}
S2E1.texts["de"] = {}
S2E1.texts["en"] = {}
S2E1.texts["es"] = {}
S2E1.texts["fr"] = {}
S2E1.texts["ru"] = {}


S2E1.texts["de"][1] = {"Aus der Rinde kommt ein leichtes Knirschen.",
"Die leisen Knirsch-Geräusche fangen an dich zu nerven.",
"Die Rinde scheint sich unter deinen Füßen zu winden...",
"Du hast Schwierigkeiten geradeaus zu gehen; vielleicht war die Shooki nicht sehr frisch.",
"Ein Rindenbeben muss irgendwo weit weg von hier passiert sein...."}
S2E1.texts["de"][2] = {"Ein dumpfes Schlaggeräusch kommt von der Rinde.",
"Plötzliche Beben erschüttern die Rinde und die Vegetation um dich herum.",
"Sie bringen dich aus dem Gleichgewicht...",
"Man muss aufpassen, dass man nicht hinfällt.",
"Das Beben lässt nach, die Rinde kommt wieder zur Ruhe."}
S2E1.texts["de"][3] = {"Aus der Rinde steigt ein dumpfes Rumoren auf.",
"Eine erste Erschütterung bringt dich ins Wanken.",
"Die, die folgen, sind heftig...",
"Es fällt dir schwer aufzustehen.",
"Die Erschütterungen werden schwächer, die Rinde kommt wieder zur Ruhe."}
S2E1.texts["de"][4] = {"Aus der Rinde steigt ein Grollen auf.",
"Plötzlich gibt die Rinde unter deinen Füßen nach und du fällst zu Boden.",
"Du stehst auf und versuchst zu fliehen...",
"aber fällst gleich wieder hin.",
"Die Erschütterungen werden schwächer..."}
S2E1.texts["de"][5] = {"Aus der Rinde dringt ein rumpelndes Geräusch empor.",
"Plötzlich gibt sie unter deinen Füßen nach und du fällst hin.",
"Du stehst wieder auf."}


S2E1.texts["en"][1] = {"A slight squeak raise from the Bark.",
"The little squeals begin to irk you.",
"The Bark seems to wriggle under your feet…",
"You have trouble walking straight; maybe the Shooki wasn't very fresh.",
"A barkquake must have happened somewhere far from here…"}
S2E1.texts["en"][2] = {"A thumping sound rises from the Bark.",
"Jolts suddenly shake the Bark and the vegetation around you.",
"They throw you off balance…",
"You have to pay attention not to fall down.",
"The shaking subsides, the Bark reverts to a stable state."}
S2E1.texts["en"][3] = {"A thudding murmur raises from the Bark.",
"A first jolt makes you waver.",
"Those following are violent…",
"You're having a hard time standing up.",
"The jolts become weaker, the Bark becomes stable again."}
S2E1.texts["en"][4] = {"A rumble comes out of the Bark.",
"The Bark suddenly gives way under your feet and you fall to the ground.",
"You get up, trying to get away…",
"But fall back immediately.",
"The jolts are beginning to weaken…"}
S2E1.texts["en"][5] = {"Out of the Bark comes a rumbling sound.",
"Suddenly it gives way under your feet and you fall to the ground.",
"You get up again."}


S2E1.texts["es"][1] = {"Un ligero chirrido de la corteza.",
"Los pequeños chillidos comienzan a molestarte.",
"La corteza parece retorcerse bajo tus pies...",
"Tienes problemas para caminar recto; tal vez el Shooki no era muy fresco.",
"Un terremoto debe haber ocurrido en algún lugar lejos de aquí..."}
S2E1.texts["es"][2] = {"Un sonido de golpeteo se eleva desde la corteza.",
"Las sacudidas repentinamente sacuden la corteza y la vegetación a su alrededor.",
"Te desequilibran...",
"Tienes que prestar atención para no caerte.",
"El temblor disminuye, la corteza vuelve a un estado estable."}
S2E1.texts["es"][3] = {"Un estruendoso murmullo se eleva desde la corteza.",
"La primera sacudida te hace temblar.",
"Los siguientes son violentos...",
"Te cuesta mucho trabajo estar de pie.",
"Las sacudidas se debilitan, la corteza se vuelve estable de nuevo."}
S2E1.texts["es"][4] = {"Un estruendo sale de la corteza.",
"La corteza de repente cede bajo tus pies y caes al suelo.",
"Te levantas, tratando de escapar...",
"Pero retroceda inmediatamente.",
"Las sacudidas están empezando a debilitarse..."}
S2E1.texts["es"][5] = {"De la Corteza sale un sonido estruendoso.",
"De repente cede bajo tus pies y caes al suelo.",
"Te levantas de nuevo."}


S2E1.texts["fr"][1] = {"Un léger grincement s'échappe de l'Écorce.",
"Les petits crissements commencent à vous agacer.",
"L'Écorce semble frétiller sous vos pieds…",
"Vous avez du mal à marcher droit ; peut-être la Shooki n'était-elle pas très fraîche.",
"Un tremblement d'écorce a dû se produire quelque part, loin d'ici…"}
S2E1.texts["fr"][2] = {"Un bruit sourd monte de l'Écorce.",
"Des secousses agitent soudain l'Écorce et la végétation autour de vous.",
"Elles vous déséquilibrent…",
"Vous devez faire attention pour ne pas tomber.",
"Le tremblement s'apaise, l'Écorce redevient stable."}
S2E1.texts["fr"][3] = {"Une sourde rumeur monte de l'Écorce.",
"Une première secousse vous fait vaciller.",
"Celles qui suivent sont violentes…",
"Vous avez beaucoup de mal à tenir debout.",
"Les secousses deviennent plus faibles, l'Écorce redevient stable."}
S2E1.texts["fr"][4] = {"Un grondement monte de l'Écorce.",
"L'Écorce se dérobe soudain sous vos pieds et vous tombez sur le sol.",
"Vous vous relevez, cherchant à vous éloigner…",
"Mais retombez aussitôt.",
"Les secousses commencent à s'affaiblir…"}
S2E1.texts["fr"][5] = {"De l'Écorce s'élève un grondement.",
"Elle se dérobe soudain sous vos pieds et vous tombez sur le sol.",
"Vous vous relevez."}

S2E1.texts["ru"][1] = {"Легкий скрип коры.",
"Эти тихие скрипы начинают тебя раздражать.",
"Кора, похоже, виляет у тебя под ногами...",
"Тебе трудно шагать прямо, может быть, Шуки был не очень свежим.",
"Наверное, где-то произошло трясение коры..."}
S2E1.texts["ru"][2] = {"Из коры поднимается стук.",
"Толчки внезапно трясут Кору и растительность вокруг тебя.",
"Они сбивают тебя с пути...",
"Тебе нужно сосредоточиться, чтобы не упасть.",
"Тряска утихает, кора возвращается в стабильное состояние."}
S2E1.texts["ru"][3] = {"Громкий шум поднимается из коры.",
"Первый толчок заставляет тебя колебаться.",
"Следующие жестокие...",
"Тебе тяжело стоять.",
"Толчки слабеют, кора снова становится стабильной."}
S2E1.texts["ru"][4] = {"Грохот исходит из коры.",
"Кора внезапно поддается под ногами, и ты падаешь на поверхность.",
"Ты встаешь, пытаешься убежать...",
"Но немедленно опять падаешь.",
"Толчки начинают ослабляться..."}
S2E1.texts["ru"][5] = {"Из коры доносится грохот.",
"Кора внезапно поддается под ногами, и ты падаешь на поверхность.",
"Ты снова встаешь."}

function utf8decode(text)
	local utext = ucstring()
	utext:fromUtf8(text)
	return utext
end

function point_inside_poly(x,y,poly)
	local inside = false
	local p1x = poly[1][1]
	local p1y = poly[1][2]

	for i=0,#poly do

		local p2x = poly[((i)%#poly)+1][1]
		local p2y = poly[((i)%#poly)+1][2]

		if y > math.min(p1y,p2y) then
			if y <= math.max(p1y,p2y) then
				if x <= math.max(p1x,p2x) then
					if p1y ~= p2y then
						xinters = (y-p1y)*(p2x-p1x)/(p2y-p1y)+p1x
					end
					if p1x == p2x or x <= xinters then
						inside = not inside
					end
				end
			end
		end
		p1x,p1y = p2x,p2y
	end
	return inside
end

function S2E1:loopActions()
	local action_params = S2E1.AutoActions[S2E1.AutoActionId]
	if action_params ~= nil then
		local action = action_params[1]
		local param = action_params[2]
		local ttime = action_params[3]
		S2E1.crazy_time = nltime.getLocalTime()
		if action == "broadcast" then
			displaySystemInfo(utf8decode("@{9E9F}"..param), "BC")
			action = "wait"
		elseif action == "crunsh" then
			runAH(nil, "run_action", "forward")
			runAH(nil, "force_sit", "")
			action = "wait"
		elseif action == "open_window" then
			S2E1:openWindow(param)
			action = "wait"
		elseif action ~= "Cam" then
			runAH(nil, action, param)
			action = "wait"
		end
		S2E1.camPosX = 0
		S2E1.camPosY = 0
		getUI("ui:interface:storyline_window").active=1
		setOnDraw(getUI("ui:interface:storyline_window"), "S2E1:loopAction("..tostring(ttime)..", [["..action.."]], [["..param.."]])")
	end
end

function S2E1:loopAction(ttime, action, param)
	if action == "Cam" then
		local params = {}
		for p in string.gmatch(param, "([^:]+)") do
			table.insert(params, p)
		end
		paramX = tonumber(params[1]) / 100
		paramY = tonumber(params[2]) / 100
		S2E1.camPosX = S2E1.camPosX + paramX
		S2E1.camPosY = S2E1.camPosY + paramY
		moveCam(S2E1.camPosY, 0, S2E1.camPosX)
	end

	local tick = nltime.getLocalTime()
	if tick > S2E1.crazy_time + ttime then
		setOnDraw(getUI("ui:interface:storyline_window"), "")
		S2E1.AutoActionId = S2E1.AutoActionId + 1
		S2E1.camPosX = 0
		S2E1.camPosY = 0
		if action == "Cam" then
			moveCam(0, 0, 0)
		end
		S2E1:loopActions()
	end
end



function S2E1:AddQuake(force)
	for n=1,force*10 do
		posX = force * 4 * (math.random() - 0.5)
		posY = force * 0.1 * (math.random() - 0.5)
		timer = 50+(40*math.random())
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"Cam", tostring(posX)..":"..tostring(posY), timer}
	end
end

function S2E1:getText(size, id)
	local lang = getClientCfg("LanguageCode")
	return S2E1.texts[lang][size][id]
end

function S2E1:StartQuake(size)
	local delays = {}
	delays[1] = {3000, 4000, 300, 1000, 7700}
	delays[2] = {3000, 4000, 300, 1000, 7700, 4000, 1000}
	delays[3] = {3000, 4000, 300, 1000, 200, 4000, 5000}
	delays[4] = {5000, 1000, 500, 3000, 700, 500}
	delays[5] = {300, 300}

	local first_delay = 3000
	if size == 5 then
		first_delay = 1000
	end

	S2E1.AutoActions = {
		{"stop_event_music", "", 10},
		{"broadcast", S2E1:getText(size, 1), 10},
		{"play_event_music", "music=bg_quake"..tostring(size)..".ogg", first_delay}
	}

	if size == 1 then
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][1]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 2), delays[size][2]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 3), delays[size][3]}
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][4]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 4), delays[size][5]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 5), 10}
	end

	if size == 2 then
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][1]}
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 2), delays[size][2]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 3), delays[size][3]}
		S2E1:AddQuake(2)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][4]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 4), delays[size][5]}

		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][6]}

		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][7]}

		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 5), 10}
	end

	if size == 3 then
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][1]}
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 2), delays[size][1]}
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][4]}
		S2E1:AddQuake(2)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 3), delays[size][2]}
		S2E1:AddQuake(3)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 4), delays[size][3]}

		S2E1:AddQuake(2)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][6]}
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][7]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 5), 10}

	end

	if size == 4 then
		S2E1:AddQuake(1)
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][1]}
		S2E1:AddQuake(2)
		S2E1:AddQuake(3)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", delays[size][2]}
		S2E1:AddQuake(4)
		S2E1:AddQuake(3)

		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 2), delays[size][3]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"crunsh", "", 10}

		S2E1:AddQuake(2)
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 3), delays[size][4]}
		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"crunsh", "", 10}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 4), delays[size][5]}

		S2E1:AddQuake(1)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"run_action", "forward", 300}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"stop_action", "forward", 300}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 5), delays[size][6]}
	end

	if size == 5 then
		S2E1:AddQuake(2)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 2), delays[size][1]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"crunsh", "", 10}
		S2E1:AddQuake(2)
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"broadcast", S2E1:getText(size, 3), delays[size][2]}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"run_action", "forward", 300}
		S2E1.AutoActions[#S2E1.AutoActions+1] = {"stop_action", "forward", 300}
	end


	S2E1.AutoActions[#S2E1.AutoActions+1] = {"", "", 5000}

	S2E1.AutoActionId = 1
	S2E1:loopActions()
end


function S2E1:newQuake(timer)
	local nbr_p = 0
	local polygons = {}
	local zones = {}

	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{7846,-6097}, {9754,-6093}, {9778,-8349}, {7844,-8321}}
	zones[nbr_p] = "nexus"

	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{8128,-10208}, {11368,-10208}, {11392,-12392}, {8096,-12368}}
	zones[nbr_p] = "silan"

	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{3716,-4726}, {5228,-5118}, {5468,-5478}, {5596,-5894}, {5604,-6302}, {5524,-6958}, {5220,-7310}, {4692,-7582}, {3588,-7550}, {3060,-6681}, {3076,-5630}}
	zones[nbr_p] = "surround"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{13764,-29482}, {19540,-29434}, {19156,-30090}, {18836,-30330}, {17732,-30506}, {17060,-30458}, {16484,-31049}, {15820,-31538}, {15650,-31590}, {15496,-31578}, {14436,-30890}}
	zones[nbr_p] = "surround"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{6684,-14990}, {6431,-14820}, {6292,-14726}, {6221,-14689}, {6236,-14606}, {5972,-14414}, {5580,-12246}, {5692,-11230}, {5860,-11054}, {6164,-10854}, {7404,-11166}, {7380,-14382}}
	zones[nbr_p] = "surround"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{432,-9729}, {1644,-9693}, {1672,-11433}, {452,-11397}}
	zones[nbr_p] = "surround"

	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{308,-350}, {4004,-318}, {5428,-1918}, {6116,-3086}, {6308,-7982}, {3588,-7710}, {308,-2606}}
	zones[nbr_p] = "far"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{20064,-29262}, {19972,-34986}, {13652,-34954}, {13548,-29330}}
	zones[nbr_p] = "far"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{5300,-9485}, {7524,-9445}, {7556,-17189}, {5280,-17106}}
	zones[nbr_p] = "far"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{191,-13148}, {119,-15892}, {3103,-15868}, {3055,-13100}}
	zones[nbr_p] = "far"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{9990,-1580}, {10046,-2300}, {10246,-2676}, {10510,-2884}, {10990,-3212}, {11646,-3428}, {12486,-3124}, {12522,-712}}
	zones[nbr_p] = "far"
	nbr_p = nbr_p + 1
	polygons[nbr_p] = {{16578,-27080}, {17290,-26192}, {17638,-25860}, {17714,-25613}, {17870,-25436}, {18790,-24648}, {19534,-24168}, {20374,-23800}, {20394,-27100}}
	zones[nbr_p] = "far"


	local x,y=getPlayerPos()
	local zone = "farfar"
	for i=1,nbr_p do
		z = point_inside_poly(x, y, polygons[i])
		if z == true then
			zone = zones[i]
			break
		end
	end

	if zone == "farfar" and (timer % 6) == 0 then
		S2E1:StartQuake(1)
	elseif zone == "far" and (timer % 4) == 0 then
		S2E1:StartQuake(2)
	elseif zone == "surround" and (timer % 2) == 0 then
		S2E1:StartQuake(3)
	elseif zone == "nexus" then
		S2E1:StartQuake(4)
	end


end

-- VERSION --
RYZOM_ARK_VERSION = 366
