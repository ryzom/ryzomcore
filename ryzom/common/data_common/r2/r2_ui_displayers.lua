-----------------
-----------------
-- DISPLAYERS  --
-----------------
-----------------

-- Displayer are objects attached to instance in the scenario
-- They react to modification events (creations of new objects such as nps, groups ...)
-- and update their display accordingly
-- There is zero or one displayer attached per category of display for each instance in the map
-- For now this include :
-- UI displayers       :  - They update the scenario window to display new things added to the map
-- Property displayers :  - They update the property sheet for an instance when one is displayed
-- Visual displayers   :  - For now they are only implemented in C++. Their fonction is to update the display of a instance
--                        -  in the 3D scene
--
-- Displayer at attached at creation time by the C++ code
-- The displayers to add to a specific object are given its the class definition
-- (see r2_base_class.lua for details)

-- helper : update the context toolbar for the given instance if it is the current selected instance
local function updateContextToolbar(instance)
	if r2:getSelectedInstance() == instance then
		r2.ContextualCommands:update()
	end
end




-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------

-- The following code describes how to create a basic displayer that just
-- output the events it handles in the log
-- Mots of the time, when creating a new displayer, one will
-- just construct an existing displayer, and redefine methods of interest, 
-- possibly calling the parent one

function r2:exampleUIDisplayer()	
	local handler = {}
	local ct = colorTag(0, 0, 255)
	------------------------------------------------
	-- Called by C++ at creation
	function handler:onCreate(instance)
		debugInfo(ct .. "Instance " .. instance.InstanceId .."  was created")
	end
	------------------------------------------------
	-- Called by C++ just before object is removed (so properties are still readable)
	function handler:onErase(instance)	
		debugInfo(ct .. "Instance " .. instance.InstanceId .."  was erased")
	end
	------------------------------------------------
	-- Called by C++ just before object is moved in the object hierarchy
	function handler:onPreHrcMove(instance)
		updateContextToolbar(instance)
		debugInfo(ct .. "Instance " .. instance.InstanceId .."  is about to move")
	end
	------------------------------------------------
	-- Called by C++ just after object is move in the object hierarchy
	function handler:onPostHrcMove(instance)
		updateContextToolbar(instance)
		debugInfo(ct .. "Instance " .. instance.InstanceId .."  has moved")
	end
	------------------------------------------------
	-- Called by C++ just after object is highlighted by mouse
	function handler:onFocus(instance, hasFocus)
		if (instance.User.HasFocus ~= hasFocus) then
			if hasFocus == true then
				debugInfo(ct .. "Instance " .. instance.InstanceId .."  has gained focus")
			else
				debugInfo(ct .. "Instance " .. instance.InstanceId .."  has lost focus")
			end
			instance.User.HasFocus = hasFocus
		end		
	end
	------------------------------------------------
	-- Called by C++ just after object has been selected
	function handler:onSelect(instance, isSelected)
		if (isSelected == true) then
			debugInfo(ct .. "Instance " .. instance.InstanceId .."  is selected")
		else
			debugInfo(ct .. "Instance " .. instance.InstanceId .."  is unselected")
		end
	end
	------------------------------------------------
	-- Called by C++ when an attribute of this object has been modified
	-- An attribute inside this object has been modified
	-- attributeName  :Name of the attribute inside this object, as given by its class definition. If the attribute
    --                 is an array, then an additionnal parameter gives the index of the element being modified in the array (or -1 if the whole array is set)	
	function handler:onAttrModified(instance, attributeName, indexInArray)
		updateContextToolbar(instance)
		debugInfo(ct .. "Instance " .. instance.InstanceId .." has an attribute modified : " .. attributeName)
	end		
end


function r2:onInstanceSelectedInTree(id)
	-- is there's an active pick tool then
	local currTool = r2:getCurrentTool()
	if currTool and currTool:isPickTool() then
		local tree = getUICaller()		
		tree:cancelNextSelectLine() -- don't want real selection, actually ...
		if currTool:canPick() then
			currTool:pick()
		end		
		-- no-op else ...	
		return
	end
	--debugInfo("Seleting instance with id = " .. tostring(id) )
	r2:setSelectedInstanceId(id)
