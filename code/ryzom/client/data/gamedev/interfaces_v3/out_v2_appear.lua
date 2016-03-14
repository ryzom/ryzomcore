-- In this file we define functions that serves outgame character creation


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (outgame==nil) then
	outgame= {};
end



------------------------------------------------------------------------------------------------------------
-- Name generator.

--nb noms:
-- matis: male 621 - female 621 - FirstName 621
-- fyros: given name 14269, FirstName 841
-- zorai: given name one 318, given name two 644, FirstName 1287
-- tryker: given name 4500, FirstName 4335

-- Fyros
function outgame:getFyrosFirstName()
	local nbFyrosFirstNames = 0;
	for _ in pairs(fyrosFirstNames) do nbFyrosFirstNames = nbFyrosFirstNames + 1 end

	return fyrosFirstNames[math.random(nbFyrosFirstNames)]
end

function outgame:getFyrosLastName()
	local nbFyrosLastNames = 0;
	for _ in pairs(fyrosLastNames) do nbFyrosLastNames = nbFyrosLastNames + 1 end

	return fyrosLastNames[math.random(nbFyrosLastNames)]
end

-- Matis
function outgame:getMatisFirstName(sex)
	-- 1 = male, 2 = female
	local dbNameSex = getDbProp("UI:TEMP:NAME_SEX");

	if sex ~= nil then
		dbNameSex = sex;
	end

	local FirstName = ""
	if tonumber(dbNameSex) == 1 then
		local nbMatisMaleFirstNames = 0;
		for _ in pairs(matisMaleFirstNames) do nbMatisMaleFirstNames = nbMatisMaleFirstNames + 1 end
		FirstName = matisMaleFirstNames[math.random(nbMatisMaleFirstNames)];
	else
		local nbMatisFemaleFirstNames = 0;
		for _ in pairs(matisFemaleFirstNames) do nbMatisFemaleFirstNames = nbMatisFemaleFirstNames + 1 end
		FirstName = matisFemaleFirstNames[math.random(nbMatisFemaleFirstNames)];
	end

	return FirstName;
end

function outgame:getMatisLastName()

	local nbMatisLastNames = 0;
	for _ in pairs(matisLastNames) do nbMatisLastNames = nbMatisLastNames + 1 end

	return matisLastNames[math.random(nbMatisLastNames)]
end

-- Tryker
function outgame:getTrykerFirstName()
	local nbTrykerFirstNames = 0;
	for _ in pairs(trykerFirstNames) do nbTrykerFirstNames = nbTrykerFirstNames + 1 end

	return trykerFirstNames[math.random(nbTrykerFirstNames)]
end

function outgame:getTrykerLastName()
	local nbTrykerLastNames = 0;
	for _ in pairs(trykerLastNames) do nbTrykerLastNames = nbTrykerLastNames + 1 end

	return trykerLastNames[math.random(nbTrykerLastNames)]
end

-- Zoraï
function outgame:getZoraiFirstName()
	local nbFirstNamesOne = 0;
	for _ in pairs(zoraiFirstNamesOne) do nbFirstNamesOne = nbFirstNamesOne + 1 end
	local FirstNameOne = zoraiFirstNamesOne[math.random(nbFirstNamesOne)];

	local nbFirstNamesTwo = 0;
	for _ in pairs(zoraiFirstNamesTwo) do nbFirstNamesTwo = nbFirstNamesTwo + 1 end
	local FirstNameTwo = zoraiFirstNamesTwo[math.random(nbFirstNamesTwo)];

	return FirstNameOne .. "-" .. FirstNameTwo
end
function outgame:getZoraiLastName()
	local nbLastNames = 0;
	for _ in pairs(zoraiLastNames) do nbLastNames = nbLastNames + 1 end

	return zoraiLastNames[math.random(nbLastNames)]
end

