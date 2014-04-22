--------------
--------------
-- GROUPING --
--------------
--------------



-- TODO nico : find a better file organisation for ui & logic handling
-- TODO nico : most of these global function should be made class members
-- TODO nico : some other part are API to the C++ : these would better fit in a separate, dedicated file.

r2.MaxGroupRadius = 20

function r2.checkGroupDistance(instanceId, targetId)
	assert(instanceId)
	if not targetId then targetId = instanceId end
	assert(targetId)
	local target = r2:getInstanceFromId(targetId)
	local npc = r2:getInstanceFromId(instanceId)
	local group = target.ParentInstance
	assert(group)
	-- The two npc are not grouped
	if not group:isKindOf("NpcGrpFeature") then
			local leader = target
			local npcPosX = npc.Position.x
			local npcPosY = npc.Position.y

			local leaderPosX = leader.Position.x
			local leaderPosY = leader.Position.y
			
			local x = (npcPosX - leaderPosX)^2 --* (npcPosX - leaderPosX)
			local y = (npcPosY - leaderPosY)^2 --* (npcPosY - leaderPosY)
		

			local dist = math.sqrt(x + y)
			if dist > r2.MaxGroupRadius then return false end	
		
		
		
		
		
		
		return true
	end

	local size = table.getn(group.Components)
	local i = 0
	for i  =  0, size-1 do
		local leader = group.Components[i]
		if npc and leader and npc.Position and leader.Position then
			local npcPosX = npc.Position.x
			local npcPosY = npc.Position.y
			local leaderPosX = leader.Position.x
			local leaderPosY = leader.Position.y
			
			local x = (npcPosX - leaderPosX)^2 --* (npcPosX - leaderPosX)
			local y = (npcPosY - leaderPosY)^2 --* (npcPosY - leaderPosY)
		
			local dist = math.sqrt(x + y)
			if dist > r2.MaxGroupRadius then return false end			
		end
	end
	return true
end



function r2.checkLeaderDistAndUngroup(instanceId)
	local ok = r2.checkGroupDistance(instanceId)
	if not ok then r2:ungroup(movedNpc) end	
end
------------------------------------------------------------------------------------
-- test if an object can be grouped with 'target'
-- 'src' must be a an entity
-- 'target' may be another entity or a group
function r2:testCanGroup(src, target)	
	if target:isSameObjectThan(src) then return false end -- can't group itself

	if src:getCategory() ~= target:getCategory() or 
	   src:getSubCategory() ~= target:getSubCategory()
	then
		return false
	end
	if target:isKindOf("Npc") then
		if target:isBotObject() or target:isPlant() then return false end -- can't group bot objects together		
		-- if instance is in a group, then can't goup with an instance of the same group
		if not src:isInDefaultFeature() then			
			return target.ParentInstance ~= src.ParentInstance
		end
		if r2.checkGroupDistance(src.InstanceId, target.InstanceId) == false then return false end
		return true
	elseif target:isKindOf("NpcGrpFeature") then		
		-- can group if src is not already in the same group
		if not src:isInDefaultFeature() then			
			return target ~= src.ParentInstance
		end
		local leader = r2:getInstanceFromId(target.Components[0].InstanceId)
		assert(leader)
		if r2.checkGroupDistance(src.InstanceId, leader.InstanceId) == false then return false end
		return true
	end
end

function r2:testCanGroupSelectedInstance(targetInstanceId)	
	return r2:testCanGroup(r2:getSelectedInstance(), r2:getInstanceFromId(targetInstanceId))
end


------------------------------------------------------------------------------------
-- group an entity with another entity or group
-- 'src' must be a an entity
-- 'target' may be another entity or a group
function r2:group(src, target)

	r2.requestNewAction(i18n.get("uiR2EDGroupAction"))

	if target:getParentAct() ~= src:getParentAct() then
		src:togglePermanentCurrentAct(false)
	end

	--debugInfo(colorTag(0, 255, 255) .. "Grouping instance " .. src.InstanceId .. " with instance or group " .. target.InstanceId)	
	-- remove previous behavior (when grouped, only the group has a behavior, not his members)	
	-- if target has no group then create a new group	
	local targetGroup	
	if target:isKindOf("NpcGrpFeature") then
		targetGroup = target
		-- recompute relative position
		src:requestMakePosRelativeTo(targetGroup)
	elseif target:isInDefaultFeature() then
		assert(target:isKindOf("Npc"))
		--r2.requestSetNode(target.Behavior.InstanceId, "Type", "")	
		-- create a new group
		-- debugInfo(colorTag(255, 255, 0) .. "Creating new group")
		local newGroup = r2.newComponent("NpcGrpFeature")
		local uc_groupName = ucstring()
		uc_groupName:fromUtf8(r2.PaletteIdToGroupTranslation[src.Base].. " " .. i18n.get("uiR2EDNameGroup"):toUtf8())
		local groupName = r2:genInstanceName(uc_groupName):toUtf8()	
		newGroup.Name = groupName

		--r2.requestSetNode(target.Behavior.InstanceId, "Type", "")	
		--newGroup.Behavior.Type = src.Behavior.Type
		--newGroup.Behavior.ZoneId = src.Behavior.ZoneId
		__tmpNewGroup = newGroup
		--runCommand("luaObject", "__tmpNewGroup", maxDepth)
		__tmpNewGroup = nil
		-- create new group
		--r2.requestInsertNode(src:getParentAct().InstanceId, "Features", -1, "", newGroup)	
		r2.requestInsertNode(target:getParentAct().InstanceId, "Features", -1, "", newGroup)		
		-- move the target instance into that group
		r2.requestMoveNode(target.InstanceId, "", -1, newGroup.InstanceId, "Components", -1)
		targetGroup = newGroup
	else
		--debugInfo(colorTag(255, 0, 255) .. "Group already created")
		targetGroup = target.ParentInstance
		-- recompute relative position
		src:requestMakePosRelativeTo(targetGroup)
	end
	-- is src is in a group then ungroup before doing the move
	if not src:isInDefaultFeature() then
		r2:ungroup(src)
	end	
	-- nico patch : locaaly mark the entity so that it can't be grouped again before net msg is received
	src.User.Grouped = true
	-- force update of the available options
	if src == r2:getSelectedInstance() then
		r2.ContextualCommands:update()
	end
	-- move the selected instance into the target group	
	r2.requestMoveNode(src.InstanceId, "", -1, targetGroup.InstanceId, "Components", -1)	
	--r2.requestSetNode(src.Behavior.InstanceId, "Type", "")		
