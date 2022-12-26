function r2.getCategories()
	local categories = {
		--{Category_Id, Category_Icon, Parent_Node}
		{"uiR2EdMobSpawnersCategory", "r2ed_icon_creatures.tga", "root"},
		{"uiR2EdChestsCategory", "r2ed_icon_components_chest.tga.tga", "root"},
		{"uiR2EdTasksCategory", "r2ed_icon_macro_components.tga", "root"},
		{"uiR2EdTaskStepCategory", "", "root"},
		{"uiR2EdTriggersCategory", "r2ed_icon_components_trigger.tga", "root"},
		{"uiR2EdDialogsCategory", "r2_mini_activity_chat.tga", "root"},
		{"uiR2EdMacroComponentsCategory", "r2ed_icon_macro_components.tga", "root"},
		{"uiR2EdDevCategory", "r2ed_icon_macro_components.tga", "root"},
	}
	return categories
end

function r2.getLoadedFeaturesDynamic()
	local loadedFeatures = 
	{	
	}
	return loadedFeatures	
end

