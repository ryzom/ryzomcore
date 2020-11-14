
r2.doFile("r2_refid.lua")

if r2.InClient == true then
	r2.doFile("r2_ui_reset.lua")
end

r2.doFile("r2_config.lua")
r2.doFile("r2_debug.lua")
--
r2.doFile("r2_misc.lua")
r2.doFile("r2_console.lua")
--
r2.doFile("r2_environment.lua") -- main tables & general  editor functions
--
r2.doFile("r2_prim_render.lua")
r2.doFile("r2_components.lua")


if defaulting(config.R2EDLightPalette, 0) ~= 0 then
	r2.doFile("r2_palette_light.lua")
else
	r2.doFile("r2_palette.lua")
end

r2.doFile("r2_translator.lua")
r2.doFile("r2_utils.lua")
r2.doFile("r2_features.lua")
r2.doFile("r2_base_class.lua")
r2.doFile("r2_world_object.lua")
r2.doFile("r2_logic_entities.lua")
r2.doFile("r2_version.lua")
r2.doFile("r2_init.lua")
r2.doFile("r2_plot_item.lua")

if r2.InClient == true then
	-- edit time
	r2.doFile("r2_ui.lua")   
	r2.doFile("r2_ui_forms.lua")
	r2.doFile("r2_ui_misc.lua")
	r2.doFile("r2_ui_tools.lua")
	r2.doFile("r2_ui_palette.lua")

	r2.doFile("r2_ui_features_tree.lua")

	r2.doFile("r2_ui_displayers.lua")
	r2.doFile("r2_ui_property_sheet.lua")
	r2.doFile("r2_ui_windows.lua")
	r2.doFile("r2_ui_event_handlers.lua")
	r2.doFile("r2_ui_lua_inspector.lua")
	r2.doFile("r2_ui_displayer_npc.lua")
	r2.doFile("r2_activities.lua")
	r2.doFile("r2_mini_activities.lua")
	r2.doFile("r2_dialogs.lua")
	r2.doFile("r2_events.lua")
	r2.doFile("r2_logic_comp.lua")
	r2.doFile("r2_logic_ui.lua")
	r2.doFile("r2_ui_acts.lua")
	r2.doFile("r2_ui_custom_selection_bbox.lua")
	r2.doFile("r2_ui_scenario.lua")
	r2.doFile("r2_ui_main_loop.lua")
	r2.doFile("r2_ui_toolbar_base.lua")
	r2.doFile("r2_ui_select_bar_2.lua")
	r2.doFile("r2_ui_mini_toolbar.lua")	
	-- animation time
	r2.doFile("r2_ui_dm_gift.lua")
	r2.doFile("r2_ui_anim_bar.lua")
	r2.doFile("r2_ui_anim_acts.lua")
	r2.doFile("r2_ui_anim_user_triggers.lua")
	--r2.doFile("r2_ui_contextual_commands.lua")
	r2.doFile("r2_ui_contextual_commands_new.lua")
	r2.CustomBBox:load() -- custom bbox for ojbect selection
end



profileFunction(r2.init, "r2.init")


if r2.InClient == true then

	profileMethod(r2, "initUI", "r2:initUI")
	profileMethod(r2, "initNpcEditor", "r2:initNpcEditor")
	profileMethod(r2.logicComponents, "initLogicEditors", "r2.logicComponents:initLogicEditors")
	profileMethod(r2.acts, "initActsEditor", "r2.acts:initActsEditor")
	profileMethod(r2.ScenarioWindow, "initScenarioWindow", "r2.ScenarioWindow:initScenarioWindow")
end