end

------------------------------------------------------------------------------------
-- group the selected instance with the given instance
function r2:groupSelectedInstance(instanceId)
	r2:group(r2:getSelectedInstance(), r2:getInstanceFromId(instanceId))	
end

------------------------------------------------------------------------------------
-- ungroup a npc
function r2:ungroup(instance)	

	r2.requestNewAction(i18n.get("uiR2EDUngroupAction"))
	if instance == nil then
		debugInfo(colorTag(255, 255, 0) .. "No selected instance")
		return
	end

	local target = instance:getParentAct():getDefaultFeature()	
	--debugInfo(colorTag(255, 255, 0) .. "Parent group size is " .. tostring(instance.Parent.Size))
	if instance.Parent.Size <= 2 then		
		-- debugInfo(colorTag(255, 255, 0) .. "Empty group left, removing it")
		-- this was the last object of the group, so remove the group
		local groupId = instance.ParentInstance.InstanceId	
		local parentTable = instance.Parent -- must keep pointer on parent because parent will be changed
		                                             -- a son is moved
		for i = parentTable.Size - 1, 0, -1 do			
			parentTable[i]:requestMakePosRelativeTo(target)
			r2.requestMoveNode(parentTable[i].InstanceId, "", -1, instance:getParentAct():getDefaultFeature().InstanceId, "Components", -1)
		end		
		--debugInfo(colorTag(255, 255, 0) .. "request erase node")
		r2.requestEraseNode(groupId, "", -1)
	else
		-- make pos relative to the new target
		instance:requestMakePosRelativeTo(target)	
		r2.requestMoveNode(instance.InstanceId, "", -1, target.InstanceId, "Components", -1)
		--give a new Behavior to the degrouped member
		--r2.requestSetNode(instance.InstanceId,"Behavior",r2.newComponent("Behavior"))
	end	
end


------------------------------------------------------------------------------------
-- set instance as the new leader of its group
function r2:setAsGroupLeader(instance)
	r2.requestNewAction(i18n.get("uiR2EDSetAsLeaderAction"))
	if instance:isInDefaultFeature() then
		debugInfo("Instance is not in a group, can't set as leader")
		return
	end
	--local oldLeader = instance.ParentInstance.Components[0]
	--if (oldLeader:isSameObjectThan(instance)) then return end
	-- copy behaviour from the entity (becomes the group behavior)
	--r2:setActivityType(instance.Behavior, oldLeader.Behavior.Type, oldLeader.Behavior.ZoneId)
	--r2:setActivityType(oldLeader.Behavior, "", "")
	-- put leader in first position of the group
	r2.requestMoveNode(instance.InstanceId, "", -1, instance.ParentInstance.InstanceId, "Components", 0)
end


-------------------
-------------------
-- NPC BEHAVIOR --
-------------------
-------------------

------------------------------------------------------------------------------------
function r2:testCanPickZoneForNPC(instanceId)
	local targetInstance = r2:getInstanceFromId(instanceId)
	--if targetInstance:isKindOf("WayPoint") or targetInstance:isKindOf("RegionVertex") or 
	--  targetInstance:isKindOf("Region") or targetInstance:isKindOf("Road")
	if targetInstance:isKindOf("RegionVertex") or targetInstance:isKindOf("Region") then
		return true
	end	
	return false	  
end

------------------------------------------------------------------------------------
function r2:testCanPickRoadForNPC(instanceId)
	
	local targetInstance = r2:getInstanceFromId(instanceId)
	if  targetInstance:isKindOf("WayPoint") or targetInstance:isKindOf("Road") 
	then
		return true
	end	
	return false	  
end


------------------------------------------------------------------------------------
--if instance is a grouped npc, return the leader's behavior
--else, return the npc's behavior
function r2:getBehavior(instance)
	return r2:getLeader(instance).Behavior
end

------------------------------------------------------------------------------------
function r2:getLeader (instance)
	--local grouped = not instance:isInDefaultFeature()
	local grouped = instance:isGrouped()
	if grouped == true then
		local group = instance.ParentInstance
		assert(group ~=nil)		
		instance = group.Components[0]
	elseif instance:isKindOf("NpcGrpFeature") then
		instance = instance.Components[0]
	end
	return instance
end

------------------------------------------------------------------------------------
--the arguments must be verified before calling this function
--function r2:setActivityType(behaviorObject, activity, zoneInstanceId)
--	r2.requestSetNode(behaviorObject.InstanceId, "Type", activity)
--	r2.requestSetNode(behaviorObject.InstanceId, "ZoneId", zoneInstanceId)
--end


------------------------------------------------------------------------------------
--function r2:affectZoneToNPC(npcInstanceId, zoneInstanceId)	
--	local npcInstance = r2:getInstanceFromId(npcInstanceId)
--	if npcInstance == nil then 
--		debugInfo("No target npc")
--		return 
--	end
--	local behavior = r2:getBehavior(npcInstance)
--	local activity
--	local targetInstance = r2:getInstanceFromId(zoneInstanceId)
--	if targetInstance:isKindOf("WayPoint") or targetInstance:isKindOf("RegionVertex") then
--		debugInfo(colorTag(255, 255, 0) .. "Selecting parent from WayPoint of RegionVertex")
--		targetInstance = targetInstance.ParentInstance
--	end	
--	r2:blink(targetInstance)	
--	if targetInstance:isKindOf("Road") then
--		activity = "follow_route"		
--	else
--		activity = "wander"
--	end
--	local act = npcInstance:getParentAct()
--	if	act~= nil
--	then
		--debugInfo(colorTag(255,0,0).."act class: "..act.Class)
--	else	
--		debugInfo(colorTag(255,0,0).."act niiil!! ")
--	end	
--	r2:setActivityType(behavior,activity,targetInstance.InstanceId)
--end



function r2:affectZoneToSelectedNPC(zoneInstanceId)	
	local npcInstanceId = r2:getLeader(r2:getSelectedInstance()).InstanceId
	--r2:affectZoneToNPC(npcInstanceId, zoneInstanceId)
	r2:setBehaviorToNPC(npcInstanceId, zoneInstanceId, "Wander")
end

function r2:affectRestZoneToSelectedNPC(zoneInstanceId)	
	local npcInstanceId = r2:getLeader(r2:getSelectedInstance()).InstanceId
	--r2:affectZoneToNPC(npcInstanceId, zoneInstanceId)
	r2:setBehaviorToNPC(npcInstanceId, zoneInstanceId, "Rest In Zone")
