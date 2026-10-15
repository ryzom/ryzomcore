-- In this file we define functions that serves for guild windows (info, inv, forum ...)

-- WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
-- WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

-- This file is not used !!! It can be plugged easily but to avoid test we do not plug it !
-- When plugged : do not forget to set dynamic_display_size="true" to guild_members container

-- WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
-- WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

------------------------------------------------------------------------------------------------------------
--
function game:guildIsPresent()

	local name = getDbProp('SERVER:GUILD:NAME');
	if (name == 0) then
		return false;
	else
		return true;
	end
end

------------------------------------------------------------------------------------------------------------
-- called when something change (new player etc...)
function game:guildBuildInterface()


	-- sort the members in Guild Manager
	sortGuildMembers();

	-- update interface with data of Guild Manager

	-- freeze / unfreeze quit button
	local uiQuitButton = getUI('ui:interface:guild:content:quit_guild');
	uiQuitButton.frozen = isGuildQuitAvailable();

	-- member count
	local nbMember = getNbGuildMembers();
	local uiTextCnt = getUI('ui:interface:guild:content:member_count');
	uiTextCnt.hardtext = nbMember;

	-- fill with guild icon : automatically done at draw time of the ctrl sheet

	-- fill with guild members
	local sMemberList = 'ui:interface:guild_members:content';
	local uiMemberList = getUI(sMemberList);
	uiMemberList:clear();
	for i = 0,(nbMember-1) do

		local sTemplateId = 'm' .. i;
		local uiMember = createGroupInstance('member_template', sMemberList, { id = sTemplateId });
		if (uiMember ~= nil) then
			uiMember.name.hardtext = getGuildMemberName(i);
			local memberGrade = getGuildMemberGrade(i);
			if (memberGrade == 'Leader') then
				uiMember.grade.uc_hardtext = i18n.get('uiGuildLeader');
			elseif (memberGrade == 'HighOfficer') then
				uiMember.grade.uc_hardtext = i18n.get('uiGuildHighOfficer');
			elseif (memberGrade == 'Officer') then
				uiMember.grade.uc_hardtext = i18n.get('uiGuildOfficer');
			else
				uiMember.grade.uc_hardtext = i18n.get('uiGuildMember');
			end
			uiMemberList:addChild(uiMember);
		end
	end
end


------------------------------------------------------------------------------------------------------------
-- called when we open the guild main container
function game:guildActive()

	setDbProp('UI:VARIABLES:ISACTIVE:GUILD', 1);

	game:guildBuildInterface();

	local ui = getUI('ui:interface:guild');
	if (not game:guildIsPresent()) then
		ui.active = false;
	end
	ui.w = 328;
end

------------------------------------------------------------------------------------------------------------
-- 
function game:guildDeactive()

	setDbProp('UI:VARIABLES:ISACTIVE:GUILD', 0);
end
