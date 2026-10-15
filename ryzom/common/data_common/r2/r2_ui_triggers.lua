r2.maxActivities = 14
r2.sequencesNb = 0

r2.TimeLimitsProp = {
						[tostring(i18n.get("uiR2EdNoTimeLimit"))]="No Limit",
						--["Until a certain time"]="Until",
						[tostring(i18n.get("uiR2EdWhileChat"))]="Chat",
						[tostring(i18n.get("uiR2EdForCertainTime"))]="Few Sec"
				    }
r2.TimeLimitsCB =   {
						["No Limit"]=tostring(i18n.get("uiR2EdNoTimeLimit")),
						--["Until"]="Until a certain time",
						["Chat"]=tostring(i18n.get("uiR2EdWhileChat")),
						["Few Sec"]=tostring(i18n.get("uiR2EdForCertainTime")),
				    }

r2.activityTypeMenu =   {
							["Inactive"]=tostring(i18n.get("uiR2EdInactive")),
							["Stand Still"]=tostring(i18n.get("uiR2EdStandStill")),
							["Follow Route"]=tostring(i18n.get("uiR2EdFollowRoad")),
							["Patrol"]=tostring(i18n.get("uiR2EdPatrol")),
							["Repeat Road"]=tostring(i18n.get("uiR2EdRepeatRoad")),
							--["Deploy"]=tostring(i18n.get("uiR2EdDeploy")),
							["Wander"]=tostring(i18n.get("uiR2EdWander")),
							["Stand On Start Point"]=tostring(i18n.get("uiR2EdStandOnStartPoint")),
							["Go To Start Point"]=tostring(i18n.get("uiR2EdGoToStartPoint")),
							["Go To Zone"]=tostring(i18n.get("uiR2EdGoToZone")),
							["Sit Down"]=tostring(i18n.get("uiR2EdSitDown")),
							["Stand Up"]=tostring(i18n.get("uiR2EdStandUp")),
							
						}

r2.fromNPCNameToId = {}
r2.fromEmoteIdToName = {}

r2.ownCreatedInstances = {}


------------------ INIT TRIGGERS EDITOR ----------------------------------------------------------------
function r2:initTriggersEditor()

	-- emote
	local menuName = "ui:interface:r2ed_triggers_menu"
	local emoteMenu = getUI(menuName)
	local emoteMenu = emoteMenu:getRootMenu()
	assert(emoteMenu)
	emoteMenu:reset()

	local emoteTable = initEmotesMenu(menuName, "")
	for id, name in pairs(emoteTable) do
		r2.fromEmoteIdToName[id] = name
	end
end

------------------ INIT TRIGGERS EDITOR ----------------------------------------------------------------
function r2:initActivityEditor(activityEditor)

	-- time limit
	local timeLimitCB = activityEditor:find("time_limit"):find("combo_box")
	assert(timeLimitCB)
	timeLimitCB:resetTexts()
	timeLimitCB:addText(ucstring(i18n.get("uiR2EdNoTimeLimit")))
	--timeLimitCB:addText(ucstring("Until a certain time"))
	timeLimitCB:addText(ucstring(i18n.get("uiR2EdForCertainTime")))
	timeLimitCB:addText(ucstring(i18n.get("uiR2EdWhileChat")))
end

----------------------------------------------------------------------------------------------------
function r2:getSelectedEltUI(uiName)
	local id = r2:getSelectedEltUIId(uiName)
	if id then
		return getUI(id)
	else
		return nil
	end
end

----------------------------------------------------------------------------------------------------
function r2:getSelectedEltUIId(uiName)

	if uiName == nil then
		dumpCallStack(1)
		assert(false)
	end
	local windowUI = getUI("ui:interface:"..uiName)
	assert(windowUI)

	return windowUI.Env.selectedElementId
end

----------------------------------------------------------------------------------------------------
function r2:setSelectedEltUIId(uiName, eltUIId)
	local windowUI = getUI("ui:interface:"..uiName)
	assert(windowUI)

	windowUI.Env.selectedElementId = eltUIId
end

----------------------------------------------------------------------------------------------------
function r2:getSelectedEltInstId(uiName)

	local element = r2:getSelectedEltUI(uiName)
	if element ~= nil then
		return element.Env.elementId
	else
		return nil
	end
end

----------------------------------------------------------------------------------------------------
function r2:setSelectedEltInstId(uiName, instId)
	local element = r2:getSelectedEltUI(uiName)
	assert(element)
	element.Env.elementId = instId
end

----------------------------------------------------------------------------------------------------
function r2:getSelectedEltInst(uiName)
	local id = r2:getSelectedEltInstId(uiName)
	if id then
		return r2:getInstanceFromId(id)
	else
		return nil
	end
end

----------------------------------------------------------------------------------------------------
function r2:getSelectedSequInstId(uiName)
	local windowUI = getUI("ui:interface:"..uiName)
	assert(windowUI)

	local tab = windowUI:find("sequence_tabs")
	assert(tab)

	local sequence = windowUI:find(tab.associatedGroupSelection)
	assert(sequence)

	if sequence.Env == nil then return nil end

	return sequence.Env.sequenceId
end

----------------------------------------------------------------------------------------------------
function r2:getSelectedSequInst(uiName)
	local id = r2:getSelectedSequInstId(uiName)
	if id then
		return r2:getInstanceFromId(id)
	else
		return nil
	end
end

----------------------------------------------------------------------------------------------------
function r2:getSelectedSequ(uiName)
	local windowUI = getUI("ui:interface:"..uiName)
	assert(windowUI)

	local tab = windowUI:find("sequence_tabs")
	
	if tab~=nil then
		local associatedGroup = tab.associatedGroupSelection
		if associatedGroup == "" then
			return nil
		end

		local sequence = windowUI:find(associatedGroup)
		assert(sequence)

		return sequence
	else
		return windowUI:find("sequence_elts")
	end
end

----------------------------------------------------------------------------------------------------
function r2:setSelectedSequInstId(uiName, instanceId)

	local windowUI = getUI("ui:interface:"..uiName)
	assert(windowUI)

	local tab = windowUI:find("sequence_tabs")
	assert(tab)

	local sequence = windowUI:find(tab.associatedGroupSelection)
	assert(sequence)

	sequence.Env.sequenceId = instanceId
end

------------------ CLOSE ALL UI --------------------------------------------------------------------
function r2:closeActivitySequenceUI()

	local window = getUI("ui:interface:r2ed_edit_activity_sequence")
	if window~=nil then
		window.active = false

		r2:openAndUpdateMiniActivityView()
	end
end

------------------ NEW SEQUENCE --------------------------------------------------------------------
function r2:newActivitiesSequence(firstRequest, activitySequence, reset)	


	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)

	local tab = triggersUI:find("sequence_tabs")
	assert(tab)

	if firstRequest and tab.tabButtonNb == 7 then return -1 end

	if firstRequest == true then
		activitySequence = r2.newComponent("ActivitySequence")

		--local name = tostring(i18n.get("uiR2EdSeq"))..tab.tabButtonNb+1
		--activitySequence.Name = name

		local npcGroup = r2:getSelectedInstance()
		assert(npcGroup)

		r2.requestInsertNode(npcGroup:getBehavior().InstanceId, "Activities", -1, "", activitySequence)
--		r2.requestInsertNode(npcGroup.InstanceId,"ActivitiesId",-1,"",activitySequence.InstanceId)
--		r2.requestInsertNode(r2:getCurrentAct().InstanceId,"ActivitiesIds",-1,"",activitySequence.InstanceId)

		r2.ownCreatedInstances[activitySequence.InstanceId] = true

		return activitySequence.InstanceId
	else

		local updateMiniActivityView = r2.ownCreatedInstances[activitySequence.InstanceId]
		if reset then
			r2.ownCreatedInstances[activitySequence.InstanceId] = true
		end
		
		local templateParams = {
									newElt="r2:newActivity(true)",
									newEltText=tostring(i18n.get("uiR2EdNewActivity")),
									eltOrderText=tostring(i18n.get("uiR2EdActivityOrder")),
									upElt="r2:upActivity()",
									downElt="r2:downActivity()",
									maxMinElts="r2:maximizeMinimizeActivities()",
									downUpColor="200 120 80 255",
									colPushed = "200 80 80 255",
									paramsL= "r2:selectSequenceTab('r2ed_triggers', "..tostring(tab.tabButtonNb)..")"
							   }

		local editorEltTemplate = "template_edit_activity"
		local sequence = r2:newElementsSequence("r2ed_triggers", templateParams, editorEltTemplate, 
			activitySequence, tostring(i18n.get("uiR2EdSeq")))
		r2:initActivityEditor(sequence:find("edit_element"))

		local activities = activitySequence.Components
		for a = 0, activities.Size - 1 do
			r2:newActivity(false, nil, activities[a])	
		end

		-- new MiniActivity sequence
		if updateMiniActivityView==true then		
			--r2:updateSequencesButtonBar(tab.tabButtonNb-1, activitySequence.Name)
			r2:updateSequencesButtonBar(tab.tabButtonNb-1, r2:getSequenceName(activitySequence))
			r2:openAndUpdateMiniActivityView()
		end
	end
end

------------------ REMOVE SEQUENCE -----------------------------------------------------------------
function r2:removeActivitiesSequence()

	local sequenceId = r2:removeElementsSequence("r2ed_triggers", "Activities", tostring(i18n.get("uiR2EdSeq")))
	
-- 	local group = r2:getSelectedInstance()
-- 	assert(group)
-- 	local sequenceIndex = -1
-- 	for i=0,group.ActivitiesId.Size-1 do
-- 		if group.ActivitiesId[i] == sequenceId then
-- 			sequenceIndex = i
-- 			break
-- 		end
-- 	end
-- 	if sequenceIndex ~= -1 then
-- 		r2.requestEraseNode(group.InstanceId, "ActivitiesId", sequenceIndex)
-- 	end
-- 
-- 	local currentAct = r2:getCurrentAct()
-- 	sequenceIndex = -1
-- 	for i=0,currentAct.ActivitiesIds.Size-1 do
-- 		if currentAct.ActivitiesIds[i] == sequenceId then
-- 			sequenceIndex = i
-- 			break
-- 		end
-- 	end
-- 	if sequenceIndex ~= -1 then
-- 		r2.requestEraseNode(currentAct.InstanceId, "ActivitiesIds", sequenceIndex)
-- 	end
end

function r2:removeActivitiesSequenceUI(tabIndex) 
	local firstEltName = r2:removeElementsSequenceUI(tabIndex, "r2ed_triggers", "Activities", tostring(i18n.get("uiR2EdSeq")))
	
	r2:updateSequencesButtonBar(0, firstEltName)
end

------------------ SELECT ACTIVITY ------------------------------------------------------------------
function r2:selectActivity()
	r2:selectTriggerElement(nil, "r2ed_triggers")

	if getUICaller().pushed == true then
		r2:updateActivityEditor() 
	end
end

------------------ NEW ACTIVITY --------------------------------------------------------------------
function r2:newActivity(firstRequest, tableInit, instanceElement, sequenceUI, sequenceInstId)
	


	local uiName = "r2ed_triggers"

	if sequenceUI == nil then
		sequenceUI = r2:getSelectedSequ(uiName)
	end
	local activityList
	if sequenceUI~=nil then
		activityList = sequenceUI:find("elements_list")
		assert(activityList)

		if activityList.childrenNb-1 == r2.maxActivities then return false end
	end

	if firstRequest == true then

		instanceElement = r2.newComponent("ActivityStep")

		if tableInit ~= nil then
			instanceElement.Activity = tableInit.Activity
			instanceElement.ActivityZoneId = r2.RefId(tableInit.ActivityZoneId)
			instanceElement.TimeLimit = tableInit.TimeLimit
			instanceElement.TimeLimitValue = tableInit.TimeLimitValue
		else
			instanceElement.TimeLimit = "No Limit"
			instanceElement.Activity = "Stand Still"
		end

		if sequenceInstId == nil then
			sequenceInstId = r2:getSelectedSequInstId(uiName)
		end
		assert(sequenceInstId)

		r2.requestInsertNode(sequenceInstId, "Components", -1, "", instanceElement)

		r2.ownCreatedInstances[instanceElement.InstanceId] = true
	else
	
		local templateParams =	{
								selectElt="r2:selectActivity()", 
								openEltEditor="r2:openActivityEditor()", 
								maxMinElt="r2:maximizeMinimizeActivity()", 
								removeElt="r2:removeActivity()",
								colOver="200 120 80 120",
								colPushed="200 120 80 255"
								}

		local element = r2:newTriggerElement(uiName, tostring(i18n.get("uiR2EdActivity")), templateParams, sequenceUI, instanceElement.InstanceId)

		r2:updateActivityChatSequence(element)

		-- active chat button if a chat sequence is associated to this activity
		if instanceElement.Chat ~= "" then
			
			local activityEditor = sequenceUI:find("edit_element")
			assert(activityEditor)
			local chatButtonEditor = activityEditor:find("open_chat_sequence")
			assert(chatButtonEditor)

			chatButtonEditor.active = true
		end
	end

	return true
end


function r2:buildActivityTitle(activityUI, erase)

	local activityInst = r2:getInstanceFromId(activityUI.Env.elementId)
	assert(activityInst)

	-- part1
	local index = r2:searchElementIndex(activityInst)
	if erase==true then index = index-1 end
	local part1 = tostring(i18n.get("uiR2EdActivity")).." "..index.." : "

	-- part2
	if  not r2.activityTypeMenu[activityInst.Activity] then
		debugInfo(colorTag(255,0,0).."The activity '".. activityInst.Activity.."' is not properly registred")
	end
	
	local part2 = r2.activityTypeMenu[activityInst.Activity].." "
	if activityInst.ActivityZoneId~= "" then
		local place = r2:getInstanceFromId(tostring(activityInst.ActivityZoneId))
		if place~=nil then
			part2 = part2.. place.Name .." "
		end	
	end

	-- part3
	local part3 = ""
	if activityInst.TimeLimit == "Few Sec" then
	
		local hourNb, minNb, secNb = r2:calculHourMinSec(tonumber(activityInst.TimeLimitValue))

		local timeLimitText = tostring(i18n.get("uiR2EdFor")) .. " "
		if hourNb ~= 0 then timeLimitText = timeLimitText .. hourNb .. tostring(i18n.get("uiR2EdShortHours")) .. " " end
		if minNb ~= 0 then timeLimitText = timeLimitText .. minNb .. tostring(i18n.get("uiR2EdShortMinutes")) .. " " end
		timeLimitText = timeLimitText .. secNb .. tostring(i18n.get("uiR2EdShortSeconds")) 
		
		part3 = timeLimitText

	elseif r2.TimeLimitsCB[activityInst.TimeLimit] ~= nil then
		part3 = string.lower(r2.TimeLimitsCB[activityInst.TimeLimit])
	end

	-- title
	local title = activityUI:find("title")
	assert(title)
	title.uc_hardtext= part1..part2..part3
end