end

function r2:affectFeedZoneToSelectedNPC(zoneInstanceId)	
	local npcInstanceId = r2:getLeader(r2:getSelectedInstance()).InstanceId
	--r2:affectZoneToNPC(npcInstanceId, zoneInstanceId)
	r2:setBehaviorToNPC(npcInstanceId, zoneInstanceId, "Feed In Zone")
end

function r2:affectHuntZoneToSelectedNPC(zoneInstanceId)	
	local npcInstanceId = r2:getLeader(r2:getSelectedInstance()).InstanceId
	--r2:affectZoneToNPC(npcInstanceId, zoneInstanceId)
	r2:setBehaviorToNPC(npcInstanceId, zoneInstanceId, "Hunt In Zone")
end

function r2:affectGuardZoneToSelectedNPC(zoneInstanceId)	
	local npcInstanceId = r2:getLeader(r2:getSelectedInstance()).InstanceId
	--r2:affectZoneToNPC(npcInstanceId, zoneInstanceId)
	r2:setBehaviorToNPC(npcInstanceId, zoneInstanceId, "Guard Zone")
end





------------------------------------------------------------------------------------
function r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, primLook, primInvalidLook, primCanCloseLook, affectFunction, activity)	
	if affectFunction == nil then
		dumpCallStack(1)
		assert(false)
	end
	
	r2.requestNewPendingMultiAction(i18n.get("uiR2EDAddActivityAction"), 3)

	-- TODO nico : use co-routine here instead of the callback stuff...
	local npcInstanceId = r2:getSelectedInstance().InstanceId
	local selectInstance = false
	local params = 
	{		
		Look = primLook,
		InvalidLook = primInvalidLook,
		CanCloseLook = primCanCloseLook,
		Vertices = { { x = startX, y = startY, z = startZ } }, -- start with single vertex where the user clicked
		CookieKey = "CreateFunc", -- special function called by the road/region displayer when it is created
		CookieValue = function(primitive)
			-- this is a pending action with 3 steps here			
			affectFunction(r2, npcInstanceId, primitive.InstanceId, activity)
		end,
		SelectInstance = selectInstance, -- when road or place is created to associate new activity to a NPC, 
										 -- it must not be selected
		OnCancel = function()
			r2.requestCancelAction()
		end
	}
	if params.CanCloseLook and params.CanCloseLook.Shape == r2.PrimRender.Shape.ClosedPolyLine then
		params.Type = "Region"
	else
		params.Type = "Route"
	end
	r2:setCurrentTool("R2::CToolDrawPrim", params)
	--debugInfo("createZoneAndAffectBehaviorToNPC END")
end


------------------------------------------------------------------------------------
function r2:createZoneAndAffectZoneToNPC(startX, startY, startZ)				
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RegionCreateLook, r2.PrimRender.RegionCreateInvalidLook, r2.PrimRender.RegionCreateCanCloseLook, r2.setBehaviorToNPC, "Wander")
end

function r2:createRestZoneAndAffectZoneToNPC(startX, startY, startZ)				
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RegionCreateLook, r2.PrimRender.RegionCreateInvalidLook, r2.PrimRender.RegionCreateCanCloseLook, r2.setBehaviorToNPC, "Rest In Zone")
end

function r2:createFeedZoneAndAffectZoneToNPC(startX, startY, startZ)				
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RegionCreateLook, r2.PrimRender.RegionCreateInvalidLook, r2.PrimRender.RegionCreateCanCloseLook, r2.setBehaviorToNPC, "Feed In Zone")
end

function r2:createHuntZoneAndAffectZoneToNPC(startX, startY, startZ)				
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RegionCreateLook, r2.PrimRender.RegionCreateInvalidLook, r2.PrimRender.RegionCreateCanCloseLook, r2.setBehaviorToNPC, "Hunt In Zone")
end

function r2:createGuardZoneAndAffectZoneToNPC(startX, startY, startZ)				
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RegionCreateLook, r2.PrimRender.RegionCreateInvalidLook, r2.PrimRender.RegionCreateCanCloseLook, r2.setBehaviorToNPC, "Guard Zone")
end

------------------------------------------------------------------------------------
function r2:setBehaviorToNPC(npcInstanceId, zoneInstanceId, activity)

	local npcInstance = r2:getInstanceFromId(npcInstanceId)
	if npcInstance == nil then return end	
	
	local behavior = r2:getBehavior(npcInstance)
	local targetInstance = r2:getInstanceFromId(zoneInstanceId)

	if targetInstance:isKindOf("WayPoint") or targetInstance:isKindOf("RegionVertex") then
		zoneInstanceId = targetInstance.ParentInstance.InstanceId
	end	
	
	local act = npcInstance:getParentAct()
	if	act~= nil
	then
		--debugInfo(colorTag(255,0,0).."act class: "..act.Class)
	else	
		debugInfo(colorTag(255,0,0).."act niiil!! ")
	end

	r2.activities:initEditorAfterFirstCall()

	local tableInit = {}
	tableInit.Activity = activity
	tableInit.ActivityZoneId = zoneInstanceId
	if activity=="Follow Route" or activity=="Repeat Road" or activity=="Patrol" then
		tableInit.TimeLimit = "No Limit"
		tableInit.TimeLimitValue = "0"
	else
		tableInit.TimeLimit = "Few Sec"
		tableInit.TimeLimitValue = "20"
	end

	if activity=="Repeat Road" or activity=="Patrol" then
		tableInit.RoadCountLimit = "2"
	end
	-- if current display mode for prim is 'hide all', then force to 'contextual' at
	if r2.PrimDisplayVisible == false then
		r2:primDisplayShowContextual()
		displaySystemInfo(i18n.get("uiR2EDPrimDisplayModeChangedToContextual"), 'BC')
	end
	if not r2.activities:newElementInst(tableInit) then return end	
end

function r2:setBehaviorToSelectedNPC(zoneInstanceId, activity)
	local npcInstanceId = r2:getLeader(r2:getSelectedInstance()).InstanceId
	r2:setBehaviorToNPC(npcInstanceId, zoneInstanceId, activity)
end

------------------------------------------------------------------------------------
function r2:setBehaviorFollowRouteToNPC(roadInstanceId)
	r2:setBehaviorToSelectedNPC(roadInstanceId, "Follow Route")
end

function r2:createRouteAndSetBehaviorFollowRouteToNPC(startX, startY, startZ)
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RoadCreateLook, r2.PrimRender.RoadCreateInvalidLook, nil, self.setBehaviorToNPC, "Follow Route")
end


