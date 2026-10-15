r2.miniActivities = {


activityTextures = {	["Follow Route"]	= "r2_mini_activity_follow_road.tga",
						["Patrol"]			= "r2_mini_activity_patrol_road.tga",
						["Repeat Road"]		= "r2_mini_activity_repeat_road.tga",
						["Wander"]			= "r2_mini_activity_wander_zone.tga",
						["Stand Still"]		= "r2_mini_activity_stand_still.tga",
						["Rest In Zone"]	= "r2_mini_activity_rest_zone.tga",
						["Feed In Zone"]	= "r2_mini_activity_feed_zone.tga",
						["Work In Zone"]	= "r2_mini_activity_work_zone.tga",
						["Hunt In Zone"]	= "r2_mini_activity_hunt_zone.tga",
						["Guard Zone"]		= "r2_mini_activity_guard_zone.tga",
				   },

maxActivities = 14,

uiId = "ui:interface:r2ed_mini_activity_view",

}

---------------------------------------------------------------------------------------------------------
-- Show the mini activity view for this instance
function r2.miniActivities:openEditor()	
	
	local selectedInstance = r2:getSelectedInstance()
	if not (selectedInstance:isPlant() or selectedInstance:isBotObject()) then
		r2.miniActivities:updateMiniActivityView()	
		r2.miniActivities:updateSequenceButtonBar()

		local miniActivityView = getUI(self.uiId)
		assert(miniActivityView)

		miniActivityView.active = true
	end
end

------ CLOSE EDITOR ------------------------------------------------------
function r2.miniActivities:closeEditor()

	local ui = getUI(self.uiId)
	assert(ui)
	if ui.active then
		ui.active = false
	end
end

--- UPDATE SEQUENCE BUTTON BAR -------------------------------------------------
function r2.miniActivities:updateSequenceButtonBar()
	
	local selectBar = getUI("ui:interface:r2ed_select_bar")
	assert(selectBar)

	local sequencesButton = selectBar:find("sequences")
	assert(sequencesButton)
	
	local sequenceInst
	if r2.activities.isInitialized then
		sequenceInst = r2.activities:currentSequInst()
	else
		local logicEntity = r2:getSelectedInstance()
		if logicEntity== nil then return end

		if logicEntity:getBehavior().Activities.Size > 0 then
			sequenceInst = logicEntity:getBehavior().Activities[logicEntity:getSelectedSequenceIndex()]
		end
	end

	local uc_sequ = ucstring()
	if sequenceInst and sequenceInst.User.Deleted~=true then
		uc_sequ:fromUtf8(sequenceInst:getName())
	else
		uc_sequ = i18n.get("uiR2EDSequences") 	
	end
	sequencesButton.uc_hardtext = uc_sequ
end


--- UPDATE MINI ACTIVITIES VIEW ----------------------------------------------------
function r2.miniActivities:updateMiniActivityView()

	local miniActivityView = getUI(self.uiId)
	assert(miniActivityView)

	local miniActivities = miniActivityView:find("mini_activities")
	assert(miniActivities)

	local noActivityLabel = miniActivityView:find("no_activity")
	assert(noActivityLabel)

	local startCount = 0

	local sequenceInst
	if r2.activities.isInitialized then
		sequenceInst = r2.activities:currentSequInst()
	else
		local logicEntity = r2:getSelectedInstance()
		if logicEntity == nil then return end

		if logicEntity:getBehavior().Activities.Size > 0 then
			sequenceInst = logicEntity:getBehavior().Activities[logicEntity:getSelectedSequenceIndex()]
		end
	end

	if sequenceInst~=nil and sequenceInst.User.Deleted~=true then

		local decalErased = 0

		for i=0, sequenceInst.Components.Size-1 do

			local activityInst = sequenceInst.Components[i]
			assert(activityInst)

			if activityInst then
			
				local activityZoneId = activityInst.ActivityZoneId
				if activityInst.User.Deleted~=true 
					-- this zone just has been deleted
					and not (activityZoneId~="" and r2:getInstanceFromId(activityZoneId)==nil) then

					local miniIndex = i - decalErased
					
					local miniActivity = miniActivities[tostring(miniIndex)]
					assert(miniActivity)

					miniActivity.active = true
					miniActivity.Env.InstanceId = activityInst.InstanceId

					-- activity type button
					local activityButton = miniActivity:find("activity"):find("button")
					assert(activityButton)
					local activityTexture = activityInst:getMiniButtonTexture()
					if activityTexture then
						activityButton.texture = activityTexture
						activityButton.texture_pushed = activityTexture
						activityButton.texture_over = activityTexture
					end

					-- activity type text
					local activityText = miniActivity:find("activity_name")
					assert(activityText)
					local activityType = activityInst:getVerb()
					if activityZoneId~="" then
						local place = r2:getInstanceFromId(activityZoneId)
						assert(place)
						activityType = activityType .. " '" .. place.Name .."'"
					end
					local uc_type = ucstring()
					uc_type:fromUtf8(activityType)
					activityText.uc_hardtext = uc_type
				else
					decalErased = decalErased+1
				end
			end
		end
		startCount = sequenceInst.Components.Size - decalErased

		--label "No activity"
		if (sequenceInst.Components.Size==0) or (sequenceInst.Components.Size==1 and erasedInstId~=nil) 
			or (startCount == 0) then
			noActivityLabel.active = true
			noActivityLabel.uc_hardtext = i18n.get("uiR2EdNoActivity")
		else
			noActivityLabel.active = false
		end

	else

		noActivityLabel.active = true
		noActivityLabel.uc_hardtext = i18n.get("uiR2EdNoSequence")
	end

	-- hide remaining mini activity templates
	for i=startCount, r2.activities.maxElements-1 do
		local miniActivity = miniActivities[tostring(i)]
		assert(miniActivity)
		miniActivity.active = false
	end
end

-- OPEN ACTIVITIES EDITOR ON SELECTED MINI ACTIVITY -------------------------------
function r2.miniActivities:openActivity()

	r2.activities:openEditor()

	local miniActivity = getUICaller().parent.parent.parent
	assert(miniActivity)

	local sequenceUI = r2.activities:currentSequUI()
	assert(sequenceUI)

	local activityList = sequenceUI:find("elements_list")
	assert(activityList)

	local activityUI
	for i=0, activityList.childrenNb-1 do
		local activity = activityList:getChild(i)
		assert(activity)

		if r2.logicUI:getEltUIInstId(activity) == miniActivity.Env.InstanceId then
			activityUI = activity
			break
		end
	end

	if activityUI then
		local selectedButtonElt = activityUI:find("select")
		assert(selectedButtonElt)

		selectedButtonElt.pushed = true
		r2.activities:selectElement(selectedButtonElt)
	end
end
