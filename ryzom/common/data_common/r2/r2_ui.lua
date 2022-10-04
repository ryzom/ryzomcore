-- In this file we define functions that serves as an interface between the framework of the
-- editor (C++ code), and the ui. Ideally the C++ should never manipulate the ui directly.
-- It should rather call functions in this interface to do the job.
-- This allow for easy customisation, because users can replace any function with their
-- own lua code (possibly forwarding to the previous function).
-- This file works in close relation with the r2.xml file with contains the actual ui



-------------
-------------
-- GLOBALS --
-------------
-------------

-- The following table is intended to contain ui related classes & functions
-- For now the ui stuffs are being migrated into this table
-- TODO nico : move ui stuff here to 'depolute' the r2 table

r2.ui = {}


-----------------
-----------------
-- GLOBAL INIT --
-----------------
-----------------

function r2:initUI()	
	self:buildPaletteUI()
	r2.FeatureTree.buildFeatureTreeUI()
	r2:refreshWindowButtons()		
	--updateAllLocalisedElements()	
	-- init all forms
	for k, form in pairs(r2.Forms) do
		form.NameToProp = {}
		for l, prop in pairs(form.Prop) do
			form.NameToProp[prop.Name] = prop
		end
	end
	-- make the admin window point on the main page (obsolete, replaced by Session Browser)
	-- if browseRingWindowHomePage ~= nil then
	-- 	browseRingWindowHomePage()	
	-- else
	-- 	debugInfo("---> ERROR: what is browseRingWindowHomePage?")
	-- end
	r2.SelectBar:init()
	local paletteWnd = getUI("ui:interface:r2ed_palette")
	local debugPanel = paletteWnd:find("debug_panel").button_enclosing	
	local extDbg = (config.R2EDExtendedDebug == 1)

	local chooseLocationWnd = getUI("ui:interface:r2ed_acts")
	local ringLevelDebug = chooseLocationWnd:find("ring_level")

	if debugPanel then		
		debugPanel.reset_ed.active = extDbg
		debugPanel.reset_ed_and_reload_ui.active = extDbg
		debugPanel.reload_core.active = extDbg
		debugPanel.inspect_r2.active = extDbg
		debugPanel.reset_display.active = extDbg
		ringLevelDebug.active = extDbg
		-- debugPanel.test.active = extDbg
		-- debugPanel.create_scenario.active = extDbg
		-- debugPanel.create_act.active = extDbg
		-- debugPanel.connect_scenario.active = extDbg
		-- debugPanel.reconnect_scenario.active = extDbg
		 -- debugPanel.list_adventures.active = extDbg      
	end
	paletteWnd:find("tab2").active = true
	paletteWnd:find("tab2_bm").active = true
	paletteWnd:find("tab4").active = extDbg
	paletteWnd:find("tab4_bm").active = extDbg			
end


-----------
-----------
-- TESTS --
-----------
-----------
function testTree()
	local headNode = SNode()
	local rootNode = SNode()
	headNode:addChild(rootNode)
	rootNode.Text = "Root"
	rootNode.Opened = true
	rootNode.Bitmap = "ICO_Atys.tga"
	local tree = getUI(r2.instanceTree);
	for i = 1,3 do
		local son = SNode()
		son.Text = "Node " .. tostring(i)
		rootNode:addChild(son)
	end
	tree:setRootNode(headNode)
	tree:forceRebuild()
end