------------------------------------------------------------------------------------
function r2:setBehaviorPatrolRouteToNPC(roadInstanceId)	
	r2:setBehaviorToSelectedNPC(roadInstanceId, "Patrol")
end

function r2:createRouteAndSetBehaviorPatrolRouteToNPC(startX, startY, startZ)
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RoadCreateLook, r2.PrimRender.RoadCreateInvalidLook, nil, self.setBehaviorToNPC, "Patrol")
end


------------------------------------------------------------------------------------
function r2:setBehaviorRepeatRoadToNPC(roadInstanceId)
	r2:setBehaviorToSelectedNPC(roadInstanceId, "Repeat Road")
end

function r2:createRoadAndSetBehaviorRepeatRoadToNPC(startX, startY, startZ)
	r2:createZoneAndAffectBehaviorToNPC(startX, startY, startZ, r2.PrimRender.RoadCreateLook, r2.PrimRender.RoadCreateInvalidLook, nil, self.setBehaviorToNPC, "Repeat Road")
end


------------------------------------------------------------------------------------
function r2:setNPCStandInPlace(instance)
	
	r2.activities:initEditorAfterFirstCall()

	local tableInit = {}
	tableInit.Activity = "Stand Still"
	tableInit.ActivityZoneId = ""
	tableInit.TimeLimit = "Few Sec"
	tableInit.TimeLimitValue = "20"
	if not r2.activities:newElementInst(tableInit) then return end
end


----------------
----------------
-- MENU SETUP --
----------------
----------------


------------------------------------------------------------------------------------
function r2:selectParent(index)
	local parent = r2:getSelectedInstance()
	for i = 1,index do
		parent = parent.ParentInstance
		if parent == nil then return end
	end
	r2:setSelectedInstanceId(parent.InstanceId)
end


-------------------
-------------------
-- COPY AND PASTE --
-------------------
-------------------

------------------------------------------------------------------------------------
-- called by C++ when copy has been pressed (key or ui button)
function r2:copy()
   local selection = r2:getSelectedInstance()
	if selection == nil then
		displaySystemInfo(i18n.get("uiR2EDCantCopyEmptySelection"), "BC")
		return
	end
   if not selection:isCopyable() then
      displaySystemInfo(i18n.get("uiR2EDSelectionDoNotSupportCopy"), "BC")
		return
   end
   r2.ClipBoard = selection:copy()
   r2.ClipBoardSrcInstanceId = selection.InstanceId
   r2.ClipBoardDisplayName = selection:getDisplayName()
   displaySystemInfo(concatUCString(selection:getDisplayName(), i18n.get("uiR2EDSelectionCopied")), "BC")
end

------------------------------------------------------------------------------------
-- called by C++ when copy has been pressed (key or ui button)
function r2:paste()	
	r2.requestNewAction(concatUCString(i18n.get("uiR2EDPasteAction"), r2.ClipBoardDisplayName))
	if r2.ClipBoard == nil then
		displaySystemInfo(i18n.get("uiR2EDEmptyClipBoard"), "BC")
		return
	end
	-- call the paste function (not a method here)
	local newCopy = r2.Classes[r2.ClipBoard.Class].newCopy(r2.ClipBoard) 
	r2.requestNewAction(concatUCString(i18n.get("uiR2EDPasteAction"), r2.ClipBoardDisplayName))
	r2.Classes[r2.ClipBoard.Class].paste(newCopy,  true, r2.ClipBoardSrcInstanceId)
	r2.requestEndAction()	
end

-- tmp tmp for debug
function doublePaste()
	debugInfo("1 " .. tostring(r2.UIMainLoop.LeftQuotaModified))
	r2:paste()
	debugInfo("2 " .. tostring(r2.UIMainLoop.LeftQuotaModified))
	r2:paste()
	debugInfo("3 " .. tostring(r2.UIMainLoop.LeftQuotaModified))
end


-- tmp tmp for debug
function testPaste()
	for k = 1, 99 do 	
		r2:paste()	
	end	
end



--r2.CuttedSelectionId = "" -- instance id for the selection to be cutted
--
------------------------------------------------------------------------------------
--function r2:getCuttedSelection()	
-- return r2:getInstanceFromId(self.CuttedSelectionId)
-- end
--------------------------------------------------------------------------------------
--function r2:cut(instance)
--	local oldSelection = r2:getCuttedSelection()
--	if oldSelection and oldSelection.DisplayerUI then
--		oldSelection.DisplayerUI:onCut(oldSelection, false) -- not cut anymore
--	end
--	if instance then 
--		r2.CuttedSelectionId = instance.InstanceId
--	else
--		r2.CuttedSelectionId = ""
--	end
--	local newSelection = r2:getCuttedSelection()
--	if newSelection and newSelection.DisplayerUI then
--		newSelection.DisplayerUI:onCut(newSelection, true) -- new instance being cut
--	end
--end
--
--------------------------------------------------------------------------------------
--function r2:paste()
--	local target = r2:getSelectedInstance()
--	local src = r2:getCuttedSelection()
--	assert(target)
--	assert(src)
--	r2:cut(nil)	
--	-- check that target accept the paste
--	assert(target.accept)
--	local destArray = target:accept(src)	
--	-- do the move
--	if target.insert ~= nil then
--		-- if an 'insert' then use it to do the insertion
--		target:insert(src)
--	else		
--		-- else just move the node at the end of the array
--		assert(type(destArray) == "string")
--		r2.requestMoveNode(src.InstanceId, "", -1, target.InstanceId, destArray, -1)
--	end
--end

------------------------
------------------------
-- INSTANCE SELECTION --
------------------------
------------------------

------------------------------------------------------------------------------------
-- called by the framework when an instance is selected
function r2:onSelectInstance(instance)   
   --local st = nltime.getPreciseLocalTime()
   if instance then    
      instance:onSelect() 
	  -- reset slect bar type
	  r2.SelectBar.InstancesType = instance:getSelectBarType()   
   end      
	if r2.isPropertyWindowVisible() then
		r2:showProperties(instance)		
	end	
	r2.CustomBBox:updateUI()	
	r2.ContextualCommands:setupToolbar(instance)		
	r2.MiniToolbar:setupToolbar(instance)
	if not instance then 
		r2.ContextualCommands:setupMenu(nil)
	end	
	r2.SelectBar:touch()	
	r2.SelectBar:getBar().active = (instance ~= nil and not instance.Ghost)-- ensure that 'active' is properly set for the select bar 
												   -- (do not wait per frame update)
	getUI("ui:interface:r2ed_select_bar"):updateCoords()
	getUI("ui:interface:r2ed_contextual_toolbar_new"):updateCoords()
	getUI("ui:interface:r2ed_mini_activity_view"):updateCoords()
	--local et = nltime.getPreciseLocalTime()
    --debugInfo("select instance : " .. tostring(1000 * (et - st)) .." ms")
