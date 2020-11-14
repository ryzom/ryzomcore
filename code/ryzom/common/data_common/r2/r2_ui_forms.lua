
r2.Forms = {}

r2.EventTeamScenarios = 
{
	simple = {},
	odd_ball = {},
	adventure = {},
	challenge = {},
}

r2.LevelDesignScenarios = 
{ 
	cat1 = {},
	cat2 = {},
	cat3 = {},
}
--///////////////////
--// EDITION FORMS //
--///////////////////

-- a test form
r2.Forms.TestForm =
{
	Prop =
	{
		-- following field are tmp for property sheet building testing
		{Name="PVP", Type="Number", WidgetStyle="Boolean", Category="uiR2EDRollout_Test" },
		{Name="Slider1", Type="Number", WidgetStyle="Slider", Category="uiR2EDRollout_Test"},
		{Name="Slider2", Type="Number", WidgetStyle="Slider", Category="uiR2EDRollout_Test"},
		{Name="ComboBox1", Type="Number", WidgetStyle="EnumDropDown",
		 Enum= { "Toto", "Tata", "Titi" }
		},
		{Name="ComboBox2", Type="Number", WidgetStyle="EnumDropDown",
		 Enum= { "A", "B", "C" }
		},
		{Name="Title", Type="String", Category="uiR2EDRollout_EditBoxesTest"},
		{Name="Text1", Type="String", Category="uiR2EDRollout_EditBoxesTest"},
		{Name="Text2", Type="Number", Category="uiR2EDRollout_EditBoxesTest", Min="1", Max="10", Default="1"},
	}
	-- "onPostRender" : called by the framework after each frame (may be nil)
	-- onPostRender = function(formInstance)  
	--  ...
	-- end
	--
	-- "onShow" : called by the framework when the form is displayed
	-- onShow = function(formInstance)  
	--  ...
	-- end
}



r2.Forms.CreateNewAdventureStep1 =
{
	Caption = "uiR2EDScenarioParameters",
	Prop =
	{
 		{Name="Level", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Aventure",
		Enum= { "0-20", "0 - 50", "50 - 100"}
		},
 		{Name="Rule", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Aventure",
		 Enum= { "Liberal", "Strict"}
		},
		{Name="MaxEntities", Type="Number", Category="uiR2EDRollout_Aventure",  Min="0", Max="250", Default="50"}
	}
}

r2.Forms.CreateNewAdventureStep2 =
{
	Caption = "uiR2EDScenarioParameters",
	Prop =
	{
 		{
			Name="Location", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Aventure",
			Enum = r2.getIslandsLocation()
		},
 		{Name="EntryPoint", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Aventure",
		 Enum= { "Main Entry Point"}
		},
	}
}

r2.Forms.CreateNewAdventureScenarioDescription =
{
	Caption = "uiR2EDScenarioParameters",
	Prop =
	{
		{Name="Title", Type="String", Category="uiR2EDRollout_Aventure"},
	 	{Name="Category", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Aventure",
		 Enum= { "Invasion"}
		},
		{Name="Description", Type="String", Category="uiR2EDRollout_Aventure"}
	}
}

r2.Forms.CreateNewAdventureActDescription =
{
	Caption = "uiR2EDNewActParameters",
	Prop =
	{
		{Name="ActName", Type="String", Category="uiR2EDRollout_Aventure"},
 		{Name="Location", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Aventure",
		 Enum= { "Fyros", "Tryker"}
		},
 		{Name="EntryPoint", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Aventure",
		 Enum= { "EntryPoint1", "EntryPoint2"}
		}
	}
}


r2.Forms.ConnectAdventure =
{
	Caption = "uiR2EDConnectAdventure",
	Prop =
	{
		{Name="AdventureId", Type="Number", Category="uiR2EDRollout_Load"} 		
	}
}

r2.Forms.NewActForm =
{
	Caption = "uiR2EDNewActParameters",
	Prop =
	{		
		{Name="ActTitle", Type="String" },
		-- {Name="EntryPoint", Type="Number", WidgetStyle="EnumDropDown", Enum= { "Spawn Point 1", "Spawn Point 2" } }
	}	
}