function outgame:procGenerateName()
	local uiNameFull = getUI("ui:outgame:appear_name:name_full");
	local uiGenText = getUI("ui:outgame:appear_name:eb");
	local dbNameRace = getDbProp("UI:TEMP:NAME_RACE");
	local dbNameSubRaceFirstName = getDbProp("UI:TEMP:NAME_SUB_RACE_FIRST_NAME");
	local dbNameSubRaceLastName = getDbProp("UI:TEMP:NAME_SUB_RACE_LAST_NAME");

	local nameResult = "";
	local fullnameResult = "";

	-- Look at outgame:procUpdateNameRaceLabel() for the "race" list.
	-- fy ma try zo -->
	local firstName = "test2"
	local lastName = "test"
	if  tonumber(dbNameRace) == 1 then
	-- Fyros
		firstName = self:getFyrosFirstName()
		lastName = self:getFyrosLastName()
	elseif  tonumber(dbNameRace) == 2 then
	-- Matis
		firstName = self:getMatisFirstName()
		lastName = self:getMatisLastName()
	elseif  tonumber(dbNameRace) == 3 then
	-- Tryker
		firstName = self:getTrykerFirstName()
		lastName = self:getTrykerLastName()
	elseif  tonumber(dbNameRace) == 4 then
	-- Zorai
		firstName = self:getZoraiFirstName()
		lastName = self:getZoraiLastName()
	elseif  tonumber(dbNameRace) == 5 then
	-- Maraudeurs

		-- firstName
		if tonumber(dbNameSubRaceFirstName) == 1 then
		-- Fyros
			firstName = self:getFyrosFirstName()
		elseif  tonumber(dbNameSubRaceFirstName) == 2 then
		-- Matis M
			firstName = self:getMatisFirstName(1)
		elseif  tonumber(dbNameSubRaceFirstName) == 3 then
		-- Matis F
			firstName = self:getMatisFirstName(2)
		elseif  tonumber(dbNameSubRaceFirstName) == 4 then
		-- Tryker
			firstName = self:getTrykerFirstName()
		elseif  tonumber(dbNameSubRaceFirstName) == 5 then
		-- Zorai
			firstName = self:getZoraiFirstName()
		end

		-- lastName
		if tonumber(dbNameSubRaceLastName) == 1 then
		-- Fyros
			lastName = self:getFyrosLastName()
		elseif  tonumber(dbNameSubRaceLastName) == 2 then
		-- Matis
			lastName = self:getMatisLastName()
		elseif  tonumber(dbNameSubRaceLastName) == 3 then
		-- Tryker
			lastName = self:getTrykerLastName()
		elseif  tonumber(dbNameSubRaceLastName) == 4  then
		-- Zorai
			lastName = self:getZoraiLastName()
		end
	end

	fullnameResult = firstName .. " " .. lastName
	nameResult = firstName

	uiNameFull.hardtext = fullnameResult;

	nameResult = string.gsub(nameResult, "'", "");
	nameResult = string.gsub(nameResult, " ", "");
	nameResult = string.gsub(nameResult, "-", "");
	nameResult = string.lower( nameResult );
	nameResult = nameResult:gsub("^%l", string.upper);
	uiGenText.input_string = nameResult;
end

-- Name sex slider update.
function outgame:procUpdateNameSexLabel()
	local nameSexType = { "uiCP_Sex_Male", "uiCP_Sex_Female" }
	local uiNameSexText = getUI("ui:outgame:appear_name:name_sex_slider:name_sex");
	local uiNameSex = getDbProp("UI:TEMP:NAME_SEX");

	tempstr = tostring(i18n.get(nameSexType[tonumber(uiNameSex)]));
	tempstr = string.lower(tempstr);
	tempstr = (tempstr:gsub("^%l", string.upper));

	uiNameSexText.hardtext = tempstr;
end

-- Name race slider update.
function outgame:procUpdateNameRaceLabel()
	local nameRaceType = { "Fyros", "Matis", "Tryker", "Zoraï", "uiCP_Maraudeur" }

	local uiNameRaceText = getUI("ui:outgame:appear_name:name_race_slider:name_race");
	local dbNameRace = getDbProp("UI:TEMP:NAME_RACE");

	local uiNameSexSlider = getUI("ui:outgame:appear_name:name_sex_slider");

	local uiNameSubRaceFirstNameSlider = getUI("ui:outgame:appear_name:name_sub_race_first_name_slider");
	local uiNameSubRaceLastNameSlider = getUI("ui:outgame:appear_name:name_sub_race_last_name_slider");

	local uiNameGenerate = getUI("ui:outgame:appear_name:generate");
	-- Show/Hide sex slider

	uiNameGenerate.y = "-50"
	if tonumber(dbNameRace) == 2 then
		uiNameSexSlider.active = true;
		uiNameGenerate.y = "-65"
	else
		uiNameSexSlider.active = false;
	end

	-- Show/Hide sub race slider
	if tonumber(dbNameRace) == 5 then
		uiNameSubRaceFirstNameSlider.active = true;
		uiNameSubRaceLastNameSlider.active = true;
		uiNameGenerate.y = "-105"
	else
		uiNameSubRaceFirstNameSlider.active = false;
		uiNameSubRaceLastNameSlider.active = false;
	end

	uiNameRaceText.hardtext = tostring(nameRaceType[tonumber(dbNameRace)]);
end


local matisF = "Matis " .. (string.lower(tostring(i18n.get("uiCP_Sex_Female")) )):gsub("^%l", string.upper);
local matisM = "Matis " .. (string.lower(tostring(i18n.get("uiCP_Sex_Male")) )):gsub("^%l", string.upper);

function outgame:procUpdateNameSubRaceFirstNameLabel()
	local nameSubRaceFirstNameType = { "Fyros", matisM, matisF, "Tryker", "Zoraï" }
	local uiNameSubRaceFirstNameText = getUI("ui:outgame:appear_name:name_sub_race_first_name_slider:name_race");
	local dbNameSubRaceFirstName = getDbProp("UI:TEMP:NAME_SUB_RACE_FIRST_NAME");

	uiNameSubRaceFirstNameText.hardtext= tostring(nameSubRaceFirstNameType[tonumber(dbNameSubRaceFirstName)]);
end

function outgame:procUpdateNameSubRaceLastNameLabel()
	local nameSubRaceLastNameType = { "Fyros", "Matis", "Tryker", "Zoraï" }
	local uiNameSubRaceLastNameText = getUI("ui:outgame:appear_name:name_sub_race_last_name_slider:name_race");
	local dbNameSubRaceLastName = getDbProp("UI:TEMP:NAME_SUB_RACE_LAST_NAME");

	uiNameSubRaceLastNameText.hardtext= tostring(nameSubRaceLastNameType[tonumber(dbNameSubRaceLastName)]);
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
