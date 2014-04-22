-- obsolete ?
local registerFeature = function()
	local feature={}
	feature.Name = "EventHandler"

	feature.Components =
	{
		{
			Name = "EventHandlerEntry",
			Prop=
			{
				{Name="InstanceId", Type="String"},
				{Name="Who", Type="String"},
				{Name="States", Type="String"},
				{Name="Event", Type="String"},
				{Name="Actions", Type="String"}
			}
		},
		{
			Name = "EventHandler",
			Prop=
			{
				{Name="InstanceId",Type="String"},
				{Name="Name",Type="String", MaxNumChar="32"},
				{Name="Entries", Type="Table"}
			}
		}
	}
	
	

	feature.createEvent = function(event,who,states)
		event = r2.split(event)
		--debugInfo("event: "..event[1])
		local toRet = Actions.createEvent(event[1],states)
		assert(toRet)
		toRet.GroupsByName=who
		return toRet
	end

	feature.createAction = function(action)	
		if type (action) == "table"
		then					
			local multi_action = r2.newComponent("RtNpcEventHandlerAction")
			multi_action.Name = "multi_actions"
			multi_action.Action="multi_actions"		
			local k,v = next(action, nil)
			while k ~= nil
			do	if (k ~= "Keys")
				then
					local actionTxt = r2.split(v, "#")
					local tmpAction = Actions.createAction(actionTxt[1], actionTxt[2])
					table.insert(multi_action.Children, tmpAction)				
				end
				k,v = next(action,  k)
			end
			return multi_action
		end
		
		if action=="multi_action1"
		then
			local multi_action = r2.newComponent("RtNpcEventHandlerAction")
			multi_action.Name = "multi_actions"
			multi_action.Action="multi_actions"
		
			local action

			action = Actions.createAction("set_timer_t0",100)
			action.Name="set end of day timer"
			table.insert(multi_action.Children,action)

			action = Actions.createAction("stand_up")
			action.Name="stand up"
			table.insert(multi_action.Children,action)	
			return multi_action
		end

		if action=="multi_action2"
		then
			local multi_action = r2.newComponent("RtNpcEventHandlerAction")
			multi_action.Name = "multi_actions"
			multi_action.Action="multi_actions"

			local action=Actions.createAction("set_timer_t0",100)
			action.Name="set end of night timer"
			table.insert(multi_action.Children,action)

			action = Actions.createAction("sit_down")
			action.Name="sit down"
			table.insert(multi_action.Children,action)
			return multi_action
		end
		action = r2.split(action, "#")
		toRet = Actions.createAction(action[1],action[2])
		return toRet
	end

	

	feature.Translator = function(context)
		local eventHandler = context.Feature
		local max = table.getn(eventHandler.Entries)
		for i=1, max do
			context.Feature = eventHandler.Entries[i]
			feature.EntryTranslator(context)
		end
	end


	feature.EntryTranslator = function(context)
		local entry = context.Feature
		local states
		local eventType
		local who = context.Components[entry.Who].InstanceId
		--debugInfo("Who: "..who)
		if entry.States == "ALL"
		then
			states = ""
		else
			states = entry.States
		end
		--debugInfo("States: "..states)
		if entry.Event == "INIT"
		then
			--add the actions in the group's initial state, 
			--in the event "start of state"
		else
			eventType = entry.Event
		end

		local event = feature.createEvent(eventType,who,states)
		table.insert(context.RtAct.Events,event)

		local action = feature.createAction(entry.Actions)
		assert(action)
		table.insert(context.RtAct.Actions,action)
		table.insert(event.ActionsId,action.Id)
	end
	return feature
end

r2.Features["EventHandler"] = registerFeature()