end


------------------------------------------------------------------------------------
--function r2:showEditOrTestButton(isEdition)
--	if isEdition == nil then
--		debugInfo(debug.traceback())
--	end
--	local editButton = getUI("ui:interface:r2ed_toolbar_admin:r2ed_tool_go_edition")
--	if editButton then editButton.active = not isEdition end
--	local testButton = getUI("ui:interface:r2ed_toolbar_admin:r2ed_tool_go_test")
--	if testButton then testButton.active = isEdition end
--end



------------------------------------------------------------------------------------
--function r2:setEditorMode(isEdition)	
--	-- tmp hack here to know what is the current mode		
--	if isEdition then
--		r2:setupUIForEdition()
--	else
--		r2:setupUIForTest()
--	end		
--	--r2:showEditOrTestButton(isEdition)
--	r2.IsEdition = isEdition
--end


---------------
---------------
-- MAIN MENU --
---------------
---------------

------------------------------------------------------------------------------------
function r2:popMainMenu()	

	r2:setCurrentTool('')
	local menuName = select(r2.Mode == "Edit", "ui:interface:r2ed_main_menu", "ui:interface:r2ed_main_menu_animation")
	--runAH(nil, "active_menu", "menu=" .. menuName)
	launchContextMenuInGame(menuName)
	local menu = getUI(menuName)	
	assert(menu)
	local function buildKeyName(action, params)
		local keyName = ucstring(runExpr(string.format("getKey('%s', '%s')", action, params)))
		assert(isUCString(keyName))		
		if keyName == i18n.get("uiNotAssigned")	then
			-- no associated key...
			return keyName
		else
			local result = concatUCString(ucstring("(") , keyName, ucstring(")"))
			--debugInfo(result:toUtf8())
			return result
		end
	end
	-- fill menu entries common to edition & test
	menu:find("preferences_key").t.uc_hardtext = buildKeyName("show_hide", "game_config")
	menu:find("keys_key").t.uc_hardtext = buildKeyName("show_hide", "keys")
	--menu:find("debug_console_key").t.uc_hardtext = buildKeyName("show_hide", "debug_info")
	menu:find("chat_window_key").t.uc_hardtext = buildKeyName("show_hide", "main_chat")		
	menu:find("quit_key").t.uc_hardtext = buildKeyName("quit_ryzom_now", "")   
	menu:find("mail_box_key").t.uc_hardtext = buildKeyName("show_hide", "mailbox")
	menu:find("guild_forum_key").t.uc_hardtext = buildKeyName("show_hide", "guild_forum")    
	-- fill name of key for each menu entry
	if r2.Mode == "Edit" then
		menu:find("go_test_key").t.uc_hardtext = buildKeyName("r2ed_try_go_test", "")
		menu:find("palette_key").t.uc_hardtext = buildKeyName("show_hide", "r2ed_palette")
		menu:find("scenario_key").t.uc_hardtext = buildKeyName("show_hide", "r2ed_scenario")
		menu:find("cust_bbox").active = (config.R2EDExtendedDebug == 1)		
	else
		menu:find("stop_test_key").t.uc_hardtext = buildKeyName("r2ed_stop_test", "")
		menu:find("stop_test").active = (r2.AccessMode == "Editor")
	end			

	if r2.Mode ~= "Edit" then
		menu:find("live").active = not (r2.AccessMode == "Editor") 
		menu:find("stop_live").active = not (r2.AccessMode == "Editor")
		menu:find("player_admin").active = not (r2.AccessMode == "Editor")
	end

	menu:find("map_key").t.uc_hardtext = buildKeyName("show_hide", "map")
	menu:find("debug_console").active = (config.R2EDExtendedDebug == 1)

	menu:find("fixed_lighting_bm").bm.texture = select(r2:getFixedLighting(), "r2_icon_light_on_small.tga", "r2_icon_light_off_small.tga")
	menu:find("toggle_fixed_lighting").hardtext = select(r2:getFixedLighting(), "uiR2EDTurnLightOff", "uiR2EDTurnLightOn")

	-- setup position
	local scrW, scrH = getWindowSize()
	menu:updateCoords()	
	menu.x = scrW - menu.w_real
	menu.y = 28
	menu:updateCoords()	
end

-- called by main menu in edition mode to go to test mode
function r2:tryGoTest()

	-- remove any keyboard focus so that any editbox properties will be updated correctly
	resetCaptureKeyboard()

	r2.acts.deleteOldScenario = true

	r2:defaultUIDisplayer():storeClosedTreeNodes()

	if r2:getLeftQuota() < 0 then
		messageBox(i18n.get("uiR2EDCantGoTest"))
		return
	end
	-- freeze go test in menu & button
	local goTestButton = getUI("ui:interface:r2ed_toolbar"):find("r2ed_tool_start").unselected.button
	local goTestMenu = getUI("ui:interface:r2ed_main_menu"):find("go_test")
	if not goTestButton.frozen or goTest.grayed then	
		goTestButton.frozen = true
		goTestMenu.grayed = true
		runAH(nil, "r2ed_go_test", "")
	end	
end

------------------------------
------------------------------
-- NO MORE ROOM IN SCENARIO --
------------------------------
------------------------------

-- called by C++ (or lua) to signal that there's no more room to create new entities
function r2:makeRoomMsg()
	displaySystemInfo(i18n.get("uiR2EDMakeRoom"), "BC")	
end



local instanceTrees = nil -- private to r2:getInstanceFromUIUnderMouse

-- test if mouse is currently over a widget in the ui that represents an entity in the scenario (used by the pick tool)
-- TODO nico : put this in the cpp API !!!
function r2:getInstanceIdFromUIUnderMouse()
	-- implemented for the scenario tree only for now ...
	-- build pointers on all tree if not already done
	if instanceTrees == nil then
		instanceTrees = {}
		table.insert(instanceTrees, getUI("ui:interface:r2ed_scenario"):find("content_tree_list"))
		local maxNumActs = getDefine('r2ed_max_num_additionnal_acts')
		for i = 0, r2:getMaxNumberOfAdditionnalActs() - 1 do
			table.insert(instanceTrees, getUI("ui:interface:r2ed_scenario"):find("act_tree_" .. tostring(i)))
			table.insert(instanceTrees, getUI("ui:interface:r2ed_scenario"):find("macro_act_tree_" .. tostring(i)))
		end
	end
	-- look in all scenario trees ....
	for k, v in pairs(instanceTrees) do
		if v and v.active then
			local node = v:getNodeUnderMouse()
			if node then				
				return node.Id				
			end
		end	
	end
	return nil