-----------------
-- LOAD / SAVE --
-----------------

-- file list
local fileListXML = 
[[
   <group id="tb_enclosing" sizeref="wh" w="-16" h="0" x="16" y="0" posref="TL TL">
		<instance template="inner_thin_border" inherit_gc_alpha="true"/>				
	</group>
	<group id="enclosing" sizeref="w" w="-10" h="196" x="5" y="-5" posref="TL TL">
		<group id="file_list" 
		  type="list"
		  active="true" x="16" y="0" posref="TL TL"				  
		  sizeref="w"
		  child_resize_h="true"
		  max_sizeref="h"
		  max_h="0"		  
		>			
		</group>
		<ctrl style="skin_scroll" id="scroll_bar" align="T" target="file_list" />
	</group>
   <group id="gap" posref="BL TL" posparent="enclosing" w="1" h="6" />
]]

--scenario tree
local scenarioTreeXML =
[[	
	<group id="tb_enclosing" sizeref="wh" w="-16" h="0" x="16" y="0" posref="TL TL">
		<instance template="inner_thin_border" inherit_gc_alpha="true"/>				
	</group>
	<group id="enclosing" sizeref="w" w="-10" h="196" x="5" y="-5" posref="TL TL">
		<group id="tree_list" tooltip="uiR2EdPropertyToolTip_LoadScenario_LoadScenario_TreeInfo" type="tree" active="true" posref="TL TL" x="16" y="0" col_over="255 255 255 48" col_select="255 255 255 80"
			sizeref="w" max_sizeref="h" max_h="0">				
		</group>						
		<ctrl style="skin_scroll" id="scroll_bar" align="T" target="tree_list" />
	</group>
	<group id="gap" posref="BL TL" posparent="enclosing" w="1" h="6" />
]]


function r2.setCurrSelectedFile(filename, path)

	local formInstance = r2.CurrentForm.Env.FormInstance
	if formInstance.LastFileButton and formInstance.LastFileButton.pushed then
		formInstance.LastFileButton.pushed = false
	end
	if getUICaller().pushed then
		getUICaller().pushed = true
	end
	formInstance.LastFileButton = getUICaller()
	r2.CurrentForm.Env.FormInstance.Name = filename
	if path ~= nil and r2.CurrentForm.Env.FormInstance.Path then
		r2.CurrentForm.Env.FormInstance.Path = path
		
		local header = r2.getFileHeader(path..filename)
		if header then
			if header.Title then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Title = header.Title
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Title then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Title = ""	
			end
			if header.ShortDescription then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Description = header.ShortDescription
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Description then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Description = ""	
			end
			if header.Rules then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Rules = header.Rules
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Rules then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Rules = ""	
			end
			if header.Level then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Level = header.Level
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Level then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Level = ""	
			end
			if header.Language then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Language = i18n.get("uiR2ED"..header.Language):toUtf8()
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Language then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Language = ""	
			end
			if header.RingPointLevel then
				if r2.checkRingAccess(header.RingPointLevel) == true then
					r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel = i18n.get("uiR2EDLoadScenario_Yes"):toUtf8()
				else 
					r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel = i18n.get("uiR2EDLoadScenario_No"):toUtf8()
				end
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel then
				r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel = ""	
			end

		end
	end
	
	r2.CurrentForm.Env.updateAll()
	r2.CurrentForm.Env.updateSize()
	local eb =   r2.CurrentForm:find("eb")  
	setCaptureKeyboard(eb)
	eb:setSelectionAll()
end


