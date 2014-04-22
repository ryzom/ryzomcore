-- In this file we define functions that serves outgame character creation


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (outgame==nil) then
	outgame= {};
end


------------------------------------------------------------------------------------------------------------
-- called to construct icons
function outgame:activePackElement(id, icon)
	local uiDesc = getUI("ui:outgame:appear:job_options:options:desc");
	uiDesc['ico' .. tostring(id)].active= true;
	uiDesc['ico' .. tostring(id)].texture= icon;
	uiDesc['ico' .. tostring(id) .. 'txt'].active= true;
end


------------------------------------------------------------------------------------------------------------
-- called to construct pack text
function outgame:setPackJobText(id, spec)
	-- Set Pack content
	local uiPackText = getUI("ui:outgame:appear:job_options:options:desc:pack_" .. id);
	uiPackText.hardtext= "uiCP_Job_" .. id .. tostring(spec);

	-- Set specialization text
	local uiResText = getUI("ui:outgame:appear:job_options:options:result:res");
	if(spec==2) then
		uiResText.hardtext= "uiCP_Res_" .. id;
	end
end

------------------------------------------------------------------------------------------------------------
-- called to construct pack
function outgame:buildActionPack()

	local uiDesc = getUI("ui:outgame:appear:job_options:options:desc");
	if (uiDesc==nil) then
		return;
	end

	-- Reset All
	for i = 1,20 do
		uiDesc['ico' .. tostring(i)].active= false;
		uiDesc['ico' .. tostring(i) .. 'txt'].active= false;
	end

	-- Build Default Combat
	self:activePackElement(1, 'f1.tga');		-- Dagger
	self:activePackElement(2, 'f2.tga');		-- Accurate Attack
	
	-- Build Default Magic
	self:activePackElement(6, 'm2.tga');		-- Gloves
	self:activePackElement(7, 'm1.tga');		-- Acid
	
	-- Build Default Forage
	self:activePackElement(11, 'g1.tga');	-- Forage Tool
	self:activePackElement(12, 'g2.tga');	-- Basic Extract
	
	-- Build Default Craft
	self:activePackElement(16, 'c2.tga');	-- Craft Tool
	self:activePackElement(17, 'c1.tga');	-- 50 raw mat
	self:activePackElement(18, 'c3.tga');	-- Craft Root
	self:activePackElement(19, 'c4.tga');	-- Boots Plan

	-- Build Option
	if (getDbProp('UI:TEMP:JOB_FIGHT') == 2) then
		self:activePackElement(3, 'f3.tga');		-- Increase damage
	elseif (getDbProp('UI:TEMP:JOB_MAGIC') == 2) then
		self:activePackElement(8, 'm5.tga');		-- Fear
	elseif (getDbProp('UI:TEMP:JOB_FORAGE') == 2) then
		self:activePackElement(13, 'g3.tga');	-- Basic Prospection
	elseif (getDbProp('UI:TEMP:JOB_CRAFT') == 2) then
		self:activePackElement(20, 'c6.tga');	-- Gloves Plan
		self:activePackElement(17, 'c5.tga');	-- Replace 17, with 100x RawMat
	end


	-- Reset Text
	self:setPackJobText('F', 1);
	self:setPackJobText('M', 1);
	self:setPackJobText('G', 1);
	self:setPackJobText('C', 1);
	
	-- Set correct text for specalized version		
	if (getDbProp('UI:TEMP:JOB_FIGHT') == 2) then
		self:setPackJobText('F', 2);
	elseif (getDbProp('UI:TEMP:JOB_MAGIC') == 2) then
		self:setPackJobText('M', 2);
	elseif (getDbProp('UI:TEMP:JOB_FORAGE') == 2) then
		self:setPackJobText('G', 2);
	elseif (getDbProp('UI:TEMP:JOB_CRAFT') == 2) then
		self:setPackJobText('C', 2);
	end
	
end


------------------------------------------------------------------------------------------------------------
-------------------
-- BG DOWNLOADER --
-------------------

--function outgame:getProgressGroup()
--	--debugInfo("*** 1 ***")
--	local grp = getUI("ui:outgame:charsel:bgd_progress")
--	--debugInfo(tostring(grp))
--	return grp
--end
--
--function outgame:setProgressText(ucstr, color, progress, ellipsis)
--	--debugInfo("*** 2 ***")
--	local text = self:getProgressGroup():find("text")
--	local ellipsisTxt = self:getProgressGroup():find("ellipsis")
--	text.color = color
--	text.uc_hardtext = ucstr
--	if ellipsis then
--		ellipsisTxt.hardtext = ellipsis
--	else
--		ellipsisTxt.hardtext = ""
--	end
--	ellipsisTxt.color = color
--	local progressCtrl = self:getProgressGroup():find("progress")
--	progressCtrl.range = 100
--	progressCtrl.value = progress * 100
--	progressCtrl.active = true
--end
--
--
--local progress progressSymbol = { ".", "..", "..." }
--
---- set patching progression (from 0 to 1)
--function outgame:setPatchProgress(progress)
--	--debugInfo("*** 3 ***")
--	local progressPercentText = string.format("%d%%", 100 * progress)
--	local progressPostfix = math.fmod(os.time(), 3)	
--	--debugInfo("Patch in progress : " .. tostring(progress))		
--	local progressDate = nltime.getLocalTime() / 500
--	local colValue = math.floor(230 + 24 * math.sin(progressDate))
--	local color = string.format("%d %d 	%d %d", colValue, colValue, colValue, 255)
--	self:setProgressText(concatUCString(i18n.get("uiBGD_Progress"), ucstring(progressPercentText)), color, progress, progressSymbol[progressPostfix + 1])
--end
--
--function outgame:setPatchSuccess()
--	--debugInfo("*** 4 ***")
--	--debugInfo("Patch up to date")
--	self:setProgressText(i18n.get("uiBGD_PatchUpToDate"), "0 255 0 255", 1)
--end
--
--
--function outgame:setPatchError()
--	--debugInfo("*** 5 ***")
--	--debugInfo("Patch error")	
--	self:setProgressText(i18n.get("uiBGD_PatchError"), "255 0 0 255", 0)
--end
--
--function outgame:setNoPatch()
--	--self:getProgressGroup().active = false
--end


------------------------------------------------------------------------------------------------------------
----------------
--LAUNCH GAME --
----------------
function outgame:launchGame()
	if not isPlayerSlotNewbieLand(getPlayerSelectedSlot()) then
		if not isFullyPatched() then
			messageBoxWithHelp(i18n.get("uiBGD_MainlandCharFullPatchNeeded"), "ui:outgame")		
			return
		end
	end		
	runAH(getUICaller(), "proc", "proc_charsel_play")
end