end

----------
----------
-- ACTS --
----------
----------

-- called by C++ when a new act has been selected
function r2:onActChanged(previousAct, currentAct)
	-- update the scenario window
	r2.ScenarioWindow:updateUIFromCurrentAct()
	-- update the select bar
	r2.SelectBar:touch()

	if r2:isScenarioUpdating() == 1 then
		return
	end


	-- tp if change location
	local baseAct = r2.Scenario:getBaseAct()
	-- change position
	if previousAct and currentAct and baseAct then

		local previousActInstanceId = tostring(r2.Scenario.User.SelectedActInstanceId);
		local previousLocationInstanceId = tostring(r2.Scenario.User.SelectedLocationInstanceId)
		local currentActInstanceId = tostring(currentAct.InstanceId)
		local currentActLocationInstanceId = tostring(currentAct.LocationId)
		if currentAct.InstanceId ~= tostring(baseAct.InstanceId) then			
			r2.Scenario.User.SelectedActInstanceId  = tostring(currentAct.InstanceId) 
			r2.Scenario.User.SelectedLocationInstanceId = tostring(currentAct.LocationId)
			debugInfo("Location id: " ..tostring(currentAct.LocationId))
			if ( r2.Scenario.User.SelectedLocationInstanceId ~= previousLocationInstanceId) then
				local actIndex = 0
				local k,v = next(r2.Scenario.Acts)
				while k do

					if tostring(v.InstanceId) == tostring(currentAct.InstanceId) then
						debugInfo("::BEFORE TP::Location id: " ..tostring(currentAct.LocationId))
						r2.requestSetStartingAct(actIndex)
						r2.requestTpToEntryPoint(actIndex)		
					end				
					actIndex = actIndex + 1				
					k,v = next(r2.Scenario.Acts, k)
				end			
			end
		end	
	end
end


------------------
------------------
-- EDITOR MODES --
------------------
------------------


-- called by C ++ : cleanup tasks when the editor is released
function r2:onFinalRelease()
	r2.DMGift:cancel() -- if a dm gift was current, cancel it
end

-- called by C ++ when current mode of the editor is about to change
function r2:onPreModeChanged()
	r2.DMGift:cancel() -- if a dm gift was current, cancel it
end

-- Called by the C++ framework when there's a mode change between editing, dm etc
-- the r2.Mode variable is changed to one of the following:
-- 'Edit' for editing mode
-- 'Test' for test mode
-- 'DM'   for DM mode
-- 'GoingToDM' when switching from editing to dm mode (when the transition screen is displayed)
-- 'BackToEditing' when switching from dm/test mode to editing (when the transition screen is displayed)

function r2:onModeChanged()
	-- if a form was displayed, just cancel it
	if r2.CurrentForm and r2.CurrentForm.active then
		r2:cancelForm(r2.CurrentForm)
	end
		
	-- if we're back in edition and there's an error message left, display it now
	if r2.Mode == "Edit" and r2.LastTranslationErrorMsg ~= nil then		
		local str = r2.LastTranslationErrorMsg 
		assert(type(str) == "string")
		local ucStringMsg = ucstring("Translation Error")
		--ucStringMsg:fromUtf8(r2.LastTranslationErrorMsg)		
		displaySystemInfo(ucStringMsg, "BC")
		messageBox(str)
		r2.LastTranslationErrorMsg = nil
	end
	
	-- reset the list af acts and triggers until they are received
	if mode == "Edit" then
		r2.AnimGlobals:reset()	
	end

end

---------------------------------------------------------------------------------------------
-------------
-------------
-- WEATHER --
-------------
-------------

-- pop the weather edition dialog in test mode
function r2:changeWeatherDialog()
	local function onOk(form)
		setWeatherValue(true) -- back to auto weather
		-- send request message to dss
		local newWeatherValue = select(form.ManualWeather == 1, form.WeatherValue + 1, 0)
		if newWeatherValue ~= getDbProp("SERVER:WEATHER:VALUE") then
			r2.requestSetWeather(newWeatherValue) -- predicted weather
		end
		--if form.Season ~= getDbProp("SERVER:SEASON:VALUE") then
		--	r2.requestSetSeason(form.Season)
		--end
	end
	local function onCancel(form)
		setWeatherValue(true) -- back to auto weather
	end
	local params = 
	{
		WeatherValue = getWeatherValue() * 1022,
		ManualWeather = select(getDbProp("SERVER:WEATHER:VALUE") ~= 0, 1, 0),
		--Season = getServerSeason()
	}
	r2:doForm("ChangeWeatherForm", params, onOk, onCancel)
end


-- teleport
--  tpPosition when player click on the minimap
--  Whe can not use the  /a Position command because we need admin power to do it
function r2:activeTeleportTool()	
	local function posOk(x, y, z)		
		if config.Local == 1 then
			runCommand("pos", x, y, z);
		else			
			r2.requestTpPosition(x, y, z)
		end
	end
	local function posCancel()
		debugInfo("Cancel teleport pos")
	end	
	r2:choosePos("", posOk, posCancel, "teleport", "r2ed_tool_can_pick.tga", "r2ed_tool_pick.tga")
end

-- teleport
--  tpPosition when player click on the minimap
--  Whe can not use the  /a Position command because we need admin power to do it
function r2:teleportToTarget()	

	local target = r2:getSelectedInstance()
	if not target then return end
	local pos = r2.getWorldPos(target)

	local x = pos.x
	local y = pos.y
	local z = pos.z
	
	local x2, y2, z2 = r2:getUserEntityPosition()

	local dx = (x - x2)^2 --* (npcPosX - leaderPosX)
	local dy = (y - y2)^2 --* (npcPosY - leaderPosY)
	local dist = math.sqrt(dy + dx)
	if dist > 300 then return end

	if config.Local == 1 then
		runCommand("pos", x, y, z);
	else			
		r2.requestTpPosition(x, y, z)
	end
end

