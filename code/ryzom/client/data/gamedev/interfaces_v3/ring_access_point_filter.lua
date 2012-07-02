
-- table containing ring access point function
game.RingAccessPointFilter = 
{
	-- map level index in the combo_box to the url param string
	LevelToURLParam = 
	{
		[0] = "",
		[1] = "sl_a",
		[2] = "sl_b",
		[3] = "sl_c",
		[4] = "sl_d",
		[5] = "sl_e"
	},
	OrientationToURLParam = 
	{
		[0] = "",
		[1] = "so_newbie_training",
		[2] = "so_story_telling",
      [3] = "so_mistery",
      [4] = "so_hack_slash",
      [5] = "so_guild_training",
      [6] = "so_other"
	},
   InitDone = false
}

-----------------------------------------------------------------------------------------
-- Get reference on the filter window
function game.RingAccessPointFilter:getWnd()
   return getUI("ui:interface:ring_access_point_filter")
end


-----------------------------------------------------------------------------------------
-- Init content of the filter window if needed
function game.RingAccessPointFilter:init()
   if self.InitDone then return end
   self.InitDone = true 
   local filterScenarioWnd = self:getWnd()
	local owner = filterScenarioWnd:find("owner"):find("edit_box_group")
	assert(owner)
	owner.input_string = ""

	local levelCB = filterScenarioWnd:find("level"):find("combo_box")
	assert(levelCB)
	levelCB:resetTexts()
	levelCB:addText(ucstring(i18n.get("uiScenarioLevel_Any")))
	levelCB:addText(ucstring("1-50"))
	levelCB:addText(ucstring("51-100"))
	levelCB:addText(ucstring("101-150"))
	levelCB:addText(ucstring("151-200"))
	levelCB:addText(ucstring("201-250"))
	levelCB.selection = 0

	local genreCB = filterScenarioWnd:find("genre"):find("combo_box")
	assert(genreCB)
	genreCB:resetTexts()
	genreCB:addText(ucstring(i18n.get("uiScenarioOrient_Any")))
	genreCB:addText(ucstring(i18n.get("uiScenarioOrient_NewbieTraining")))
	genreCB:addText(ucstring(i18n.get("uiScenarioOrient_StoryTelling")))
	genreCB:addText(ucstring(i18n.get("uiScenarioOrient_Mistery")))
	genreCB:addText(ucstring(i18n.get("uiScenarioOrient_HackSlash")))
	genreCB:addText(ucstring(i18n.get("uiScenarioOrient_GuildTraining")))
	genreCB:addText(ucstring(i18n.get("uiScenarioOrient_Other")))
	genreCB.selection = 0
	
	local name = filterScenarioWnd:find("name"):find("edit_box_group")
	assert(name)
	name.input_string = ""	
end


-----------------------------------------------------------------------------------------
-- Open the filter window
function game.RingAccessPointFilter:open()
   self:init()
	self:getWnd().active = true	   
end

-----------------------------------------------------------------------------------------
-- Get the url parameters to append to the url to browse with the current filter
function game.RingAccessPointFilter:getURLParameters()	
   self:init()
	local filterScenarioWnd = self:getWnd()
	assert(filterScenarioWnd)

	local owner = filterScenarioWnd:find("owner"):find("edit_box_group").uc_input_string	
	local level = self.LevelToURLParam[filterScenarioWnd:find("level"):find("combo_box").selection]	   
	local orientation = self.OrientationToURLParam[filterScenarioWnd:find("genre"):find("combo_box").selection]
	local name = filterScenarioWnd:find("name"):find("edit_box_group").uc_input_string	
	local charSlot = getCharSlot()

	local result = string.format("&owner=%s&level=%s&orientation=%s&name=%s&charSlot=%u", encodeURLUnicodeParam(owner), level, orientation, encodeURLUnicodeParam(name), charSlot)
    debugInfo(result)
    return result
end

-----------------------------------------------------------------------------------------
-- Validate current filter and do the browse
function game.RingAccessPointFilter:validate()   
   debugInfo('*')
   local filterScenarioWnd = self:getWnd()	
	filterScenarioWnd.active = false
	if game.NpcWebPage.UrlTextId == 0 then
		assert(0)
	end
   local ucUrl
   if config.Local == 1 then
			ucUrl = ucstring(NicoMagicURL) -- for test in local mode				
   else
         ucUrl = getDynString(game.NpcWebPage.UrlTextId);
   end	
   debugInfo(tostring(ucUrl))
	local utf8Url = ucUrl:toUtf8()	
	local browser = getUI("ui:interface:npc_web_browser"):find("html")
	-- when in ring mode, add the parameters ourselves. 60 second is the timout for zope....
	browseNpcWebPage(getUIId(browser), utf8Url .. self:getURLParameters(), false, 60)
end


