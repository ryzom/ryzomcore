
r2.FeatureTree = 
{
	Path = "ui:interface:r2ed_palette:content:feature_tree_panel:feature_enclosing:tree_list",
}

local featureTree = r2.FeatureTree

function featureTree.buildFeatureTreeUI() 
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return end

	local rootNode = SNode()
	rootNode.Text = "Features"
	rootNode.Opened = false 
	--rootNode.Bitmap = "r2ed_icon_macro_components.tga"
	tree:setRootNode(rootNode)

	featureTree.addAllFeatures()
	
	
	tree:forceRebuild()
	
end

function featureTree.getCategoryPicture(category)
	local categories = r2.getCategories()
	local k, v = next(categories, nil)
	while k do
		if v then
			if v[1] == category then
				return v[2]
			end
		end
		k, v = next(categories, k)
	end 
	return "r2ed_icon_macro_components.tga"
end

function featureTree.getFatherNode(category)
	local categories = r2.getCategories()
	local k, v = next(categories, nil)
	while k do
		if v then
			if v[1] == category then
				return v[3]
			end
		end
		k, v = next(categories, k)
	end 
	return "root"
end

function featureTree.createParentNode(parentCategory)
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return end
	
	local root = tree:getRootNode()
	if not root then return end	
	
	categoryNode = SNode()
	local text = i18n.hasTranslation(parentCategory)
	if not text then text = parentCategory else text = i18n.get(parentCategory) end
	categoryNode.Text = text
	categoryNode.Id = parentCategory
	categoryNode.Opened = false
	--categoryNode.Bitmap = featureTree.getCategoryPicture(parentCategory)
	root:addChild(categoryNode)
	return categoryNode
end


function featureTree.addNodeWithId(featureName, category)
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return end
	
	local root = tree:getRootNode()
	if not root then return end

	local categoryNode = root:getNodeFromId(category)
	
	if not categoryNode then	
		categoryNode = SNode()
		local text = i18n.hasTranslation(category)
		if not text then text = category else text = i18n.get(category) end
		categoryNode.Text = text
		categoryNode.Id = category
		categoryNode.Opened = false
		local catFather = featureTree.getFatherNode(category)
		if catFather == "root" then
			--categoryNode.Bitmap = featureTree.getCategoryPicture(category)
			root:addChild(categoryNode)
		else
			local fatherNode = root:getNodeFromId(catFather)
			if not fatherNode then --if the parent node doesn't exist, attach new category to root anyway
				fatherNode = featureTree.createParentNode(catFather)
			end
			fatherNode:addChild(categoryNode)
		end
	end
	
	local featureNode = SNode()
	local componentName = featureName

	--special case: chat sequence is not a feature like the others
	if featureName == "ChatSequence" then
		featureNode.Text = i18n.get("uiR2EDChatSequence")
		featureNode.AHName = "lua"
		featureNode.AHParams = "r2.Features['ActivitySequence'].Components.ChatSequence:createProtected()"
		categoryNode:addChild(featureNode)
		return
	end

	if string.find(componentName, "Feature") ~= nil then
		componentName = string.sub(componentName, 1, string.len(componentName) - 7)
	end
	featureNode.Text = i18n.get("uiR2Ed"..featureName.. "Node")
	featureNode.AHName = "lua"
	featureNode.AHParams = "r2.Features['".. featureName.."'].Components." ..componentName.. ":createProtected()"

	categoryNode:addChild(featureNode)
end

function featureTree.addUserComponentNode(userComponentName)
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return false end
	
	local root = tree:getRootNode()
	if not root then return false end

	local userComponentsBranch = root:getNodeFromId("uiR2EdUserComponentCategory")
	if not userComponentsBranch then return end
	
	local presentNode = root:getNodeFromId(userComponentName)
	if presentNode ~= nil then 
		messageBox("The user component '"..userComponentName.."' is already loaded. Please unload it before loading a user component with the same name.")
		return false
	end

	local categoryNode = root:getNodeFromId("uiR2EdLoadedUserComponentCategory")
	
	if not categoryNode then	
		categoryNode = SNode()
		categoryNode.Text = i18n.get("uiR2EdLoadedUserComponentCategory")
		categoryNode.Id = "uiR2EdLoadedUserComponentCategory"
		categoryNode.Opened = false
		userComponentsBranch:addChild(categoryNode)
	end
	
	local featureNode = SNode()
	
	featureNode.Id = userComponentName
	featureNode.Text = userComponentName
	featureNode.AHName = "lua"
	featureNode.AHParams = "r2.Translator.CreateUserComponent('"..userComponentName.."')"

	categoryNode:addChild(featureNode)
	tree:forceRebuild()
	return true
