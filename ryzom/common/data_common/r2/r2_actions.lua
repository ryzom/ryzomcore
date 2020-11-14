-- if not r2.Actions then r2.Actions={} end	

-- obsolete
r2._obsolete_Actions_createActionWithCondition = function(name, conditions, actions)
	assert(name)
	assert(type(conditions) == "table")
	assert(actions)

	local first = nil
	local previous = nil

	local k, condition = next(conditions, nil)
	while condition do
		local condition_if = r2.newComponent("RtNpcEventHandlerAction")
		condition_if.Action = "condition_if"
		condition_if.Parameters = condition
		if (previous) then table.insert(previous, condition_if) end
		if (first == nil) then first = condition_if end
		previous = condition_if.Children		
		k, condition = next(conditions, k)
	end
	
	do
		local multi_actions = r2.newComponent("RtNpcEventHandlerAction")
		multi_actions.Action = "multi_actions"
		multi_actions.Parameters = "" 
		multi_actions.Children = actions
		assert(multi_actions)
		if (previous) then table.insert(previous, multi_actions) end
		if (first == nil) then first = multi_actions end
	end

--	table.insert(multi_actions.Children, actions)
	return first	
end




--debugInfo("actions ok!!")
