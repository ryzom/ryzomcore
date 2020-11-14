r2.logicComponents = {}

r2.logicComponents.logicEditors = {
									r2.activities,
									r2.dialogs,
									r2.events,
								 }

r2.logicComponents.undoRedoInstances = {}


----- INIT ALL EDITORS ----------------------------------------------------
function r2.logicComponents:initLogicEditors()

	for k, uiClass in pairs(self.logicEditors) do
		uiClass:initEditor()
	end
end

------ SELECT ELEMENT --------------------------------------------
function r2.logicComponents:selectElement(classUI, selectedButtonElt)

	if selectedButtonElt == nil then
		selectedButtonElt = getUICaller()
	end
	assert(selectedButtonElt)

	local sequenceUI = classUI:currentSequUI()
	assert(sequenceUI)

	local upDown = sequenceUI:find("order_group")
	assert(upDown)

	-- new selected element
	if selectedButtonElt.pushed == true then
		
		if classUI:currentEltUIId() then

			local lastSelectedElement = classUI:currentEltUI()
			assert(lastSelectedElement)

			local lastEltsList = lastSelectedElement.parent
			local editElt = lastEltsList:find("edit_element")
			assert(editElt)

			if classUI:currentEltUIId() == selectedButtonElt.parent.parent.parent.id then 
				return 
			end
			
			lastSelectedElement.active = true 
			lastSelectedElement:find("select").pushed = false
			
			editElt.active = false
		end

		classUI:setCurrentEltUIId(selectedButtonElt.parent.parent.parent.id)
		
		local selectedElement = selectedButtonElt.parent.parent.parent
		assert(selectedElement)

		-- update element editor position in list
		local eltsList = sequenceUI:find("elements_list")
		assert(eltsList)
		local editElt = eltsList:find("edit_element")
		assert(editElt)

		local indexSelectedElt = eltsList:getElementIndex(selectedElement)
		local indexEltEditor = eltsList:getElementIndex(editElt)
		
		if indexEltEditor<indexSelectedElt then
			for i=indexEltEditor, indexSelectedElt-1 do
				eltsList:downChild(editElt)
			end
		else
			for i=indexSelectedElt, indexEltEditor-2 do
				eltsList:upChild(editElt)
			end
		end

		editElt.active = true
		selectedElement.active = false
		classUI:updateElementEditor()

		upDown.active = (classUI.elementOrder==true and eltsList.childrenNb>2)

	-- cancel current selection 
	else

		local lastSelectedElement = classUI:currentEltUI()
		assert(lastSelectedElement)

		upDown.active = false

		local lastEltsList = lastSelectedElement.parent
		local editElt = lastEltsList:find("edit_element")
		assert(editElt)

		editElt.active = false
		lastSelectedElement.active = true
		
		classUI:setCurrentEltUIId(nil)
	end
end

------ SELECT SEQUENCE --------------------------------------
function r2.logicComponents:selectSequence(classUI, sequenceUI)

	local sequenceUI = classUI:currentSequUI()
	assert(sequenceUI)

	-- select first chat of dialog
	local eltsList = sequenceUI:find("elements_list")
	assert(eltsList)
--	if eltsList.childrenNb > 1 then

--		local firstElt = eltsList:getChild(0)
		-- element editor
--		if r2.logicUI:getEltUIInstId(firstElt) == nil then
--			firstElt = eltsList:getChild(1)	
--		end

--		local selectedButton = firstElt:find("select")
--		assert(selectedButton)

--		selectedButton.pushed = true
--		classUI:selectElement(selectedButton)
--	end

	-- deselect old selected element
	if classUI:currentEltUIId() then

		local lastSelectedElement = classUI:currentEltUI()
		assert(lastSelectedElement)

		local lastEltsList = lastSelectedElement.parent
		local editElt = lastEltsList:find("edit_element")
		assert(editElt)

		lastSelectedElement.active = true 
		lastSelectedElement:find("select").pushed = false
		
		editElt.active = false

		classUI:setCurrentEltUIId(nil)
	end

	eltsList.Env.Minimize = true
end

------ CREATE EDITOR -----------------------------------------------
function r2.logicComponents:createElementEditor(classUI)

	-- not necessary current sequenceUI 
	local sequenceUI = classUI:updatedSequUI()
	assert(sequenceUI)

	-- create element editor
	local elementsList = sequenceUI:find("elements_list")
	assert(elementsList)

	local newEditorElt = createGroupInstance(classUI.elementEditorTemplate, elementsList.id, {id="edit_element", active="false"})
	assert(newEditorElt)
	elementsList:addChild(newEditorElt)
	elementsList.parent:updateCoords()

	newEditorElt.active = false

	return newEditorElt
