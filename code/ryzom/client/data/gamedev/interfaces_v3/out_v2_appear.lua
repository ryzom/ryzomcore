-- In this file we define functions that serves outgame character creation


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (outgame==nil) then
	outgame= {};
end



------------------------------------------------------------------------------------------------------------
-- Name generator.

--nb noms:
-- matis: male 621 - female 621 - surname 621
-- fyros: given name 14269, surname 841
-- zorai: given name one 318, given name two 644, surname 1287
-- tryker: given name 4500, surname 4335

function outgame:getFyrosName()
    local nameResult = "";
    local fullnameResult = "";

    local nbFyrosGivenNames = 0;
    for _ in pairs(fyrosGivenNames) do nbFyrosGivenNames = nbFyrosGivenNames + 1 end
    local givenName = fyrosGivenNames[math.random(nbFyrosGivenNames)];

    local nbFyrosSurnames = 0;
    for _ in pairs(fyrosSurnames) do nbFyrosSurnames = nbFyrosSurnames + 1 end
    local surname = fyrosSurnames[math.random(nbFyrosSurnames)];
    fullnameResult = givenName .. " " .. surname;
    nameResult = surname;
    return fullnameResult, nameResult
end

function outgame:getMatisName(sex)
    local nameResult = "";
    local fullnameResult = "";
    local dbNameSex = getDbProp("UI:TEMP:NAME_SEX");

    if sex ~= nil then
        dbNameSex = sex
    end

    if tonumber( dbNameSex )== 1 then
        local nbMatisMaleNames = 0;
        for _ in pairs(matisMaleNames) do nbMatisMaleNames = nbMatisMaleNames + 1 end
        givenName = matisMaleNames[math.random(nbMatisMaleNames)];
    else
        local nbMatisFemaleNames = 0;
        for _ in pairs(matisFemaleNames) do nbMatisFemaleNames = nbMatisFemaleNames + 1 end
        givenName = matisFemaleNames[math.random(nbMatisFemaleNames)];
    end

    local nbMatisSurnames = 0;
    for _ in pairs(matisSurnames) do nbMatisSurnames = nbMatisSurnames + 1 end
    local surname = matisSurnames[math.random(nbMatisSurnames)];
    fullnameResult = givenName .. " " .. surname;
    nameResult = givenName;

    return fullnameResult, nameResult
end

function outgame:getTrykerName()
    local nameResult = "";
    local fullnameResult = "";

    local nbTrykerGivenNames = 0;
    for _ in pairs(trykerGivenNames) do nbTrykerGivenNames = nbTrykerGivenNames + 1 end
    local givenName = trykerGivenNames[math.random(nbTrykerGivenNames)];

    local nbTrykerSurnames = 0;
    for _ in pairs(trykerSurnames) do nbTrykerSurnames = nbTrykerSurnames + 1 end
    local surname = trykerSurnames[math.random(nbTrykerSurnames)];

    fullnameResult = surname .. " " .. givenName;
    nameResult = givenName;

    return fullnameResult, nameResult
end


function outgame:getZoraiName()
    local nameResult = "";
    local fullnameResult = "";

    local nbGivenNameOne = 0;
    for _ in pairs(zoraiGivenNameOne) do nbGivenNameOne = nbGivenNameOne + 1 end
    local givenNameOne = zoraiGivenNameOne[math.random(nbGivenNameOne)];

    local nbGivenNameTwo = 0;
    for _ in pairs(zoraiGivenNameTwo) do nbGivenNameTwo = nbGivenNameTwo + 1 end
    local givenNameTwo = zoraiGivenNameTwo[math.random(nbGivenNameTwo)];

    local nbSurnames = 0;
    for _ in pairs(zoraiSurnames) do nbSurnames = nbSurnames + 1 end
    local surname = zoraiSurnames[math.random(nbSurnames)];

    fullnameResult = surname .. " " .. givenNameOne .. "-" .. givenNameTwo;
    nameResult = givenNameOne .. givenNameTwo;

    return fullnameResult, nameResult
end