function r2:getActivityName(activityInst)

	-- part1
	local index = r2:searchElementIndex(activityInst)
	if erase==true then index = index-1 end
	local part1 = tostring(i18n.get("uiR2EdActivity")).." "..index.." : "

	-- part2
	if  not r2.activityTypeMenu[activityInst.Activity] then
		debugInfo(colorTag(255,0,0).."The activity '".. activityInst.Activity.."' is not properly registred")
	end
	
	local part2 = r2.activityTypeMenu[activityInst.Activity].." "
	if activityInst.ActivityZoneId~= "" then
		local place = r2:getInstanceFromId(tostring(activityInst.ActivityZoneId))
		if place~=nil then
			part2 = part2.. place.Name .." "
		end	
	end

	-- part3
	local part3 = ""
	if activityInst.TimeLimit == "Few Sec" then
	
		local hourNb, minNb, secNb = r2:calculHourMinSec(tonumber(activityInst.TimeLimitValue))

		local timeLimitText = tostring(i18n.get("uiR2EdFor")) .. " "
		if hourNb ~= 0 then timeLimitText = timeLimitText .. hourNb .. tostring(i18n.get("uiR2EdShortHours")) .. " " end
		if minNb ~= 0 then timeLimitText = timeLimitText .. minNb .. tostring(i18n.get("uiR2EdShortMinutes")) .. " " end
		timeLimitText = timeLimitText .. secNb .. tostring(i18n.get("uiR2EdShortSeconds")) 
		
		part3 = timeLimitText

	elseif r2.TimeLimitsCB[activityInst.TimeLimit] ~= nil then
		part3 = string.lower(r2.TimeLimitsCB[activityInst.TimeLimit])
	end

	return part1..part2..part3
end

function r2:getElementName(elementInst)

	if elementInst:isKindOf("ActivityStep") then
		return r2:getActivityName(elementInst)
	elseif elementInst:isKindOf("ChatStep") then
		return r2:getChatName(elementInst)
	elseif elementInst:isKindOf("LogicEntityAction") then
		return r2:getActionName(elementInst)
	elseif elementInst:isKindOf("LogicEntityReaction") then
		return r2:getReactionName(elementInst)
	elseif elementInst:isKindOf("ChatSequence") or elementInst:isKindOf("ActivitySequence") then
		return r2:getSequenceName(elementInst)
	end

	return ""
end

function r2:updateActivityChatSequence(elementUI, canceledChatStepId)

	if elementUI == nil then
		elementUI = r2:getSelectedEltUI("r2ed_triggers")
	end
	assert(elementUI)

	local instance = r2:getInstanceFromId(elementUI.Env.elementId)
	assert(instance)

	local chatSequence = r2:getInstanceFromId(tostring(instance.Chat))

	local activityText = elementUI:find("text_list")
	assert(activityText)

	local sep = elementUI:find("sep")
	assert(sep)

	activityText:clear()

	if chatSequence ~= nil then
		local counterTime = 0

		if chatSequence.Components.Size > 0 then
			
			local emptyText = true

			activityText:addColoredTextChild("\n"..tostring(i18n.get("uiR2EdSequenceStart")), 255, 175, 135, 255)
			
			for c = 0, chatSequence.Components.Size - 1 do
				local chat = chatSequence.Components[c]

				if chat.InstanceId ~= canceledChatStepId then
					counterTime = counterTime + tonumber(chat.Time)

					local firstLine = true
				
					local who = tostring(chat.Actions[0].Who)
					if who ~= "" then

						local facing = tostring(chat.Actions[0].Facing)
						local emote = chat.Actions[0].Emote
						local says = chat.Actions[0].Says

						local minNb, secNb = r2:calculMinSec(counterTime)
						local countInfo = "*"
						if minNb ~= 0 then
							countInfo = countInfo..tostring(minNb)..tostring(i18n.get("uiR2EdShortMinutes"))
						end
						countInfo = countInfo.." "..tostring(secNb)..tostring(i18n.get("uiR2EdShortSeconds"))..":"

						if facing ~= "" then
							facing = r2:getInstanceFromId(who).Name .. " "..tostring(i18n.get("uiR2EdFaces")).." " .. r2:getInstanceFromId(facing).Name
							if firstLine then
								activityText:addColoredTextChild(countInfo, 220, 140, 100, 255)
								firstLine = false
							end
							activityText:addTextChild(ucstring(facing))
							emptyText = false
						end
						if r2.fromEmoteIdToName[emote] ~= nil then
							emote = r2:getInstanceFromId(who).Name .. " "..string.lower(tostring(i18n.get("uiR2EdEmote"))).." : " .. r2.fromEmoteIdToName[emote]
							if firstLine then
								activityText:addColoredTextChild(countInfo, 220, 140, 100, 255)
								firstLine = false
							end
							activityText:addTextChild(ucstring(emote))
							emptyText = false
						end
						if r2:getInstanceFromId(says)~=nil and r2:getInstanceFromId(says).Text ~= "" then
							says = r2:getInstanceFromId(who).Name .. " "..tostring(i18n.get("uiR2EdSays")).." : " .. r2:getInstanceFromId(says).Text 
							if firstLine then
								activityText:addColoredTextChild(countInfo, 220, 140, 100, 255)
								firstLine = false
							end
							activityText:addTextChild(ucstring(says))
							emptyText = false
						end
					end
				end		
			end

			if emptyText == true then
				activityText:clear()
				sep.active = false
			else
				sep.active = true

				local hourNb, minNb, secNb = r2:calculHourMinSec(counterTime)
				local totalChatTime = ""
				if hourNb ~= 0 then
					totalChatTime = tostring(hourNb)..tostring(i18n.get("uiR2EdShortHours"))
				end
				if minNb ~= 0 then
					totalChatTime = totalChatTime.." "..tostring(minNb)..tostring(i18n.get("uiR2EdShortMinutes"))
				end
				totalChatTime = totalChatTime.." "..tostring(secNb)..tostring(i18n.get("uiR2EdShortSeconds"))
				activityText:addColoredTextChild(tostring(i18n.get("uiR2EdSequenceEnd")).." (".. totalChatTime ..")\n", 255, 175, 135, 255)
			end
		end	
	else
		sep.active = false	
	end

	--update title
	r2:buildActivityTitle(elementUI, false)

	if r2:getSelectedEltInstId("r2ed_triggers") == instance.InstanceId then
		local eltEditor = r2:getSelectedSequ("r2ed_triggers"):find("edit_element")
		assert(eltEditor)
		r2:updateActivityEditor()
	end
	
	r2:updateMiniActivityView()	
end

function r2:updateActivitiesWhithThisChatSequence(chatStep, canceled)

	local chatSequenceId = chatStep.Parent.Parent.InstanceId

	local chatStepId = nil
	if canceled == true then chatStepId = chatStep.InstanceId end

	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)

	local tab = triggersUI:find("sequence_tabs")
	assert(tab)

	for s = 0,tab.tabButtonNb-1 do
		local activitySequence = tab:getGroup(s)
		assert(activitySequence)

		local activityList = activitySequence:find("elements_list")
		assert(activityList)

		for a=0, activityList.childrenNb-1 do
			local activity = activityList:getChild(a)
			assert(activity)

			if activity.Env.elementId~=nil then
				local activityInst = r2:getInstanceFromId(activity.Env.elementId)
				assert(activityInst)
				if tostring(activityInst.Chat) == chatSequenceId then
					r2:updateActivityChatSequence(activity, chatStepId)
				end
			end
		end
	end
end

------------------ REMOVE ACTIVITY ------------------------------------------------------------------
function r2:removeActivity()
	r2:removeTriggerElement("r2ed_triggers", tostring(i18n.get("uiR2EdActivity")))
end

function r2:removeActivityUI(sequenceUI, elementUI)

	r2:removeElementUI(sequenceUI, "r2ed_triggers", elementUI)
	r2:closeActivitySequenceUI()

	local activityList = sequenceUI:find("elements_list")
	assert(activityList)
end

------------------ UP ACTIVITY -----------------------------------------------------------------------
function r2:upActivity()
	r2:upTriggerElement("r2ed_triggers")
end

------------------ DOWN ACTIVITY ---------------------------------------------------------------------
function r2:downActivity()
	r2:downTriggerElement("r2ed_triggers")	
end

------------------ MAXIMIZE MINIMIZE ACTIVITIES ------------------------------------------------------
function r2:maximizeMinimizeActivities()
	r2:maximizeMinimizeTriggerElements("r2ed_triggers")
end

------------------ MAXIMIZE MINIMIZE ACTIVITY ------------------------------------------------------
function r2:maximizeMinimizeActivity()
	r2:maximizeMinimizeElement("r2ed_triggers")
end

------------------ OPEN ACTIVITY EDITOR ----------------------------------------------------------------
function r2:openActivityEditor()
	r2:updateActivityEditor()
	r2:openElementEditor("r2ed_triggers", "uiR2EDActivityStepEditor")
end

function r2:updateActivityEditor()

	local uiName = "r2ed_triggers"
	local instanceActivity = r2:getSelectedEltInst(uiName)

	local activityEditor = r2:getSelectedSequ(uiName):find("edit_element")
	assert(activityEditor)

	-- activity name
	local activityName = activityEditor:find("name")
	assert(activityName)

	-- activity type
	local activityButtonText = activityEditor:find("activity"):find("text")
	assert(activityButtonText)

	-- time limit
	local comboBox = activityEditor:find("time_limit").combo_box
	assert(comboBox)

	-- chat sequence
	local chatMenuButton = activityEditor:find("chat_script"):find("menu"):find("text")
	assert(chatMenuButton)

	local chatButtonEditor = activityEditor:find("open_chat_sequence")
	assert(chatButtonEditor)

	if instanceActivity then

		local index = r2:searchElementIndex(instanceActivity)
		if index~= nil then
			activityName.uc_hardtext = tostring(i18n.get("uiR2EdActivity")).." "..index.." : "
		else
			activityName.uc_hardtext = tostring(i18n.get("uiR2EdActivity")).." : "
		end
		
		-- activity type
		local activityText = r2.activityTypeMenu[instanceActivity.Activity]
		if activityText then
			activityButtonText.uc_hardtext = activityText
		end
		if instanceActivity.ActivityZoneId ~= "" then
			local place = r2:getInstanceFromId(instanceActivity.ActivityZoneId)
			assert(place)

			activityButtonText.uc_hardtext = activityButtonText.hardtext .. " " .. place.Name
		end

		-- time limit
		local timeLimit = instanceActivity.TimeLimit
		
		local certainTime = activityEditor:find("certain_time")
		assert(certainTime)

		comboBox.Env.locked = true
		if timeLimit == "Few Sec" then
			local timeLimitValue = tonumber(instanceActivity.TimeLimitValue)

			if timeLimitValue ~= nil then
				
				local hoursNb, minNb, secNb = r2:calculHourMinSec(timeLimitValue)

				local timeLimitText = tostring(i18n.get("uiR2EdFor")) .. " "
				if hoursNb ~= 0 then timeLimitText = timeLimitText .. hoursNb .. tostring(i18n.get("uiR2EdShortHours")) .. " " end
				if minNb ~= 0 then timeLimitText = timeLimitText .. minNb .. tostring(i18n.get("uiR2EdShortMinutes")) .. " " end
				timeLimitText = timeLimitText .. secNb .. tostring(i18n.get("uiR2EdShortSeconds")) 

				certainTime.active = true
				local hoursMenu = certainTime:find("hours"):find("text")
				assert(hoursMenu)
				hoursMenu.uc_hardtext = tostring(hoursNb)

				local minutesMenu = certainTime:find("minutes"):find("text")
				assert(minutesMenu)
				minutesMenu.uc_hardtext = tostring(minNb)

				local secondsMenu = certainTime:find("seconds"):find("text")
				assert(secondsMenu)
				secondsMenu.uc_hardtext = tostring(secNb)

				comboBox.view_text = timeLimitText
			end
		else
			certainTime.active = false
			timeLimit = r2.TimeLimitsCB[timeLimit]
			if timeLimit~= nil then
				comboBox.selection_text = timeLimit
			end
		end
		comboBox.Env.locked = false

		-- chat sequence  TEMP TEMP TEMP TEMP
		local sequenceChat = r2:getInstanceFromId(tostring(instanceActivity.Chat))
		local repeatLabelButton = activityEditor:find("repeat")
		assert(repeatLabelButton)

		if sequenceChat ~= nil then
			--chatMenuButton.uc_hardtext = sequenceChat.Name
			chatMenuButton.uc_hardtext = r2:getSequenceName(sequenceChat)

			repeatLabelButton.active = true
			local repeatButton = repeatLabelButton:find("toggle_butt")
			assert(repeatButton)
			repeatButton.pushed = not (instanceActivity.Type == "Repeating")

			chatButtonEditor.active = true
		else
			repeatLabelButton.active = false
			chatMenuButton.uc_hardtext = i18n.get("uiR2EdNoElt")

			chatButtonEditor.active = false
		end
		-- TEMP TEMP TEMP

	else

		local name = tostring(i18n.get("uiR2EdActivity")).." : "
		activityName.uc_hardtext = name
		
		activityButtonText.uc_hardtext = i18n.get("uiR2EdStandStill")
		comboBox.selection_text = i18n.get("uiR2EdNoTimeLimit")
		chatMenuButton.uc_hardtext = i18n.get("uiR2EdNoElt")

		chatButtonEditor.active = false
	end
end

------------------ REPEAT ACTIVITY SEQUENCE -------------------------------------------------------------
function r2:repeatActivitySequence()
	r2.requestNewAction(i18n.get("uiR2EDRepeatActivitySequenceAction"))

	local sequenceInstId = r2:getSelectedSequInstId("r2ed_triggers")
	assert(sequenceInstId)

	local sequenceType = not getUICaller().pushed
	if sequenceType==false then sequenceType=0 else sequenceType=1 end

	r2.requestSetNode(sequenceInstId, "Repeating", sequenceType) 
end

------------------ CLOSE ACTIVITY EDITOR ----------------------------------------------------------------
function r2:closeChatSequencesUI()

	local window = getUI("ui:interface:r2ed_chat_sequence")
	assert(window)
	window.active = false

	window = getUI("ui:interface:r2ed_edit_chat_sequence")
	assert(window)
	window.active = false
end

------------------ OPEN ACTIVITY MENU -------------------------------------------------------------------
function r2:openActivityMenu()

	local menuName = "ui:interface:r2ed_triggers_menu"
	local activityMenu = getUI(menuName)
	local activityMenu = activityMenu:getRootMenu()
	assert(activityMenu)
	activityMenu:reset()

	-- Inactive
