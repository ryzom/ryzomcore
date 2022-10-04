function r2.getCategories()
	local categories = {
		{"uiR2EdMobSpawnersCategory", "r2ed_icon_creatures.tga", "root"},

		{"uiR2EdChestsCategory", "r2ed_icon_components_chest.tga.tga", "root"},

		-- {"uiR2EdTasksSubTasksCategory", "r2ed_icon_components_tasks.tga", "root"},
		--	{"uiR2EdTasksCategory", "r2ed_icon_macro_components.tga", "uiR2EdTasksSubTasksCategory"},
		--	{"uiR2EdTaskStepCategory", "", "uiR2EdTasksSubTasksCategory"},
		{"uiR2EdTasksCategory", "r2ed_icon_macro_components.tga", "root"},
		{"uiR2EdTaskStepCategory", "", "root"},

		{"uiR2EdTriggersCategory", "r2ed_icon_components_trigger.tga", "root"},

		{"uiR2EdDialogsCategory", "r2_mini_activity_chat.tga", "root"},

		{"uiR2EdMacroComponentsCategory", "r2ed_icon_macro_components.tga", "root"},

		{"uiR2EdDevCategory", "r2ed_icon_macro_components.tga", "root"},
		--{Category_Id, Category_Icon, Parent_Node}
	}
	return categories
end

function r2.getLoadedFeaturesDynamic()
	local loadedFeatures = 
	{	
		--Dev	
		--{"r2_features_teleport_near.lua", "TeleportNearFeature",	"uiR2EdDevCategory"},
		{"r2_features_quest.lua", "Quest",	"uiR2EdDevCategory"},
		{"r2_features_npc_interaction.lua", "NpcInteraction",	"uiR2EdDevCategory"},

		--{filename, feature_name, category}
	}
	return loadedFeatures	
end



	