end

function featureTree.getUserComponentList()
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return false end
	
	local root = tree:getRootNode()
	if not root then return false end
	
	local featureNameTable = {}
	local userComponentCategoryNode = root:getNodeFromId("uiR2EdLoadedUserComponentCategory")
	if not userComponentCategoryNode then
		return {}
	end
	local nodeSize = userComponentCategoryNode:getNumChildren() 
	local i = 0
	while i < nodeSize do
		local node = userComponentCategoryNode:getChild(i)
		table.insert(featureNameTable, node.Id)
		i = i + 1
	end
	return featureNameTable
end


function featureTree.addAllFeatures()
	local loadedFeatures = r2.getLoadedFeaturesStatic()
	
	local k, v = next(loadedFeatures, nil)
	while k do
		if v then
			featureTree.addNodeWithId(v[2], v[3])
		end
		k, v = next(loadedFeatures, k)
	end

	if config.R2EDLoadDynamicFeatures == 1 then

		local loadBt = getUI("ui:interface:r2ed_palette:content:feature_tree_panel:user_component_buttons:load")
		local unloadBt = getUI("ui:interface:r2ed_palette:content:feature_tree_panel:user_component_buttons:unload")
		
		--loadBt.active = true
		--unloadBt = true
		featureTree.addLoadedUserComponents()

		local btPanel = getUI("ui:interface:r2ed_palette:content:feature_tree_panel:user_component_buttons")
		btPanel.active = true

		local loadedFeatures = r2.getLoadedFeaturesDynamic()
	
		local k, v = next(loadedFeatures, nil)
		while k do
			if v then
				featureTree.addNodeWithId(v[2], v[3])
			end
			k, v = next(loadedFeatures, k)
		end

	end
end


function featureTree.addLoadedUserComponents()
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return end
	
	local root = tree:getRootNode()
	if not root then return end
	
	categoryNode = SNode()
	categoryNode.Text = i18n.get("uiR2EdUserComponentCategory")
	categoryNode.Id = "uiR2EdUserComponentCategory"
	categoryNode.Opened = false
	--categoryNode.Bitmap = "r2ed_icon_macro_components.tga"
	root:addChild(categoryNode)
	
	local featureNode = SNode()
	
	featureNode.Id = "NewComponent"
	featureNode.Text = "New Component"
	featureNode.AHName = "lua"
	featureNode.AHParams = "r2.Features['DefaultFeature'].Components.UserComponentHolder.create()"
	

	categoryNode:addChild(featureNode)
	
	local loadedUserComponentTable = r2_core.UserComponentTable
	if table.getn(loadedUserComponentTable) == 0 then
		debugInfo("No UserComponent were previously loaded")
		return
	end
	
	local k, v = next(loadedUserComponentTable, nil)
	while k do
		local userComponentName = v[1]
		featureTree.addUserComponentNode(userComponentName)
		k, v = next(loadedUserComponentTable, k)
	end
end
function featureTree.removeUCFromTree(featureName)
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return end
	
	local root = tree:getRootNode()
	if not root then return end
	
	local featureNode = root:getNodeFromId(featureName)
	if featureNode:getFather() then
		featureNode:getFather():deleteChild(featureNode)
	end
	
	local categoryNode = root:getNodeFromId("uiR2EdLoadedUserComponentCategory")
	local num = categoryNode:getNumChildren() 
	if num == 0 then
		root:deleteChild(categoryNode)
	end
	tree:forceRebuild()

	categoryNode:addChild(featureNode)
	
	local loadedUserComponentTable = r2_core.UserComponentTable
	if table.getn(loadedUserComponentTable) == 0 then
		debugInfo("No UserComponent were previously loaded")
		return
	end
	
	local k, v = next(loadedUserComponentTable, nil)
	while k do
		local userComponentName = v[1]
		featureTree.addUserComponentNode(userComponentName)
		k, v = next(loadedUserComponentTable, k)
	end
end
function featureTree.removeUCFromTree(featureName)
	local tree = getUI(r2.FeatureTree.Path)
	if not tree then return end
	
	local root = tree:getRootNode()
	if not root then return end
	
	local featureNode = root:getNodeFromId(featureName)
	if featureNode:getFather() then
		featureNode:getFather():deleteChild(featureNode)
	end
	
	local categoryNode = root:getNodeFromId("uiR2EdLoadedUserComponentCategory")
	local num = categoryNode:getNumChildren() 
	if num == 0 then
		root:deleteChild(categoryNode)
	end
	tree:forceRebuild()

end