--	activityMenu:addLine(ucstring(i18n.get("uiR2EdInactive")), "lua", "r2:setActivity('Inactive')", "Inactive")

	-- Stand still
	activityMenu:addLine(ucstring(i18n.get("uiR2EdStandStill")), "lua", "r2:setActivity('Stand Still')", "Stand Still")

	-- Follow route
	activityMenu:addLine(ucstring(i18n.get("uiR2EdFollowRoad")), "", "", "Follow Route")
	local menuButton = createGroupInstance("r2_menu_button", "", { bitmap = "r2ed_icon_road.tga", size="14" })
	activityMenu:setUserGroupLeft(1, menuButton)
	activityMenu:addSubMenu(1)
	local roadsMenu = activityMenu:getSubMenu(1)
	local roadsTable = r2.Scenario:getAllInstancesByType("Road")
	for key, road in pairs(roadsTable) do
		roadsMenu:addLine(ucstring(road.Name), "lua", "r2:setActivity('Follow Route', '".. road.InstanceId .."')", road.InstanceId)
	end
	if table.getn(roadsTable) == 0 then
		roadsMenu:addLine(ucstring(i18n.get("uiR2EdNoSelelection")), "lua", "r2:setActivity()", "")
	end

	-- Patrol
	activityMenu:addLine(ucstring(i18n.get("uiR2EdPatrol")), "", "", "Patrol")
	menuButton = createGroupInstance("r2_menu_button", "", { bitmap = "r2ed_icon_road.tga", size="14"})
	activityMenu:setUserGroupLeft(2, menuButton)
	activityMenu:addSubMenu(2)
	roadsMenu = activityMenu:getSubMenu(2)
	for key, road in pairs(roadsTable) do
		roadsMenu:addLine(ucstring(road.Name), "lua", "r2:setActivity('Patrol', '".. road.InstanceId .."')", road.InstanceId)
	end
	if table.getn(roadsTable) == 0 then
		roadsMenu:addLine(ucstring(i18n.get("uiR2EdNoSelelection")), "lua", "r2:setActivity()", "")
	end

	-- Repeat Road
	activityMenu:addLine(ucstring(i18n.get("uiR2EdRepeatRoad")), "", "", "Repeat Road")
	menuButton = createGroupInstance("r2_menu_button", "", { bitmap = "r2ed_icon_road.tga", size="14"})
	activityMenu:setUserGroupLeft(3, menuButton)
	activityMenu:addSubMenu(3)
	roadsMenu = activityMenu:getSubMenu(3)
	for key, road in pairs(roadsTable) do
		roadsMenu:addLine(ucstring(road.Name), "lua", "r2:setActivity('Repeat Road', '".. road.InstanceId .."')", road.InstanceId)
	end
	if table.getn(roadsTable) == 0 then
		roadsMenu:addLine(ucstring(i18n.get("uiR2EdNoSelelection")), "lua", "r2:setActivity()", "")
	end

	-- Wander
	activityMenu:addLine(ucstring(i18n.get("uiR2EdWander")), "", "", "Wander")
	menuButton = createGroupInstance("r2_menu_button", "", { bitmap = "r2ed_icon_region.tga", size="14"})
	activityMenu:setUserGroupLeft(4, menuButton)
	activityMenu:addSubMenu(4)
	local regionsMenu = activityMenu:getSubMenu(4)
	local regionsTable = r2.Scenario:getAllInstancesByType("Region")
	for key, region in pairs(regionsTable) do
		regionsMenu:addLine(ucstring(region.Name), "lua", "r2:setActivity('Wander', '".. region.InstanceId .."')", region.InstanceId)
	end
	if table.getn(regionsTable) == 0 then
		regionsMenu:addLine(ucstring(i18n.get("uiR2EdNoSelelection")), "lua", "r2:setActivity()", "")
	end

	-- Deploy
--	activityMenu:addLine(ucstring(i18n.get("uiR2EdDeploy")), "", "", "Deploy")
--	menuButton = createGroupInstance("r2_menu_button", "", { bitmap = "r2ed_icon_region.tga", size="14"})
--	activityMenu:setUserGroupLeft(6, menuButton)
--	activityMenu:addSubMenu(6)
--	local regionsMenu = activityMenu:getSubMenu(6)
--	local regionsTable = r2.Scenario:getAllInstancesByType("Region")
--	for key, region in pairs(regionsTable) do
--		regionsMenu:addLine(ucstring(region.Name), "lua", "r2:setActivity('Deploy', '".. region.InstanceId .."')", region.InstanceId)
--	end
--	if table.getn(regionsTable) == 0 then
--		regionsMenu:addLine(ucstring(i18n.get("uiR2EdNoSelelection")), "lua", "r2:setActivity()", "")
--	end

	r2:openTriggersMenu(getUICaller())
end

function r2:setActivity(activityType, placeId)



	local activityInstId = r2:getSelectedEltInstId("r2ed_triggers")
	assert(activityInstId)

	if activityType == nil then
		return
	elseif placeId == nil then
		r2.requestSetNode(activityInstId, "Activity", activityType)
		r2.requestSetNode(activityInstId, "ActivityZoneId", r2.RefId(""))
	else
		r2.requestSetNode(activityInstId, "Activity", activityType)
		r2.requestSetNode(activityInstId, "ActivityZoneId", r2.RefId(placeId))
	end
end

------------------ SET TIME LIMIT --------------------------------------------------------------------
function r2:setTimeLimit(timeLimit)
	
	if timeLimit == nil then
		timeLimit = getUICaller().selection_text 
		if getUICaller().Env.locked then return end
	end

	local uiName = "r2ed_triggers"
	local activity = r2:getSelectedEltUI(uiName)
	if activity == nil then return end

	local viewText
	local hasTimeLimitValue = false

	if timeLimit == tostring(i18n.get("uiR2EdForCertainTime")) then
		hasTimeLimitValue = true
		viewText = tostring(i18n.get("uiR2EdFor")) ..  " 20" .. tostring(i18n.get("uiR2EdShortSeconds")) 
		
		local eltEditor = r2:getSelectedSequ(uiName):find("edit_element")
		assert(eltEditor)
		local comboTime = eltEditor:find("time_limit"):find("combo_box") 
		assert(comboTime)																				
		comboTime.view_text = viewText
	else
		viewText = timeLimit
	end

	timeLimit = r2.TimeLimitsProp[timeLimit]

	local activityInstId = r2:getSelectedEltInstId(uiName)
	r2.requestSetNode(activityInstId, "TimeLimit", timeLimit)
	
	if hasTimeLimitValue then
		r2.requestSetNode(activityInstId, "TimeLimitValue", tostring(20))
	else	
		r2.requestSetNode(activityInstId, "TimeLimitValue", "")
	end
end

function r2:calculHourMinSec(totalSecNb)

	local minSecNb, hourNb = totalSecNb, 0
	while minSecNb > 3599 do
		hourNb = hourNb+1
		minSecNb = minSecNb - 3600
	end

	local minNb, secNb = 0, minSecNb
	while secNb > 59 do
		minNb = minNb+1
		secNb = secNb - 60
	end

	return hourNb, minNb, secNb
end

function r2:calculMinSec(totalSecNb)

	local minNb, secNb = 0, totalSecNb
	while secNb > 59 do
		minNb = minNb+1
		secNb = secNb - 60
	end

	return minNb, secNb
end

function r2:activityForHours(hourNb)

	local activityInst = r2:getSelectedEltInst("r2ed_triggers")
	assert(activityInst)

	local lastHourNb, minNb, secNb = r2:calculHourMinSec(tonumber(activityInst.TimeLimitValue))
	
	r2:setLimitTimeValue(hourNb, minNb, secNb)
end

function r2:activityForMinutes(minNb)

	local activityInst = r2:getSelectedEltInst("r2ed_triggers")
	assert(activityInst)

	local hoursNb, lastMinNb, secNb = r2:calculHourMinSec(tonumber(activityInst.TimeLimitValue))
	
	r2:setLimitTimeValue(hoursNb, minNb, secNb)
end

function r2:activityForSeconds(secNb)

	local activityInst = r2:getSelectedEltInst("r2ed_triggers")
	assert(activityInst)

	local hoursNb, minNb, lastSecNb = r2:calculHourMinSec(tonumber(activityInst.TimeLimitValue))
	
	r2:setLimitTimeValue(hoursNb, minNb, secNb)
end

------------------ SET LIMIT TIME VALUE -------------------------------------------------------------------
function r2:setLimitTimeValue(hourNb, minNb, secNb)

	local totalSec = tostring(hourNb*3600 + minNb*60 + secNb)
	r2.requestSetNode(r2:getSelectedEltInstId("r2ed_triggers"), "TimeLimitValue", totalSec)
end


------------------ OPEN SELECT CHAT MENU -------------------------------------------------------------------
function r2:openSelectChatMenu()

	-- triggers menu initialization
	local chatMenu = getUI("ui:interface:r2ed_triggers_menu")
	assert(chatMenu)

	local chatMenu = chatMenu:getRootMenu()
	assert(chatMenu)

	chatMenu:reset()

	-- first line "None"
	chatMenu:addLine(ucstring(i18n.get("uiR2EdNoElt")), "lua", "r2:selectChatSequence('None')", "None")

	-- second line "More"
	chatMenu:addLine(ucstring(i18n.get("uiR2EdMore")), "lua", "r2:openChatSequences()", "More")
	
	local entityInst = r2:getSelectedInstance()
	assert(entityInst)

	-- list of avoidable chat sequences
	local chatSequences = entityInst:getBehavior().ChatSequences
	for i=0, chatSequences.Size-1 do
		local chatS = chatSequences[i]
		assert(chatS)
		--chatMenu:addLine(ucstring(chatS.Name), "lua", "r2:selectChatSequence('" .. chatS.InstanceId .. "')", chatS.InstanceId)
		chatMenu:addLine(ucstring(r2:getSequenceName(chatS)), "lua", "r2:selectChatSequence('" .. chatS.InstanceId .. "')", chatS.InstanceId)
	end

	r2:openTriggersMenu(getUICaller())
end

function r2:openChatSequences()
	local chatSequencesUI = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatSequencesUI)

	chatSequencesUI.active = true
	chatSequencesUI:updateCoords()

	if chatSequencesUI.Env.openFirst == nil then
		chatSequencesUI:center()
		chatSequencesUI.Env.openFirst = true
	end
end

-----------------------------------------------------------------------------------------------------------
-- the commun "logic entity" menu is open
function r2:openTriggersMenu(caller)

	local menuName = "ui:interface:r2ed_triggers_menu"
	launchContextMenuInGame(menuName)
	local menu = getUI(menuName)

	menu:updateCoords()	
	menu.y = caller.y_real - (menu.h + 2)
	menu.x = caller.x_real
	menu:setMinW(caller.w)
	menu:updateCoords()	
end

------------------ SELECT CHAT SEQUENCE  -------------------------------------------------------------
function r2:selectChatSequence(choice, activityId)

	local chatSequenceUI = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatSequenceUI)

	local uiName = "r2ed_triggers"

	if activityId == nil then 
		activityId = r2:getSelectedEltInstId(uiName)
	end

	local activityUI = r2:getSelectedEltUI(uiName)
	assert(activityUI)

	if choice == "None" then
		r2.requestSetNode(activityId, "Chat", r2.RefId(""))
		local sep = activityUI:find("sep")
		assert(sep)
		sep.active = false
	elseif choice == "More" then
		r2:openChatSequences()
	elseif choice then
		local chatSId = choice
		local activityInstId = r2:getSelectedEltInstId(uiName)
		r2.requestSetNode(activityId, "Chat", r2.RefId(chatSId))
		r2.requestSetNode(activityInstId, "Type", "Non Repeating")
	else
		debugInfo("r2:selectChatSequence : unknown menu selection")
	end
end

------------------ REPEAT OR NOT CHAT SEQUENCE  ------------------------------------------------------
function r2:repeatChatSequence()

	local activityInstId = r2:getSelectedEltInstId("r2ed_triggers")  
	assert(activityInstId)											 
	
	local sequenceType = "Repeating"
	if getUICaller().pushed then sequenceType = "Non Repeating" end

	r2.requestSetNode(activityInstId, "Type", sequenceType)					
end

------------------ NEW CHAT SEQUENCE  -------------------------------------------------------------
function r2:newChatsSequence(firstRequest, instanceSequence, reset)

	local chatSequencesUI = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatSequencesUI)

	local tab = chatSequencesUI:find("sequence_tabs")
	assert(tab)

	-- limit sequences number
	if firstRequest and tab.tabButtonNb == 7 then return -1 end

	if firstRequest == true then
		instanceSequence = r2.newComponent("ChatSequence")

		--local name = tostring(i18n.get("uiR2EdChat"))..tab.tabButtonNb+1
		--instanceSequence.Name = name
		
		local npcGroup = r2:getSelectedInstance()
		assert(npcGroup)
		
		r2.requestInsertNode(npcGroup:getBehavior().InstanceId, "ChatSequences", -1, "", instanceSequence)

		r2.ownCreatedInstances[instanceSequence.InstanceId] = true
	else

		local templateParams = {
									newElt="r2:newChat(true)",
									newEltText=tostring(i18n.get("uiR2EdNewChat")),
									eltOrderText=tostring(i18n.get("uiR2EdChatOrder")),
									upElt="r2:upChat()",
									downElt="r2:downChat()",
									maxMinElts="r2:maximizeMinimizeChats()",
									downUpColor="120 150 140 255",
									colPushed = "0 255 0 255",
									paramsL= "r2:selectSequenceTab('r2ed_chat_sequence', "..tostring(tab.tabButtonNb)..")"
							   }

		if reset == true then
			r2.ownCreatedInstances[instanceSequence.InstanceId] = true
		end

		local editorEltTemplate = "template_edit_chat"
		local sequence = r2:newElementsSequence("r2ed_chat_sequence", templateParams, editorEltTemplate, 
			instanceSequence, tostring(i18n.get("uiR2EdSequChat")))

		local chats = instanceSequence.Components
		for c = 0, chats.Size - 1 do
			r2:newChat(false, chats[c])	
		end

		local eltEditor = sequence:find("edit_element")
		assert(eltEditor)

		local editBox = eltEditor:find("edit_box_group")
		assert(editBox)

		local scroll = sequence:find("edit_box_scroll_ed")
		assert(scroll)

		scroll:setTarget(editBox.id)
	end

	return instanceSequence.InstanceId
end

------------------- REMOVE CHATS SEQUENCE ---------------------------------------------------------
function r2:removeChatsSequence()

	-- remove sequence
	r2:removeElementsSequence("r2ed_chat_sequence", "ChatSequences", tostring(i18n.get("uiR2EdSequChat")))
end

function r2:removeChatsSequenceUI(sequIndex) 

	r2:closeChatEditor()

	-- remove sequence UI
	r2:removeElementsSequenceUI(sequIndex, "r2ed_chat_sequence", "ChatSequences", tostring(i18n.get("uiR2EdSequChat")))
end

------------------- SELECT CHAT -------------------------------------------------------------------
function r2:selectChat()
	r2:selectTriggerElement(nil, "r2ed_chat_sequence")

	if getUICaller().pushed == true then
		r2:updateChatEditor() 
	end
end

------------------- OPEN CHAT EDITOR --------------------------------------------------------------
function r2:openChatEditor()
	r2:updateChatEditor()
	r2:openElementEditor("r2ed_chat_sequence", "uiR2EDChatStepEditor")