---------------------------------------------------------------------------------------------
------------------------
------------------------
-- TOOLS CONTEXT HELP --
------------------------
------------------------

-- call by C++ to change the context help that is displayed for the current tool
function r2:setToolContextHelp(ucHelp)
	--debugInfo(tostring(ucHelp))
   local helpGroup = getUI("ui:interface:r2ed_tool_context_help")
   helpGroup.t.uc_hardtext = ucHelp
   helpGroup.t:updateCoords()
   helpGroup.w = helpGroup.t.w
   helpGroup:invalidateCoords()
end

-- call by C++ to display a message saying that the max number of points has been reached 
-- while drawing a primitive
function r2:noMoreRoomLeftInPrimitveMsg()
	local keyName = ucstring(runExpr("getKey('r2ed_context_command', 'commandId=delete')"))
	local msg = concatUCString(i18n.get("uiR2EDNoMoreRoomInPrimStart"),
							   keyName,
							   i18n.get("uiR2EDNoMoreRoomInPrimEnd"))
	displaySystemInfo(msg, "TAGBC")
end


---------------------------------------------------------------------------------------------
--------------------
--------------------
-- ANIMATION TIME --
--------------------
--------------------
function r2:updateAnimBarActions(...)
	-- forward to the real anim bar
	r2.ui.AnimBar:updateActions(arg)
end


---------------------------------------------------------------------------------------------
------------------------
------------------------
-- UNDO / REDO EVENTS --
------------------------
------------------------

-- called by C++ when an action has been undone 
function r2:onUndo(actionUCName)
	debugInfo("*undo*")
	displaySystemInfo(concatUCString(i18n.get("uiR2EDUndoing"), actionUCName), "BC")
	r2.ToolUI:updateUndoRedo()	
end

-- called by C++ when an action has been redone 
function r2:onRedo(actionUCName)
	debugInfo("*redo*")
	displaySystemInfo(concatUCString(i18n.get("uiR2EDRedoing"), actionUCName), "BC")
	r2.ToolUI:updateUndoRedo()
end

-- called by C++ when a new action has been added in the action historic
function r2:onNewActionAddedInHistoric()	
	debugInfo("*new action*")
	r2.ToolUI:updateUndoRedo()
end

-- called by C++ when an action  new pending action has begun
function r2:onCancelActionInHistoric()
	r2.ToolUI:updateUndoRedo()
end

-- called by C++ when a new pending action has begun
function r2:onPendingActionBegin()
	r2.ToolUI:updateUndoRedo()
end

-- called by C++ when an undo was attempted but failed because there were no actions left in the historic
function r2:onCantUndo(actionUCName)
	displaySystemInfo(i18n.get("uiR2EDCantUndo"), "BC")
end

-- called by C++ when a redo was attempted but failed because there are no more actions at the end of the historic
function r2:onCantRedo(actionUCName)
	displaySystemInfo(i18n.get("uiR2EDCantRedo"), "BC")
end

-- called by C++ when the action historic has been cleared
-- this may happen after scenario version update
function r2:onClearActionHistoric()
	r2.ToolUI:updateUndoRedo()
end


---------------------------------------------------------------------------------------------
------------------------
------------------------
-- DISPLAY MODE MENU  --
------------------------
------------------------

-- call by the toolabr button to pop the 'display mode' menu
function r2:popDisplayModeMenu()
	local menuName = "ui:interface:r2ed_primitive_display"
	local menu = getUI(menuName)
	launchContextMenuInGame(menuName)	
	-- setup position
	local parentIcon = getUI("ui:interface:r2ed_toolbar:r2ed_tool_display_mode")
	local scrW, scrH = getWindowSize()
	--
	menu:find("freeze_all").grayed = not r2.PrimDisplayVisible
	menu:find("freeze_all").checked = r2.PrimDisplayFrozen
	menu:find("unfreeze_all").grayed = not r2.PrimDisplayVisible
	menu:find("unfreeze_all").checked = not r2.PrimDisplayFrozen
	menu:find("hide_all").checked = not r2.PrimDisplayVisible
	menu:find("show_contextual").checked = (r2.PrimDisplayVisible and r2.PrimDisplayContextualVisibility)
	menu:find("show_all").checked = (r2.PrimDisplayVisible and not r2.PrimDisplayContextualVisibility)
	--
    menu:setPosRef("TL TL")
	menu:updateCoords()	
	menu.x = parentIcon.x_real
	menu.y = parentIcon.y_real - scrH
	menu:updateCoords()	
end



local scratchDisplayModeTableSrc = {} -- 'reserve' for list of objects we want to change display mode
local scratchDisplayModeTableDest = {}


-- reset the display mode to 'Visibl' for all objects in all acts
function r2:resetDisplayModeForAllObjects()
	local objList = scratchDisplayModeTableSrc -- alias
	table.clear(objList)
	r2.Scenario:getSons(objList)
	r2.requestNewAction(i18n.get("uiR2EDChangeDisplayAction"))	
	for k, v in pairs(objList) do
		if v:isKindOf("WorldObject") then			
			v.DisplayerVisual.DisplayMode = 0			
		end
	end	
end

----------------------------------------------------------------
-- primitive display mode menu handlers
-- current display mode for primitive
-- NB : these flags may be accessed by C++ when a new route or zone is drawn, to switch 'to display all' mode
-- which isdone by calling 'r2:primDisplayShowAll'
r2.PrimDisplayMode = 0 -- Visible
-- is contextual selection on for primitives ?
r2.PrimDisplayFrozen = false
r2.PrimDisplayVisible = true
r2.PrimDisplayContextualVisibility = false


-- 0,  Visible 
-- 1,  Hidden
-- 2,  Frozen
-- 3,  Locked
function r2:getPrimDisplayMode()
	if not r2.PrimDisplayVisible then
		return 1
	else
		if r2.PrimDisplayFrozen then 
			return 2
		else
			return 0
		end
	end
end

function r2:updatePrimDisplayMode()
	r2:setDisplayMode("places", r2:getPrimDisplayMode(), r2.PrimDisplayContextualVisibility) 
end

function r2:primDisplayFreezeAll()
	r2.PrimDisplayFrozen = true	
	r2:updatePrimDisplayMode()
end
--
function r2:primDisplayUnfreezeAll()
	r2.PrimDisplayFrozen = false
	r2:updatePrimDisplayMode()	
end
--
function r2:primDisplayHideAll()
	r2.PrimDisplayVisible = false
	r2.PrimDisplayContextualVisibility = false
	r2:updatePrimDisplayMode()