function outgame:procGenerateName()
    local uiNameFull = getUI("ui:outgame:appear_name:name_full");
	local uiGenText = getUI("ui:outgame:appear_name:eb");
    local dbNameRace = getDbProp("UI:TEMP:NAME_RACE");
    local dbNameSubRace = getDbProp("UI:TEMP:NAME_SUB_RACE");
    local dbNameSubRace2 = getDbProp("UI:TEMP:NAME_SUB_RACE2");

    local nameResult = "";
    local fullnameResult = "";

    -- Look at outgame:procUpdateNameRaceLabel() for the "race" list.
    -- fy ma try zo -->
    local givenName = "";
    if  tonumber( dbNameRace ) == 1 then
    -- Fyros
        fullnameResult, nameResult = self:getFyrosName()
    elseif  tonumber( dbNameRace ) == 2 then
    -- Matis
        fullnameResult, nameResult = self:getMatisName()
    elseif  tonumber( dbNameRace ) == 3 then
    -- Tryker
        fullnameResult, nameResult = self:getTrykerName()
    elseif  tonumber( dbNameRace ) == 4  then
    -- Zorai
        fullnameResult, nameResult = self:getZoraiName()
    elseif  tonumber( dbNameRace ) == 5  then
    -- Maraudeurs
        tempResult_1 = "";
        tempResult_2 = "";
        if tonumber(dbNameSubRace) == 1 then
        -- Fyros
        fullnameResult, tempResult_1 = self:getFyrosName()
        elseif  tonumber( dbNameSubRace ) == 2 then
        -- Matis F
            fullnameResult, tempResult_1 = self:getMatisName(2)
        elseif  tonumber( dbNameSubRace ) == 3 then
        -- Matis M
            fullnameResult, tempResult_1 = self:getMatisName(1)
        elseif  tonumber( dbNameSubRace ) == 4 then
        -- Tryker
            fullnameResult, tempResult_1 = self:getTrykerName()
        elseif  tonumber( dbNameSubRace ) == 5  then
        -- Zorai
            fullnameResult, tempResult_1 = self:getZoraiName()
        end

        if tonumber(dbNameSubRace2) == 1 then
        -- Fyros
        fullnameResult, tempResult_2 = self:getFyrosName()
        elseif  tonumber( dbNameSubRace2 ) == 2 then
        -- Matis F
            fullnameResult, tempResult_2 = self:getMatisName(2)
        elseif  tonumber( dbNameSubRace2 ) == 3 then
        -- Matis M
            fullnameResult, tempResult_2 = self:getMatisName(1)
        elseif  tonumber( dbNameSubRace2 ) == 4 then
        -- Tryker
            fullnameResult, tempResult_2 = self:getTrykerName()
        elseif  tonumber( dbNameSubRace2 ) == 5  then
        -- Zorai
            fullnameResult, tempResult_2 = self:getZoraiName()
        end

        fullnameResult = tempResult_1 .. " " .. tempResult_2
        nameResult = tempResult_2
    end

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
    tempstr = string.lower( tempstr );
    tempstr = (tempstr:gsub("^%l", string.upper));

	uiNameSexText.hardtext= tempstr;
end
-- Name race slider update.
function outgame:procUpdateNameRaceLabel()
    local nameRaceType = { "Fyros", "Matis", "Tryker", "Zoraï", "uiCP_Maraudeur" }

	local uiNameRaceText = getUI("ui:outgame:appear_name:name_race_slider:name_race");
    local dbNameRace = getDbProp("UI:TEMP:NAME_RACE");

	local uiNameSexSlider = getUI("ui:outgame:appear_name:name_sex_slider");

	local uiNameSubRaceSlider = getUI("ui:outgame:appear_name:name_sub_race_slider");
	local uiNameSubRace2Slider = getUI("ui:outgame:appear_name:name_sub_race2_slider");

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
        uiNameSubRaceSlider.active = true;
        uiNameSubRace2Slider.active = true;
        uiNameGenerate.y = "-105"
    else
        uiNameSubRaceSlider.active = false;
        uiNameSubRace2Slider.active = false;
    end


	uiNameRaceText.hardtext= tostring(nameRaceType[tonumber(dbNameRace)]);
end


local matisF = "Matis " .. (string.lower(tostring(i18n.get("uiCP_Sex_Female")) )):gsub("^%l", string.upper);
local matisM = "Matis " .. (string.lower(tostring(i18n.get("uiCP_Sex_Male")) )):gsub("^%l", string.upper);

function outgame:procUpdateNameSubRaceLabel()
    local nameSubRaceType = { "Fyros", matisF, matisM, "Tryker", "Zoraï" }
	local uiNameSubRaceText = getUI("ui:outgame:appear_name:name_sub_race_slider:name_race");
    local dbNameSubRace = getDbProp("UI:TEMP:NAME_SUB_RACE");


	uiNameSubRaceText.hardtext= tostring(nameSubRaceType[tonumber(dbNameSubRace)]);
end
function outgame:procUpdateNameSubRace2Label()
    local nameSubRace2Type = { "Fyros", matisF, matisM, "Tryker", "Zoraï" }
	local uiNameSubRace2Text = getUI("ui:outgame:appear_name:name_sub_race2_slider:name_race");
    local dbNameSubRace2 = getDbProp("UI:TEMP:NAME_SUB_RACE2");


	uiNameSubRace2Text.hardtext= tostring(nameSubRace2Type[tonumber(dbNameSubRace2)]);
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