end

function r2:updateChatEditor()

	local instanceChat = r2:getSelectedEltInst("r2ed_chat_sequence")
	
	local chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	assert(chatEditor)

	-- chat name
	local chatName = chatEditor:find("name")
	assert(chatName)

	-- time
	local minutesText = chatEditor:find("minutes"):find("text")
	assert(minutesText)

	local secondsText = chatEditor:find("seconds"):find("text")
	assert(secondsText)

	-- update NPC name lists
	local whoMenuText = chatEditor:find("whoMenu"):find("text")
	assert(whoMenuText)

	local toWhoMenuText = chatEditor:find("toWhoMenu"):find("text")
	assert(toWhoMenuText)

	local editBox = chatEditor:find("says"):find("edit_box_group")
	assert(editBox)

	local emoteButtonText = chatEditor:find("emote"):find("menu"):find("text")
	assert(emoteButtonText)

	if instanceChat then

		local index = r2:searchElementIndex(instanceChat)
		if index~= nil then
			chatName.uc_hardtext = tostring(i18n.get("uiR2EdChat")).." "..index.." : "
		else
			chatName.uc_hardtext = tostring(i18n.get("uiR2EdChat")).." : "
		end

		-- after value
		local time = instanceChat.Time
		local minNb, secNb = r2:calculMinSec(time)

		minutesText.uc_hardtext = tostring(minNb)
		secondsText.uc_hardtext = tostring(secNb)
			
		-- who
		local whoInst = r2:getInstanceFromId(tostring(instanceChat.Actions[0].Who))
		whoMenuText.uc_hardtext = whoInst.Name
		
		-- says what
		local textID = instanceChat.Actions[0].Says
		if textID ~= "" then
			editBox.uc_input_string = r2:getInstanceFromId(textID).Text
		else
			editBox.uc_input_string = ""
		end

		-- to who
		local toWhoInst = r2:getInstanceFromId(tostring(instanceChat.Actions[0].Facing))
		if toWhoInst then
			toWhoMenuText.uc_hardtext = toWhoInst.Name
		else
			toWhoMenuText.uc_hardtext = tostring(i18n.get("uiR2EdNobody"))
		end

		-- emote
		local emoteName = r2.fromEmoteIdToName[instanceChat.Actions[0].Emote]
		if emoteName then
			emoteButtonText.uc_hardtext = emoteName
		else
			emoteButtonText.uc_hardtext = tostring(i18n.get("uiR2EdNoElt"))
		end

	else

		local name = tostring(i18n.get("uiR2EdChat")).." : "
		chatName.uc_hardtext = name

		minutesText.uc_hardtext = tostring(0)
		secondsText.uc_hardtext = tostring(0)

		whoMenuText.uc_hardtext = ""

		editBox.uc_input_string = ""

		toWhoMenuText.uc_hardtext = tostring(i18n.get("uiR2EdNobody"))

		emoteButtonText.uc_hardtext = tostring(i18n.get("uiR2EdNoElt"))
	end
end

------------------ OPEN CHAT SEQUENCE EDITOR  --------------------------------------------------
function r2:openChatSequenceEditor()

	local editor = getUI("ui:interface:r2ed_edit_chat_sequence")
	assert(editor)

	local chatsUI = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatsUI)

	editor.x = chatsUI.x
	editor.y = chatsUI.y
	editor.active = true
end

------------------- MAX / MIN CHAT ----------------------------------------------------------------
function r2:maximizeMinimizeChat()
	r2:maximizeMinimizeElement("r2ed_chat_sequence")
end

-------------------- NEW CHAT ---------------------------------------------------------------------
function r2:newChat(firstRequest, instanceElement, sequenceUI)
	
	local uiName = "r2ed_chat_sequence"
	
	if firstRequest == true then
		instanceElement = r2.newComponent("ChatStep")

		instanceElement.Time = 3

		local who
		local selectedInst = r2:getSelectedInstance()
		assert(selectedInst)

		if selectedInst:isGrouped() then
			who = r2:getLeader(selectedInst)
		else
			who = selectedInst
		end
		local chatAction = r2.newComponent("ChatAction")
		chatAction.Who = who.InstanceId
		table.insert(instanceElement.Actions, chatAction)

		local sequenceInstId = r2:getSelectedSequInstId(uiName)

		r2.requestInsertNode(sequenceInstId, "Components", -1, "", instanceElement)

		r2.ownCreatedInstances[instanceElement.InstanceId] = true
	else

		local templateParams =	{
									selectElt="r2:selectChat()", 
									openEltEditor="r2:openChatEditor()", 
									maxMinElt="r2:maximizeMinimizeChat()", 
									removeElt="r2:removeChat()",
									colOver="120 150 140 100",
									colPushed="120 150 140 255"
								}

		if sequenceUI == nil then
			sequenceUI = r2:getSelectedSequ(uiName)
		end
		local element = r2:newTriggerElement(uiName, tostring(i18n.get("uiR2EdChat")), templateParams, sequenceUI, instanceElement.InstanceId)
		
		r2:updateChatText(element)
	end
end

function r2:searchElementIndex(eltInst)

	local components = eltInst.Parent
	local index 
	for i=0, components.Size-1 do
		local elt = components[i]
		if elt.InstanceId == eltInst.InstanceId then
			return (i+1)
		end
	end
end

function r2:buildChatTitle(chatUI, erase)

	local chatInst = r2:getInstanceFromId(chatUI.Env.elementId)
	assert(chatInst)

	-- part1
	local index = r2:searchElementIndex(chatInst)
	if erase==true then index = index-1 end
	local part1 = tostring(i18n.get("uiR2EdChat")).." "..index.." : "

	-- part2
	local minNb, secNb = r2:calculMinSec(chatInst.Time)
	local time = ""
	if minNb ~= 0 then
		time = tostring(minNb)..tostring(i18n.get("uiR2EdShortMinutes"))
	end
	time = time.." " ..tostring(secNb)..tostring(i18n.get("uiR2EdShortSeconds"))
	local part2 = "(" ..tostring(i18n.get("uiR2EdAfter")).." ".. time..") "

	-- part3
	local part3 = ""
	local action = chatInst.Actions[0]
	local who = action.Who
	if who ~= "" then
		who = r2:getInstanceFromId(who)
		assert(who)
		part3 = who.Name .. " " ..tostring(i18n.get("uiR2EdSays")).. " " 

		local says = action.Says
		if says ~= "" then
			says = r2:getInstanceFromId(says).Text 
			says = string.substr(says, 0, 4)
		end

		part3 = part3 .. says .. "..."
	end

	-- title
	local title = chatUI:find("title")
	assert(title)
	title.uc_hardtext= part1..part2..part3
end

function r2:getChatName(chatInst)

	-- part1
	local index = r2:searchElementIndex(chatInst)
	if erase==true then index = index-1 end
	local part1 = tostring(i18n.get("uiR2EdChat")).." "..index.." : "

	-- part2
	local minNb, secNb = r2:calculMinSec(chatInst.Time)
	local time = ""
	if minNb ~= 0 then
		time = tostring(minNb)..tostring(i18n.get("uiR2EdShortMinutes"))
	end
	time = time.." " ..tostring(secNb)..tostring(i18n.get("uiR2EdShortSeconds"))
	local part2 = "(" ..tostring(i18n.get("uiR2EdAfter")).." ".. time..") "

	-- part3
	local part3 = ""
	local action = chatInst.Actions[0]
	local who = action.Who
	if who ~= "" then
		who = r2:getInstanceFromId(who)
		assert(who)
		part3 = who.Name .. " " ..tostring(i18n.get("uiR2EdSays")).. " " 

		local says = action.Says
		if says ~= "" then
			says = r2:getInstanceFromId(says).Text 
			says = string.sub(says, 1, 4)
		end

		part3 = part3 .. says .. "..."
	end

	return part1..part2..part3

end

function r2:updateChatText(elementUI)

	if elementUI == nil then
		elementUI = r2:getSelectedEltUI("r2ed_chat_sequence")
	end
	assert(elementUI)

	local instance = r2:getInstanceFromId(elementUI.Env.elementId)
	assert(instance)

	local chatText = elementUI:find("text_list")
	assert(chatText)

	chatText:clear()

	local who = tostring(instance.Actions[0].Who)
	if who ~= "" then
		
		local text = ""
		local textEmpty = true

		local facing = tostring(instance.Actions[0].Facing)
		local emote = instance.Actions[0].Emote
		local says = instance.Actions[0].Says

		if facing ~= "" then
			text = "\n"..r2:getInstanceFromId(who).Name .." "..tostring(i18n.get("uiR2EdFaces")).." ".. r2:getInstanceFromId(facing).Name
			textEmpty = false
		end
		if r2.fromEmoteIdToName[emote] ~= nil then
			text = text .. "\n" .. r2:getInstanceFromId(who).Name .. " "..string.lower(tostring(i18n.get("uiR2EdEmote"))).." : " .. r2.fromEmoteIdToName[emote]
			textEmpty = false
		end
		if r2:getInstanceFromId(says)~=nil and r2:getInstanceFromId(says).Text ~= "" then
			text = text.."\n"..r2:getInstanceFromId(who).Name .. " "..tostring(i18n.get("uiR2EdSays")).." : " .. r2:getInstanceFromId(says).Text 
			textEmpty = false
		end
		text = text.."\n"

		chatText:addTextChild(ucstring(text))

		local sep = elementUI:find("sep")
		assert(sep)
		if textEmpty == false then
			sep.active = true
		else
			chatText:clear()
			sep.active = false
		end
	end

	r2:buildChatTitle(elementUI, false)

	local chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	assert(chatEditor)

	if instance.InstanceId == r2:getSelectedEltInstId("r2ed_chat_sequence") then
		r2:updateChatEditor()
	end

	r2:updateActivitiesWhithThisChatSequence(instance, false)
end


-------------------- UP CHAT ----------------------------------------------------------------------
function r2:upChat()
	r2:upTriggerElement("r2ed_chat_sequence")
end

-------------------- DOWN CHAT --------------------------------------------------------------------
function r2:downChat()
	r2:downTriggerElement("r2ed_chat_sequence")	
end

-------------------- MAX / MIN CHATS --------------------------------------------------------------
function r2:maximizeMinimizeChats()
	r2:maximizeMinimizeTriggerElements("r2ed_chat_sequence")
end

-------------------- REMOVE CHAT ------------------------------------------------------------------
function r2:removeChat()
	r2:removeTriggerElement("r2ed_chat_sequence", tostring(i18n.get("uiR2EdChat")))
end

-------------------- CHAT TIME --------------------------------------------------------------------
function r2:setTime(minNb, secNb)

	local chatSequenceId = r2:getSelectedEltInstId("r2ed_chat_sequence")
	
	r2.requestSetNode(chatSequenceId, "Time", minNb*60+secNb)
end

-------------------- INIT TIME MENU -----------------------------------------------------------------
function r2:initTimeMenu(timeFunction, isHours)

	local timeMenu = getUI("ui:interface:r2ed_triggers_menu")
	assert(timeMenu)

	local timeMenu = timeMenu:getRootMenu()
	assert(timeMenu)

	timeMenu:reset()

	for i=0,9 do
		timeMenu:addLine(ucstring(tostring(i)), "lua", timeFunction .. "(" .. tostring(i) .. ")", tostring(i))
	end

	if isHours == true then
		timeMenu:addLine(ucstring(tostring(10)), "lua", timeFunction .. "(" .. tostring(10) .. ")", tostring(10))
	else

		local lineNb = 9
		for i=10, 50, 10 do
			local lineStr = tostring(i).."/"..tostring(i+9)
			timeMenu:addLine(ucstring(lineStr), "", "", tostring(i))
			lineNb = lineNb+1

			timeMenu:addSubMenu(lineNb)
			local subMenu = timeMenu:getSubMenu(lineNb)

			for s=0,9 do
				lineStr = tostring(i+s) 
				subMenu:addLine(ucstring(lineStr), "lua", timeFunction .. "(" .. tostring(i+s) .. ")", lineStr)
			end
		end
	end

	r2:openTriggersMenu(getUICaller())
end

-------------------- CHAT SECONDS -----------------------------------------------------------------
function r2:chatAfterSeconds(secNb)
	
	local chatStepInst = r2:getSelectedEltInst("r2ed_chat_sequence")

	local lastTime = chatStepInst.Time

	local minNb = 0
	while  lastTime > 59 do
		lastTime = lastTime - 60
		minNb = minNb + 1
	end

	r2:setTime(minNb, secNb)

	-- recover "says what" (equiv change focus)
	local chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	assert(chatEditor)

	local saysWhat = chatEditor:find("says"):find("edit_box_group").input_string
	if chatStepInst.Actions[0].Says ~= r2.registerText(saysWhat).InstanceId then
		r2:setSaysWhat(saysWhat)
	end
end

-------------------- CHAT MINUTES -----------------------------------------------------------------
function r2:chatAfterMinutes(minNb)
	
	local chatStepInst = r2:getSelectedEltInst("r2ed_chat_sequence")

	local lastTime = chatStepInst.Time

	local secNb = lastTime
	while  secNb > 59 do
		secNb = secNb - 60
	end

	r2:setTime(minNb, secNb)

	-- recover "says what" (equiv change focus)
	local chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	assert(chatEditor)

	local saysWhat = chatEditor:find("says"):find("edit_box_group").input_string
	if chatStepInst.Actions[0].Says ~= r2.registerText(saysWhat).InstanceId then
		r2:setSaysWhat(saysWhat)
	end
end

-------------------- WHO --------------------------------------------------------------------------
function r2:setWho(who, chatStepInst)

	if chatStepInst == nil then
		chatStepInst = r2:getSelectedEltInst("r2ed_chat_sequence")
	end
	assert(chatStepInst)
	
	r2.requestSetNode(chatStepInst.Actions[0].InstanceId, "Who", r2.RefId(who))
	
	if who == chatStepInst.Actions[0].Facing then
		r2:setToWho(tostring(i18n.get("uiR2EdNobody")))
	end

	-- recover "says what" (equiv change focus)
	local chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	assert(chatEditor)

	local saysWhat = chatEditor:find("says"):find("edit_box_group").input_string
	if chatStepInst.Actions[0].Says ~= r2.registerText(saysWhat).InstanceId then
		r2:setSaysWhat(saysWhat)
	end
end