function r2.setCurrSelectedFileToLoad(filename, path)

	local formInstance = r2.CurrentForm.Env.FormInstance
	if formInstance.LastFileButton and formInstance.LastFileButton.pushed then
		formInstance.LastFileButton.pushed = false
	end
	if getUICaller().pushed then
		getUICaller().pushed = true
	end
	formInstance.LastFileButton = getUICaller()
	r2.CurrentForm.Env.FormInstance.LoadScenario_Name = ucstring(filename):toUtf8()

	if path ~= nil and r2.CurrentForm.Env.FormInstance.Path then
		r2.CurrentForm.Env.FormInstance.Path = path
		
		local header = r2.getFileHeader(path..filename)
		if header then
			if header.Title then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Title = header.Title
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Title then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Title = ""	
			end
			if header.ShortDescription then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Description = header.ShortDescription
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Description then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Description = ""	
			end
			if header.Rules then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Rules = header.Rules
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Rules then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Rules = ""	
			end
			if header.Level then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Level = header.Level
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Level then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Level = ""	
			end
			if header.Language then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Language = i18n.get("uiR2ED"..header.Language):toUtf8()
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_Language then
				r2.CurrentForm.Env.FormInstance.LoadScenario_Language = ""	
			end
			if header.RingPointLevel then
				if r2.checkRingAccess(header.RingPointLevel) == true then
					r2.CurrentForm.Env.FormInstance.RingAccess = 1
					r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel = i18n.get("uiR2EDLoadScenario_Yes"):toUtf8()
				else 
					r2.CurrentForm.Env.FormInstance.RingAccess = 0
					r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel = i18n.get("uiR2EDLoadScenario_No"):toUtf8()
				end
			elseif r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel then
				r2.CurrentForm.Env.FormInstance.RingAccess = 0
				r2.CurrentForm.Env.FormInstance.LoadScenario_RingPointsLevel = ""	
			end

		end
	end
	
	r2.CurrentForm.Env.updateAll()
	r2.CurrentForm.Env.updateSize()
	--local eb =   r2.CurrentForm:find("eb")  
	--setCaptureKeyboard(eb)
	--eb:setSelectionAll()
end

-- called at init to fill the file list
local function showFileList(formInstance)
	local fileGroupList = r2.CurrentForm:find('file_list')
	--local searchPath = select(config.R2ScenariiPath, "save")
	local searchPath =r2.getScenarioSavePath()
	local files = getPathContent(searchPath)	
	table.sort(files)
	fileGroupList:clear()  
	for k, v in pairs(files) do
		if string.lower(nlfile.getExtension(v)) == "r2" then			
			local shortFilename = nlfile.getFilename(v)
			local shortFileEscape=string.gsub(shortFilename, "'", "\\'")

			local shortFilename2 = ucstring(shortFilename):toUtf8()
			local shortFileEscape2 = ucstring(shortFileEscape):toUtf8()


			local entry = createGroupInstance("r2ed_filelist_entry", "", 
				{ id = tostring(k),  text = shortFilename2,
				params_l="r2.setCurrSelectedFile('" .. shortFileEscape2 .. "')" })
			fileGroupList:addChild(entry)
		end
	end
   setCaptureKeyboard(r2.CurrentForm:find("eb"))
end



local function saveScenarioOnChange(formInstance)
	r2.print(formInstance.Name)
	local name = string.gsub(formInstance.Name,  "[\\/:*?\"<>|]", "_")
	if name ~= formInstance.Name then
		fromInstance.Name = name
		formInstance.Modified = true
	end
end




