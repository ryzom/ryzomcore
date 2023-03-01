----------------
-- PALETTE UI --
----------------

------------------------------------------------------------------------------------------------------------
-- handle right click on palette item
function r2:onPaletteRightClick(paletteNode)
    -- store palette path for futur display	
	self.tmpPaletteNode = paletteNode
end

------------------------------------------------------------------------------------------------------------
function r2:onShowPaletteLuaTable(maxDepth)
	if (self.tmpPaletteNode == nil) then return end
	runCommand("luaObject", self.tmpPaletteNode, maxDepth)
end



------------------------------------------------------------------------------------------------------------
function r2:buildPaletteUI()	

	--debugInfo(colorTag(0, 255, 127) .."Begin building palette UI")	
	local tree = getUI(r2.Palette.UIPath)
	local botTree = getUI(r2.Palette.BotObjectsUIPath)
	if tree==nil or botTree==nil then return end		

	local shortElementId = 0

	r2.Palette.ShortIdToElement = {}

	local function buildBranch(luaBranch, branchStrId, depth, openDepth)
		local newNode = SNode()
		newNode.Opened = depth < openDepth		
		newNode.Id = "branch"
		newNode.Text = i18n.get(branchStrId)
		for k, v in pairs(luaBranch) do			
			if (k ~= "instances") then

				if type(v)=="table" and v.Display~=false then
	
					-- this is a sub branch
					newNode:addChild(buildBranch(v, "uiR2ED" .. k, depth + 1, openDepth))
				end
			else
				for instKey, instValue in pairs(v) do
					-- the value is a table containing a list of instances					
					local subNode = SNode()	
					if instValue.DirectName ~= nil then				
						subNode.Text = instValue.DirectName						
					else
						subNode.Text = i18n.get(instValue.Translation)						
					end

					r2.PaletteIdToTranslation[instValue.Id] = subNode.Text:toUtf8()
					r2.PaletteIdToGroupTranslation[instValue.Id] = newNode.Text:toUtf8()
					r2.PaletteIdToType[instValue.Id] = v

					subNode.Id = tostring(shortElementId)
					local paletteElement = r2.getPaletteElement(instValue.Id)
					if (paletteElement == nil)
					then
						debugInfo("invalid nil Palette: " .. instValue.Id)
					else
						r2.Palette.ShortIdToElement[tostring(shortElementId)] = paletteElement
						shortElementId = shortElementId + 1
						subNode.Opened = false
						subNode.AHName = "r2ed_create_entity"
						subNode.AHParams = "PaletteId=" .. instValue.Id
						--subNode.AHNameRight = "lua"
						--subNode.AHParamsRight = "r2ed:onPaletteRightClick(" .. instValue.Id .. ")"
						local ringAccess = r2.getPropertyValue(paletteElement, "RingAccess")
						local insertNode = true
						if ringAccess then
							insertNode = r2.RingAccess.testAccess(ringAccess)
						end
						if insertNode then
							newNode:addChild(subNode)
						end
					end
				end
			end
		end
		return newNode
	end
	-- create root node & call function
	local rootNode = buildBranch(r2.Palette.Entries, r2.Palette.StrId, 0, 2)
	rootNode:sort()
	tree:setRootNode(rootNode)	

	local botRootNode = buildBranch(r2.Palette.BotEntries, r2.Palette.StrId, 0, 1)
	botRootNode:sort()
	botTree:setRootNode(botRootNode)	
	--
	local paletteWindow = tree:getEnclosingContainer()
	if paletteWindow then
		-- paletteWindow.active = true
		-- paletteWindow:updateCoords()
		local selection = paletteWindow:find("entity_selection")
		local enclosing = paletteWindow:find("entity_enclosing")
		local delta = 6
		enclosing.h = - selection.h_real - delta
		enclosing.y = - selection.h_real - delta
		paletteWindow:invalidateCoords()
	end

	--debugInfo(colorTag(0, 255, 127) .. "Palette UI built")
	r2:setupPaletteAccessibleContent()
	--r2:setupDefaultCustomBBoxes()