function r2:openWhoMenu(whoFunction, towho)

	local menuName = "ui:interface:r2ed_triggers_menu"
	
	local whoMenu = getUI(menuName)
	local whoMenu = whoMenu:getRootMenu()
	assert(whoMenu)

	whoMenu:reset()

	local npcTable = r2.Scenario:getAllInstancesByType("Npc")

	if towho == true then
		whoMenu:addLine(ucstring(i18n.get("uiR2EdNobody")), "lua", whoFunction.."('" ..tostring(i18n.get("uiR2EdNobody")).. "')", "Nobody")
	end

	for key, npc in npcTable do
		local addLine = true
		if not npc:isBotObject() and not npc:isPlant() and not r2.isCreature(npc.InstanceId) then
			if towho == true then
				local chatStepInst= r2:getSelectedEltInst("r2ed_chat_sequence")
				assert(chatStepInst)

				local whoId = chatStepInst.Actions[0].Who
				if whoId~="" and whoId == npc.InstanceId then
					addLine = false	
				end
			end
			if addLine then
				whoMenu:addLine(ucstring(npc.Name), "lua", whoFunction.."('" ..npc.InstanceId.. "')", npc.InstanceId)	
			end
		end
	end
	
	r2:openTriggersMenu(getUICaller())
end

-------------------- SAYS WHAT --------------------------------------------------------------------
function r2:setSaysWhat(what)

	if what == nil then
		what = getUICaller().input_string
	end

	local says = what

	local chatStep = r2:getSelectedEltInst("r2ed_chat_sequence")
	if chatStep ~= nil then
		if what ~= "" then
			what=r2.registerText(what).InstanceId
		end
		r2.requestSetNode(chatStep.Actions[0].InstanceId, "Says", what)

		local chatStepUI = r2:getSelectedEltUI("r2ed_chat_sequence")
		assert(chatStepUI)
	end
end

-------------------- CLOSE CHAT EDITOR ------------------------------------------------------------
function r2:closeChatEditor(chatEditor)

	if chatEditor == nil then
		chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	end
	assert(chatEditor)

	local editSaysWhat = chatEditor:find("says"):find("edit_box_group")
	assert(editSaysWhat)

	if r2.callSetSaysWhat == true then
		r2:setSaysWhat(editSaysWhat.input_string)
	end
end

-------------------- TO WHO -----------------------------------------------------------------------
function r2:setToWho(toWho)
	
	local chatSequence = r2:getSelectedEltInst("r2ed_chat_sequence")
	assert(chatSequence)

	local chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	assert(chatEditor)

	-- recover "says what" (equiv change focus)
	local saysWhat = chatEditor:find("says"):find("edit_box_group").input_string
	if chatSequence.Actions[0].Says ~= r2.registerText(saysWhat).InstanceId then
		r2:setSaysWhat(saysWhat)
	end

	if toWho == tostring(i18n.get("uiR2EdNobody")) then toWho="" end

	r2.requestSetNode(chatSequence.Actions[0].InstanceId, "Facing", r2.RefId(toWho))
end

-------------------- EMOTE ------------------------------------------------------------------------
function r2:openEmoteMenu()

	local menuName = "ui:interface:r2ed_triggers_menu"
	
	local emoteMenu = getUI(menuName)
	local emoteMenu = emoteMenu:getRootMenu()
	assert(emoteMenu)

	emoteMenu:reset()
	initEmotesMenu(menuName, "r2:setEmote")
	r2:openTriggersMenu(getUICaller())
end

function r2:setEmote(emoteId)
	
	local chatSequence = r2:getSelectedEltInst("r2ed_chat_sequence")
	assert(chatSequence)

	r2.requestSetNode(chatSequence.Actions[0].InstanceId, "Emote", emoteId)

	-- recover "says what" (equiv change focus)
	local chatEditor = r2:getSelectedSequ("r2ed_chat_sequence"):find("edit_element")
	assert(chatEditor)

	local saysWhat = chatEditor:find("says"):find("edit_box_group").input_string
	if chatSequence.Actions[0].Says ~= r2.registerText(saysWhat).InstanceId then
		r2:setSaysWhat(saysWhat)
	end
end

------------------ SELECT CHAT SEQUENCE ---------------------------------------------------------
function r2:selectChatsSequence(index)
	r2:selectElementsSequence("r2ed_chat_sequence", index)
	r2:updateMiniActivityView()
end

---------------------------------------------------------------------------------------------------
------------------------------  FACTORISATION  ----------------------------------------------------
---------------------------------------------------------------------------------------------------

------------------------------  Remove element  ---------------------------------------------------
function r2:removeTriggerElement(uiName, elementName)

	local wndUI = getUI("ui:interface:"..uiName)
	assert(wndUI)

	local tab = wndUI:find("sequence_tabs")
	assert(tab)

	local sequenceUI = wndUI:find(tab.associatedGroupSelection)
	assert(sequenceUI)

	r2:removeElement(sequenceUI, uiName, elementName)
end

-----------------------------------------------------------------------------------------------------
function r2:removeElement(sequence, uiName, elementName, removedElement)

	local listElements = sequence:find("elements_list")
	assert(listElements)

	local toErasedInstId
	if removedElement == nil then
		toErasedInstId = r2:getSelectedEltInstId(uiName)
	else
		toErasedInstId = removedElement.Env.elementId
	end
	assert(toErasedInstId)

	-- request erase node
	if toErasedInstId ~= nil and r2:getInstanceFromId(toErasedInstId) ~= nil then
		r2.requestEraseNode(toErasedInstId, "", -1)
	end
end

function r2:removeElementUI(sequence, uiName, removedElement)

	local listElements = sequence:find("elements_list")
	assert(listElements)

	-- update follow elements number
	local removedIndex = listElements:getElementIndex(removedElement)
	for i = removedIndex+1, (listElements.childrenNb-1) do
		local element = listElements:getChild(i)
		assert(element)

		if not element.Env.isEditor then
			r2:buildElementTitle(uiName, element, true)
		end
	end
	
	-- delete element and update coordinates of elements list
	if removedElement == r2:getSelectedEltUI(uiName) then
		r2:setSelectedEltUIId(uiName, nil)

		-- inactive element editor
		local eltEditor = listElements:find("edit_element")
		assert(eltEditor)
		eltEditor.active = false

		-- disactive up and down element buttons
		local upElement = sequence:find("up_element")
		assert(upElement)
		local downElement = sequence:find("down_element")
		assert(downElement)
		local elementOrder = sequence:find("element_order")
		assert(elementOrder)

		upElement.active = false
		downElement.active = false
		elementOrder.active = false
	end
	listElements.Env.elementsNb = listElements.Env.elementsNb - 1

	listElements:delChild(removedElement)
	listElements.parent:invalidateCoords()

	-- if any activity in list, disactive global minimize / maximize button
	if listElements.childrenNb == 1 then
		local minElts = sequence:find("minimize_elements")
		assert(minElts)

		local maxElts = sequence:find("maximize_elements")
		assert(maxElts)

		minElts.active = false
		maxElts.active = false
	end
end

------------------------------  Max / Min elements  -----------------------------------------------
function r2:maximizeMinimizeTriggerElements(uiName)

	local sequence = r2:getSelectedSequ(uiName)
	assert(sequence)

	r2:maximizeMinimizeElements(sequence, uiName)
end

------------------------------  Max / Min elements  -----------------------------------------------
function r2:maximizeMinimizeElements(sequence, uiName)

	local elements = sequence:find("elements_list")
	assert(elements)

	if elements.Env.minimize == nil then
		elements.Env.minimize = true
	end

	elements.Env.minimize = not elements.Env.minimize

	for i = 0, elements.childrenNb-1 do
		local element = elements:getChild(i)
		assert(element)

		if element.Env.isEditor ~= true then
			r2:maximizeMinimizeElement(uiName, element, elements.Env.minimize)
		end
	end

	local minElts = sequence:find("minimize_elements")
	assert(minElts)
	local maxElts = sequence:find("maximize_elements")
	assert(maxElts)

	if elements.Env.minimize == true then
		minElts.active = false
		maxElts.active = true
	else
		minElts.active = true
		maxElts.active = false
	end
end

------------------------------  Max / Min element  ------------------------------------------------
function r2:maximizeMinimizeElement(uiName, element, allMinimize)

	if element == nil then
		element = getUICaller().parent.parent.parent.parent.parent.parent
	end
	assert(element)

	local eltText = element:find("element_text")
	assert(eltText)

	local allMin, allMax = true, true
	if allMinimize ~= nil then
		allMin = allMinimize 
		allMax = not allMin
	end

	local maxButton = element:find("maximize_element")
	assert(maxButton)
	local minButton = element:find("minimize_element")
	assert(minButton)

	-- maximize
	if allMax and eltText.active==false then
		eltText.active = true
		maxButton.active = false
		minButton.active = true

	-- minimize
	elseif allMin and eltText.active==true then
		eltText.active = false
		maxButton.active = true
		minButton.active = false
	end
end

------------------------------------ Down element --------------------------------------------------
function r2:downTriggerElement(uiName)
	
	local eltSequenceUI = getUI("ui:interface:"..uiName)
	assert(eltSequenceUI)

	local tab = eltSequenceUI:find("sequence_tabs")
	assert(tab)

	local sequence = eltSequenceUI:find(tab.associatedGroupSelection)
	assert(sequence)

	local listElements = sequence:find("elements_list")
	assert(listElements)

	local selectedElement = r2:getSelectedEltUI(uiName)
	local index = listElements:getElementIndex(selectedElement)

	local sequenceId = r2:getSelectedSequInstId(uiName)
	if index < r2:getInstanceFromId(sequenceId).Components.Size-1 then
		r2.requestMoveNode(sequenceId, "Components", index+1,
						   sequenceId, "Components", index)
	end
end

------------------------------------ Down or up element UI ------------------------------------------

function r2:downUpElement(elementUI, uiName)

	local listElements = elementUI.parent
	assert(listElements)

	local eltEditor = listElements:find("edit_element")
	assert(eltEditor)

	local index = listElements:getElementIndex(elementUI)

	if index > 0 then
		local previousElement = listElements:getChild(index - 1)

		local editorBetweenBothElts = false
		if previousElement.Env.elementId==nil then
			previousElement = listElements:getChild(index - 2)
			editorBetweenBothElts = true
		end

		listElements:upChild(elementUI)
		if editorBetweenBothElts==true then
			listElements:upChild(elementUI)
		end

		local selectedEltUI = r2:getSelectedEltUI(uiName)
		if selectedEltUI~=nil then
			local selectedEltIndex = listElements:getElementIndex(selectedEltUI)
			local eltEditorIndex = listElements:getElementIndex(eltEditor)
			if eltEditorIndex~=selectedEltIndex+1 then
				if eltEditorIndex<selectedEltIndex then
					for i=eltEditorIndex, selectedEltIndex-1 do
						listElements:downChild(eltEditor)
					end
				else
					for i=selectedEltIndex, eltEditorIndex-2 do
						listElements:upChild(eltEditor)
					end
				end

			end

			if selectedEltUI==previousElement then
				previousElement.active = false
			elseif selectedEltUI==elementUI then
				elementUI.active = false
			end

			r2:updateElementEditor(uiName)
		end

		r2:buildElementTitle(uiName, previousElement, false)
		r2:buildElementTitle(uiName, elementUI, false)
	end
end

function r2:updateElementEditor(uiName)

	if uiName == "r2ed_triggers" then
		r2:updateActivityEditor()
	elseif uiName == "r2ed_chat_sequence" then
		r2:updateChatEditor()
	end
end

function r2:buildElementTitle(uiName, elementUI, erase)

	if uiName == "r2ed_triggers" then
		r2:buildActivityTitle(elementUI, erase)
	elseif uiName == "r2ed_chat_sequence" then
		r2:buildChatTitle(elementUI, erase)
	elseif uiName == r2.logicEntityUIPath.."actions" then
		r2:buildActionTitle(elementUI, erase)
	elseif uiName == r2.logicEntityUIPath.."reactions" then
		r2:buildReactionTitle(elementUI, erase)
	end
end

------------------------------------ Up element ------------------------------------------------------
function r2:upTriggerElement(uiName)
	
	local eltSequenceUI = getUI("ui:interface:"..uiName)
	assert(eltSequenceUI)

	local tab = eltSequenceUI:find("sequence_tabs")
	assert(tab)

	local sequence = eltSequenceUI:find(tab.associatedGroupSelection)
	assert(sequence)

	local listElements = sequence:find("elements_list")
	assert(listElements)

	local selectedElement = r2:getSelectedEltUI(uiName)
	local index = listElements:getElementIndex(selectedElement)

	if index>0 then
		local sequenceId = r2:getSelectedSequInstId(uiName)
		r2.requestMoveNode(sequenceId, "Components", index,
						   sequenceId, "Components", index-1)
	end
end

---------------------------------------------------------------------------------------------------
function r2:selectTriggerElement(sequenceUI, uiName, selectedButtonElt)

	if sequenceUI == nil then
		sequenceUI = r2:getSelectedSequ(uiName)
	end
	r2:selectElement(sequenceUI, uiName, true, selectedButtonElt)
end

---------------------------------------------------------------------------------------------------
function r2:closeElementEditor(uiName)
	
	if uiName == "r2ed_chat_sequence" then
		r2:closeChatEditor(getUICaller())
	end

	local sequenceUI = r2:getSelectedSequ(uiName)
	assert(sequenceUI)

	local selectedEltUI = r2:getSelectedEltUI(uiName)
	assert(selectedEltUI)

	local selectedEltButton = selectedEltUI:find("select")
	assert(selectedEltButton)

	selectedEltButton.pushed = false

	r2:selectElement(sequenceUI, uiName, false, selectedEltButton)
end

---------------- SELECT ELEMENT -------------------------------------------------------------------
function r2:selectElement(sequence, uiName, eltOrder, selectedButtonElt)

	local upElement = sequence:find("up_element")
	assert(upElement)
	local downElement = sequence:find("down_element")
	assert(downElement)
	local orderElt = sequence:find("element_order")

	if selectedButtonElt == nil then
		selectedButtonElt = getUICaller()
	end

	-- new selected element
	if selectedButtonElt.pushed == true then
		
		if r2:getSelectedEltUIId(uiName) then

			local lastSelectedElement = r2:getSelectedEltUI(uiName)
			assert(lastSelectedElement)

			local lastEltsList = lastSelectedElement.parent
			local editElt = lastEltsList:find("edit_element")
			assert(editElt)

			if r2:getSelectedEltUIId(uiName) == selectedButtonElt.parent.parent.parent.id then 
				return 
			end
			
			lastSelectedElement.active = true 
			lastSelectedElement:find("select").pushed = false
			
			editElt.active = false
		end

		r2:setSelectedEltUIId(uiName, selectedButtonElt.parent.parent.parent.id)
		
		local selectedElement = selectedButtonElt.parent.parent.parent
		assert(selectedElement)

		if eltOrder then
			upElement.active = true
			downElement.active = true
			orderElt.active = true
		end

		-- update element editor position in list
		local eltsList = sequence:find("elements_list")
		assert(eltsList)
		local editElt = eltsList:find("edit_element")
		assert(editElt)

		local indexSelectedElt = eltsList:getElementIndex(selectedElement)
		local indexEltEditor = eltsList:getElementIndex(editElt)
		
		if indexEltEditor<indexSelectedElt then
			for i=indexEltEditor, indexSelectedElt-1 do
				eltsList:downChild(editElt)
			end
		else
			for i=indexSelectedElt, indexEltEditor-2 do
				eltsList:upChild(editElt)
			end
		end

		editElt.active = true
		selectedElement.active = false

	-- cancel current selection 
	else
		local lastSelectedElement = r2:getSelectedEltUI(uiName)
		r2:setSelectedEltUIId(uiName, nil)
		
		upElement.active = false
		downElement.active = false
		orderElt.active = false

		local lastEltsList = lastSelectedElement.parent
		local editElt = lastEltsList:find("edit_element")
		assert(editElt)

		editElt.active = false
		lastSelectedElement.active = true
	end