end

function r2:onInstanceRightClickInTree(id)	
	r2:setSelectedInstanceId(id)
	r2:displayContextMenu()
end


r2.VerboseEvents = false;

-- before to go to "test mode", store opened/closed nodes in scenario window tree
-- to correctly initialize tree when go back in edition mode
r2.storedClosedTreeNodes = {}

-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------

-- displayer that update the tree control (scenario window)
function r2:defaultUIDisplayer()
	local function eventDebugInfo(msg)
		if r2.VerboseEvents == true then
			debugInfo(msg)
		end
	end

	local handler = {}
	local ct = colorTag(255, 0, 255)
	------------------------------------------------
	-- helper function : notify current act ui displayer that its quota has been modified
	function handler:updateCurrentActQuota()
		-- defer update to the next frame (many element can be added at once)
		r2.UIMainLoop.LeftQuotaModified = true
	end
	------------------------------------------------
	function handler:onCut(instance, cutted)				
		-- NOT IMPLEMENTED
		-- debugInfo("On cut " .. tostring(cutted))
		-- local tree = getUI(r2.InstanceTreePath)
		-- debugInfo(tostring(select(cutted, 127, 255)))
		-- instance.User.TreeNode.Color.G = select(cutted, 0, 255)		
		-- tree:forceRebuild()
	end	
	------------------------------------------------
	function handler:onCreate(instance)		
		--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  was created")		
		self:addTreeNode(instance)		
		-- if my quota is not 0, then we should update
		-- the current act quota ..		
		--if instance:getUsedQuota() ~= 0 then
		--	self:updateCurrentActQuota()
		--end
		if instance:hasScenarioCost() ~= false then
			self:updateCurrentActQuota()
		end
		
	end
	------------------------------------------------
	function handler:onPostCreate(instance)

		-- Special : if the cookie 'AskName' is set (by C++ or lua), then show property and ask name
		-- to user for that object		    
		if instance.User.AskName then			
			if instance.User.ShowProps then
				r2:showProperties(instance)
				instance.User.ShowProps = nil
			end			
			if instance.User.Select then
				r2:setSelectedInstanceId(instance.InstanceId)
			end			
			local propWindow = r2.CurrentPropertyWindow

			-- tmp : quick & dirty access to the widget ...
			if propWindow and propWindow.active then
				local editBox = propWindow:find("Name"):find("eb")
				if editBox then
					setCaptureKeyboard(editBox)
					editBox:setSelectionAll()
				end
			end
			instance.User.AskName = nil -- get rid of cookie
		end	
		-- Special : if the cookie 'Select' is set (by C++ or lua), then the object should be selected after creation
		if instance.User.Select then			
			r2:setSelectedInstanceId(instance.InstanceId)				
		end
		if type(instance.User.CreateFunc) == "function" then
			instance.User.CreateFunc(instance)
		end
	end
	------------------------------------------------
	function handler:onErase(instance)						
		--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  was erased")
		self:removeTreeNode(instance)
		-- if my quota is not 0, then we should update
		-- the current act quota ..
		if instance:hasScenarioCost() ~= false then
			self:updateCurrentActQuota()
		end		
	end
	------------------------------------------------
	function handler:onPreHrcMove(instance)
		updateContextToolbar(instance)
		--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  is about to move")
		self:removeTreeNode(instance)
	end
	------------------------------------------------
	function handler:onPostHrcMove(instance)

		-- if parent is a group, for its creation you don't know category of children : people or creature
		-- you check it for first child
		local parent = instance.ParentInstance
		if instance:isGrouped() and parent.Components.Size==1 then
			self:onErase(parent)
			self:onCreate(parent)
			self:onPostCreate(parent)
		end

		updateContextToolbar(instance)
		--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  has moved")
		--eventDebugInfo(ct .. "New parent is " .. instance.ParentInstance.InstanceId)		
		local nodes = self:addTreeNode(instance)
		if (r2:getSelectedInstance() == instance) and nodes then
			for k, node in pairs(nodes) do
				assert(node)			
				assert(node:getParentTree())
				assert(node:getParentTree().selectNodeById)
				node:getParentTree():selectNodeById(node.Id, false)
			end
		end
		-- if my quota is not 0, then we should update
		-- the current act quota ..
		if instance:hasScenarioCost() ~= false then
			self:updateCurrentActQuota()
		end

		-- if instance has Components, its children's nodes have been deleted at onPreHrcMove call
		if instance.Components then	
			for i=0, instance.Components.Size-1 do
				local child = instance.Components[i]
				self:onCreate(child)
			end

			self:onPostCreate(instance)
		end
	end
	------------------------------------------------
	function handler:onFocus(instance, hasFocus)
		if (instance.User.HasFocus ~= hasFocus) then
			if hasFocus == true then
				--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  has gained focus")
			else
				--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  has lost focus")
			end
			instance.User.HasFocus = hasFocus
		end		
	end
	------------------------------------------------
	function handler:onSelect(instance, isSelected)		
		if not instance.User.TreeNodes then			
			return
		end	
		for k, treeNode in pairs(instance.User.TreeNodes) do

			if not (treeNode == nil or treeNode.isNil == true) then 		

				local tree = treeNode:getParentTree()	
				if (isSelected == true) then
					--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  is selected")			
					tree:selectNodeById(instance.InstanceId, false)
				else
					--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .."  is unselected")
					tree:unselect()
				end
			end
		end
	end
	------------------------------------------------
	function handler:onAttrModified(instance, attributeName, indexInArray)
		if attributeName == "Position" or attributeName == "Angle" then
			return
		end
		if attributeName == "Selectable" then
			self:removeTreeNode(instance)
			self:addTreeNode(instance)
		end

		updateContextToolbar(instance)
		if not instance.User.TreeNodes then			
			return
		end	
		local nodes = instance.User.TreeNodes
		for k, node in pairs(nodes) do
			local tree = node:getParentTree()		
			if attributeName == 'Name' then
				setupNodeName(instance)
				if node:getFather() then
					node:getFather():sortByBitmap()
				end
				tree:forceRebuild()			
				tree:selectNodeById(node.Id, false) -- reforce the selection
			end
		end

		if attr == "Ghost" then
			if instance.Ghost then
				self:removeTreeNode(instance)
			end
		end
		--eventDebugInfo(ct .. "Instance " .. instance.InstanceId .." has an attribute modified : " .. attributeName)
	end

	function setupNodeName(instance)				
		local treeNodes = instance.User.TreeNodes		
		if not treeNodes then return end

		for k, treeNode in pairs(treeNodes) do
			if not (treeNode == nil or treeNode.isNil == true) then 
				local tree = treeNode:getParentTree()				
						
				treeNode.Text = instance:getDisplayName()		
				if tree then -- nb : tree may be nil if node is setupped before being attached to its parent tree
					tree:forceRebuild()
				end
			end
		end
	end

	function handler:storeClosedTreeNodes()

		function downInTree(node, nodeTable)

			for i=0, node:getNumChildren()-1 do
				local child = node:getChild(i)
				assert(child)

				nodeTable[child.Id] = child.Opened

				if child:getNumChildren()>0 then
					downInTree(child, nodeTable)
				end
			end
		end

		r2.storedClosedTreeNodes = {}

		-- scenary objects
		r2.storedClosedTreeNodes[r2.Scenario:getBaseAct().InstanceId] = {}
		local objectNodes = r2.storedClosedTreeNodes[r2.Scenario:getBaseAct().InstanceId]

		local container = getUI("ui:interface:r2ed_scenario")
		--local objectsRoot = container:find("content_tree_list"):getRootNode():getNodeFromId("scenery_objects")
		local objectsRoot = container:find("content_tree_list"):getRootNode()
		assert(objectsRoot)
		downInTree(objectsRoot, objectNodes)

		-- entities and components
		if r2.Scenario.Acts.Size>1 then
			for i=1, r2.Scenario.Acts.Size-1 do
				local act = r2.Scenario.Acts[i]
				local peopleRoot = act:getContentTree():getRootNode():getNodeFromId("people")
				assert(peopleRoot)
				local creatureRoot = act:getContentTree():getRootNode():getNodeFromId("creatures")
				assert(creatureRoot)
				--local componentRoot = act:getMacroContentTree():getRootNode():getNodeFromId("macro_components")
				local componentRoot = act:getMacroContentTree():getRootNode()
				assert(componentRoot)

				r2.storedClosedTreeNodes[act.InstanceId] = {}
				local actNodes = r2.storedClosedTreeNodes[act.InstanceId]

				downInTree(peopleRoot, actNodes)
				downInTree(creatureRoot, actNodes)
				downInTree(componentRoot, actNodes)
			end
		end
	end

	function handler:addPermanentNodes()

		if r2.ScenarioInstanceId then
			local scenario = r2:getInstanceFromId(r2.ScenarioInstanceId)
			if scenario and scenario.Acts.Size>0 then
				local addToTreesTable = {}
				scenario:getBaseAct():appendInstancesByType(addToTreesTable, "LogicEntity")
				for k, instance in pairs(addToTreesTable) do
					self:addTreeNode(instance)
				end
			end
		end
	end

	-- private
	function handler:addTreeNode(instance)	

		if instance.Ghost then return nil end

		local parentNodes = instance:getParentTreeNode()

		if parentNodes==nil then return nil end

		if instance.User.TreeNodes==nil then instance.User.TreeNodes = {} end

		for actId,parentNode in pairs(parentNodes) do

			local alreadyAdded = false		

			for k2, treeNode in pairs(instance.User.TreeNodes) do
				if not (treeNode==nil or treeNode.isNil==true) then
					
					local father = treeNode:getFather()
					if father==parentNode then 
						alreadyAdded=true 
						break
					end
				end
			end

			if not alreadyAdded then
				
				if parentNode == nil then
					return nil -- one of the ancestors may be unselectable			
				end
				if not instance.SelectableFromRoot then
					return nil
				end
				local tree = parentNode:getParentTree()		
				local treeNode = SNode()
					
				-- store reference in object
				table.insert(instance.User.TreeNodes, treeNode)

				treeNode.Bitmap = instance:getPermanentStatutIcon()
				local openTree = true
				if r2.storedClosedTreeNodes[actId] then
					openTree = (r2.storedClosedTreeNodes[actId][instance.InstanceId]==true)
				end
				treeNode.Opened = openTree
				
				treeNode.Id = instance.InstanceId
				treeNode.AHName = "lua"
				local ahParams = "r2:onInstanceSelectedInTree('" .. instance.InstanceId .. "')"
				--eventDebugInfo(ahParams)
				treeNode.AHParams = ahParams 
				treeNode.AHNameRight = "lua"
				treeNode.AHParamsRight = "r2:onInstanceRightClickInTree('" .. instance.InstanceId .. "')"
				treeNode.AHNameClose = "lua"
				treeNode.AHParamsClose = "r2.storedClosedTreeNodes = {}"

				setupNodeName(instance)

				assert(parentNode)
				parentNode:addChildSortedByBitmap(treeNode)
				parentNode.Show = (parentNode:getNumChildren() ~= 0)
						
				tree:forceRebuild()
			end
		end

		return instance.User.TreeNodes						
	end
	function handler:removeTreeNode(instance)
			
		local nodes = instance.User.TreeNodes		
		if  nodes == nil or nodes.isNil then 			
			return
		end
		for k, node in pairs(nodes) do
			if not (node == nil or node.isNil == true) then
				local tree = node:getParentTree()
				if node:getFather().isNil then			
					if (node == node:getParentTree():getRootNode()) then
						--debugInfo("ROOT NODE")
						node:getParentTree():setRootNode(nil)
					else
						--debugInfo("ISOLATED NODE")
						deleteReflectable(node) -- isolated node (the tree was never built ?)
					end
				else
				 -- update parent node visibility only if a direct son of the root node
				 if node:getFather() then
					if (node:getFather():getFather() == tree:getRootNode()) then
						node:getFather().Show = (node:getFather():getNumChildren() > 1)
					end
					node:getFather():deleteChild(node)
				 end
					
				end
				tree:forceRebuild()
			end
		end
		instance.User.TreeNodes = nil
	end	

	return handler
	