local function buildScenarioTree(formInstance)
	
	local tree = r2.CurrentForm:find('tree_list')

	local rootNode = SNode()
	rootNode.Id = "root"
	rootNode.Text = "Scenarios"
	rootNode.Opened = false 
	--rootNode.Bitmap = "r2ed_icon_macro_components.tga"
	tree:setRootNode(rootNode)

	function addNodeToParent(name, id, parentId)
		local parentNode = rootNode:getNodeFromId(parentId)
		
		addedNode = SNode()
		--local text = i18n.hasTranslation(parentCategory)
		--if not text then text = parentCategory else text = i18n.get(parentCategory) end
		addedNode.Text = name
		addedNode.Id = id
		addedNode.Opened = false
		parentNode:addChild(addedNode)
	end
	
	function parseDirectory(nodeId, searchPath)
		local destNode = rootNode:getNodeFromId(nodeId)
		local autosaveNode = rootNode:getNodeFromId("autosave")
		local files = getPathContent(searchPath)
		table.sort(files)
		for k, v in pairs(files) do
			if string.lower(nlfile.getExtension(v)) == "r2" then			
				local shortFilename = nlfile.getFilename(v)
				local shortFileEscape=string.gsub(shortFilename, "'", "\\'")
				
				local fileLeaf= SNode()
				fileLeaf.Id = shortFilename
				local len = string.len(shortFilename) - 3
				fileLeaf.Text = string.sub(shortFilename,0, len)
				fileLeaf.AHName = "lua"
				fileLeaf.AHParams = "r2.setCurrSelectedFileToLoad('" .. shortFileEscape .. "', '"..searchPath.."')"				
				--if autosave file, plug it in the autosave branch
				if string.find(shortFilename, "autosave") ~= nil then 
					autosaveNode:addChild(fileLeaf)
				else
					destNode:addChild(fileLeaf)
				end
			end
		end
	end
	
	function loadLDScenariosFromFile()
		--if using a bnp, use doFile method instead
		local fileListOk = loadfile("examples/r2_leveldesign_scenarios.lua")
		if fileListOk then
			fileListOk()
		end
		local files = r2.LevelDesignScenarios
		for parentId, fileEntries in pairs(files) do
			local parentNode = rootNode:getNodeFromId(parentId)
			if not parentNode then
				addNodeToParent(parentId, parentId, "root")
				parentNode = rootNode:getNodeFromId(parentId)
			end
			for k, v in pairs(fileEntries) do
				local fileLeaf= SNode()
				fileLeaf.Id = v[1]
				--fileLeaf.Text = v[1]
				local len = string.len(v[1]) - 3
				fileLeaf.Text = string.gsub(string.sub(v[1],0, len), "_", " ")
				fileLeaf.AHName = "lua"
				fileLeaf.AHParams = "r2.setCurrSelectedFileToLoad('" .. v[1] .. "', './Examples/')"				
				parentNode:addChild(fileLeaf)
			end
		end 
	end
	
	function loadETScenariosFromFile()
		--if using a bnp, use doFile method instead
		local fileListOk = loadfile("examples/r2_event_team_scenarios.lua") 
		if fileListOk then
			fileListOk()
		end
		local files = r2.EventTeamScenarios
		for parentId, fileEntries in pairs(files) do
			local parentNode = rootNode:getNodeFromId(parentId)
			if not parentNode then
				addNodeToParent(parentId, parentId, "root")
				parentNode = rootNode:getNodeFromId(parentId)
			end
			for k, v in pairs(fileEntries) do
				local fileLeaf= SNode()
				fileLeaf.Id = v[1]
				--fileLeaf.Text = v[1]
				local len = string.len(v[1]) - 3
				fileLeaf.Text = string.gsub(string.sub(v[1],0, len), "_", " ")
				fileLeaf.AHName = "lua"
				fileLeaf.AHParams = "r2.setCurrSelectedFileToLoad('" .. v[1] .. "', './Examples/')"				
				parentNode:addChild(fileLeaf)
			end
		end 
	end
	
	function loadNewbieScenariosFromFile()
		--if using a bnp, use doFile method instead
		local fileListOk = loadfile("examples/r2_newbieland_scenarios.lua")
		if fileListOk then
			fileListOk()
		end
		local files = r2.NewbielandScenarios
		if files then
			for parentId, fileEntries in pairs(files) do
				local parentNode = rootNode:getNodeFromId(parentId)
				if not parentNode then
					addNodeToParent(parentId, parentId, "root")
					parentNode = rootNode:getNodeFromId(parentId)
				end
				for k, v in pairs(fileEntries) do
					local fileLeaf= SNode()
					fileLeaf.Id = v[1]
					local len = string.len(v[1]) - 3
					fileLeaf.Text = string.gsub(string.sub(v[1],0, len), "_", " ")
					fileLeaf.AHName = "lua"
					fileLeaf.AHParams = "r2.setCurrSelectedFileToLoad('" .. v[1] .. "', './Examples/')"				
					parentNode:addChild(fileLeaf)
				end
			end
		end
	end

	--LEVELDESIGN SCENARIOS
	--addNodeToParent("Nevrax Examples", "examples", "root")
	--addNodeToParent("Simple", "simple", "examples")
	--addNodeToParent("Odd Ball", "odd_ball", "examples")
	--addNodeToParent("Adventure", "adventure", "examples")
	--addNodeToParent("Challenge", "challenge", "examples")
	loadLDScenariosFromFile()
	
	--EVENTTEAM SCENARIOS
	--addNodeToParent("Pioneer Scenarios", "event_team_offerings", "root")
	loadETScenariosFromFile()

	--NEWBIE SCENARIOS
	loadNewbieScenariosFromFile()

	--MY SCENARIOS
	addNodeToParent("My Files", "my_files", "root")
	addNodeToParent("Autosave", "autosave", "my_files")
	addNodeToParent("My scenarios", "my_scenarios", "my_files")
	parseDirectory("my_scenarios", "./My_scenarios/")

	--Tmp branch for old scenarios
	addNodeToParent("Old Scenarios", "old", "root")
	parseDirectory("old", "./")

	tree:forceRebuild()