end

------------------ SELECT SEQUENCE ---------------------------------------------------------
function r2:selectElementsSequence(uiName, index)
	local eltsWnd = getUI("ui:interface:"..uiName)
	assert(eltsWnd)

	local sequencesTab = eltsWnd:find("sequence_tabs")
	assert(sequencesTab)

	if (index>=0) and (sequencesTab.tabButtonNb > index) then
		sequencesTab.selection = tonumber(index)

		local repeatButton = eltsWnd:find("repeat_group"):find("repeat"):find("toggle_butt")
		assert(repeatButton)

		local sequenceUI = sequencesTab:getGroup(index)
		assert(sequenceUI)
		local sequenceInstId = sequenceUI.Env.sequenceId

		if sequenceInstId and r2:getInstanceFromId(sequenceInstId) then
			local sequenceInst = r2:getInstanceFromId(sequenceInstId)
			repeatButton.pushed = (sequenceInst.Repeating == 0)
		end
	end
end


---------------------------------------- Open element editor --------------------------------------
function r2:openElementEditor(uiName, title, baseWindowName)

	local elementEditor = r2:getSelectedSequ(uiName):find("edit_element")
	assert(elementEditor)

	local selectedElement = r2:getSelectedEltUI(uiName)
	assert(selectedElement)

	elementEditor.active = true
	selectedElement.active = false
end

----------------------------------- Remove elements sequence --------------------------------------
function r2:removeElementsSequence(uiName, elementsTable, sequName)

	local eltSequenceUI = getUI("ui:interface:"..uiName)
	assert(eltSequenceUI)

	local tab = eltSequenceUI:find("sequence_tabs")
	assert(tab)

	local activeLogicEntity = r2:getSelectedInstance()
	assert(activeLogicEntity)
	local activitySequences = activeLogicEntity:getBehavior()[elementsTable]
	
	--local seq = sequName

	--for i=tab.selection+1,(tab.tabButtonNb-1) do
	--	local buttonTab = tab:find("tab"..i)
	--	assert(buttonTab)
		
	--	if activitySequences[i].Name == seq..(i+1) then
	--		local newText = seq..(i)
	--		r2.requestSetNode(activitySequences[i].InstanceId, "Name", newText)
	--	end	
	--end

	local sequenceId = r2:getSelectedSequInstId(uiName)
	-- request erase node
	r2.requestEraseNode(sequenceId, "", -1)

	return sequenceId
end

function r2:removeElementsSequenceUI(tabIndex, uiName, elementsTable, sequName)

	local wndUI = getUI("ui:interface:"..uiName)
	assert(wndUI)

	local tab = wndUI:find("sequence_tabs")
	assert(tab)

	local activeLogicEntity = r2:getSelectedInstance()
	assert(activeLogicEntity)
	local activitySequences = activeLogicEntity:getBehavior()[elementsTable]
	
	for i=tabIndex+1,(tab.tabButtonNb-1) do
		local buttonTab = tab:find("tab"..i)
		assert(buttonTab)
		
		buttonTab.params_l = "r2:selectSequenceTab('"..uiName.."', "..tostring(i-1)..")"

		if buttonTab.hardtext == sequName..(i+1) then
			buttonTab.uc_hardtext = sequName..i
		end
	end

	-- recover name of the future "first sequence"
	local firstEltName 
	local indexName
	if tab.tabButtonNb > 1 then
		if tabIndex==0 then
			indexName = 1
		else
			indexName = 0
		end
		--if activitySequences[indexName].Name==sequName..(indexName+1) then
		if r2:getSequenceName(activitySequences[indexName])==sequName..(indexName+1) then
			firstEltName = sequName.."1"
		else
			--firstEltName = activitySequences[indexName].Name
			firstEltName = r2:getSequenceName(activitySequences[indexName])
		end
	else
		firstEltName = tostring(i18n.get("uiR2EDSequences"))
	end

	local selectedElt = r2:getSelectedEltUI(uiName)
	if selectedElt and selectedElt.parent.parent.parent.id == r2:getSelectedSequ(uiName).id then
		r2:setSelectedEltUIId(uiName, nil)
	end

	tab:removeTab(tabIndex)

	if tab.tabButtonNb == 0 then
		r2:cleanSequenceEditor(wndUI)
	end

	return firstEltName
end

function r2:cleanSequenceEditor(eltSequenceUI)

	local sepTop = eltSequenceUI:find("sep_top")
	assert(sepTop)

	local sepBottom = eltSequenceUI:find("sep_bottom")
	assert(sepBottom)

	local sepLeft = eltSequenceUI:find("sep_left")
	assert(sepLeft)

	local sepRight = eltSequenceUI:find("sep_right")
	assert(sepRight)

	local removeSequence = eltSequenceUI:find("remove_sequence_button")
	assert(removeSequence)

	local editSequence = eltSequenceUI:find("edit_sequence")
	assert(editSequence)

	sepTop.active = false
	sepBottom.active = false
	sepLeft.active= false
	sepRight.active= false
	removeSequence.active = false
	editSequence.active = false

	local repeatButtonGr = eltSequenceUI:find("repeat_group")
	repeatButtonGr.active = false
end

------------------ new elements sequence  -------------------------------------------------------------
function r2:newElementsSequence(uiName, templateParams, editorEltTemplate, instance, sequName)

	local eltSequenceUI = getUI("ui:interface:"..uiName)
	assert(eltSequenceUI)

	local menu = eltSequenceUI:find("sequence_menu")
	assert(menu)

	local tab = menu:find("sequence_tabs")
	assert(tab)

	local newTabNb = tab.tabButtonNb+1
	local posParent, posRef, id, hardText, group
	if newTabNb == 1 then
		posParent = "parent"
		posRef = "TL TL"
	else
		posParent = "tab"..(newTabNb-2)
		posRef = "TR TL"
	end

	id = "tab"..(newTabNb-1)
	local group = "sequence"..r2.sequencesNb

	r2.sequencesNb = r2.sequencesNb + 1

	local newTabGroup = createUIElement("sequence_elements_template", menu.id, {id=group,
		new_elt=templateParams.newElt, new_elt_text=templateParams.newEltText, 
		elt_order_text=templateParams.eltOrderText, up_elt=templateParams.upElt, 
		down_elt=templateParams.downElt, max_min_elts=templateParams.maxMinElts,
		down_up_color=templateParams.downUpColor})
	assert(newTabGroup)

	menu:addGroup(newTabGroup)

	newTabGroup.Env.sequenceId = instance.InstanceId
	
	local tabName
	if instance.Name~= "" then
		tabName = instance.Name
	else
		local comps = instance.Parent
		for i=0, comps.Size-1 do
			if comps[i].InstanceId == instance.InstanceId then
				tabName = sequName..(i+1)
				break
			end
		end
	end

	local newTab = createUIElement("sequence_tab_template", tab.id, 
		{id=id, posparent=posParent, posref=posRef, hardtext=tabName, group=group,
		 col_pushed =templateParams.colPushed, params_l=templateParams.paramsL})
	assert(newTab)
	
	tab:addTab(newTab)

	if r2.ownCreatedInstances[instance.InstanceId] == true then
		tab.selection = tab.tabButtonNb-1
		r2.ownCreatedInstances[instance.InstanceId] = nil

		--local repeatButton = eltSequenceUI:find("repeat_group"):find("repeat"):find("toggle_butt")
		--assert(repeatButton)
		--repeatButton.pushed = (instance.Repeating == 0)

		r2:selectElementsSequence(uiName, tab.tabButtonNb-1)
	end

	if tab.tabButtonNb == 1 then
		local sepTop = menu:find("sep_top")
		assert(sepTop)

		local sepBottom = menu:find("sep_bottom")
		assert(sepBottom)

		local sepLeft = menu:find("sep_left")
		assert(sepLeft)

		local sepRight = menu:find("sep_right")
		assert(sepRight)

		local removeSequence = eltSequenceUI:find("remove_sequence_button")
		assert(removeSequence)

		local editSequence = eltSequenceUI:find("edit_sequence")
		assert(editSequence)

		sepTop.active = true
		sepBottom.active = true
		sepLeft.active= true
		sepRight.active= true
		removeSequence.active = true
		editSequence.active = true

		if uiName == "r2ed_triggers" then   --TEMP TEMP TEMP
			local repeatButtonGr = eltSequenceUI:find("repeat_group")
			repeatButtonGr.active = true	
		end									--TEMP TEMP TEMP
	end

	local listElements = newTabGroup:find("elements_list")
	assert(listElements)
	listElements.Env.elementsCount = nil

	-- add element editor in list
	local newEditorElt = createGroupInstance(editorEltTemplate, listElements.id, {id="edit_element", active="false"})
	assert(newEditorElt)
	listElements:addChild(newEditorElt)
	listElements.parent:updateCoords()

	newEditorElt.Env.isEditor = true

	newEditorElt.active = false
	
	return newTabGroup
end

---------------------------- new element --------------------------------------------------------------
function r2:newTriggerElement(uiName, elementName, templateParams, sequenceUI, instanceId)

	if sequenceUI == nil then
		eltSequenceUI = getUI("ui:interface:"..uiName)
		assert(eltSequenceUI)

		local tab = eltSequenceUI:find("sequence_tabs")
		assert(tab)

		local sequenceUI = eltSequenceUI:find(tab.associatedGroupSelection)
	end
	assert(sequenceUI)

	local newElement = r2:newElement(sequenceUI, elementName, templateParams, true)

	newElement.Env.elementId = instanceId

	if r2.ownCreatedInstances[instanceId] == true then
		newElement:find("select").pushed = true 
		r2:selectTriggerElement(sequenceUI, uiName, newElement:find("select"))
		r2.ownCreatedInstances[instanceId] = nil

		if uiName == "r2ed_triggers" then
			r2:updateActivityEditor()
		elseif uiName == "r2ed_chat_sequence" then
			r2:updateChatEditor()
		end
	end

	r2:maximizeMinimizeElement(uiName, newElement)

	return newElement
end

---------------------------- new base element --------------------------------------------------------------
function r2:newElement(sequence, elementName, templateParams, eltOrder)

	local listElements = sequence:find("elements_list")
	assert(listElements)

	-- counter for element group id
	if listElements.Env.elementsCount == nil then
		listElements.Env.elementsCount = 0
		listElements.Env.elementsNb = 1
	else
		listElements.Env.elementsCount = listElements.Env.elementsCount+1
		listElements.Env.elementsNb = listElements.Env.elementsNb+1
	end

	local elementId = "elt"..listElements.Env.elementsCount

	local hardText = elementName.." "..listElements.Env.elementsNb.." : "

	-- create new element
	local newElement = createGroupInstance("element_template", listElements.id, 
		{id=elementId, posref="TL TL", x="0", y="0", sizeref="w", hardtext=hardText,
		 select_elt=templateParams.selectElt, open_elt_editor=templateParams.openEltEditor,
		 max_min_elt=templateParams.maxMinElt, remove_elt=templateParams.removeElt, open_chat=templateParams.openChat,
		 col_over=templateParams.colOver, col_pushed=templateParams.colPushed})

	-- add element to list
	listElements:addChild(newElement)
	listElements.parent:updateCoords()
	
	-- active global minimize / maximize button
	if listElements.childrenNb == 2 then
		local maxElts = sequence:find("maximize_elements")
		assert(maxElts)
		maxElts.active = true

		if eltOrder then
			
			local elementOrder = sequence:find("element_order")
			assert(elementOrder)
			elementOrder.active=true
		end
	end

	-- scroll goes down to new element
	local sequenceContent = newElement.parent.parent.parent
	assert(sequenceContent)

	local scrollBar = sequenceContent:find("scroll_objects")
	assert(scrollBar)
	scrollBar.trackPos = 0

	-- target sroll text
	local scroll = newElement:find("scroll_bar_text")
	assert(scroll)

	local scrollText = newElement:find("scroll_text_gr")
	assert(scrollText)
	scroll:setTarget(scrollText.id)

	return newElement
end

------------------ OPEN SEQUENCE EDITOR  --------------------------------------------------
function r2:openSequenceEditor(uiName, editorName)

	local editor = getUI("ui:interface:"..editorName)
	assert(editor)

	local eltsUI = getUI("ui:interface:"..uiName)
	assert(eltsUI)

	editor.x = eltsUI.x
	editor.y = eltsUI.y
	editor.active = true

	-- update edit box text with current sequence name
	local editName = editor:find("sequence_name"):find("edit_box_group")
	assert(editName)

	local tab = eltsUI:find("sequence_tabs")
	assert(tab)

	local buttonTab = tab:find("tab"..tab.selection)
	assert(buttonTab)

	editName.uc_input_string = buttonTab.uc_hardtext
end

------------------ SET SEQUENCE NAME  -----------------------------------------------------
function r2:setSequenceName(uiName, editorName)

	local editor = getUI("ui:interface:"..editorName)
	assert(editor)

	local editName = editor:find("sequence_name"):find("edit_box_group")
	assert(editName)

	local name = editName.input_string

	local sequenceInstId = r2:getSelectedSequInstId(uiName)
	r2.requestSetNode(sequenceInstId, "Name", name)
end

function r2:updateSequenceName(uiName, instance)

	--local name = instance.Name
	local name = r2:getSequenceName(instance)
	local sequInstId = instance.InstanceId
	 
	local eltsUI = getUI("ui:interface:"..uiName)
	assert(eltsUI)

	local tab = eltsUI:find("sequence_tabs")
	assert(tab)

	local tabId
	for i=0, tab.tabButtonNb-1 do
		local sequenceUI = tab:getGroup(i)
		if sequenceUI.Env.sequenceId == sequInstId then
			tabId = i
			break
		end
	end

	local buttonTab = tab:find("tab"..tabId)
	assert(buttonTab)
	
	buttonTab.uc_hardtext = name

	if uiName == "r2ed_triggers" then
		r2:updateSequencesButtonBar(tabId, name)
	end
end

function r2:updateSequenceRepeatingOption(uiName, instance)

	local sequenceUI = r2:getSelectedSequ(uiName)

	if sequenceUI.Env.sequenceId == instance.InstanceId then

		local eltsUI = getUI("ui:interface:"..uiName)
		local repeatButton = eltsUI:find("repeat_group"):find("repeat"):find("toggle_butt")
		repeatButton.pushed = (instance.Repeating == 0)
	end
end


--------------------------------------------------------------------------------------------------
-------------------------- ACTIVE LOGIC ENTITY DisplayerProperties -----------------------------------------
--------------------------------------------------------------------------------------------------

local activeLogicEntityPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
r2.activitiesAndChatsUIUpdate = false
function activeLogicEntityPropertySheetDisplayerTable:onSelect(instance, isSelected)

	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
	
	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)

	if not isSelected then
		triggersUI.active = false
		r2:closeMiniActivityView()
		r2:closeChatSequencesUI()

		r2:cleanActivitiesAndChatsUI()
		r2:cleanLogicEntityUI()
	else
		r2.activitiesAndChatsUIUpdate = false
	end
end

r2.callSetSaysWhat = true
------------------------------------------------
function r2:cleanActivitiesAndChatsUI()

	-- update chat sequence combo box
	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)
	local chatSequencesUI = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatSequencesUI)

	-- remove all tabs
	local sequenceMenu = triggersUI:find("sequence_menu")
	assert(sequenceMenu)

	local tabActivities = triggersUI:find("sequence_tabs")
	assert(tabActivities)

	for i=0, tabActivities.tabButtonNb-1 do
		local groupTab = tabActivities:getGroup(i)
		assert(groupTab)
		sequenceMenu:delGroup(groupTab)
	end

	tabActivities:removeAll()
	r2:setSelectedEltUIId("r2ed_triggers", nil)
	r2:cleanSequenceEditor(triggersUI)

	-- delete chat sequences
	sequenceMenu = chatSequencesUI:find("sequence_menu")
	assert(sequenceMenu)

	local tabChats = chatSequencesUI:find("sequence_tabs")
	assert(tabChats)

	for i=0, tabChats.tabButtonNb-1 do
		local groupTab = tabChats:getGroup(i)
		assert(groupTab)
		sequenceMenu:delGroup(groupTab)
	end

	tabChats:removeAll()
	r2:setSelectedEltUIId("r2ed_chat_sequence", nil)
	r2:cleanSequenceEditor(chatSequencesUI)
end

function r2:updateActivitiesAndChatsUI(instance)

	if r2.activitiesAndChatsUIUpdate==true then
		return
	end

	r2.activitiesAndChatsUIUpdate = true

	if r2.lastSelectedActivitySequence==nil then

		r2:cleanActivitiesAndChatsUI()

		-- update chat sequence combo box
		local triggersUI = getUI("ui:interface:r2ed_triggers")
		assert(triggersUI)
		local chatSequencesUI = getUI("ui:interface:r2ed_chat_sequence")
		assert(chatSequencesUI)

		-- remove all tabs
		local tabActivities = triggersUI:find("sequence_tabs")
		assert(tabActivities)

		-- delete chat sequences
		local tabChats = chatSequencesUI:find("sequence_tabs")
		assert(tabChats)


		local activitySequences = instance:getBehavior().Activities
		local chatSequences = instance:getBehavior().ChatSequences

		-- build sequences of selected NPC group
		local uiName = "r2ed_triggers"
		
		for s = 0, activitySequences.Size - 1 do
			local sequence = activitySequences[s]
			r2:newActivitiesSequence(false, sequence, true)
		end
		
		if tabActivities.tabButtonNb>1 then
			tabActivities.selection = 0
			r2:selectElementsSequence(uiName, 0)
		end
		for s=0, tabActivities.tabButtonNb-1 do 
			local sequenceUI = tabActivities:getGroup(s)
			assert(sequenceUI)
			
			local eltsList = sequenceUI:find("elements_list")
			assert(eltsList)
			if eltsList.childrenNb > 1 then
				local firstElt = eltsList:getChild(1)
				local selectedButton = firstElt:find("select")
				selectedButton.pushed = true
				
				r2:selectElement(sequenceUI, uiName, true, selectedButton)
				
				r2:updateActivityEditor()

				break
			end
		end

		uiName = "r2ed_chat_sequence"
		r2.callSetSaysWhat = false
		for s = 0, chatSequences.Size - 1 do
			local sequence = chatSequences[s]
			r2:newChatsSequence(false, sequence, true)
		end
		r2.callSetSaysWhat = true
		if tabChats.tabButtonNb>1 then
			tabChats.selection = 0
			r2:selectElementsSequence(uiName, 0)
		end
		for s=0, tabChats.tabButtonNb-1 do 
			local sequenceUI = tabChats:getGroup(s)
			assert(sequenceUI)

			local eltsList = sequenceUI:find("elements_list")
			assert(eltsList)
			if eltsList.childrenNb > 1 then
				local firstElt = eltsList:getChild(1)
				local selectedButton = firstElt:find("select")
				r2.callSetSaysWhat = false
				selectedButton.pushed = true
				
				r2:selectElement(sequenceUI, uiName, true, selectedButton)
			
				r2.callSetSaysWhat = true

				r2:updateChatEditor()
			end
		end
		
		triggersUI.uc_title = tostring(i18n.get("uiR2EDActivitySequenceEditor")) .. r2:getSelectedInstance().Name
		chatSequencesUI.uc_title = tostring(i18n.get("uiR2EDChatSequenceEditor")) .. r2:getSelectedInstance().Name
	end
end

------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onAttrModified(instance, attributeName)

	r2:logicEntityPropertySheetDisplayer():onAttrModified(instance, attributeName)

	if not r2.activitiesAndChatsUIUpdate or instance ~= r2:getSelectedInstance() then
		return
	end

	if attributeName == "Name" then

		local triggersUI = getUI("ui:interface:r2ed_triggers")
		assert(triggersUI)

		local chatSequencesUI = getUI("ui:interface:r2ed_chat_sequence")
		assert(chatSequencesUI)

		triggersUI.uc_title = tostring(i18n.get("uiR2EDActivitySequenceEditor")) .. instance[attributeName]
		chatSequencesUI.uc_title = tostring(i18n.get("uiR2EDChatSequenceEditor")) .. instance[attributeName]	
	end
end	

------------------------------------------------
function r2:activeLogicEntityPropertySheetDisplayer()	
	return activeLogicEntityPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end


--------------------------------------------------------------------------------------------------
-------------------------- NPC GROUP DisplayerProperties -----------------------------------------
--------------------------------------------------------------------------------------------------

local npcGroupPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onSelect(instance, isSelected)	
	r2:activeLogicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
	r2:activeLogicEntityPropertySheetDisplayer():onAttrModified(instance, attributeName)
end	

------------------------------------------------
function r2:npcGroupPropertySheetDisplayer()	
	return npcGroupPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end

	




--------------------------------------------------------------------------------------------------
-------------------------------- ACTIVITY SEQUENCE DisplayerProperties ---------------------------
--------------------------------------------------------------------------------------------------
local activitySequencePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onPostCreate(instance)	

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	r2:newActivitiesSequence(false, instance, false)
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onErase(instance)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)

	local sequenceTabs = triggersUI:find("sequence_tabs")
	assert(sequenceTabs)

	local sequIndex
	for i=0,sequenceTabs.tabButtonNb-1 do
		local sequence = sequenceTabs:getGroup(i)
		if sequence.Env.sequenceId == instance.InstanceId then
			sequIndex = i
			break
		end
	end
	
	if sequIndex~=nil then
		r2:removeActivitiesSequenceUI(sequIndex)
	end
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onAttrModified(instance, attributeName)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	if attributeName == "Name" then
		r2:updateSequenceName("r2ed_triggers", instance)
	elseif attributeName == "Repeating" then
		r2:updateSequenceRepeatingOption("r2ed_triggers", instance)
	end			
end	

------------------------------------------------
function r2:activitySequencePropertySheetDisplayer()	
	return activitySequencePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end


--------------------------------------------------------------------------------------------------
-------------------------------- CHAT SEQUENCE DisplayerProperties ---------------------------
--------------------------------------------------------------------------------------------------
local chatSequencePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onPostCreate(instance)	

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	r2:newChatsSequence(false, instance)
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onErase(instance)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local chatsUI = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatsUI)

	local sequenceTabs = chatsUI:find("sequence_tabs")
	assert(sequenceTabs)

	local sequIndex
	for i=0,sequenceTabs.tabButtonNb-1 do
		local sequence = sequenceTabs:getGroup(i)
		if sequence.Env.sequenceId == instance.InstanceId then
			sequIndex = i
			break
		end
	end
	
	if sequIndex~=nil then
		r2:removeChatsSequenceUI(sequIndex)
	end

	r2:updateMiniActivityView()
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onAttrModified(instance, attributeName)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	if attributeName == "Name" then
		r2:updateSequenceName("r2ed_chat_sequence", instance)
	elseif attributeName == "Repeating" then
		r2:updateSequenceRepeatingOption("r2ed_chat_sequence", instance)
	end			
end	

------------------------------------------------
function r2:chatSequencePropertySheetDisplayer()	
	return chatSequencePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end


--------------------------------------------------------------------------------------------------
-------------------------------- ACTIVITY STEP DisplayerProperties--------------------------------
--------------------------------------------------------------------------------------------------
local activityStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onPostCreate(instance)	

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local activitySequInst = instance.Parent.Parent

	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)

	local sequenceTabs = triggersUI:find("sequence_tabs")
	assert(sequenceTabs)

	local sequenceUI
	for i=0,sequenceTabs.tabButtonNb-1 do
		local sequence = sequenceTabs:getGroup(i)
		if sequence.Env.sequenceId == activitySequInst.InstanceId then
			sequenceUI = sequence
			break
		end
	end

	r2:newActivity(false, nil, instance, sequenceUI)
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onErase(instance)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local sequenceUI, elementUI = r2:findSequenceAndElementUIFromInstance(instance, "r2ed_triggers")

	if elementUI ~= nil then
		r2:removeActivityUI(sequenceUI, elementUI)
	end
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onPostHrcMove(instance)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local sequenceUI, activityStepUI = r2:findSequenceAndElementUIFromInstance(instance, "r2ed_triggers")
	if activityStepUI then
		r2:downUpElement(activityStepUI, "r2ed_triggers")
	end	
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
	
	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local sequenceUI, elementUI = r2:findSequenceAndElementUIFromInstance(instance, "r2ed_triggers")
	if elementUI then
		r2:updateActivityChatSequence(elementUI)
	end
end	

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)

	if refIdName == "ActivityZoneId" then
		r2.requestSetNode(instance.InstanceId, "Activity", "Stand Still")
		r2.requestSetNode(instance.InstanceId, "ActivityZoneId", r2.RefId(""))
		r2.requestSetNode(instance.InstanceId, "TimeLimit", "No Limit")
		r2.requestSetNode(instance.InstanceId, "TimeLimitValue", "")
	elseif refIdName == "Chat" then
		r2.requestSetNode(instance.InstanceId, "Chat", r2.RefId(""))
	end
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
end

-------------------------------------------------
function r2:activityStepPropertySheetDisplayer()	
	return activityStepPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- CHAT STEP DisplayerProperties------------------------------------
--------------------------------------------------------------------------------------------------
local chatStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function chatStepPropertySheetDisplayerTable:onPostCreate(instance)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local chatSequInst = instance.Parent.Parent

	local chatsUI = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatsUI)

	local sequenceTabs = chatsUI:find("sequence_tabs")
	assert(sequenceTabs)

	local sequenceUI
	for i=0,sequenceTabs.tabButtonNb-1 do
		local sequence = sequenceTabs:getGroup(i)
		if sequence.Env.sequenceId == chatSequInst.InstanceId then
			sequenceUI = sequence
			break
		end
	end

	r2:newChat(false, instance, sequenceUI)	
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onErase(instance)

	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local sequenceUI, elementUI = r2:findSequenceAndElementUIFromInstance(instance, "r2ed_chat_sequence")
	
	if elementUI ~= nil then
		local chatStepInst = r2:getInstanceFromId(elementUI.Env.elementId)
		assert(chatStepInst)
		r2:updateActivitiesWhithThisChatSequence(chatStepInst, true)
		r2:removeElementUI(sequenceUI, "r2ed_chat_sequence", elementUI)
	end
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onPostHrcMove(instance)
	
	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	r2:updateActivitiesWhithThisChatSequence(instance, false)	

	local sequenceUI, chatStepUI = r2:findSequenceAndElementUIFromInstance(instance, "r2ed_chat_sequence")
	if chatStepUI then
		r2:downUpElement(chatStepUI, "r2ed_chat_sequence")
	end
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function chatStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function chatStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
	
	local activeLogicEntity = r2:getSelectedInstance()
	local activeLogicEntityParent = instance.Parent.Parent.Parent.Parent.Parent
	
	if not r2.activitiesAndChatsUIUpdate or activeLogicEntity==nil or activeLogicEntity ~= activeLogicEntityParent then
		return
	end

	local sequenceUI, chatStepUI = r2:findSequenceAndElementUIFromInstance(instance, "r2ed_chat_sequence")
	if chatStepUI then
		r2:updateChatText(chatStepUI)
	end
end	

function r2:findSequenceAndElementUIFromInstance(instance, uiName)

	local sequenceInst = instance.Parent.Parent

	local wndUI = getUI("ui:interface:"..uiName)
	assert(wndUI)

	local sequenceTabs = wndUI:find("sequence_tabs")
	assert(sequenceTabs)

	local sequenceUI
	for i=0,sequenceTabs.tabButtonNb-1 do
		local sequence = sequenceTabs:getGroup(i)
		if sequence.Env.sequenceId == sequenceInst.InstanceId then
			sequenceUI = sequence
			break
		end
	end

	if sequenceUI ~= nil then
		local eltsList = sequenceUI:find("elements_list")
		assert(eltsList)

		for i=0,eltsList.childrenNb-1 do
			local element = eltsList:getChild(i)
			if element.Env.elementId == instance.InstanceId then
				return sequenceUI, element
			end
		end
	end
end

--------------------------------------------------
function r2:chatStepPropertySheetDisplayer()	
	return chatStepPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- CHAT ACTION DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local chatActionPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onPostCreate(instance)
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
end	

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	if refIdName == "Who" then
		r2.requestSetNode(instance.InstanceId, "Who", r2.RefId(""))
		r2.requestSetNode(instance.InstanceId, "Says", "")
		r2.requestSetNode(instance.InstanceId, "Emote", "")
		r2.requestSetNode(instance.InstanceId, "Facing", r2.RefId(""))
	elseif refIdName == "Facing" then
		r2.requestSetNode(instance.InstanceId, "Facing", r2.RefId(""))
	end
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
end

------------------------------------------------
function r2:chatActionPropertySheetDisplayer()	
	return chatActionPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end











r2.lastSelectedActivitySequence = nil

r2.activityTexture = {	["Follow Route"]	= "r2_mini_activity_follow_road.tga",
						["Patrol"]			= "r2_mini_activity_patrol_road.tga",
						["Repeat Road"]		= "r2_mini_activity_repeat_road.tga",
						["Wander"]			= "r2_mini_activity_wander_zone.tga",
						["Stand Still"]		= "r2_mini_activity_stand_still.tga",
--						["Inactive"]		= "r2_mini_activity_inactive.tga"
					}