end

-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------

-- special display for groups in scenario window
function r2:groupUIDisplayer()
	local handler = self:defaultUIDisplayer()	
   function handler:updateLeaderColor(instance)
	  if not instance.User.TreeNodes then 
		return
	  end
	  for k, node in pairs(instance.User.TreeNodes) do
		  local tree = node:getParentTree()
		  for i = 0, instance.Components.Size - 1 do
			  --debugInfo("I = " .. tostring(i))
			 local treeNodes = instance.Components[i].User.TreeNodes
			 if treeNodes then
				for k2, treeNode in pairs(treeNodes) do
					if i == 0 then
					   treeNode.Color = CRGBA(255, 0, 0) -- mark leader in red
					else
					   treeNode.Color = CRGBA(255, 255, 255)
					end
				end
			 end
		  end						
		  tree:forceRebuild()
	  end
   end
   --
   local oldOnAttrModified = handler.onAttrModified
	function handler:onAttrModified(instance, attrName, indexInArray)
		if attrName == "Components" then				
         self:updateLeaderColor(instance)
		end
		oldOnAttrModified(self, instance, attrName, indexInArray)
	end
   --
   -- local oldOnCreate = handler.onCreate
   -- function handler:onCreate(instance)      
   -- debugInfo("On create group")
   -- oldOnCreate(self, instance)      
   -- end
   --
   local oldOnPostCreate = handler.onPostCreate
   function handler:onPostCreate(instance)      
      oldOnPostCreate(self, instance)
      self:updateLeaderColor(instance)
   end
   --
	return handler