end
--
function r2:primDisplayShowContextual()
	r2.PrimDisplayVisible = true
	r2.PrimDisplayContextualVisibility = true
	r2:updatePrimDisplayMode()
end
--
function r2:primDisplayShowAll()
	r2.PrimDisplayVisible = true
	r2.PrimDisplayContextualVisibility = false
	r2:updatePrimDisplayMode()
end
--
function r2:notifyPrimDisplayShowAll()
	displaySystemInfo(i18n.get("uiR2EDPrimDisplayModeChangedToShowAll"), 'BC')
	r2:primDisplayShowAll()
end





-- change display mode for all objects of the given category
-- filter may be :
-- 'all'
-- 'places' : roads and regions
-- "bot_objects' : pieces of furniture, buildings ...
function r2:setDisplayMode(filter, displayMode, contextualVisibility)
	local objList = scratchDisplayModeTableSrc -- alias
	local finalObjList = scratchDisplayModeTableDest -- alias
	table.clear(objList)
	table.clear(finalObjList)
	-- operate in all acts
	for k, v in specPairs(r2.Scenario.Acts) do
		v:getSons(objList)
	end	
	for k,v in pairs(objList) do
      if v:isKindOf("WorldObject") then
         if filter == "all" then
            if v:canChangeDisplayMode() then
               table.insert(finalObjList, v)
            end
         elseif filter == "places" then
            if v:isKindOf("Region") or v:isKindOf("Road") then
               table.insert(finalObjList, v)
            end
         elseif filter == "bot_objects" then
            if v:isKindOf("Npc") and v:isBotObject() then
               table.insert(finalObjList, v)
            end
         end
      end
	end
	if table.getn(finalObjList) == 0 then
		--displaySystemInfo(i18n.get("uiR2EDNoTargetForDisplayModeChange"), "BC")
		return
	end
	-- change display mode only for object that need it
   --r2.requestNewAction(i18n.get("uiR2EDChangeDisplayAction"))	
	for k, v in pairs(finalObjList) do
		v.DisplayerVisual.DisplayMode = displayMode
		if v:isKindOf("BasePrimitive") then
			v.DisplayerVisual.ContextualVisibilityActive = contextualVisibility
		end
		--if v.DisplayMode == srcDisplayMode or (srcDisplayMode == nil and v.DisplayMode ~= displayModeValue) then
			--debugInfo("Changing display mode for " .. v:getDisplayName():toUtf8())
			--r2.requestSetNode(v.InstanceId, "DisplayMode", displayModeValue)
			-- now a local property			
		--end
	end	
end


r2.BotObjectsFrozen = false

function r2:setupFreezeBotObjectButton()
	getUI("ui:interface:r2ed_toolbar"):find("r2ed_freeze_bot_objects").active = not r2.BotObjectsFrozen
	getUI("ui:interface:r2ed_toolbar"):find("r2ed_unfreeze_bot_objects").active = r2.BotObjectsFrozen
end


function r2:freezeUnfreezeBotObjects()
	if r2.BotObjectsFrozen then
		r2:unfreezeBotObjects()
	else
		r2:freezeBotObjects()
	end
end

function r2:freezeBotObjects()
	r2.BotObjectsFrozen = true
	r2:setDisplayMode("bot_objects", 2, false)
	r2:setupFreezeBotObjectButton()	
end

function r2:unfreezeBotObjects()
	r2.BotObjectsFrozen = false
	r2:setDisplayMode("bot_objects", 0, false)
	r2:setupFreezeBotObjectButton()
end



function r2:setUndoRedoInstances(instanceId)
	if instanceId then
		r2.logicComponents.undoRedoInstances[tostring(instanceId)] = true
	end
end


--- 
--Called by C++ when upload message are send
--

local lastMessage = {MessageName="", Size=0, Received=0 }

function r2:onMessageSendingStart(messageName, nbPacket, size)
	debugInfo("--- Start "..messageName.. " " .. tostring(nbPacket).. " " ..tostring(size))
	lastMessage.MessageName = messageName
	lastMessage.Size = size
	lastMessage.Received =0

	local msg = nil
	if lastMessage.MessageName == "SSUA" then
		msg = i18n.get("uiR2EDUploadScenario"):toUtf8()
	elseif lastMessage.MessageName == "RSS2" then
		msg = i18n.get("uimR2EDGoToDMMode"):toUtf8()

	else 
		return
	end
 	local ui = getUI("ui:interface:r2ed_uploading_bar") 
	if not ui then
		return
	end

	local bar = ui:find("uploading_bar")
	if not bar then
		return
	end

	bar.hardtext = msg .. " 0%"
	ui.active = true
end


function r2:onMessageSendingUpdate(packetId, packetSize)
	debugInfo("--- Update " .. tostring(packetId) .. " " .. tostring(packetSize) )
	lastMessage.Received = lastMessage.Received  + packetSize
	if lastMessage.Size and lastMessage.Size ~= 0 then
		debugInfo("--- " .. tostring( math.floor( (100*lastMessage.Received) / lastMessage.Size) ) )

		if lastMessage.MessageName == "SSUA" then
			msg = i18n.get("uiR2EDUploadScenario"):toUtf8()

		elseif lastMessage.MessageName == "RSS2" then
			msg = i18n.get("uimR2EDGoToDMMode"):toUtf8()

		else 
			return
		end

		local ui = getUI("ui:interface:r2ed_uploading_bar") 
		if not ui then
			return
		end

		local bar = ui:find("uploading_bar")
		if not bar then
			return
		end

		bar.hardtext = msg .. " ".. tostring( math.floor( (100*lastMessage.Received) / lastMessage.Size ) ) .."%"
		ui.active = true
	end
end

function r2:onMessageSendingFinish()
	debugInfo("--- Finish ")
	debugInfo(lastMessage.MessageName)
	
	local ui = getUI("ui:interface:r2ed_uploading_bar") 
	if not ui then
		return
	end
	local bar = ui:find("uploading_bar")
	if not bar then
		return
	end
	bar.hardtext = ""
	ui.active = true
end


-- called by C++ to signal that there are too many entity displayed simultaneously
function r2:setMaxVisibleEntityExceededFlag(tooMany)
   local msgGroup = getUI("ui:interface:r2ed_max_visible_entity_count_exceeded")   
   msgGroup.active = tooMany
   if tooMany then
	   msgGroup.t:updateCoords()
	   msgGroup.w = msgGroup.t.w
	   msgGroup:invalidateCoords()
	end
end