end

----- CLOSE ELEMENT EDITOR ---------------------------------------------
function r2.logicComponents:closeElementEditor(classUI)
	
	local selectedEltUI = classUI:currentEltUI()  
	if selectedEltUI then
		local selectedEltButton = selectedEltUI:find("select")
		assert(selectedEltButton)

		selectedEltButton.pushed = false
		classUI:selectElement(selectedEltButton)
	end
end

------ NEW ELEMENT INST ------------------------------------------
function r2.logicComponents:newElementInst(classUI)
end

------ UPDATE ELEMENT TITLE -------------------------------------------
function r2.logicComponents:updateElementTitle(classUI, eltUI, showPartIndex)

	local eltInst = r2:getInstanceFromId(r2.logicUI:getEltUIInstId(eltUI))
	assert(eltInst)

	-- part index
	local partIndex = ""

	if showPartIndex then
		local index = self:searchElementIndex(eltInst)
		partIndex = classUI.elementInitialName.." "..index.." : "
	end

	local eltName = eltInst:getName()

	-- title
	local title = eltUI:find("title")
	assert(title)
	local uc_title = ucstring()
	uc_title:fromUtf8(partIndex..eltName)
	title.uc_hardtext_format = uc_title 
end

------ REMOVE ELEMENT INST ----------------------------------------
function r2.logicComponents:removeElementInst(classUI)

	--r2.requestNewAction(i18n.get("uiR2EDRemoveLogicElementAction"))

	local toErasedInstId = classUI:currentEltInstId()
	assert(toErasedInstId)

	-- request erase node
	if toErasedInstId and r2:getInstanceFromId(toErasedInstId) then
		r2.requestEraseNode(toErasedInstId, "", -1)
	end
end

------ UP ELEMENT INST -------------------------------------------
function r2.logicComponents:upElementInst(classUI)

	r2.requestNewAction(i18n.get("uiR2EDMoveLogicElementUpAction"))

	local sequenceUI = classUI:currentSequUI()
	assert(sequenceUI)

	local listElements = sequenceUI:find("elements_list")
	assert(listElements)

	local selectedElement = classUI:currentEltUI()
	assert(selectedElement)

	local index = listElements:getElementIndex(selectedElement)

	if index>0 then
		local sequenceId = classUI:currentSequInstId() 
		assert(sequenceId)

		r2.requestMoveNode(sequenceId, "Components", index,
						   sequenceId, "Components", index-1)
	end
end

------ DOWN ELEMENT INST -----------------------------------------
function r2.logicComponents:downElementInst(classUI)

	r2.requestNewAction(i18n.get("uiR2EDMoveLogicElementDownAction"))
	
	local sequenceUI = classUI:currentSequUI()
	assert(sequenceUI)

	local listElements = sequenceUI:find("elements_list")
	assert(listElements)

	local selectedElement = classUI:currentEltUI()
	assert(selectedElement)
	
	local index = listElements:getElementIndex(selectedElement)

	local sequenceId = classUI:currentSequInstId() 
	assert(sequenceId)
	if index < r2:getInstanceFromId(sequenceId).Components.Size-1 then
		r2.requestMoveNode(sequenceId, "Components", index,
						   sequenceId, "Components", index+1)
	end
end

---- TRANSLATE SECONDS IN HOURS, MINUTES AND ECONDS
function r2.logicComponents:calculHourMinSec(totalSecNb)

	local minSecNb, hourNb = totalSecNb, 0
	while minSecNb > 3599 do
		hourNb = hourNb+1
		minSecNb = minSecNb - 3600
	end

	local minNb, secNb = 0, minSecNb
	while secNb > 59 do
		minNb = minNb+1
		secNb = secNb - 60
	end

	return hourNb, minNb, secNb
end


--- SEARCH INDEX OF INSTANCE IN COMPONENT TABLE
function r2.logicComponents:searchElementIndex(instance, fun)
	
	local components = instance.Parent
	local decalSequ = 0

	for i=0, components.Size-1 do
		local comp = components[i]
		if fun and not fun(comp) then
			decalSequ = decalSequ+1
		elseif comp.User.Deleted == true then
			decalSequ = decalSequ+1
		end
		if comp.InstanceId == instance.InstanceId then
			return i+1-decalSequ
		end
	end

	return -1
end