end


-- load / save forms
r2.Forms.SaveScenario =
{
	Caption = "uiR2EDSaveScenario",
	PropertySheetHeader = fileListXML,
	Prop =
	{
		{Name="Name", Type="String", EntryType="filename", Category="uiR2EDRollout_Save", ValidateOnEnter = true, MaxNumChar="512" , onChange = saveScenarioOnChange} 		
	},
	onShow = showFileList
}


r2.Forms.LoadScenario =
{
	Caption = "uiR2EDLoadScenario",
	PropertySheetHeader = scenarioTreeXML, --fileListXML,
	Prop =
	{
		{Name="LoadScenario_Name", Type="String", WidgetStyle="StaticText", Category="uiR2EDRollout_Load"},
		{Name="Path", Type="String", Visible=false},
		{Name="RingAccess", Type="Number", Visible=false},
		{Name="LoadScenario_Title", Type="String", WidgetStyle="StaticText", Category="uiR2EDRollout_ScenarioInfo"},
		{Name="LoadScenario_Description", Type="String", WidgetStyle="StaticTextMultiline", Category="uiR2EDRollout_ScenarioInfo"},
		{Name="LoadScenario_Rules", Type="String", WidgetStyle="StaticText", Category="uiR2EDRollout_ScenarioInfo"},
		{Name="LoadScenario_Level", Type="String", WidgetStyle="StaticText", Category="uiR2EDRollout_ScenarioInfo"},
		{Name="LoadScenario_Language", Type="String", WidgetStyle="StaticText", Category="uiR2EDRollout_ScenarioInfo"},
		{Name="LoadScenario_RingPointsLevel", Type="String", WidgetStyle="StaticText", Category="uiR2EDRollout_ScenarioInfo"},
	},
	onShow = buildScenarioTree
}


--/////////////////////
--// ANIMATION FORMS //
--/////////////////////


-------------
-- WEATHER --
-------------

local function updateWeather(formInstance) 
      setWeatherValue(formInstance.ManualWeather == 0, formInstance.WeatherValue / 1022)
	  formInstance.Modified = true
end

r2.Forms.ChangeWeatherForm =
{
	Caption = "uiR2EDChangeWeather",
   Width="300",
	PropertySheetHeader = 
	[[
		<view type="bitmap" id="wicon" w="32" h="32" global_color="false" posref="TL TL" texture="r2_icon_weather.tga"/>
		<view type="text" id="t" multi_line="true" sizeref="w" w="-36" x="4" y="-2" posparent="wicon" posref="TR TL" global_color="true" fontsize="14" shadow="true" hardtext="uiR2EDWeatherInfo"/>
	]],
	Prop =
	{		
		{Name="ManualWeather", Type="Number", WidgetStyle="Boolean",
         onChange = updateWeather
		},
		{ Name="WeatherValue", Type="Number", WidgetStyle="Slider", Min=0, Max=1022, 
		  LeftBitmap="r2ed_sun.tga", MiddleBitmap="r2ed_clouds.tga", RightBitmap="r2ed_storm.tga", ActiveBitmaps="true",
		  onChange = updateWeather,
          Visible = function(form) return form.ManualWeather == 1 end
		},
		--{Name="Season", Type="Number", WidgetStyle="EnumDropDown", DefaultValue="0",
        --   Enum= { "uiR2EDSeasonAuto", "uiR2EDSpring", "uiR2EDSummer", "uiR2EDAutumn", "uiR2EDWinter" },			  
		--}
	},	
	onPostRender = function(formInstance)  
	  -- if form hasn't been modified, then continuously update weather from the db value 
	  if not formInstance.Modified and formInstance.ManualWeather then
		formInstance.WeatherValue = getWeatherValue() * 1022
		r2.CurrentForm.Env.updateAll()
	  end
	end,
}

