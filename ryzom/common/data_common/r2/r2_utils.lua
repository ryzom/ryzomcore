if r2.Utils == nil
then
	r2.Utils={}
end


r2.Utils.addReaction = function(this,event,action)
	local reactions = this.Reactions[event]
	if reactions == nil
	then
		reactions = {}
		this.Reactions[event]=reactions
	end
	table.insert(reactions,action)
end

------------------------------------------------------
--Create the states and groups to represent:
--		-A counter mission
--		-The counters used for this mission
r2.Utils._obsolete_initCounterStates = function(counter,context)
	
	if counter.Class =="Counter"
	then
		
		local pGroup = r2.createPseudoGroup(context)
		context.RtCounters[counter.InstanceId]=pGroup

		for k,v in pairs(counter.Counters)
		do
			Translator.LogicEntityTranslator(v,context)
		end
	end
end


r2.Utils.createGroup = function(x,y,n,base,name,mode)
	
	if mode == nil
	then
		mode = "circle"
	end

	local isGeneric = false

	if  string.find(base, "palette.entities.players.") ~= nil then
		isGeneric = true	
	end


	local npcGroup = r2.newComponent("NpcGrpFeature")
	assert(npcGroup)
	npcGroup.Name = name

	if mode == "circle"
	then
		
		local pas = (2 * math.pi)/n
		local r = (n/(2*math.pi))+2
		for i=1,n do
			local npc = nil
			if ( isGeneric == true) then
				npc = r2.newComponent("NpcCustom")	
			else
				npc = r2.newComponent("Npc")	
			end
				
			npc.Name = name.."_".."Npc"..i
			npc.Base = base
			npc.Position.x = x + (r-1) * math.cos((i-1)*pas)
			npc.Position.y = y + (r-1) * math.sin((i-1)*pas)
			npc.Position.z = r2:snapZToGround(npc.Position.x, npc.Position.y)
			npc.Angle = (i-1)*pas + math.pi
			table.insert(npcGroup.Components,npc)
		end
	end
	if mode == "line"
	then
		local pas = 1
		for i=1,n do
			local npc = r2.newComponent("Npc")
			npc.Name = name.."_".."Npc"..i
			npc.Base = base
			npc.Position.x = x + i * pas
			npc.Position.y = y 
			npc.Position.z = r2:snapZToGround(npc.Position.x, npc.Position.y)
			npc.Angle = 0
			table.insert(npcGroup.Components,npc)
		end
	end
	return npcGroup
end

--region = region to fill
--x,y = region's center
--r = region's ray
--nbPoints = number of points to create the region
r2.Utils.createRegion = function(region,x, y, r,nbPoints)
	region.Points = {}
	local tmpPositions = region.Points
	local pas = (2 * math.pi)/nbPoints
	local Angle = 0
	while nbPoints ~= 0
	do
		local tmpVertex =  r2.newComponent("RegionVertex")
		local tmpPosition = r2.newComponent("Position")
		local sx, sy, sz 
		sx = x + r * math.cos(Angle)
		sy = y + r * math.sin(Angle)
		sx, sy, sz = r2:snapPosToGround(sx, sy)
		tmpPosition.x = sx
		tmpPosition.y = sy
		tmpPosition.z = sz		
		tmpVertex.Position = tmpPosition
		table.insert(tmpPositions, tmpVertex)
		Angle = Angle + pas
		nbPoints = nbPoints - 1
	end
end

--region = region to fill
--x,y = region's center
--r = region's ray
--nbPoints = number of points to create the region
r2.Utils.createNonDeleteableRegion = function(region,x, y, r,nbPoints)
	region.Deletable = 0
	region.Points = {}
	local tmpPositions = region.Points
	local pas = (2 * math.pi)/nbPoints
	local Angle = 0
	while nbPoints ~= 0
	do
		local tmpVertex =  r2.newComponent("RegionVertex")
		tmpVertex.Deletable = 0
		local tmpPosition = r2.newComponent("Position")
		local sx, sy, sz 
		sx = x + r * math.cos(Angle)
		sy = y + r * math.sin(Angle)
		sx, sy, sz = r2:snapPosToGround(sx, sy)
		tmpPosition.x = sx
		tmpPosition.y = sy
		tmpPosition.z = sz		
		tmpVertex.Position = tmpPosition
		table.insert(tmpPositions, tmpVertex)
		Angle = Angle + pas
		nbPoints = nbPoints - 1
	end
end

--region  = trigger Zone
--x,y = region's center
--r = region's ray
--nbPoints = number of points to create the region
r2.Utils.createTriggerRegion = function(region,x, y, r)
	return r2.Utils.createNonDeleteableRegion(region, x, y, r, 4)
end



