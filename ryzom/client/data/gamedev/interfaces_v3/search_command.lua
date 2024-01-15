if not SearchCommand then
    --global SearchCommand class
	SearchCommand = {
	    identifier_found = {},
	    command_self = "",
	    command_parameter_list = {},
	    valid_commands_list = {},
	    commands_list = {},
	    key_tab_down = 0,
	    modal_open_list = {},
	    process_list = {},
	    player_list = {}
	}
end

--setup data
--commands_list[x] = {"type(client/shard)","priv(player/privs)","uitranslation for description","command", "parameter1(playername/text/number)", "parameter2(playername/text/number)" ..}
SearchCommand.commands_list = {}

local player_priv = isPlayerPrivilege()
if(player_priv)then
    table.insert(SearchCommand.commands_list,{"client", "player", "help_desc", "?", {{"Text:<Command>",""}, {"all","help_all_desc"}}, {{"shard",""},{"client",""},{"eScript",""}}})
else
    table.insert(SearchCommand.commands_list,{"client", "player", "help_desc", "?", {{"Text:<Command>",""}, {"all","help_all_desc"}}})
end

--client commands
table.insert(SearchCommand.commands_list,{"client", "player", "time_desc", "time"})
table.insert(SearchCommand.commands_list,{"client", "player", "version_desc", "version"})
table.insert(SearchCommand.commands_list,{"client", "player", "where_desc", "where"})
table.insert(SearchCommand.commands_list,{"client", "player", "playedTime_desc", "playedTime"})
table.insert(SearchCommand.commands_list,{"client", "player", "who_desc", "who", {{"gm","who_gm_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guildinvite_desc", "guildinvite",{{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guildmotd_desc", "guildmotd",{{"Text:<Message>",""},{"?","guildmotd_arg1_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "league_desc", "league",{{"Text:<LeagueName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "leagueinvite_desc", "leagueinvite",{{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "leaguequit_desc", "leaguequit"})
table.insert(SearchCommand.commands_list,{"client", "player", "leaguekick_desc", "leaguekick",{{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "afk_desc", "afk",{{"Text:<Autoresponse>","afk_autoresponse_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "assist_desc", "assist"})
table.insert(SearchCommand.commands_list,{"client", "player", "assist_desc", "as"})
table.insert(SearchCommand.commands_list,{"client", "player", "self_desc", "self"})
table.insert(SearchCommand.commands_list,{"client", "player", "brutalQuit_desc", "brutalQuit"})
table.insert(SearchCommand.commands_list,{"client", "player", "chatLog_desc", "chatLog"})
table.insert(SearchCommand.commands_list,{"client", "player", "follow_desc", "follow"})
table.insert(SearchCommand.commands_list,{"client", "player", "ignore_desc", "ignore",{{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "invite_desc", "invite",{{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "mount_desc", "mount"})
table.insert(SearchCommand.commands_list,{"client", "player", "unmount_desc", "unmount"})
table.insert(SearchCommand.commands_list,{"client", "player", "random_desc", "random", {{"Number:<LowestNumber>",""}}, {{"Number:<HighestNumber>",""}}, {{"hide ","random_hide_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "skiptutorial_desc", "skiptutorial"})
table.insert(SearchCommand.commands_list,{"client", "player", "sleep_desc", "sleep", {{"Number:<Secound>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "tar_desc", "tar", {{"Text:<Name>",""}}, {{"|quiet=true","tar_quiet_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "target_quiet_desc", "target_quiet"})
table.insert(SearchCommand.commands_list,{"client", "player", "target_quiet_desc", "tarq"})
table.insert(SearchCommand.commands_list,{"client", "player", "lmtar_desc", "lmtar", {{"Text:<Name>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "chat_desc", "chat"})
table.insert(SearchCommand.commands_list,{"client", "player", "go_desc", "go"})
table.insert(SearchCommand.commands_list,{"client", "player", "appzone_desc", "appzone", {{"<AppId>",""},{"hide","appzone_hide_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "setuiscale_desc", "setuiscale", {{"Number:<ScaleFactor>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "createGroup_desc", "createGroup", {{"Text:<OutfitGroupName>",""}},{{"[true]",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "moveGroup_desc", "moveGroup", {{"Text:<OutfitGroupName>",""}},{{"Text:<PetAnimal1>",""},{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "equipGroup_desc", "equipGroup", {{"Text:<OutfitGroupName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "deleteGroup_desc", "deleteGroup", {{"Text:<OutfitGroupName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "naked_desc", "naked"})
table.insert(SearchCommand.commands_list,{"client", "player", "nude_desc", "nude"})
table.insert(SearchCommand.commands_list,{"client", "player", "listGroup_desc", "listGroup"})
table.insert(SearchCommand.commands_list,{"client", "player", "say_desc", "say",{{">","s_param1_desc"}}, {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "say_desc", "s",{{">","s_param1_desc"}}, {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "shout", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "sh", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "y", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "yell", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guild_desc", "guild", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guild_desc", "g", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guild_desc", "gu", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "region_desc", "region", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "region_desc", "r", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "region_desc", "re", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "team", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "te", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "party", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "p", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "universe_desc", "universe", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "universe_desc", "u", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "0_desc", "0", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "1_desc", "1", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "2_desc", "2", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "3_desc", "3", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "4_desc", "4", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "channel_desc", "channel",{{"Text:<Channelname>",""}}, {{"Text:<Password>","channel_password_desc"},{"*","channel_leave_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "tell_desc", "tell",{{"Text:<PlayerName>",""}}, {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "tell_desc", "t",{{"Text:<PlayerName>",""}}, {{"Text:<Message>",""}}})
--END client commands


--shard commands
table.insert(SearchCommand.commands_list,{"shard", "player", "a_desc", "a", {{"Text:<Command>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:PR:OBSERVER:EM:EG:TESTER:", "b_desc", "b", {{"Text:<Command>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:PR:OBSERVER:EM:EG:TESTER:", "c_desc", "c", {{"Text:<TargetName>",""}}, {{"Text:<Command>",""}}})

table.insert(SearchCommand.commands_list,{"shard", "player", "showOnline_desc", "showOnline", {{"1","showOnline_1_desc"}, {"2","showOnline_2_desc"},{"0","showOnline_0_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "setLeague_desc", "setLeague", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "roomInvite_desc", "roomInvite", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "roomKick_desc", "roomKick", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "dodge_desc", "dodge"})
table.insert(SearchCommand.commands_list,{"shard", "player", "parry_desc", "parry"})
table.insert(SearchCommand.commands_list,{"shard", "player", "setPvPTag_desc", "setPvPTag", {{"0",""},{"1",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "clearGuildMessage_desc", "clearGuildMessage"})
table.insert(SearchCommand.commands_list,{"shard", "player", "setGuildMessage_desc", "setGuildMessage", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "setDontTranslateLangs_desc", "setDontTranslateLangs", {{"codelang|codelang","setDontTranslateLangs_codelang_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "connectLangChannel_desc", "connectLangChannel",{{"fr",""},{"en",""},{"de",""},{"es",""},{"ru",""}},{{"[<0|1>]","connectLangChannel_1_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "updateTarget_desc", "updateTarget"})
table.insert(SearchCommand.commands_list,{"shard", "player", "teamInvite_desc", "teamInvite", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "summonPet_desc", "summonPet", {{"Number:<PetNumber>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "setTeamLeader_desc", "setTeamLeader", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "resetName_desc", "resetName"})
table.insert(SearchCommand.commands_list,{"shard", "player", "lockItem_desc", "lockItem", {{"Text:<Inventory>",""}}, {{"Number:<Slot>",""}}, {{"lock=0,1",""}}})



if(player_priv)then
    table.insert(SearchCommand.commands_list,{"shard", "player", "connectUserChannel_desc", "connectUserChannel", {{"Text:<Channelname>",""}}, {{"Text:<Password>",""},{"*","channel_leave_desc"},{"***","channel_remove_admin_desc"}}})
else
    table.insert(SearchCommand.commands_list,{"shard", "player", "connectUserChannel_desc", "connectUserChannel", {{"Text:<Channelname>",""}}, {{"Text:<Password>",""},{"*","channel_leave_desc"}}})
end

table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "addGuildMember_desc", "addGuildMember", {{"Text:<GuildName>",""}},{{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "addGuildXp_desc", "addGuildXp", {{"Text:<GuildName>",""}},{{"<+-Xp>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "addPetAnimal_desc", "addPetAnimal", {{"Text:<PetTicket>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "addPosFlag_desc", "addPosFlag", {{"Text:<FlagName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "addSkillPoints_desc", "addSkillPoints", {{"<Fight=0>",""},{"<Magic=1>",""},{"<Craft=2>",""},{"<Harvest=3>",""}}, {{"Number:<SkillPoints>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "addXPToSkill_desc", "addXPToSkill", {{"<Xp>",""}}, {{"Text:<Skill>",""}}, {{"Number:<Count>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:", "broadcast_desc", "broadcast", {{"<Xp>",""}}, {{"Text:<Skill>",""}}, {{"Number:<Count>",""}}, {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "changeHairCut_desc", "changeHairCut", {{"Text:<Sheet>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "changeMode_desc", "changeMode", {{"Text:<Mode>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "changeVar_desc", "changeVar", {{"Text:<Variable>",""},{"BaseConstitution",""},{"BaseIntelligence",""},{"BaseStrength",""},{"BaseDexterity",""},{"BaseMetabolism",""},{"BaseWisdom",""},{"BaseWellBalanced",""},{"BaseWill",""}}, {{"Number:<Value>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "checkTargetSP_desc", "checkTargetSP"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "clearEventFaction_desc", "clearEventFaction", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "clearFriendsList_desc", "clearFriendsList"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "clearIgnoreList_desc", "clearIgnoreList"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "clearIsFriendOfList_desc", "clearIsFriendOfList"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "createItemInBag_desc", "createItemInBag", {{"Text:<Sheet>",""}}, {{"Text:<Quantity>",""}}, {{"Text:<Quality>",""}}, {{"[force]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:", "createItemInInv_desc", "createItemInInv", {{"Number:<InventoryID>",""}}, {{"Text:<Sheet>",""}}, {{"Text:<Quantity>",""}}, {{"Text:<Quality>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "createItemInTmpInv_desc", "createItemInTmpInv", {{"Text:<Sheet>",""}}, {{"Text:<Quantity>",""}}, {{"Text:<Quality>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "createNamedItemInBag_desc", "createNamedItemInBag", {{"Text:<ItemName>",""}}, {{"Text:<Quantity>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "createFullArmorSet_desc", "createFullArmorSet", {{"fyros",""},{"matis",""},{"zorai",""},{"tryker",""}}, {{"light",""},{"medium",""},{"heavy",""}}, {{"Text:<Quality>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "delPosFlag_desc", "delPosFlag", {{"Text:<FlagName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:EG:", "dismiss_desc", "dismiss", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "displayForageRM_desc", "displayForageRM", {{"<exactPos=1>",""}}, {{"<extendedInfo=0>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "displayInfosOnTarget_desc", "displayInfosOnTarget"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "execPhrase_desc", "execPhrase", {{"<Cyclic 0/1>",""}}, {{"[<BrickIds>...]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "executeSabrinaPhrase_desc", "executeSabrinaPhrase", {{"<Cyclic 0/1>",""}}, {{"<PhraseId>...",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "failMission_desc", "failMission", {{"Number:<MissionIdex>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "forceTargetToDie_desc", "forceTargetToDie"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "getEventFaction_desc", "getEventFaction", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "giveRespawnPoint_desc", "giveRespawnPoint", {{"Text:<RespawnPointName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:EG:", "ignoreTells_desc", "ignoreTells", {{"<0/false/1/true>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:EG:", "infos_desc", "infos"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:EG:", "killMob_desc", "killMob"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "learnAllBricks_desc", "learnAllBricks"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "learnAllForagePhrases_desc", "learnAllForagePhrases", {{"Number:<Index>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "learnAllPhrases_desc", "learnAllPhrases"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "learnBrick_desc", "learnBrick", {{"Text:<BrickSheet>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "unlearnBrick_desc", "unlearnBrick", {{"Text:<BrickSheet>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "learnPhrase_desc", "learnPhrase", {{"Text:<PhraseSheet>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "listGuildMembers_desc", "listGuildMembers", {{"Text:<GuildName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:G:EM:EG:", "listPosFlags_desc", "listPosFlags", {{"Number:<Radius>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "loadFromXML_desc", "loadFromXML", {{"Text:<Filename>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "loadFromPDR_desc", "loadFromPDR", {{"Text:<Filename>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "logXpGain_desc", "logXpGain", {{"<on/off>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:G:EM:EG:", "listPosFlags_desc", "lPosFlags", {{"Number:<Radius>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "monitorMissions_desc", "monitorMissions", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:", "motd_desc", "motd", {{"Text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:SG:", "mute_desc", "mute", {{"Text:<PlayerName>",""}}, {{"Number:<Duration>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:SG:", "muteUniverse_desc", "muteUniverse", {{"Text:<PlayerName>",""}}, {{"Number:<Duration>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "outpostBanGuild_desc", "outpostBanGuild", {{"Number:<OutpostId>",""}}, {{"Text:<GuildName>",""}}, {{"[<all|atk|def>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "outpostBanPlayer_desc", "outpostBanPlayer", {{"Number:<OutpostId>",""}}, {{"Number:<PlayerEID>",""}}, {{"[<all|atk|def>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "outpostUnbanGuild_desc", "outpostUnbanGuild", {{"Number:<OutpostId>",""}}, {{"Text:<GuildName>",""}}, {{"[<all|atk|def>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "outpostUnbanPlayer_desc", "outpostUnbanPlayer", {{"Number:<OutpostId>",""}}, {{"Number:<PlayerEID>",""}}, {{"[<all|atk|def>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "progressMission_desc", "progressMission", {{"Number:<MissionIdex>",""}}, {{"[repeat]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "renameGuild_desc", "renameGuild", {{"Text:<GuildName>",""}},{{"Text:<NewGuildName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "renamePlayer_desc", "renamePlayer", {{"Text:<PlayerName>",""}},{{"Text:<NewPlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:EG:", "renamePlayerForEvent_desc", "renamePlayerForEvent", {{"Text:<PlayerName>",""}},{{"Text:<NewPlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "resetPowerFlags_desc", "resetPowerFlags"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:SG:", "root_desc", "root", {{"Text:<PlayerName>",""}}, {{"Number:<Duration>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "saveToPDR_desc", "saveToPDR", {{"Text:<Filename>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "saveToXML_desc", "saveToXML", {{"Text:<Filename>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setEventFaction_desc", "setEventFaction", {{"Text:<PlayerName>",""}}, {{"Text:<EventFaction>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "setGMGuild_desc", "setGMGuild"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setGuildChargePoint_desc", "setGuildChargePoint", {{"Text:<GuildName>",""}},{{"Number:<Points>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setGuildDescription_desc", "setGuildDescription", {{"Text:<GuildName>",""}},{{"Text:<Description>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setGuildLeader_desc", "setGuildLeader", {{"Text:<GuildName>",""}},{{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setGuildMemberGrade_desc", "setGuildMemberGrade", {{"Text:<GuildName>",""}},{{"Text:<PlayerName>",""}},{{"Member",""},{"Officer",""},{"HighOfficer",""},{"Leader",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setItemSapLoad_desc", "setItemSapLoad", {{"<slot index in bag (starts at 0)>",""}}, {{"Float:<Value>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setPosFlag_desc", "setPosFlag", {{"Text:<FlagName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setFamePlayer_desc", "setFamePlayer", {{"Text:<Faction>",""}}, {{"Number:<Fame>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "resetPVPTimers_desc", "resetPVPTimers", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setSkillsToMaxValue_desc", "setSkillsToMaxValue"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:G:EM:EG:", "showCSR_desc", "showCSR"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "showFBT_desc", "showFBT"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "startEvent_desc", "startEvent", {{"Text:<EventName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "stopEvent_desc", "stopEvent"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "stopMonitorMissions_desc", "stopMonitorMissions"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "summon_desc", "summon", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:", "targetInfos_desc", "targetInfos"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:G:OBSERVER:EM:EG:", "teleport_desc", "teleport", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:G:EM:EG:", "tpPosFlag_desc", "tpPosFlag", {{"Text:<FlagName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:SG:", "unmute_desc", "unmute", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:SG:", "unmuteUniverse_desc", "unmuteUniverse", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:SG:", "unroot_desc", "unroot", {{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "updateGuildMembersList_desc", "updateGuildMembersList", {{"Text:<GuildName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "allowSummonPet_desc", "allowSummonPet", {{"Number:<PetNumber>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "displayShopSelector_desc", "displayShopSelector"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:", "addFactionAttackableToTarget_desc", "addFactionAttackableToTarget"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:", "forceMissionProgress_desc", "forceMissionProgress"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "savePlayerActiveChar_desc", "savePlayerActiveChar", {{"Text:<Filename>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:", "reloadPlayer_desc", "reloadPlayer", {{"Number:<CharIndex>",""}}, {{"Text:<Filename>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:", "farTPPush_desc", "farTPPush", {{"Number:<DestSessionId>",""}}, {{"[<X> <Y> [<Z> [<Heading>]]]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:", "farTPReturn_desc", "farTPReturn"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "characterMissionDump_desc", "characterMissionDump"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "removeMission_desc", "removeMission", {{"Number:<MissionIdex>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:", "addMission_desc", "addMission", {{"Number:<MissionGiverAlias>",""}}, {{"Number:<MissionIdex>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "characterInventoryDump_desc", "characterInventoryDump", {{"Text:<Inventory>",""}}, {{"Number:<MinSlot>",""}}, {{"Number:<MaxSlot>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "deleteInventoryItem_desc", "deleteInventoryItem", {{"Text:<Inventory>",""}}, {{"Number:<Slot>",""}}, {{"Text:<Sheet>",""}}, {{"Text:<Quantity>",""}}, {{"Text:<Quality>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setPetAnimalSatiety_desc", "setPetAnimalSatiety", {{"<petIndex in 0..3>",""}}, {{"full|<Value>",""}}, {{"Text:<NameForAnswer>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "getPetAnimalSatiety_desc", "getPetAnimalSatiety", {{"<petIndex in 0..3>",""}}, {{"Text:<NameForAnswer>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:EG:", "setPetAnimalName_desc", "setPetAnimalName", {{"<petIndex in 0..3>",""}}, {{"Text:<Name>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "setSimplePhrase_desc", "setSimplePhrase", {{"Number:<Id>",""}}, {{"Text:<Phrase>",""}}, {{"fr",""},{"en",""},{"de",""},{"es",""},{"ru",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:EG:", "Aggro_desc", "Aggro",{{"[<0/off/false/1/on/true>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "CreateCharacterStartSkillsValue_desc", "CreateCharacterStartSkillsValue"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:EM:", "FBT_desc", "FBT", {{"[<0/off/false/1/on/true>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "God_desc", "God", {{"[<0/off/false/1/on/true>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "Invulnerable_desc", "Invulnerable", {{"[<0/off/false/1/on/true>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:SG:EM:EG:", "Invisible_desc", "Invisible", {{"[<0/off/false//1/on/true>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:VG:SG:G:", "ShowFactionChannels_desc", "ShowFactionChannels"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "HP_desc", "HP", {{"Number:<Hp>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "MaxHP_desc", "MaxHP", {{"Number:<Hp>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "Money_desc", "Money", {{"Number:+-<Value>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "Name_desc", "Name"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:", "Priv_desc", "Priv", {{"Text:<Priv>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:EG:", "PriviledgePVP_desc", "PriviledgePVP", {{"[<0/off/false/1/on/pvp/true>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:EG:", "FullPVP_desc", "FullPVP", {{"[<0/off/false/1/on/pvp/true>]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "RyzomDate_desc", "RyzomDate"})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "RyzomTime_desc", "RyzomTime"})

table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "eventCreateNpcGroup_desc", "eventCreateNpcGroup",{{"Number:<Bots>",""}},{{"Text:<Sheet>",""}},{{"Number:<DispersionRadius>",""}},{{"<0/1>",""}},{{"random|self|-360..360",""}},{{"Text:<Name>",""}},{{"Number:<PositionX>",""}},{{"Number:<PositionY>",""}},{{"Number:<PositionZ>",""},{"[*]",""}},{{"Text:<Sheet>",""}},{{"[inVIllage?inOutpost?inStable?inAtys?]",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "eventSetBotName_desc", "eventSetBotName", {{"Text:<Name>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "eventSetBotScale_desc", "eventSetBotScale", {{"<0...255>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "eventSetBotFacing_desc", "eventSetBotFacing", {{"<0-360|random>",""}}, {{"[<0,1>]","eventSetBotFacing_group_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "eventNpcSay_desc", "eventNpcSay", {{"Text:<Message>",""}}, {{"[say,shout,...]",""}}})

table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:PR:OBSERVER:EM:EG:", "Position_desc", "Position", {{"<PositionX>,<PositionY>[,<PositionZ>]",""},{"Text:<BotName>",""},{"Text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "eScript_desc", "eScript", {{"Text:<ContinentName>@",""},{"Text:<EventNpcGroup>",""}}, {{"Text:<ScriptCommand>",""}}})
--END shard commands

--eScript commands
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "despawn_desc", "despawn(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "rename_desc", "rename(\"arg1$#arg2$\")", {{"Text:wk[<Name>]",""}},{{"Text:wk[<Title>]",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setAggro_desc", "setAggro(arg1)", {{"Number:<Range>",""}}, {{"Number:<Ticks>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setPlayerAttackable_desc", "setPlayerAttackable(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setBotAttackable_desc", "setBotAttackable(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setAttackable_desc", "setAttackable(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setActivity_desc", "setActivity(\"arg1\")", {{"<no_change/escorted/guard/guard_escorted/normal/faction/faction_no_assist/bandit>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setFactionProp_desc", "setFactionProp(\"arg1\",\"arg2|arg3\")", {{"<faction/ennemyFaction/friendFaction/player/predator/outpost:<id>:<side>>",""}}, {{"Text:<FameName>FameMin",""}}, {{"Text:<FameName>FameMax",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addProfileParameter_desc", "addProfileParameter(\"arg1\",\"arg2\")", {{"event_group_killed",""},{"event_bot_killed",""},{"running",""},{"faction",""}},{{"Text:<Url>",""},{"Text:<Factionname>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "removeProfileParameter_desc", "removeProfileParameter(\"arg1\")",{{"Text:<Parameter>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "clearAggroList_desc", "clearAggroList()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "startMoving_desc", "startMoving(arg1,arg2,arg3)", {{"Number:<PositionX>",""}}, {{"Number:<PositionY>",""}}, {{"Number:<Radius>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "stopMoving_desc", "stopMoving()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "startWander_desc", "startWander(arg1)", {{"Number:<Meter>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setAutoSpawn_desc", "setAutoSpawn(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setDespawnTime_desc", "setDespawnTime(arg1)", {{"Number:<Ticks>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setRespawnTime_desc", "setDespawnTime(arg1)", {{"Number:<Ticks>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "standUp_desc", "standUp()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "sitDown_desc", "sitDown()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setZoneState_desc", "setZoneState(\"arg1\",arg2)", {{"Text:<ZoneName>",""}}, {{"<0.0/1.0>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setMode_desc", "setMode(\"arg1\")", {{"<Normal/Sit/Eat/Rest/Alert/Hungry/Death>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "aiActionSelf_desc", "aiActionSelf(\"arg1\")", {{"Text:<NameAiaction>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "aiAction_desc", "aiAction(\"arg1\")", {{"Text:<NameAiaction>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "emote_desc", "emote(\"arg1\")", {{"Text:<Emote>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "vpx_desc", "vpx(\"VPA:arg1\")", {{"Hex:<Value>","vpx_a_desc"}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "vpx_desc", "vpx(\"VPB:arg1\")", {{"Hex:<Value>","vpx_b_desc"}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "vpx_desc", "vpx(\"VPC:arg1\")", {{"Hex:<Value>","vpx_c_desc"}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setMaxHP_desc", "setMaxHP(arg1,arg2)", {{"Number:<Hp>",""}}, {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "npcSay_desc", "npcSay(\"arg1\", \"arg2\")", {{"Text:<Message>",""}}, {{"<say/shout>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "ignoreOffensiveActions_desc", "ignoreOffensiveActions(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "maxHitRange_desc", "maxHitRange(arg1)", {{"Number:<Meter>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addHP_desc", "addHP(arg1)", {{"Number:<Hp>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setClientSheet_desc", "setClientSheet(\"arg1\")", {{"Text:<Sheet>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setSheet_desc", "setSheet(\"arg1\")", {{"Text:<Sheet>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setUrl_desc", "setUrl(\"arg1\", \"arg2\")", {{"Text:<MENU_NAME>",""}}, {{"Text:<Url>",""},{"*",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "facing_desc", "facing(arg1)", {{"<3.14/-3.14>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setCanAggro_desc", "setCanAggro(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setHealer_desc", "setHealer(arg1)", {{"<0/1>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setHPScale_desc", "setHPScale(arg1)", {{"<0.0/..0.5../1.0>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "followPlayer_desc", "followPlayer(arg1,arg2)", {{"Number:<PlayerEID>",""}}, {{"Number:<Meter>",""}}})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setFactionAttackableBelow_desc", "setFactionAttackableBelow(\"arg1\",arg2,arg3)", {{"Text:<TribeName>",""}}, {{"<0/1>",""}}, {{"<6000 points per 1 fame>",""}}})

--END eScript commands

function SearchCommand:find(tbl, value)
    for k, v in pairs(tbl) do
        if v == value then
            return k
        end
    end
    return nil
end

function SearchCommand:check_prvis(command_privs)
    local player_priv = isPlayerPrivilege()
    local command_allowed = "false"
    local prvis = getPlayerPrivs()
    
    if(command_privs == "player" or command_privs == "")then
        return "false"
    else
        if(player_priv)then
            local prvis = getPlayerPrivs()
            --debug("Priv: "..prvis)
            if(prvis ~= nil)then
                for prvissubstring in prvis:gmatch("[^: ]+") do
                    for substring in command_privs:gmatch("[^: ]+") do
                        if(substring == prvissubstring)then
                            command_allowed = "true"
                        end
                    end
                end
            else
                command_allowed = "false"
            end
            return command_allowed
        else
            return "false"
        end
    end
end

function SearchCommand:htmlentities(text)
    local html_help_content=""

    html_help_content = text:gsub("<", "&lt;")
    
    return html_help_content
end

function SearchCommand:pars_command_parameter(command_parameter)
    local finish_translation_text = ""
    
    --debug("param: "..command_parameter)
    
    if(string.find(string.lower(command_parameter), ":") ~= nil and string.find(string.lower(command_parameter), ":") < 20)then
        --split text to commands
        for substring in string.gmatch(command_parameter, "([^:]+)") do
            --debug(substring)
            if(string.find(string.lower(substring), "<"))then
            --debug(substring:match("<(.*)>"))
                finish_translation_text=finish_translation_text.."<"..substring:gsub("<(.-)>", i18n.get("uiSearchCommand"..substring:match("<(.*)>")):toUtf8())..">"
            else
                finish_translation_text=finish_translation_text..""..i18n.get("uiSearchCommand"..substring):toUtf8()..":"
            end
        end
    else
        finish_translation_text = command_parameter
    end
    
    --debug("trans: "..finish_translation_text)
    
    return finish_translation_text
end


function SearchCommand:pars_help_on_window(content_of_window,height)
    local whm = getUI("ui:interface:web_transactions")
    local whm_html = getUI("ui:interface:web_transactions:content:html")
    
    local html_help_content=""
    html_help_content=[[<table width="100%" border="0"><tr><td>]]..content_of_window..[[</td></tr></table>]]
    
    whm.title = i18n.get("uiSearchCommandHelp"):toUtf8()
    whm.active = true
    whm.w = 750
    whm.h = height
    whm_html:renderHtml(html_help_content)
end

function SearchCommand:help_show_all(parameter)
    local count = 0
    local build_content = ""
    
    --debug("help parameter: "..parameter)
    build_content=build_content.."<table width='100%' border=0>"
    
    if(parameter ~= "eScript" and parameter ~= "client" and parameter ~= "shard" and parameter ~= "all")then
        parameter = "all"
    end
    
    if(parameter == "all")then
        build_content=build_content.."<tr><td colspan=4>######################## "..i18n.get("uiSearchCommandHelp"):toUtf8().." "..i18n.get("uiSearchCommandAll"):toUtf8().." #######################</td></tr>"
    else
        build_content=build_content.."<tr><td colspan=4>######################## "..i18n.get("uiSearchCommandHelp"):toUtf8().." "..i18n.get("uiSearchCommandAll"):toUtf8().." / "..i18n.get("uiSearchCommandFilter"):toUtf8().." '"..SearchCommand:htmlentities(parameter).."' #######################</td></tr>"
    end
    
    for c = 1, #self.commands_list do
        local command_are_allowed = 0
        if(self.commands_list[c][2] == "player")then
            command_are_allowed = 1
        else
            if(SearchCommand:check_prvis(self.commands_list[c][2]) == "true")then
                command_are_allowed = 1
            end
        end
        
        if(command_are_allowed == 1)then
            if(self.commands_list[c][1] == parameter or parameter == "all")then
                local arg_display=""
                build_content=build_content.."<tr><td><table width='100%' border=0>"
                build_content=build_content.."<tr><td colspan=3>&nbsp;</td></tr>"
                
                max_arguments = #self.commands_list[c] - 4
                
                if(self.commands_list[c][1] ~= "eScript")then
                    for ad = 1, max_arguments do
                        if(ad > 1)then
                            arg_display=arg_display.." [arg"..ad.."]"
                        else
                            arg_display="[arg"..ad.."]"
                        end
                    end
                else
                    arg_display = ""
                end
                
                count=count+1
                
                build_content=build_content.."<tr><td width='10px'>"..count..".</td><td colspan=3>"..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.."</td></tr>"
                build_content=build_content.."<tr><td>&nbsp;</td><td>"..i18n.get("uiR2EDScenarioDescription"):toUtf8()..": '"..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][3]))).."'</td></tr>"
                build_content=build_content.."<tr><td>&nbsp;</td><td>"..i18n.get("uiFrontSelectionType"):toUtf8()..": "..SearchCommand:htmlentities(self.commands_list[c][1]).."</td></tr>"
                build_content=build_content.."<tr><td>&nbsp;</td><td>"..i18n.get("uiSearchCommandPriv"):toUtf8()..": "..SearchCommand:htmlentities(self.commands_list[c][2]).."</td></tr>"
                
                for ac = 1, max_arguments do
                    build_content=build_content.."<tr><td>&nbsp;</td><td>arg"..ac.." :</td></tr>"
                    for pc = 1, #self.commands_list[c][4+ac] do
                    
                        local translation_parm = SearchCommand:pars_command_parameter(self.commands_list[c][4+ac][pc][1])
                        
                        if(self.commands_list[c][4+ac][pc][2] == "")then
                            build_content=build_content.."<tr><td>&nbsp;</td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(translation_parm).."</td></tr>"
                        else
                            build_content=build_content.."<tr><td>&nbsp;</td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(translation_parm).." '"..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][4+ac][pc][2]))).."'</td></tr>"
                        end
                    end
                end
                if(self.commands_list[c][4] ~= "a" and self.commands_list[c][4] ~= "b" and self.commands_list[c][4] ~= "c")then
                    if(self.commands_list[c][1] == "shard")then
                        if(SearchCommand:check_prvis(self.commands_list[c][2]) == "true")then
                            build_content=build_content.."<tr><td>&nbsp;</td><td colspan=3>example: /a "..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.." | /c riasan "..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.."</td></tr>"
                        else
                            build_content=build_content.."<tr><td>&nbsp;</td><td colspan=3>example: /a "..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.."</td></tr>"
                        end
                        
                    end
                end
            end
            build_content=build_content.."</table></td></tr>"
        end
    end
    
    build_content=build_content.."<tr><td colspan=3>&nbsp;</td></tr>"
    build_content=build_content.."<tr><td colspan=3>################################################################################</td></tr>"
    build_content=build_content.."</table>"
    SearchCommand:pars_help_on_window(build_content, 600)
end

function SearchCommand:help(uiId,input)
    --debug("search_input: "..input)
    local build_content = ""
    local command_split = {}

    if(input ~= "all")then
        for substring in input:gmatch("%S+") do
            table.insert(command_split, substring)
        end
    else
       command_split[1]="all"
       command_split[2]=""
    end
    
    if(command_split[1] == "all")then
        if(command_split[2] == "")then
            SearchCommand:help_show_all(command_split[1])
        else
            SearchCommand:help_show_all(command_split[2])
        end
    else
        build_content=build_content.."<table width='100%' border=0>"
        --show help for command input
        local command_found = 0
        for c = 1, #self.commands_list do
            local command_are_allowed = 0
            if(self.commands_list[c][4] == command_split[1])then
                command_found = 1
                build_content=build_content.."<tr><td colspan=3>######################## "..i18n.get("uiSearchCommandHelp"):toUtf8().." '"..command_split[1].."' #######################</td></tr>"
                
                if(self.commands_list[c][2] == "player")then
                    command_are_allowed = 1
                else
                    if(SearchCommand:check_prvis(self.commands_list[c][2]) == "true")then
                        command_are_allowed = 1
                    end
                end
                
                if(command_are_allowed == 1)then
                    local arg_display=""
                    build_content=build_content.."<tr><td>&nbsp;</td></tr>"
                    build_content=build_content.."<tr><td><table width='100%' border=0>"
                    
                    max_arguments = #self.commands_list[c] - 4
                    
                    if(self.commands_list[c][1] ~= "eScript")then
                        for ad = 1, max_arguments do
                            if(ad > 1)then
                                arg_display=arg_display.." [arg"..ad.."]"
                            else
                                arg_display="[arg"..ad.."]"
                            end
                        end
                    else
                        arg_display = ""
                    end
                    
                    build_content=build_content.."<tr><td>"..i18n.get("uiR2EDScenarioDescription"):toUtf8()..": "..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][3]))).."</td></tr>"
                    build_content=build_content.."<tr><td>"..i18n.get("uiFrontSelectionType"):toUtf8()..": "..SearchCommand:htmlentities(self.commands_list[c][1]).."</td></tr>"
                    build_content=build_content.."<tr><td>"..i18n.get("uiSearchCommandPriv"):toUtf8()..": "..SearchCommand:htmlentities(self.commands_list[c][2]).."</td></tr>"
                    build_content=build_content.."<tr><td>&nbsp;</td></tr>"
                    build_content=build_content.."<tr><td>"..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.."</td></tr>"
                    
                    for ac = 1, max_arguments do
                        build_content=build_content.."<tr><td>    arg"..ac.." :</td></tr>"
                        for pc = 1, #self.commands_list[c][4+ac] do
                        
                            local translation_parm = SearchCommand:pars_command_parameter(self.commands_list[c][4+ac][pc][1])
                            
                            if(self.commands_list[c][4+ac][pc][2] == "")then
                                build_content=build_content.."<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(tostring(translation_parm)).."</td></tr>"
                            else
                                build_content=build_content.."<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(tostring(translation_parm)).." '"..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][4+ac][pc][2]))).."'</td></tr>"
                            end
                        end
                    end
                    if(self.commands_list[c][4] ~= "a" and self.commands_list[c][4] ~= "b" and self.commands_list[c][4] ~= "c")then
                        if(self.commands_list[c][1] == "shard")then
                            build_content=build_content.."<tr><td>example: /a "..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.." | /c riasan "..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.."</td></tr>"
                        end
                    end
                end
                build_content=build_content.."</table></td></tr>"
            end
        end
        
        build_content=build_content.."<tr><td colspan=3>&nbsp;</td></tr>"
        build_content=build_content.."<tr><td colspan=3>######################################################################</td></tr>"
        
        if(command_found == 0)then
            displaySystemInfo(ucstring(command_split[1].." : "..i18n.get("uiCommandNotExists"):toUtf8()), "SYS")
        else
            --debug("pars_help")
            SearchCommand:pars_help_on_window(build_content, 350)
        end
    end
end

function SearchCommand:check_autocomplet(uiId)
    local modal_open_list = SearchCommand:read_modal_open_list(uiId)
    local menu = getUI("ui:interface:search_command_add_menu")
    
    if (menu.active) then
        --debug("try_autocomplte: "..uiId.." modal_open_list: "..modal_open_list)
        if(modal_open_list == 1)then
            if next(self.valid_commands_list)then
                --debug("check_autocomplet"..self.valid_commands_list[1])
                SearchCommand:finish_commands(self.valid_commands_list[1],uiId)
            end
        end
    end
end

function SearchCommand:check_autocomplet_number(uiId)
    local modal_open_list = SearchCommand:read_modal_open_list(uiId)
    local menu = getUI("ui:interface:search_command_add_menu")
    local text_from_input = getUI(uiId)
    local input_text = text_from_input.input_string
    
    max_string_count = string.len(input_text)
        
    local get_last_char_from_input = tonumber(string.sub(input_text, (max_string_count), -1))
    if(type(get_last_char_from_input) == "number")then
        --debug("last_input_is_a_Number: "..get_last_char_from_input)
        
        if(get_last_char_from_input <= #self.valid_commands_list)then
            if (menu.active) then
                --debug("try_autocomplte: "..uiId.." modal_open_list: "..modal_open_list)
                if(modal_open_list == 1)then
                    if next(self.valid_commands_list)then
                        --debug("check_autocomplet"..self.valid_commands_list[get_last_char_from_input])
                        SearchCommand:finish_commands(self.valid_commands_list[get_last_char_from_input],uiId)
                    end
                end
            end
        end
    end
end

function SearchCommand:key_trigger(uiId)
    base_window_id = string.sub(uiId,0,string.len(uiId)-15);
    local check_window = getUI(base_window_id)
    
    if(check_window.active == false)then
        --debug("window closed")
        local timer_function_on_ui_window = getUI(uiId)
        removeOnDbChange(timer_function_on_ui_window,"@UI:VARIABLES:CURRENT_SERVER_TICK")
    end
    
    --check is input are empty if yes cancel all stuff
    local text_from_input = getUI(uiId)
    if(text_from_input.input_string == "")then
        SearchCommand:search(uiId)
    end
end

function SearchCommand:update_modal_open_list(uiId,p_status)
    local found_modal_open=0
    
    --check if we already have a process status for this window
    if next(self.modal_open_list)then
        for pc = 1, #self.modal_open_list do
            if(self.modal_open_list[pc][1] == uiId)then
                found_modal_open=1
            end
        end
    end
    
    if(found_modal_open == 0)then
        table.insert(self.modal_open_list,{uiId, p_status})
    else
        if next(self.modal_open_list)then
            for mc = 1, #self.modal_open_list do
                if(self.modal_open_list[mc][1] == uiId)then
                    self.modal_open_list[mc][2]=p_status
                end
            end
        end  
    end
end

function SearchCommand:read_modal_open_list(uiId)
    local found_modal_open = 0
    if next(self.process_list)then
        for mc = 1, #self.modal_open_list do
            if(self.modal_open_list[mc][1] == uiId)then
                found_modal_open=self.modal_open_list[mc][2]
            end
        end
    end
    return found_modal_open
end

function SearchCommand:update_process_list(uiId,p_status)
    local found_process_status=0
    
    --check if we already have a process status for this window
    if next(self.process_list)then
        for pc = 1, #self.process_list do
            if(self.process_list[pc][1] == uiId)then
                found_process_status=1
            end
        end
    end
    
    if(found_process_status == 0)then
        table.insert(self.process_list,{uiId, p_status})
    else
        if next(self.process_list)then
            for pc = 1, #self.process_list do
                if(self.process_list[pc][1] == uiId)then
                    self.process_list[pc][2]=p_status
                end
            end
        end  
    end
end

function SearchCommand:read_process_status(uiId)
    local process_status = 0
    if next(self.process_list)then
        for pc = 1, #self.process_list do
            if(self.process_list[pc][1] == uiId)then
                process_status=self.process_list[pc][2]
            end
        end
    end
    return process_status
end

function SearchCommand:search(uiId)
    --debug("now_onchange "..uiId)
    is_tab_down = isTabDown()
    if(is_tab_down)then
        --debug("key_tab_down")
        SearchCommand:check_autocomplet(uiId)
    end
    --trigger command by onchange a singel input
    
    SearchCommand:check_autocomplet_number(uiId)
    
    local text_from_input = getUI(uiId)
    local input_text = text_from_input.input_string
    command_identifier = string.sub(input_text, 0, 1)

    --check if first char are a "/" from text_from_input
    if(command_identifier == "/")then
        --debug("identifier found")
        
        if(SearchCommand:find(self.identifier_found, uiId) == nil)then
            table.insert(self.identifier_found, uiId)
            
            local timer_str = "@UI:VARIABLES:CURRENT_SERVER_TICK"
            local timer_function_on_ui_window = getUI(uiId)
            addOnDbChange(timer_function_on_ui_window, timer_str, "SearchCommand:key_trigger('"..uiId.."')")
        end
        
        --reset command_parameter_list for fresh input
        self.command_parameter_list = {}
        --END reset command_parameter_list for fresh input
        
        if(max_string_count == 1)then
            self.command_self=""
            SearchCommand:write_command_help(uiId,i18n.get("uiSearchCommandInitDialog"):toUtf8())
            SearchCommand:close_modal(uiId)
            
            --update process_status
            SearchCommand:update_process_list(uiId,0)
        else
            SearchCommand:close_modal(uiId)
            
            --if we found 2 space cancel all action
            if(string.find(string.lower(input_text), "  "))then
                --debug("found_two_spaces")
                SearchCommand:write_command_help_clear(uiId)
                return 0
            end
            
            local command_first = string.sub(input_text, 2, (max_string_count))
            --split text to commands
            for substring in command_first:gmatch("%S+") do
                table.insert(self.command_parameter_list, substring)
            end
            
            self.command_self=self.command_parameter_list[1]
            
            --update process_status
            SearchCommand:update_process_list(uiId,#self.command_parameter_list)
            
            --go and search we found a mathing command
            SearchCommand:write_command_help_clear(uiId)
            SearchCommand:build_command_helper(uiId)
        end
    else
        --check if we found the identifier
        if(SearchCommand:find(self.identifier_found, uiId) ~= nil)then
            --debug("clear all")
            --run clear functions here
            
            table.remove(self.identifier_found, SearchCommand:find(self.identifier_found, uiId))
            local timer_function_on_ui_window = getUI(uiId)
            removeOnDbChange(timer_function_on_ui_window,"@UI:VARIABLES:CURRENT_SERVER_TICK")
            
            SearchCommand:close_modal()
            SearchCommand:write_command_help_clear(uiId)
        end
    end
end

function SearchCommand:get_command_type(command)
    local command_type = ""
    
    for c = 1, #self.commands_list do
        if(self.commands_list[c][4] == command) then
            command_type = self.commands_list[c][1]
        end
    end
    
    return command_type
end

function SearchCommand:build_valid_command_list(command_input,uiId)
    self.valid_commands_list = {}
    local count_found=0
    local found_command=0
    
    for c = 1, #self.commands_list do
        local command_are_allowed = 0
        local command_display = 0
        
        --check if we want used a client or shared command
        if(self.command_self == "a" or self.command_self == "b" or self.command_self == "c")then
            --debug(self.commands_list[c][1])
            if(self.commands_list[c][1] == "shard" and self.command_parameter_list[2] ~= "eScript")then
                command_display = 1
            elseif(self.commands_list[c][1] == "eScript")then
                if(self.command_parameter_list[2] == "eScript")then
                    command_display = 1
                else
                    command_display = 0
                end
            else
                command_display = 0
            end
        else
            if(self.commands_list[c][1] == "shard")then
                if(self.command_self == "?")then
                    command_display = 1
                else
                    command_display = 0
                end
            elseif(self.commands_list[c][1] == "eScript")then
                if(self.command_self == "?")then
                    command_display = 1
                else
                    command_display = 0
                end
            else
                command_display = 1
            end
        end
        
        if(self.commands_list[c][2] == "player")then
            command_are_allowed = 1
        else
            if(SearchCommand:check_prvis(self.commands_list[c][2]) == "true")then
                command_are_allowed = 1
            end
        end
        
        --debug("command_are_allowed: "..command_are_allowed.." command_display: "..command_display)
        
        if(command_are_allowed == 1 and command_display == 1)then
            if(command_input ~= "")then
                if(string.lower(self.commands_list[c][4]) == string.lower(command_input))then
                    found_command=c
                    count_found=1
                else
                    if string.find(string.lower(self.commands_list[c][4]), string.lower(command_input))then
                        table.insert(self.valid_commands_list,self.commands_list[c][4])
                        count_found=count_found+1
                    end
                end
            end
        end
    end
    
    if(count_found == 0)then
        --debug("no_command_found_close_modal")
        SearchCommand:close_modal(uiId)
    end
    return found_command
end

function SearchCommand:build_valid_player_list(playername_input,uiId)
    self.valid_commands_list = {}
    local count_found=0
    local found_playername=0
    
    if(self.player_list ~= nil)then
        for c = 1, #self.player_list do
            if(playername_input ~= "")then
                if(string.lower(self.player_list[c]) == string.lower(playername_input))then
                    found_playername=c
                    count_found=1
                else
                    if string.find(string.lower(self.player_list[c]), string.lower(playername_input))then
                        table.insert(self.valid_commands_list,self.player_list[c])
                        count_found=count_found+1
                    end
                end
            end
        end
    else
        debug("Error")
    end
    
    if(count_found == 0)then
        SearchCommand:close_modal(uiId)
    end
    return found_playername
end

function SearchCommand:search_build_player_list(uiId,playername)
    local found_player=0
    local found_command=0
    found_player=SearchCommand:build_valid_player_list(playername,uiId)
    
    SearchCommand:search_build_argument_list(uiId,self.command_self)

    if(found_player ~= 0)then
        SearchCommand:close_modal(uiId)
    else
        if next(self.valid_commands_list) ~= nil then
            SearchCommand:show_more_options(uiId)
        end
    end
end

function SearchCommand:search_build_command_list(uiId,command,show_argument_help)
    local found_command=0
    --debug("search_build_command_list")
    
    found_command=SearchCommand:build_valid_command_list(command,uiId)

    if(found_command ~= 0)then
        SearchCommand:close_modal(uiId)
    else
        if next(self.valid_commands_list) then
            SearchCommand:show_more_options(uiId)
        end
    end
    
    if(show_argument_help)then
        if(found_command ~= 0)then
            SearchCommand:search_build_argument_list(uiId,command)
        end
    else
        SearchCommand:search_build_argument_list(uiId,self.command_self)
    end
end

function SearchCommand:search_build_argument_list(uiId,command_to_show_argument)
    local argument_help=""
    local command_index=0
    local max_arguments=0
    local current_args=0
    local special_offset=0
    
    if(command_to_show_argument == "a")then
        if(#self.command_parameter_list >= 2)then
            command_to_show_argument=self.command_parameter_list[2]
            special_offset=1
        end
    end
    
    if(command_to_show_argument == "b")then
        if(#self.command_parameter_list >= 2)then
            command_to_show_argument=self.command_parameter_list[2]
            special_offset=1
        end
    end
    
    if(command_to_show_argument == "c")then
        if(#self.command_parameter_list >= 3)then
            command_to_show_argument=self.command_parameter_list[3]
            special_offset=2
        end
    end
    
    for c = 1, #self.commands_list do
        if(self.commands_list[c][4] == command_to_show_argument)then
            command_index=c
        end
    end
    
    if(command_index ~= 0)then
        
        max_arguments = #self.commands_list[command_index] - 4
        
        for ac = 1, max_arguments do
            max_args=#self.command_parameter_list
            current_args=max_args - 1
            --debug("p: "..#self.command_parameter_list.." l: "..ac)
            
            if(special_offset ~= 0)then
                max_args=max_args - special_offset
                current_args=max_args - 1
            end
            
            if(max_args == ac or max_args == 1)then
                for pc = 1, #self.commands_list[command_index][4+ac] do
                
                    local translation_parm = SearchCommand:pars_command_parameter(self.commands_list[command_index][4+ac][pc][1])
                    
                    if(pc > 1)then
                        argument_help=argument_help.."/"..translation_parm
                    else
                        argument_help=argument_help.." "..translation_parm
                    end
                end
            else
                argument_help=argument_help.." "..self.command_parameter_list[ac+1+special_offset]
            end
        end
            
        if(current_args > max_arguments)then
            diff=current_args-max_arguments
            
            for ma = 1, diff do
                if(self.command_self == "a")then
                    argument_help=argument_help.." "..self.command_parameter_list[max_arguments+2+ma]
                elseif(self.command_self == "b")then
                    argument_help=argument_help.." "..self.command_parameter_list[max_arguments+2+ma]
                elseif(self.command_self == "c")then
                    argument_help=argument_help.." "..self.command_parameter_list[max_arguments+3+ma]
                else
                    argument_help=argument_help.." "..self.command_parameter_list[max_arguments+1+ma]
                end
            end
            argument_help=argument_help.." "..i18n.get("uiSearchCommandWarningParameter"):toUtf8()
        end
        
        if(self.command_self == "a" and #self.command_parameter_list >= 2)then
            SearchCommand:write_command_help(uiId,"/"..self.command_parameter_list[1].." "..self.command_parameter_list[2]..""..argument_help)
        elseif(self.command_self == "b" and #self.command_parameter_list >= 2)then
            SearchCommand:write_command_help(uiId,"/"..self.command_parameter_list[1].." "..self.command_parameter_list[2]..""..argument_help)
        elseif(self.command_self == "c" and #self.command_parameter_list >= 3)then
            SearchCommand:write_command_help(uiId,"/"..self.command_parameter_list[1].." "..self.command_parameter_list[2].." "..self.command_parameter_list[3]..""..argument_help)
        else
            SearchCommand:write_command_help(uiId,"/"..self.commands_list[command_index][4]..""..argument_help)
        end
    end
end

function SearchCommand:find_argument(command,uiId)
    --debug("find_argument")
    local argument_name = ""
    local special_offset = 0
    local current_parm = 0
    local max_command_args = 0
    
    if(command == "a")then
        if(#self.command_parameter_list > 2)then
            command=self.command_parameter_list[2]
            special_offset=2
        end
    end
    
    if(command == "b")then
        if(#self.command_parameter_list > 2)then
            command=self.command_parameter_list[2]
            special_offset=2
        end
    end
    
    if(command == "c")then
        if(#self.command_parameter_list > 3)then
            command=self.command_parameter_list[3]
            special_offset=3
        end
    end
    
    --debug("special_offset: "..special_offset.." command: "..command)
    
    for c = 1, #self.commands_list do
        if(self.commands_list[c][4] == command)then
            if(special_offset ~= 0)then
                current_parm=SearchCommand:read_process_status(uiId) - special_offset
            else
                current_parm=SearchCommand:read_process_status(uiId) - 1
            end
        
            max_command_args = #self.commands_list[c] - 4
            
            if(current_parm <= max_command_args)then
                for pc = 1, #self.commands_list[c][4+current_parm] do
                    argument_name=argument_name..","..self.commands_list[c][4+current_parm][pc][1]
                end
            end
        end
    end
    return argument_name
end


function SearchCommand:build_command_helper(uiId)
    --read process_list
    
    local player_priv = isPlayerPrivilege()
    local argu_name = ""
    local process_status=SearchCommand:read_process_status(uiId)
    
    --debug("process_status: "..process_status)
    --process_status == 0 only / identifyer found
    --process_status == 1 try for command
    --process_status > 1 try for parameter
    
    if(process_status == 1)then
        --initlial command
        SearchCommand:search_build_command_list(uiId,self.command_self,true)
    else
        --debug("process_args")
        argu_name = SearchCommand:find_argument(self.command_self,uiId)
        --debug("argu_name: "..argu_name)
        
        
        --check if argument can be a command or a playername
        if(string.find(string.lower(argu_name), string.lower("<Command>")))then
            --debug("parm is a command")
            SearchCommand:search_build_command_list(uiId,self.command_parameter_list[process_status],false)
        elseif(string.find(string.lower(argu_name), string.lower("<PlayerName>")) or string.find(string.lower(argu_name), string.lower("<TargetName>")))then 
            --debug("parm is a playername")
            
            if(player_priv)then
                --load_current_active_player_name
                webig:openUrlInBg("https://app.ryzom.com/get_playername_online/index.php")
            
                SearchCommand:search_build_player_list(uiId,self.command_parameter_list[process_status])
            else
                SearchCommand:search_build_argument_list(uiId,self.command_self)
            end
        elseif(string.find(string.lower(argu_name), string.lower("<ScriptCommand>")))then
            SearchCommand:search_build_command_list(uiId,self.command_parameter_list[process_status],false)
        else
            SearchCommand:search_build_argument_list(uiId,self.command_self)
        end
    end
end

function SearchCommand:write_command_help(uiId,text)
    local main_chat_input =  getUI(uiId)
    local read_prompt = main_chat_input.prompt
    --debug("write_command_help: "..text)
    local behind_help_text = getUI(uiId.."h")
    
    behind_help_text.prompt=read_prompt
    behind_help_text.input_string = text
end

function SearchCommand:write_command_help_clear(uiId)
    local behind_help_text = getUI(uiId.."h")
    behind_help_text.input_string = ""
    behind_help_text.prompt = ""
end

function SearchCommand:close_modal(uiId)
    --debug("close_modal")
    
    SearchCommand:update_modal_open_list(uiId,0)
    runAH(nil, "leave_modal", "group=ui:interface:search_command_add_menu")
end

function SearchCommand:show_more_options(uiId)
    SearchCommand:update_modal_open_list(uiId,1)
    --debug("build_menu")
    local display_max_found = 0
    local calc_manu_hight = 0
    local up_ok = 0
    local down_ok = 0
    
    table.sort(self.valid_commands_list)
    
    
    base_main_chat_id = string.sub(uiId,0,string.len(uiId)-3)
    local check_main_window = getUI(base_main_chat_id)

    base_window_id = string.sub(uiId,0,string.len(uiId)-15)
    local check_window = getUI(base_window_id)
    
    local interface_window = getUI("ui:interface")
    local interface_window_h = interface_window.h
    local interface_window_w = interface_window.w
    
    local offset_h_down = 20
    local offset_h_up = 35
    local offest_x = 0
    
    if(check_main_window.x == 0)then
        offest_x = 18
    else
        offest_x = check_main_window.x + 10
    end
    
    local new_modal_pos_x = check_window.x + offest_x
    local new_modal_pos_y = check_window.y - check_window.h
    
    if(#self.valid_commands_list > 9)then
        display_max_found = 9
        calc_manu_hight = (display_max_found * 16) + 39
    else
        display_max_found = #self.valid_commands_list
        calc_manu_hight = (display_max_found * 16) + 23
    end
    
    --check we need spawn menu up or down
    local cal_needed_space_up = (check_window.y - check_window.h) + calc_manu_hight + offset_h_up
    local cal_needed_space_down = (check_window.y - check_window.h) - calc_manu_hight - offset_h_down

    if(cal_needed_space_up > interface_window_h)then
        up_ok = 0
    else
        up_ok = 1
    end
    
    if(cal_needed_space_down < 0)then
        down_ok = 0
    else
        down_ok = 1
    end
    
    if(down_ok == 1 and up_ok == 1)then
        if(cal_needed_space_up < cal_needed_space_down)then
            up_ok = 1
            down_ok = 0
        else
            up_ok = 0
            down_ok = 1
        end
    end
    if(up_ok == 1)then
        new_modal_pos_y = (check_window.y - check_window.h) + offset_h_up
    end
    if(down_ok == 1)then
        new_modal_pos_y = (check_window.y - check_window.h) - calc_manu_hight
    end
    
    --setup menu window
    menu = getUI("ui:interface:search_command_add_menu")
    menu.active = true
    menu.y = new_modal_pos_y
    menu.x = new_modal_pos_x
    menu:updateCoords()
    menu = menu:getRootMenu()
    menu:reset()
    --END setup menu window
    
    --fill menu window
    menu:addLine(ucstring(i18n.get("uiSearchCommandFound"):toUtf8()..": "..#self.valid_commands_list), "lua", "SearchCommand:close_modal('"..uiId.."')", "")
    
    for c = 1, display_max_found do
        menu:addLine(ucstring(c..". "..self.valid_commands_list[c]), "lua", "SearchCommand:finish_commands('"..self.valid_commands_list[c].."','"..uiId.."')", "")
    end
    
    if(#self.valid_commands_list > 9)then
        menu:addLine(ucstring("..."), "lua", "SearchCommand:close_modal('"..uiId.."')", "")
    end
    --END fill menu window
    
    --launche menu window
    launchContextMenuInGame("ui:interface:search_command_add_menu")
    --END launche menu window
end

function SearchCommand:replace_escript_param(command_name)
    local command_index = 0
    local temp_command = ""

    for c = 1, #self.commands_list do
        if(self.commands_list[c][4] == command_name)then
            command_index=c
        end
    end
    
    if(command_index ~= 0)then
        temp_command = command_name
        max_arguments = #self.commands_list[command_index] - 4
        
        for ac = 1, max_arguments do
            
            local load_all_param = ""
            for pc = 1, #self.commands_list[command_index][4+ac] do
                debug(self.commands_list[command_index][4+ac][pc][1])
                local translation_parm = SearchCommand:pars_command_parameter(self.commands_list[command_index][4+ac][pc][1])
                if(pc > 1)then
                    load_all_param=load_all_param.."/"..translation_parm
                else
                    load_all_param=translation_parm
                end
            end
            temp_command = temp_command:gsub("arg"..ac,load_all_param)
        end
    end
    
    --debug("final_temp: "..temp_command)
    return temp_command
end

function SearchCommand:finish_commands(command_name,uiId)
    local process_status = SearchCommand:read_process_status(uiId)
    local input_search_string = getUI(uiId)
    local final_command = ""
    local new_command_par = ""
    
    --debug("process_status: "..process_status)
    
    for fc = 1, process_status do
        if(fc == process_status)then
            self.command_parameter_list[fc] = command_name
        end
    end
    
    for pc = 1, #self.command_parameter_list do
        local add_escript_prefix=""
        if(SearchCommand:get_command_type(self.command_parameter_list[pc]) == "eScript")then
            add_escript_prefix="()"
            new_command_par = SearchCommand:replace_escript_param(self.command_parameter_list[pc])
            self.command_parameter_list[pc] = new_command_par
        end
        
        if(final_command == "")then
            final_command = add_escript_prefix..""..self.command_parameter_list[pc]
        else
            final_command = final_command.." "..add_escript_prefix..""..self.command_parameter_list[pc]
        end
    end
    
    input_search_string.input_string = "/"..final_command
    --debug("/"..final_command)
    
    input_search_string:setFocusOnText()
    
    SearchCommand:close_modal(uiId)
    SearchCommand:search(uiId)
end