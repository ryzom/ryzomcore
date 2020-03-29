assert(nil) -- component oblsolete
local registerFeature = function()
local feature = {}
feature.maxId=1
feature.Name = "Piece"

feature.Description = "A npc dialog test"

feature.Components=
{
	{
		Name="Piece",
		Prop =
		{	
			{Name="Zone",Type="Region"},
			{Name="Npcs",Type="Table"},
			{Name="Actions",Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"}
		}
	},
	{
		Name="PieceEntry",
		Prop=
		{
			{Name="Who",Type="String"},
			{Name="Action",Type="String"},
			{Name="Parameters",Type="String"},
			{Name="Time",Type="String"}
		}
	}

}

feature.createDialog = function(dialog,x,y,r)
	--local dialog = r2.newComponent("Piece")
	local nbNpc = table.getn(dialog.Npcs)
	local pas = (2 * math.pi) / nbNpc
	
	local max = table.getn(dialog.Actions)
		--adding the texts to the TextManager
	for i=1,max do
		if dialog.Actions[i].Action == "npc_say"
		then
			local textParam = feature.getTextParam(dialog.Actions[i].Parameters)
			debugInfo("Text param: "..textParam)
			local entry=r2.registerText(textParam)
			dialog.Actions[i].Parameters = entry.InstanceId
			debugInfo("New params: "..dialog.Actions[i].Parameters)
		end
	end
	return dialog
end

feature.getTextParam = function(param)
	debugInfo("param:: "..param)
	local pos=string.find(param,"\n")
	if pos==nil
	then
		return param
	else
		return string.sub(param,pos+1)
	end
end

feature.setTextParam = function(param,value)
	local pos=string.find(param,"\n")
	if pos==nil
	then
		return param
	else
		local st = string.sub(param,1,pos)
		st = st..value
		return st
	end
end

feature.removeDialog = function(dialog)
	local max = table.getn(dialog.Actions)
	for i=1,max do
		if dialog.Actions[i].Action == "npc_say"
		then
			r2.unregisterText(dialog.Actions[i].Parameters)
		end
	end
end

feature.TranslateEntry = function(context)
	local entry = context.Component
	local multi_actions = r2.newComponent("RtNpcEventHandlerAction")
	multi_actions.Action = "multi_actions"
	local parameters = entry.Parameters
	local getRtId = r2.Features["TextManager"].getRtId
	if entry.Action == "npc_say"
	then
		parameters = getRtId(context,parameters)
		debugInfo("npc_say:: "..parameters)
	end
	local action = Actions.createAction(entry.Action,parameters,entry.Who)
	table.insert(multi_actions.Children,action)


	action = Actions.createAction("set_timer_t0",entry.Time)
	table.insert(multi_actions.Children,action)
	return multi_actions
end

feature.loop = function(name)
	action = Actions.createAction("begin_state",name)
	return action
end

feature.Translator = function(context)
	local actions = context.Feature.Actions
	local max = table.getn(actions)
	local switch_action = r2.newComponent("RtNpcEventHandlerAction")
	local endAction 
	local action
	--endAction = feature.loop("init_"..context.Feature.Name)
	endAction = feature.loop("dialog")
	switch_action.Action="switch_actions"
	switch_action.Parameters = "v0"
	

	for i=1,max do
		context.Component = actions[i]
		local tmpAction = feature.TranslateEntry(context)
		table.insert(switch_action.Children,tmpAction)
	end
	table.insert(switch_action.Children,endAction)
	table.insert(context.RtAct.Actions,switch_action)

	
--states creation
	local aiState = r2.newComponent("RtAiState")
	aiState.Name = "dialog"
	table.insert(context.RtAct.AiStates, aiState)
	--aiState = r2.newComponent("RtAiState")
	--aiState.Name = "init_"..context.Feature.Name
	--table.insert(context.RtAct.AiStates, aiState)


--next action
	local event
	event = Actions.createEvent("timer_t0_triggered","dialog")
	table.insert(context.RtAct.Events,event)
	local multi_action = r2.newComponent("RtNpcEventHandlerAction")
	multi_action.Action = "multi_actions"
	table.insert(multi_action.Children,switch_action)
	action = Actions.createAction("modify_variable","v0 + 1")
	table.insert(multi_action.Children,action)
	table.insert(context.RtAct.Actions,multi_action)
	table.insert(event.ActionsId,multi_action.Id)
	

--action
	event = Actions.createEvent("start_of_state","dialog")
	local m_action = r2.newComponent("RtNpcEventHandlerAction")
	m_action.Action = "multi_actions"
	table.insert(context.RtAct.Actions,m_action)
	action = Actions.createAction("modify_variable","v0 = 0")
	table.insert(m_action.Children,action)
	action = Actions.createAction("set_timer_t0","1")
	table.insert(m_action.Children,action)
	table.insert(event.ActionsId,m_action.Id)
	table.insert(context.RtAct.Events,event)

end


feature.Translator2 = function(context)
	local actions = context.Feature.Actions
	local max = table.getn(actions)
	local switch_action = r2.newComponent("RtNpcEventHandlerAction")
	local endAction 
	
	endAction = feature.loop("init_"..context.Feature.Name)

	switch_action.Action="switch_actions"
	switch_action.Parameters = "v0"
	

	for i=1,max do
		context.Component = actions[i]
		local tmpAction = feature.TranslateEntry(context)
		table.insert(switch_action.Children,tmpAction)
	end
	table.insert(switch_action.Children,endAction)
	table.insert(context.RtAct.Actions,switch_action)

	
--states creation
	local aiState = r2.newComponent("RtAiState")
	aiState.Name = "dialog"
	table.insert(context.RtAct.AiStates, aiState)
	aiState = r2.newComponent("RtAiState")
	aiState.Name = "init_"..context.Feature.Name
	table.insert(context.RtAct.AiStates, aiState)

--next action
	local event
	event = Actions.createEvent("timer_t0_triggered","dialog")
	table.insert(context.RtAct.Events,event)
	local multi_action = r2.newComponent("RtNpcEventHandlerAction")
	multi_action.Action = "multi_actions"
	local action = Actions.createAction("modify_variable","v0 + 1")
	table.insert(multi_action.Children,action)
	action = Actions.createAction("begin_state","dialog")
	table.insert(multi_action.Children,action)
	table.insert(context.RtAct.Actions,multi_action)
	table.insert(event.ActionsId,multi_action.Id)
	
--dialog start 
	event = Actions.createEvent("start_of_state","init_"..context.Feature.Name)
	local m_action = r2.newComponent("RtNpcEventHandlerAction")
	m_action.Action = "multi_actions"
	action = Actions.createAction("modify_variable","v0 = 0")
	table.insert(m_action.Children,action)
	action = Actions.createAction("begin_state","dialog")
	table.insert(m_action.Children,action)
	table.insert(context.RtAct.Actions,m_action)
	table.insert(event.ActionsId,m_action.Id)
	table.insert(context.RtAct.Events,event)

--action
	event = Actions.createEvent("start_of_state","dialog")
	table.insert(event.ActionsId,switch_action.Id)
	table.insert(context.RtAct.Events,event)

end



return feature
end

r2.Features["Piece"] = registerFeature()