-- Create a Road 
-- ex local road = createRoad("rout1", { {21570, -1363}, {21570, -1363}, {21570, -1363})
r2.Utils.createRoute = function(name, positions)
	local road = r2.newComponent("Road")
	local function wp(x, y, z)
			local wayPoint = r2.newComponent("WayPoint")
			local pos = r2.newComponent("Position")
			pos.x = x
			pos.y = y
			pos.z = z
			wayPoint.Position = pos
			return wayPoint
		end

--	road.Base = "palette.geom.road"
	road.Name = name
	local tmpPositions = road.Points -- depart a arrivée
	
	for index, points in pairs(positions)
	do
		table.insert(tmpPositions, wp(points[1], points[2], 0))
	end
	
	return road
end


--function to set an RtAiState with a npc's behavior
r2.Utils.setState = function(context,behavior,rtAiState)
	local aiMovement = behavior.Type
	rtAiState.Name = rtAiState.Id .. "|" .. aiMovement
	rtAiState.AiMovement = aiMovement
	if (aiMovement == "wander" or aiMovement == "follow_route" or aiMovement == "patrol_route" or aiMovement == "repeat_road") then		
		local id = behavior.ZoneId
		local zone = context.Components[id]
		assert( zone ~= nil)
		local points=zone.Points
		assert( points ~= nil)
		local size = table.getn(points)
		rtAiState.Pts = {}
		local k,v = next(points, nil)
		local i = 0
		while k ~= nil
		do
			if (k ~= "Keys") 
			then
				-- replacement for getworldPos				
				assert(v ~= nil)
				i = i +1
				rtAiState.Pts[i] = {}
				-- ??? v.Position.x ??				
				rtAiState.Pts[i].x = r2.getWorldPos(v).x
				rtAiState.Pts[i].y = r2.getWorldPos(v).y
				rtAiState.Pts[i].z = r2.getWorldPos(v).z
			end
				k,v = next(points, k)
		end
		-- do reverse
		if (aiMovement == "patrol_route") then
			i = 0
			for i = 1, size -1 , 1
			do
				local first = size - i
				local last = size + i
				
				rtAiState.Pts[last] = {}
				rtAiState.Pts[last].x = rtAiState.Pts[first].x
				rtAiState.Pts[last].y = rtAiState.Pts[first].y
				rtAiState.Pts[last].z = rtAiState.Pts[first].z
			end
		end		
		if (aiMovement == "patrol_route" or aiMovement == "repeat_road") then
			rtAiState.AiMovement = "follow_route"
			local eventHandler = Actions.createEvent("destination_reached", rtAiState.Id ,"")
			assert( eventHandler ~= nil)
			local eName = rtAiState.Id  .. ":destination_reached"
			eventHandler.Name=eName
			table.insert(context.RtAct.Events,eventHandler)

			local action = Actions.createAction("begin_state", rtAiState.Id)
			action.Name="begin state " .. rtAiState.Id
			table.insert(context.RtAct.Actions,action)							
			table.insert(eventHandler.ActionsId,action.Id)
		end
	end
end


r2.Utils.invertRoad = function(road)
	local road2 = r2.newComponent("Road")
	local function wp(x, y, z)
			local wayPoint = r2.newComponent("WayPoint")
			local pos = r2.newComponent("Position")
			pos.x = x
			pos.y = y
			pos.z = z
			wayPoint.Position = pos
			return wayPoint
		end
	local max = table.getn(road.Points)

	for i=1,max do
		local point = road.Points[max-i+1].Position
		table.insert(road2.Points,wp(point.x,point.y,point.z))
	end
	return road2
end


r2.Utils.createEntry = function(who,states,event,actions)
	local entry = r2.newComponent("EventHandlerEntry")
	entry.Who = who
	entry.States = states
	entry.Event = event
	entry.Actions = actions
	return entry
end

r2.Utils.createPlace = function(x,y,z,r)
	local place = r2.newComponent("Place")
	place.Position.x=x
	place.Position.y=y
	place.Position.z=z
	place.Radius = r
	return place
end

r2.Utils.searchEntry = function(activity,who,event,state)
	local max = table.getn(activity.Entries)
	for i=1, max do
		local entry = activity.Entries[i]
		if entry.Who==who and entry.Event == event and (entry.States == state or state=="")
		then
			return entry
		end
	end
	return nil
end

r2.Utils.groupActivities = function(activity1,activity2,event1,state1,event2,state2)
	local entry
	entry = r2.Utils.searchEntry(activity1,activity1.Entries[1].Who,event1,state1)

	if entry == nil
	then
		entry = r2.Utils.createEntry(activity1.Entries[1].Who,state1,event1,"begin_state\n"..state2)
		table.insert(activity1.Entries,entry)
	else
		entry.Actions = entry.Actions .. "\nbegin_state\n"..state2
	end

	entry = r2.Utils.searchEntry(activity2,activity2.Entries[1].Who,event2,state2)
	if entry == nil
	then
		entry = r2.Utils.createEntry(activity2.Entries[1].Who,state2,event2,"begin_state\n"..state1)
		table.insert(activity2.Entries,entry)
	else
		entry.Actions = entry.Actions .. "\nbegin_state\n"..state
	end
end

r2.Utils.evalCost = function(feature)
	--luaObject(feature)
	local components = feature.Components
	--luaObject(components)
	local cost = 0
	if components ~= nil
	then
		for key,comp in pairs(components)
		do
			if key~="Keys" and comp.Class == "Npc"
			then
				cost = cost + 1
			end
		end
	end
	return cost
end

r2.Utils.createChatAction = function(who,says,emote,face)
	local chatStep
	
	chatStep = r2.newComponent("ChatAction")
	chatStep.Who = who
	
	if says ~= ""
	then
		local entry=r2.registerText(says)
		chatStep.Says = entry.InstanceId
	else
		chatStep.Says = says
	end
	
	if face ~="" and face ~=nil
	then
		chatStep.Facing = face
	end

	chatStep.Emote = emote
	return chatStep
end



--replace each instanceId in the table by the new one
r2.Utils.replaceTab = function(this,ttable)
for k,v in pairs(this)
do
  if k ~="Keys"
  then
  	  this[k]=ttable[this[k]]
  end
end
end
--call the replace function for each object in the table
r2.Utils.callReplace = function(this,ttable)
for k,v in pairs(this)
do
  if k ~="Keys"
  then
  	  v:replace(ttable)
  end
end
end


r2.Utils.changeRepere = function(position,center)
	position.x=position.x - center.x
	position.y=position.y - center.y
	position.z = r2:snapZToGround(position.x,position.y)
end

r2.Utils.changeZoneRepere = function(zone,center)
	for i=1,table.getn(zone.Points)
	do
		r2.Utils.changeRepere(zone.Points[i])
	end
end

r2.Utils.changeRepereRt = function(npc,center)
	local x,y
	local position = npc.Position
	x = position.x - center.x
	y = position.y - center.y
	r2.requestSetNode(position.InstanceId,"x",x)
	r2.requestSetNode(position.InstanceId,"y",y)
	r2.requestSetNode(position.InstanceId,"z",r2:snapZToGround(x,y))
end

r2.Utils.setNewGroupCenter = function(group,x,y)
	local k,v = next(group.Components,nil)
	local center = r2.newComponent("Position")
	local first = true
	center.x = x
	center.y = y

	while k~=nil
	do
		if first == true
		then
			first = false
			local newCenter = {}
			newCenter.x = -(center.x - v.Position.x)
			newCenter.y = -(center.y - v.Position.y)
			center.z = r2:snapZToGround(center.x,center.y)
			r2.requestSetNode(v.Position.InstanceId,"x",center.x)
			r2.requestSetNode(v.Position.InstanceId,"y",center.y)
			r2.requestSetNode(v.Position.InstanceId,"z",r2:snapZToGround(center.x,center.y))
			center = newCenter
		else
			r2.Utils.changeRepereRt(v,center)
		end
		k,v = next(group.Components,k)
	end

end

-- Obsolete
r2.Utils.getRtGroup = function(context,instanceId)
	debugInfo("Call obsolete function: call r2.Translator.getRtGroup")
	-- use r2.Translator.getRtGroup instead
	return r2.Translator.getRtGroup(context, instanceId)
end

r2.Utils.concat = function(text,textSup)
	if text == ""
	then
		return textSup
	else
		return text.."\n"..textSup
	end
end


-----------------------------------------------------------
--return a string like: "group1:Npc1" for use with actions
r2.Utils.getNpcParam = function(npcId, context)

	assert( type(npcId) == "string")
	local who = r2:getInstanceFromId(tostring(npcId))
--	local group = who:getParentGroup()
	local rtNpcGrp = context.RtGroups[tostring(npcId)] 
	if rtNpcGrp == nil
	then
		debugInfo("Err: unable to know the npc's group name ("..npcId..")")
		return nil
	end

	return rtNpcGrp.Name..":"..tostring(who.Name)
end


--
-- Returns the RtNpcGrp Id for a given instanceId
--
r2.Utils.getRtIdFromInstanceId = function(context, instanceId)
	assert(instanceId ~= nil and type(instanceId) == "string")
	local instance = r2:getInstanceFromId(instanceId)
	assert(instance)
	return context.RtGroups[instanceId].Id
end


-- vianney tests
function r2.testVianney1()
	r2.requestStartAct(1)
end

function r2.testVianney2()
	r2.requestStartAct(2)
end

function r2.testVianney3()
	r2.requestStopAct()
end

function r2:lowerTranslate(uiR2EdStr)
	return string.lower(i18n.get(uiR2EdStr):toUtf8())
end