end

---------------------------------------------------------------------------------------------------------
-- Setup the content that is visible in the palette depending on the chosen ecosystem and level
function r2:setupPaletteAccessibleContent()

	local levelMin, levelMax, ecosystem = r2:getPaletteAccessibilityFactors()

	local function setupBranch(branch)
		local show = false
		if branch.Id == "branch" then			
			for index = 0, branch:getNumChildren() - 1 do
				local showChild = setupBranch(branch:getChild(index))
				if showChild then
					show = true
				end
			end
		else			
			-- this is a leaf
			local paletteNode = r2.Palette.ShortIdToElement[branch.Id]
			--assert(paletteNode)
			if paletteNode then
				local currLevel = defaulting(paletteNode.Level, 1)
				local currEcosystem = defaulting(paletteNode.Ecosystem, "")
				-- tmp : ignore level & ecosystem for objects
				if string.match(paletteNode.SheetClient, "object.*") then
					show = true
				elseif currLevel >= levelMin and currLevel <= levelMax and (currEcosystem == "" or string.match(currEcosystem, ecosystem)) then
					show = true					
				else
					--debugInfo(currEcosystem)
				end
			end
		end		
		-- if show then 
		-- branch.Color = CRGBA(255, 255, 255)
		-- else
		--	branch.Color = CRGBA(255, 0, 0)
		--end
		branch.Show = show
		return show
	end	

	if ecosystem and levelMin and levelMax then
		local tree = getUI(r2.Palette.UIPath)
		setupBranch(tree:getRootNode())	
		tree:forceRebuild()
	end
end

function r2:getPaletteAccessibilityFactors()

	local tree = getUI(r2.Palette.UIPath)	
	if tree == nil then return end	
	local levelRange = tree:getEnclosingContainer():find("level").selection + 1 -- TMP : tha 'all' field was removed ...
	local levelMin
	local levelMax
	if levelRange == 0 then 
		levelMin = 1
		levelMax = 250
	else
		levelMin = (levelRange - 1) * 50 + 1
		levelMax = levelMin + 49
	end	
	local ecosystemTable = 
	{
		".*", "Desert", "Forest", "Jungle", "Lacustre", "PrimeRoots", "Goo"
	}
	local ecosystem= ecosystemTable[tree:getEnclosingContainer():find("ecosystem").selection + 2]	-- TMP : added 2 instead of 1 because the 'all' field has been removed 
	
	return levelMin, levelMax, ecosystem
end

function r2:createRoad()

   r2:setCurrentTool('R2::CToolDrawPrim', { Look = r2.PrimRender.RoadCreateLook,
											InvalidLook = r2.PrimRender.RoadCreateInvalidLook,
											Type="Road", ForceShowPrims=true  })
end

function r2:createRegion()

   r2:setCurrentTool('R2::CToolDrawPrim', 
                     { 
					   Look = r2.PrimRender.RegionCreateLook, 
					   InvalidLook = r2.PrimRender.RegionCreateInvalidLook,
					   CanCloseLook = r2.PrimRender.RegionCreateCanCloseLook,
					   Type = "Region",
					   SelectInstance = true,
					   ForceShowPrims=true
					 }
					)
end

---------------------------------------------------------------------------------------------------------
-- TMP for demo : assign default custom bbox to mobs
--function r2:setupDefaultCustomBBoxes()
-- for id, node in pairs(r2.Palette.ShortIdToElement) do
--     if string.match(node.Base, "palette.entities.creatures.*") then
--         local box =
--         {
--             Enabled = true,
--             XMin = -0.5,
--             XMax = 0.5,
--             YMin = -0.5,
--             YMax = 0.5,
--             ZMin = 0,
--             ZMax = 2,
--         }
--         r2:setEntityCustomSelectBox(node.SheetClient, box)
--     end
-- end
--end