end



-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------

-- Displayer for ACTS. In the ui, acts are added into the act combo box --
-- in the environment of the container we store a table that gives the act Instance id,
-- and the index of the tree control for each line in the combo box
-- Table has the following look
-- ActTable = { { Act = ..., TreeIndex = ... }, -- combo box line 1
--              { Act = ..., TreeIndex = ... }, -- combo box line 2
--              { Act = ..., TreeIndex = ... }, -- combo box line 3 etc.
--            }


r2.ActUIDisplayer = {}
r2.ActUIDisplayer.ActTable = {} -- table that map each line of the combo box to an act
r2.ActUIDisplayer.LastSelfCreatedActInstanceId = nil -- id of the last act created by the pionner (not by another pionner)
                                                     -- When created, an act change will automatically occur

	------------------------------------------------
	-- helper function : notify current act ui displayer that its quota has been modified
	function r2.ActUIDisplayer:updateCurrentActQuota()
		-- defer update to the next frame (many element can be added at once)
		r2.UIMainLoop.LeftQuotaModified = true
	end

	------------------------------------------------
	function r2.ActUIDisplayer:updateActName(act)

		if act and not act:isBaseAct() then
				
			local actTable = self:getActTable()	
			for index, entry in pairs(actTable) do
				if entry.Act == act then
					local comboBox = self:getActComboBox() 

					local actTitle = act:getName()	
					if act==r2.Scenario:getCurrentAct() then
						actTitle = actTitle .. "  [" .. i18n.get("uiR2EDCurrentActComboBox"):toUtf8() .."]"	
					end
					local text = ucstring()
					text:fromUtf8(actTitle)
					comboBox:setText(index - 1, text)
					return
				end
			end 
		end
	end
	
	------------------------------------------------
	function r2.ActUIDisplayer:onAttrModified(instance, attributeName, indexInArray)

		-- if title is modified, then must update names of all entities in the scene
		if attributeName == "Name" then
			local npcs = {}
			r2:getCurrentAct():appendInstancesByType(npcs, "Npc")
			for k, npc in pairs(npcs) do
				npc.DisplayerVisual:updateName()
			end
			
			self:updateActName(instance)
		end
	end


		
	------------------------------------------------
	function r2.ActUIDisplayer:onCreate(act)
			
		local container = self:getContainer() 
		local comboBox = self:getActComboBox() 

		local tree, macroTree 
		if not act:isBaseAct() then	
			local text = ucstring()	
			local index = r2.logicComponents:searchElementIndex(act)-2
			local actTitle = act:getName()
			if type(actTitle) ~= "string" then
				text:fromUtf8("bad type for title : " .. type(actTitle))
				comboBox:insertText(index, text)
			else		
				text:fromUtf8(actTitle)
				comboBox:insertText(index, text)
			end

			tree = self:findFreeTreeCtrl()	
			macroTree = self:findFreeTreeCtrl(true)	
			local actTable = self:getActTable()
			table.insert(actTable, index+1, { Act = act, Tree = tree , MacroTree = macroTree})		
		end

		-- store tree in the act for future insertion of items
		act.User.ContentTree = tree
		act.User.MacroContentTree = macroTree
		self:updateCurrentActQuota()

		-- add permanent nodes to act node
		r2:defaultUIDisplayer():addPermanentNodes()
	end

	------------------------------------------------
	function r2.ActUIDisplayer:onPostCreate(act)
		-- when a new act is created, select this act as the default
		if act.InstanceId == self.LastSelfCreatedActInstanceId then
			-- the act was just created by pionner on that computer, so change right now		
			r2.ScenarioWindow:setAct(act)
			self.LastSelfCreatedActInstanceId = nil
		end
		r2.ScenarioWindow:updateUIFromCurrentAct()
		self:updateCurrentActQuota()
	end

	------------------------------------------------
	function r2.ActUIDisplayer:onErase(erasedAct)
		-- clean tree content
		local tree = erasedAct.User.ContentTree
		local macroTree = erasedAct.User.MacroContentTree
		if tree then
			r2:cleanTreeNode(tree, "people")
			r2:cleanTreeNode(tree, "creatures")
		end
		if macroTree then
			--r2:cleanTreeNode(macroTree, "macro_components")
			r2:cleanTreeRootNode(macroTree)
		end
		local actTable = self:getActTable()	
		for index, entry in pairs(actTable) do
			if entry.Act == erasedAct then
				self:getActComboBox():removeTextByIndex(index - 1)
				table.remove(actTable, index)
				return
			end
		end 
	
      self:updateCurrentActQuota()
	end
	------------------------------------------------
	function r2.ActUIDisplayer:getActTable()	
		return self.ActTable
	end

	------------------------------------------------
	function r2.ActUIDisplayer:getContainer()
		return getUI("ui:interface:r2ed_scenario")
	end

	------------------------------------------------
	function r2.ActUIDisplayer:getActComboBox()
		return self:getContainer():find("act_combo_box")
	end

	-----------------------------------------------	
	function r2.ActUIDisplayer:findFreeTreeCtrl(macroTree)	
	
		local treeName = "act_tree_"
		if macroTree==true then treeName="macro_act_tree_" end	
		for i = 0, r2:getMaxNumberOfAdditionnalActs() - 1 do
			local tree = self:getContainer():find(treeName .. tostring(i))
			local used = false
			for index, entry in pairs(self:getActTable()) do
				local entryTree = entry.Tree
				if macroTree==true then entryTree = entry.MacroTree end	
				if entryTree == tree then
					used = true
					break
				end
			end
			if not used then
				return tree
			end
		end
		return nil
	end


function r2:createActUIDisplayer()
	return r2.ActUIDisplayer		
end