---------------------------------------------------------------------------------------------------------
-- Show the mini activity view for this instance
function r2:setupMiniActivityView(instance)	
	if instance and instance:isKindOf("ActiveLogicEntity") then
		local selectedTab = 0
		if r2.lastSelectedActivitySequence then
			selectedTab = r2.lastSelectedActivitySequence 
		end
		r2:selectActivitiesSequence(selectedTab)	
		r2:updateSequencesButtonBar(selectedTab)
	end
end

------------------ SELECT ACTIVITY SEQUENCE ---------------------------------------------------------
function r2:selectActivitiesSequence(index)
	r2:selectElementsSequence("r2ed_triggers", index)
	r2:openAndUpdateMiniActivityView(index)
end

------------------ UPDATE SEQUENCES BUTTON BAR ---------------------------------------------------------
function r2:updateSequencesButtonBar(index, sequenceName)

	local selectBar = getUI("ui:interface:r2ed_select_bar")
	assert(selectBar)

	local sequencesButton = selectBar:find("sequences")
	assert(sequencesButton)

	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity then
		if sequenceName==nil and (index >=0) and (index < activeLogicEntity:getBehavior().Activities.Size) then
			local activitySequence = activeLogicEntity:getBehavior().Activities[index]
			assert(activitySequence)
			--sequencesButton.uc_hardtext = activitySequence.Name
			sequencesButton.uc_hardtext = r2:getSequenceName(activitySequence)
		elseif sequenceName~= nil then
			sequencesButton.uc_hardtext = sequenceName
		else
			sequencesButton.uc_hardtext = i18n.get("uiR2EDSequences")
		end
	end
end


function r2:newMiniActivitySequence()
	return r2:newActivitiesSequence(true)	
end

function r2:openAndUpdateMiniActivityView(index)
	r2:openMiniActivityView()
	-- update activities list

	r2:updateMiniActivityView(index)
end

function r2:openMiniActivityView()
	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)

	local selectedInst = r2:getSelectedInstance()

	if selectedInst and not selectedInst:isBotObject() and not selectedInst:isPlant() and not triggersUI.active then
		local miniActivityView = getUI("ui:interface:r2ed_mini_activity_view")
		assert(miniActivityView)								
		miniActivityView.active = true		
	end
end


function r2:updateMiniActivityView(index)

	local miniActivityView = getUI("ui:interface:r2ed_mini_activity_view")
	assert(miniActivityView)

	local miniActivities = miniActivityView:find("mini_activities")
	assert(miniActivities)

	local noActivityLabel = miniActivityView:find("no_activity")
	assert(noActivityLabel)

	local startCount = 0
	local sequence 
	if index~=nil then
		local activities = r2:getSelectedInstance():getBehavior().Activities
		if index < activities.Size then
			sequence = activities[index]
		end
	else
		sequence = r2:getSelectedSequInst("r2ed_triggers")
	end
	
	if sequence~=nil then

		--label "No activity"
		if sequence.Components.Size == 0 then
			noActivityLabel.active = true
			noActivityLabel.uc_hardtext = tostring(i18n.get("uiR2EdNoActivity"))
		else
			noActivityLabel.active = false
		end

		for i=0, sequence.Components.Size-1 do

			local activityInst = sequence.Components[i]
			assert(activityInst)

			if activityInst then

				local activityIndex = i
				if afterEltEditor==true then activityIndex = i-1 end

				local miniActivity = miniActivities[tostring(activityIndex)]
				assert(miniActivity)

				miniActivity.active = true
				miniActivity.Env.id = activityIndex

				-- chat button
				local chatButton = miniActivity:find("chat_sequence"):find("button")
				assert(chatButton)
				if tostring(activityInst.Chat) ~= "" then
					local chatTexture = "r2_mini_activity_chat.tga"
					chatButton.texture = chatTexture
					chatButton.texture_pushed = chatTexture
					chatButton.texture_over = chatTexture
				else
					local chatTexture = "r2_mini_activity_empty_chat.tga"
					chatButton.texture = chatTexture
					chatButton.texture_pushed = chatTexture
					chatButton.texture_over = chatTexture
				end	

				-- activity type button
				local activityButton = miniActivity:find("activity"):find("button")
				assert(activityButton)
				local activityTexture = r2.activityTexture[activityInst.Activity]
				if activityTexture then
					activityButton.texture = activityTexture
					activityButton.texture_pushed = activityTexture
					activityButton.texture_over = activityTexture
				end

				-- activity type text
				local activityText = miniActivity:find("activity_name")
				assert(activityText)
				activityText.uc_hardtext = activityInst.Activity
			end
		end
		startCount = sequence.Components.Size 
	else
		noActivityLabel.active = true
		noActivityLabel.uc_hardtext = tostring(i18n.get("uiR2EdNoSequence"))
	end

	-- hide remaining mini activity templates
	for i=startCount, r2.maxActivities-1 do
		local miniActivity = miniActivities[tostring(i)]
		assert(miniActivity)
		miniActivity.active = false
	end
end


function r2:closeMiniActivityView()
	local miniActivityView = getUI("ui:interface:r2ed_mini_activity_view")
	assert(miniActivityView)
	miniActivityView.active = false
end

function r2:openActivitiesSequence()
	
	r2:updateActivitiesAndChatsUI(r2:getSelectedInstance())

	local triggersUI = getUI("ui:interface:r2ed_triggers")
	assert(triggersUI)

	triggersUI.active = true
	triggersUI:updateCoords()
	
	if triggersUI.Env.openFirst == nil then
		triggersUI:center()
		triggersUI.Env.openFirst = true
	end

	r2:closeMiniActivityView()
end


function r2:chooseOrOpenSelectedChatSequence()

	r2:updateActivitiesAndChatsUI(r2:getSelectedInstance())

	-- init menu
	local menuName = "ui:interface:r2ed_chat_sequences_menu"
	launchContextMenuInGame(menuName)
	local menu = getUI(menuName)

	local rootMenu = menu:getRootMenu()
	assert(rootMenu)
	rootMenu:reset()

	-- update menu
	local miniActivity = getUICaller().parent.parent.parent
	assert(miniActivity)

	local miniActivityNb = tonumber(miniActivity.Env.id)
	local activitySequence = r2:getSelectedSequInst("r2ed_triggers")
	assert(activitySequence)
	local activityInst = activitySequence.Components[miniActivityNb]

	local chatSequenceId = tostring(activityInst.Chat)

	local newLine = 3
	--title "Chat sequence"
	local chatSequence
	if chatSequenceId ~= "" then
		chatSequence = r2:getInstanceFromId(chatSequenceId)
		assert(chatSequence)
		--rootMenu:addLine(ucstring(tostring(i18n.get("uiR2EDChatSequence")).." " .. chatSequence.Name .." : "), "lua", "", "Title")
		rootMenu:addLine(ucstring(tostring(i18n.get("uiR2EDChatSequence")).." " .. r2:getSequenceName(chatSequence) .." : "), "lua", "", "Title")
	else
		rootMenu:addLine(ucstring(tostring(i18n.get("uiR2EDChatSequence")).." : "), "lua", "", "Title")
	end

	rootMenu:addSeparator()

	-- "Open chat sequence"
	if chatSequenceId ~= "" then
		--rootMenu:addLine(ucstring(tostring(i18n.get("uiR2EDOpen")).." "..chatSequence.Name), "lua", "r2:openMiniActivityChatSequence("..tostring(miniActivityNb)..")", "Open")
		rootMenu:addLine(ucstring(tostring(i18n.get("uiR2EDOpen")).." "..r2:getSequenceName(chatSequence)), "lua", "r2:openMiniActivityChatSequence("..tostring(miniActivityNb)..")", "Open")
		newLine = newLine + 1
	end

	-- "Any chat sequence"
	rootMenu:addLine(ucstring(i18n.get("uiR2EdNoChat")), "lua", "r2:setSequenceChatToMiniActivity("..tostring(miniActivityNb)..")", "None")


	-- "new chat sequence"
	rootMenu:addLine(ucstring(tostring(i18n.get("uiR2EdNewChat")).."..."), "lua", "r2:newChatsSequenceAndSelect("..tostring(miniActivityNb)..")", "None")
	local menuButton = createGroupInstance("r2_menu_button", "", { bitmap = "r2_icon_create.tga", size="14" })
	rootMenu:setUserGroupLeft(newLine, menuButton)

	rootMenu:addSeparator()

	local activeLogicEntity = r2:getSelectedInstance()
	assert(activeLogicEntity)

	for i=0, activeLogicEntity:getBehavior().ChatSequences.Size-1 do
		local sequence = activeLogicEntity:getBehavior().ChatSequences[i]
		assert(sequence)

		--rootMenu:addLine(ucstring(sequence.Name), "lua", "r2:setSequenceChatToMiniActivity("..tostring(miniActivityNb)..", " .. tostring(i)..")", sequence.InstanceId)
		rootMenu:addLine(ucstring(r2:getSequenceName(sequence)), "lua", "r2:setSequenceChatToMiniActivity("..tostring(miniActivityNb)..", " .. tostring(i)..")", sequence.InstanceId)
	end

	-- display menu
	menu:updateCoords()	
	local ref = getUICaller()
	menu.y = ref.y_real - (menu.h - ref.h_real)
	menu.x = ref.x_real
	menu:updateCoords()	
end

function r2:openMiniActivityChatSequence(miniActivityNb, chatSequenceId)

	local activitySequence = r2:getSelectedSequInst("r2ed_triggers")
	assert(activitySequence)
	local activityInst = activitySequence.Components[miniActivityNb]

	if chatSequenceId == nil then
		chatSequenceId = tostring(activityInst.Chat)
	end

	r2:openChatSequence(chatSequenceId)
end

function r2:openChatSequence(chatSequenceId)

	if chatSequenceId == nil then
		chatSequenceId = r2:getSelectedEltInst("r2ed_triggers").Chat
	end

	local chatSequences = getUI("ui:interface:r2ed_chat_sequence")
	assert(chatSequences)

	local tab = chatSequences:find("sequence_tabs")
	assert(tab)

	local selectedTab
	local sequenceUI
	for i=0, tab.tabButtonNb-1 do
		sequence = tab:getGroup(i)
		assert(sequence)
		if sequence.Env.sequenceId == chatSequenceId then
			sequenceUI = sequence
			selectedTab = i
			break
		end
	end

	if selectedTab then 
		tab.selection = selectedTab 

		local repeatButton = chatSequences:find("repeat_group"):find("repeat"):find("toggle_butt")
		assert(repeatButton)

		local sequenceInstId = sequenceUI.Env.sequenceId
		local sequenceInst = r2:getInstanceFromId(sequenceInstId)

		repeatButton.pushed = (sequenceInst.Repeating == 0)
	end

	r2:openChatSequences()
end

function r2:newChatsSequenceAndSelect(miniActivityNb)
	local chatSequenceId = r2:newChatsSequence(true)
	
	if chatSequenceName ~= -1 then
		r2:setSequenceChatToMiniActivity(miniActivityNb, -1, chatSequenceId)
		r2:openMiniActivityChatSequence(miniActivityNb, chatSequenceId)
	end
end

function r2:setSequenceChatToMiniActivity(miniActivityNb, sequenceNb, chatSequenceId)

	local miniActivityView = getUI("ui:interface:r2ed_mini_activity_view")
	assert(miniActivityView)

	local miniActivities = miniActivityView:find("mini_activities")
	assert(miniActivities)

	local miniActivity = miniActivities[tostring(miniActivityNb)]
	assert(miniActivity)

	local activitySequence = r2:getSelectedSequInst("r2ed_triggers")
	assert(activitySequence)
	local activityInst = activitySequence.Components[miniActivityNb]

	local chatTexture

	if sequenceNb == nil then
		r2:selectChatSequence("None", activityInst.InstanceId)
		chatTexture = "r2_mini_activity_empty_chat.tga"
	else
		local activeLogicEntity = r2:getSelectedInstance()
		assert(activeLogicEntity)

		if chatSequenceId == nil then
			local chatSequence = activeLogicEntity:getBehavior().ChatSequences[tonumber(sequenceNb)]
			assert(chatSequence)
			chatSequenceId = chatSequence.InstanceId
		end

		r2:selectChatSequence(chatSequenceId, activityInst.InstanceId)

		chatTexture = "r2_mini_activity_chat.tga"
	end

	local chatButton = miniActivity:find("chat_sequence"):find("button")
	assert(chatButton)
	chatButton.texture = chatTexture
	chatButton.texture_pushed = chatTexture
	chatButton.texture_over = chatTexture
end

function r2:openActivity()

	r2:updateActivitiesAndChatsUI(r2:getSelectedInstance())

	local miniActivity = getUICaller().parent.parent.parent
	assert(miniActivity)

	local sequenceUI = r2:getSelectedSequ("r2ed_triggers")
	assert(sequenceUI)

	local activityList = sequenceUI:find("elements_list")
	assert(activityList)

	local eltEditor = activityList:find("edit_element")
	assert(eltEditor)
	local indexEltEditor = activityList:getElementIndex(eltEditor)
	
	local activityIndex = miniActivity.Env.id
	if indexEltEditor<=activityIndex then activityIndex=activityIndex+1 end

	local activityUI = activityList:getChild(activityIndex)
	assert(activityUI)

	local selectedButtonElt = activityUI:find("select")
	assert(selectedButtonElt)

	selectedButtonElt.pushed = true
	r2:selectTriggerElement(nil, "r2ed_triggers", selectedButtonElt)

	r2:updateActivityEditor()
	r2:openActivitiesSequence()
end

function r2:selectSequenceTab(uiName, index)

	if uiName == "r2ed_triggers" then
		r2:updateSequencesButtonBar(index)
	end

	local eltsUI = getUI("ui:interface:"..uiName)
	assert(eltsUI)

	local sequencesTab = eltsUI:find("sequence_tabs")
	assert(sequencesTab)

	local repeatButton = eltsUI:find("repeat_group"):find("repeat"):find("toggle_butt")
	assert(repeatButton)

	local sequenceUI = sequencesTab:getGroup(index)
	assert(sequenceUI)
	local sequenceInstId = sequenceUI.Env.sequenceId

	if sequenceInstId and r2:getInstanceFromId(sequenceInstId) then
		local sequenceInst = r2:getInstanceFromId(sequenceInstId)
		repeatButton.pushed = (sequenceInst.Repeating == 0)
	end
end



function r2:getSequenceName(sequenceInst)

	local name = sequenceInst.Name
	local index = -1
	if name == "" then
		local components = sequenceInst.Parent
		for i=0, components.Size-1 do
			if components[i].InstanceId == sequenceInst.InstanceId then
				index = i
				break
			end
		end

		if index >= 0 then
			local startName 
			if sequenceInst:isKindOf("ChatSequence") then
				startName = tostring(i18n.get("uiR2EdSequChat"))
			elseif sequenceInst:isKindOf("ActivitySequence") then
				startName = tostring(i18n.get("uiR2EdSeq"))
			end

			name = startName..(index+1)
		end
	end
	
	return name 
end





























