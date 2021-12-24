-- In this file we define functions that serves outgame character creation


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (outgame==nil) then
	outgame= {};
end

------------------------------------------------------------------------------------------------------------
-- 
function game:openEditorMenu()
	if not isFullyPatched() then		
		messageBoxWithHelp(i18n.get("uiBGD_FullPatchNeeded"), "ui:outgame");		
		return
	end
	local value = getDbProp('UI:TEMP:HAS_EDITSESSION')
	if value == 0 then
		runAH(nil, "proc", "proc_charsel_edit_scenario")
	else

		local editorButton = getUI("ui:outgame:charsel:edit_session_but")
		assert(editorButton)

		local menuName =  "ui:outgame:r2ed_editor_menu"
		local menu = getUI(menuName)	
		assert(menu)
		launchContextMenuInGame(menu.id)
		menu.x = editorButton.x_real
		menu.y = editorButton.y_real + editorButton.h_real
		menu:updateCoords()	
	end
end

function game:openEditorMenuWarningNewScenario()
	local menuName =  "ui:outgame:r2ed_editor_new_sceneario_warning"
	local menu = getUI(menuName)
	menu.active = true
end

function game:procCharselClickSlot()
	local value = getDbProp('UI:SELECTED_SLOT')
	runAH(nil, "proc", "proc_charsel_clickslot|"..value)
end
