------------------------------------------------------------------------------------------------------------
-------------------
-- BG DOWNLOADER --
-------------------


bgdownloader = 
{
	ChangePriorityLock = false	
}

------------------------------------------------------------------------------------------------------------
function bgdownloader:getProgressGroup()	
	if isInGame() then
		local grp = getUI("ui:interface:bg_downloader")	
		return grp		
	else
		local grp = getUI("ui:outgame:charsel:bgd_progress")	
		return grp
	end
end

------------------------------------------------------------------------------------------------------------
function bgdownloader:getPrioCB()
	return self:getProgressGroup():find("prio")
end

------------------------------------------------------------------------------------------------------------
function bgdownloader:getProgressBar()
	return self:getProgressGroup():find("progress")
end

------------------------------------------------------------------------------------------------------------
function bgdownloader:setIcon(icon)	
	local bm = self:getProgressGroup():find("bm");
	if icon == "" then
		bm.active = false
	else
		bm.active = true
		bm.texture = icon
	end
end

local pausedText = i18n.get("uiBGD_Paused")

------------------------------------------------------------------------------------------------------------
function bgdownloader:setProgressText(ucstr, color, progress, ellipsis)	
	local text = self:getProgressGroup():find("text")
	local ellipsisTxt = self:getProgressGroup():find("ellipsis")
	text.color = color
	text.uc_hardtext = ucstr
	if ellipsis then
		ellipsisTxt.hardtext = ellipsis
	else
		ellipsisTxt.hardtext = ""
	end
	ellipsisTxt.color = color			
	if getBGDownloaderPriority() == 0 then -- paused ?				
		-- don't update progress when in pause (it may change because the pause actually wait 
		-- any current download to finish, otherwise connection may be lost)
		self:setIcon("bgd_pause.tga")
		ellipsisTxt.hardtext = ""
		text.uc_hardtext = pausedText
	else				
		self:setIcon("")
		local progressCtrl = self:getProgressBar()
		progressCtrl.range = 100
		progressCtrl.value = progress * 100
		progressCtrl.active = true
	end	
	self:displayPriority()
end


local progress progressSymbol = { ".", "..", "..." }


------------------------------------------------------------------------------------------------------------
-- set patching progression (from 0 to 1)
function bgdownloader:setPatchProgress(progress)	
	if not isInGame() then	
		self:getProgressGroup().active = true		
	end			

	self:getPrioCB().active = true
	local progressPercentText = string.format("%d%%", 100 * progress)
	local progressPostfix = math.fmod(os.time(), 3)	
	local progressDate = nltime.getLocalTime() / 500
	local colValue = math.floor(230 + 24 * math.sin(progressDate))
	local color = string.format("%d %d 	%d %d", colValue, colValue, colValue, 255)
	self:setProgressText(concatUCString(i18n.get("uiBGD_Progress"), ucstring(progressPercentText)), color, progress, progressSymbol[progressPostfix + 1])
end

------------------------------------------------------------------------------------------------------------
function bgdownloader:setPatchSuccess()
	if not isInGame() then	
		self:getProgressGroup().active = true		
	end
	self:setProgressText(i18n.get("uiBGD_PatchUpToDate"), "0 255 0 255", 1)	
	self:setIcon("W_answer_16_valid.tga")
	--if isInGame() then
		self:getPrioCB().active = false
	--end
end


------------------------------------------------------------------------------------------------------------
function bgdownloader:setPatchError()
	if not isInGame() then	
		self:getProgressGroup().active = true		
	end
	local errMsg = getPatchLastErrorMessage()
	if errMsg == ucstring() then		
		self:setProgressText(i18n.get("uiBGD_PatchError"), "255 0 0 255", 0)
	else		
		self:setProgressText(errMsg, "255 0 0 255", 0)
	end
	self:setIcon("W_answer_16_cancel.tga")
	--if isInGame() then
		self:getPrioCB().active = false
	--end
end

------------------------------------------------------------------------------------------------------------
function bgdownloader:setNoDownloader()
	if isInGame() then
		self:setProgressText(i18n.get("uiBGD_NotUsed"), "255 255 255 255", 0)
	else
		self:getProgressGroup().active = false
	end
	self:setIcon("")
	--if isInGame() then
		self:getPrioCB().active = false
	--end
	self:getProgressBar().active = false
end

------------------------------------------------------------------------------------------------------------
function bgdownloader:setNoNecessaryPatch()
	if isInGame() then
		self:setProgressText(i18n.get("uiBGD_PatchUpToDate"), "255 255 255 255", 0)

	else
		self:getProgressGroup().active = false
	end
	self:setIcon("")
	--if isInGame() then
		self:getPrioCB().active = false
	--end
	self:getProgressBar().active = false
end

------------------------------------------------------------------------------------------------------------
-- the priority was changed by the user
function bgdownloader:onChangePriority()
	if not self.ChangePriorityLock then		
		requestBGDownloaderPriority(self:getPrioCB().selection)
		self:displayPriority()		
	end
end

------------------------------------------------------------------------------------------------------------
-- the priority has been changed externally, display it
function bgdownloader:displayPriority()
	--if not isInGame() then return end
	self.ChangePriorityLock = true	
	self:getPrioCB().selection = getBGDownloaderPriority()
	self.ChangePriorityLock = false	
	self:getPrioCB().active = (getBGDownloaderPriority() ~= -1)
end

------------------------------------------------------------------------------------------------------------
-- display a warning telling that the user need a fully patched client to go further
function bgdownloader:inGamePatchUncompleteWarning()
	local pg = self:getProgressGroup()
	pg.active = true
	setTopWindow(pg)
	pg:blink(2)
	messageBoxWithHelp(i18n.get("uiBGD_InGamePatchIncomplete"), "ui:interface", tonumber(getDefine("case_normal")))
	displaySystemInfo(i18n.get("uiBGD_InGamePatchIncompleteBC"), "BC")
end