--
-- User Defined Component
--

function r2:userComponentOk(shortFilename, msg)
	r2.setCurrSelectedFile(shortFilename)
	messageBox(msg)
end

function r2:userComponentKo(shortFilename, msg)
	messageBox(msg.." in " .. shortFilename)
	r2.setCurrSelectedFile("")
end


-- called at init to fill the file list
local function showUserDefinedComponent(formInstance)
	local fileGroupList = r2.CurrentForm:find('file_list')
	--local searchPath = select(config.R2ScenariiPath, "save")
	local searchPath = formInstance.Directory
	local files = getPathContent(searchPath)	
	table.sort(files)
	fileGroupList:clear()  
	for k, v in pairs(files) do
		if string.lower(nlfile.getExtension(v)) == formInstance.Extension then			
			local shortFilename = nlfile.getFilename(v)
			local package = r2.UserComponentsManager:loadPackage(v)

			if package then
				local description= string.gsub(package.Description, "\n", "\\n")
				description= "Description="..description.."\\n\\n"
				local name = "Name= "..package.Name .. "\\n\\n"
				local md5 = ""
				local tstamp = ""
				local path=""
				if package.fileinfo and package.fileinfo.MD5 and package.fileinfo.TimeStamp and package.fileinfo.Package then
					md5 = "MD5="..package.fileinfo.MD5.."\\n\\n"
					tstamp = "TimeStamp=" .. tostring(package.fileinfo.TimeStamp) .. "\\n\\n"
					path = "Package="..package.fileinfo.Package.."\\n\\n"					
				end
				local str = name..description .. md5..tstamp..path
				debugInfo(str)
		
				local entry = createGroupInstance("r2ed_filelist_entry", "", { id = tostring(k), text = "OK: " .. shortFilename, params_l="r2:userComponentOk('" .. shortFilename .. "', '"..str.."')" })
				fileGroupList:addChild(entry)
			else

				local entry = createGroupInstance("r2ed_filelist_entry", "", { id = tostring(k), text =  "KO: " .. shortFilename, params_l="r2:userComponentKo('" ..shortFilename .."', 'syntax error')" })
				fileGroupList:addChild(entry)
			end
		end
	end
   setCaptureKeyboard(r2.CurrentForm:find("eb"))
end



r2.Forms.LoadUserComponent =
{
	Caption = "uiR2EDLoadUserComponent",
	PropertySheetHeader = fileListXML,
	Prop =
	{
		{Name="Name", Type="String", Category="uiR2EDRollout_Load", ValidateOnEnter = true, MaxNumChar="512"} 	
	},
	onShow = showUserDefinedComponent
}

r2.Forms.SpecialPaste =
{
	Caption = "uiR2EDSpecialPaste",	
	Prop =
	{      
		{ Name="CopyEvents", Type="Number", WidgetStyle="Boolean", InvertWidget="true", CaptionWidth=5,
		   Visible = function(props)  
           return props.CopyEvents >= 0 
         end
		},
		{ Name="CopyActivities", Type="Number", WidgetStyle="Boolean", InvertWidget="true", CaptionWidth=5,
		   Visible = function(props)  
           return props.CopyActivities >= 0 
         end
		},
		{ Name="DuplicateGroup", Type="Number", WidgetStyle="Boolean", InvertWidget="true", CaptionWidth=5,
		   Visible = function(props)  
           return props.DuplicateGroup >= 0 
         end
		},
		--{ Name="CopyChatSequences", Type="Number", WidgetStyle="Boolean", InvertWidget="true", CaptionWidth=5,
		-- Visible = function(props)  
      -- return props.CopyChatSequences >= 0 
      -- end
		-- },
	},	
}



