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
	    process_list = {}
	}
end

--setup data
--commands_list[x] = {"type(client/shard)","priv(player/privs)","uitranslation for description","command", "parameter1(playername/text/number)", "parameter2(playername/text/number)" ..}
SearchCommand.commands_list = {}

local player_priv = isPlayerPrivilege()
if(player_priv)then
    table.insert(SearchCommand.commands_list,{"client", "player", "help_desc", "?", {{"text:<Command>",""}, {"all","help_all_desc"}}, {{"shard",""},{"client",""},{"eScript",""}}})
else
    table.insert(SearchCommand.commands_list,{"client", "player", "help_desc", "?", {{"text:<Command>",""}, {"all","help_all_desc"}}})
end

--client commands
table.insert(SearchCommand.commands_list,{"client", "player", "time_desc", "time"})
table.insert(SearchCommand.commands_list,{"client", "player", "version_desc", "version"})
table.insert(SearchCommand.commands_list,{"client", "player", "where_desc", "where"})
table.insert(SearchCommand.commands_list,{"client", "player", "playedTime_desc", "playedTime"})
table.insert(SearchCommand.commands_list,{"client", "player", "who_desc", "who", {{"gm","who_gm_desc"}}, {{"channel","who_channel_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guildinvite_desc", "guildinvite",{{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guildmotd_desc", "guildmotd",{{"text:<Message>","guildmotd_message_desc"},{"?","guildmotd_?_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "league_desc", "league",{{"text:<leaguename>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "leagueinvite_desc", "leagueinvite",{{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "leaguequit_desc", "leaguequit"})
table.insert(SearchCommand.commands_list,{"client", "player", "leaguekick_desc", "leaguekick",{{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "afk_desc", "afk",{{"text:<autoresponse>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "assist_desc", "assist"})
table.insert(SearchCommand.commands_list,{"client", "player", "assist_desc", "as"})
table.insert(SearchCommand.commands_list,{"client", "player", "self_desc", "self"})
table.insert(SearchCommand.commands_list,{"client", "player", "brutalQuit_desc", "brutalQuit"})
table.insert(SearchCommand.commands_list,{"client", "player", "chatLog_desc", "chatLog"})
table.insert(SearchCommand.commands_list,{"client", "player", "follow_desc", "follow"})
table.insert(SearchCommand.commands_list,{"client", "player", "ignore_desc", "ignore",{{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "invite_desc", "invite",{{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "mount_desc", "mount"})
table.insert(SearchCommand.commands_list,{"client", "player", "unmount_desc", "unmount"})
table.insert(SearchCommand.commands_list,{"client", "player", "random_desc", "random", {{"number:<lowest_number>",""}}, {{"number:<highest_number>",""}}, {{"hide ",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "skiptutorial_desc", "skiptutorial"})
table.insert(SearchCommand.commands_list,{"client", "player", "sleep_desc", "sleep", {{"number:<number>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "tar_desc", "tar", {{"text:<name>",""}}, {{"|quiet=true","tar_quiet_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "target_quiet_desc", "target_quiet"})
table.insert(SearchCommand.commands_list,{"client", "player", "target_quiet_desc", "tarq"})
table.insert(SearchCommand.commands_list,{"client", "player", "lmtar_desc", "lmtar", {{"text:<name>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "chat_desc", "chat"})
table.insert(SearchCommand.commands_list,{"client", "player", "go_desc", "go"})
table.insert(SearchCommand.commands_list,{"client", "player", "appzone_desc", "appzone", {{"number:<AppId>","appzone_AppId_desc"},{"hide","appzone_hide_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "setuiscale_desc", "setuiscale", {{"number:<ScaleFactor>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "createGroup_desc", "createGroup", {{"text:<OutfitGroupName>","createGroup_OutfitGroupName_desc"},{"true","createGroup_true_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "moveGroup_desc", "moveGroup", {{"text:<OutfitGroupName>",""}},{{"text:<PetAnimal1>",""},{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "equipGroup_desc", "equipGroup", {{"text:<OutfitGroupName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "deleteGroup_desc", "deleteGroup", {{"text:<OutfitGroupName>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "naked_desc", "naked"})
table.insert(SearchCommand.commands_list,{"client", "player", "nude_desc", "nude"})
table.insert(SearchCommand.commands_list,{"client", "player", "listGroup_desc", "listGroup"})
table.insert(SearchCommand.commands_list,{"client", "player", "say_desc", "say",{{">","s_param1_desc"}}, {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "say_desc", "s",{{">","s_param1_desc"}}, {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "shout", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "sh", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "y", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "shout_desc", "yell", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guild_desc", "guild", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guild_desc", "g", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "guild_desc", "gu", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "region_desc", "region", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "region_desc", "r", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "region_desc", "re", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "team", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "te", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "party", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "team_desc", "p", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "universe_desc", "universe", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "universe_desc", "u", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "0_desc", "0", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "1_desc", "1", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "2_desc", "2", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "3_desc", "3", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "4_desc", "4", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "channel_desc", "channel",{{"text:<channelname>",""}}, {{"text:<password>",""},{"*","channel_leave_desc"}}})
table.insert(SearchCommand.commands_list,{"client", "player", "tell_desc", "tell",{{"text:<PlayerName>",""}}, {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"client", "player", "tell_desc", "t",{{"text:<PlayerName>",""}}, {{"text:<Message>",""}}})
--END client commands


--shard commands
table.insert(SearchCommand.commands_list,{"shard", "player", "a_desc", "a", {{"text:<Command>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:PR:OBSERVER:EM:EG:TESTER:", "b_desc", "b", {{"text:<Command>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:PR:OBSERVER:EM:EG:TESTER:", "c_desc", "c", {{"text:<TargetName>",""}}, {{"text:<Command>",""}}})

table.insert(SearchCommand.commands_list,{"shard", "player", "showOnline_desc", "showOnline", {{"1","showOnline_1_desc"}, {"2","showOnline_2_desc"},{"0","showOnline_0_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "setLeague_desc", "setLeague", {{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "roomInvite_desc", "roomInvite", {{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "roomKick_desc", "roomKick", {{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "dodge_desc", "dodge"})
table.insert(SearchCommand.commands_list,{"shard", "player", "parry_desc", "parry"})
table.insert(SearchCommand.commands_list,{"shard", "player", "setPvPTag_desc", "setPvPTag", {{"0",""},{"1",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "clearGuildMessage_desc", "clearGuildMessage"})
table.insert(SearchCommand.commands_list,{"shard", "player", "setGuildMessage_desc", "setGuildMessage", {{"text:<Message>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "setDontTranslateLangs_desc", "setDontTranslateLangs", {{"codelang|codelang","setDontTranslateLangs_codelang_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "connectLangChannel_desc", "connectLangChannel",{{"fr",""},{"en",""},{"de",""},{"es",""},{"ru",""}},{{"1","connectLangChannel_1_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "updateTarget_desc", "updateTarget"})
table.insert(SearchCommand.commands_list,{"shard", "player", "teamInvite_desc", "teamInvite", {{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "summonPet_desc", "summonPet", {{"number:<PetNumber>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "setTeamLeader_desc", "setTeamLeader", {{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "resetName_desc", "resetName"})


if(player_priv)then
    table.insert(SearchCommand.commands_list,{"shard", "player", "connectUserChannel_desc", "connectUserChannel", {{"text:<channelname>",""}}, {{"text:<password>",""},{"*","channel_leave_desc"},{"***","channel_remove_admin_desc"}}})
else
    table.insert(SearchCommand.commands_list,{"shard", "player", "connectUserChannel_desc", "connectUserChannel", {{"text:<channelname>",""}}, {{"text:<password>",""},{"*","channel_leave_desc"}}})
end


table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:VG:PR:OBSERVER:EM:EG:", "Position_desc", "Position", {{"number:<posx>,<posy>[,<posz>]",""},{"text:<bot name>",""},{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", ":DEV:SGM:GM:EM:", "eScript_desc", "eScript", {{"text:<ContinentName>@text:<EventNpcGroup>",""}}, {{"text:<ScriptCommand>",""}}})
--END shard commands

--eScript commands
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setAggro_desc", "()setAggro(number:<25>,number:<10>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setPlayerAttackable_desc", "()setPlayerAttackable(number:<0/1>"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setBotAttackable_desc", "()setBotAttackable(number:<0/1)>"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setAttackable_desc", "()setAttackable(number:<0/1)>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setActivity_desc", "()setActivity(\"text:<no_change/escorted/guard/guard_escorted/normal/faction/faction_no_assist/bandit>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setFactionProp_desc", "()setFactionProp(\"text:<faction/ennemyFaction/friendFaction/Player/Predator/outpost:<id>:<side>>\",\"text:<FameName>FameMin|FameName<FameMax>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setAggro_desc", "()setAggro(number:<25>,number:<10>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addProfileParameter_desc", "()addProfileParameter(\"text:<event_group_killed>\",\"text:<Url>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addProfileParameter_desc", "()addProfileParameter(\"text:<event_bot_killed>\",text:<Url>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addProfileParameter_desc", "()addProfileParameter(\"text:<running>)\""})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addProfileParameter_desc", "()addProfileParameter(\"text:<faction>,text:<Factionname>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addProfileParameter_desc", "()addProfileParameter(\"text:<fame_for_guard_attack>\",number:<6000 points per 1 fame>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "removeProfileParameter_desc", "()removeProfileParameter(\"text:<Parameter>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "clearAggroList_desc", "()clearAggroList()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "startMoving_desc", "()startMoving(number:<XPos>,number:<YPos>,number:<Radius>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "stopMoving_desc", "()stopMoving()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "startWander_desc", "()startWander(number:<Meter>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setAutoSpawn_desc", "()setAutoSpawn(number:<0/1>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setDespawnTime_desc", "()setDespawnTime(number:<Ticks>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setRespawnTime_desc", "()setDespawnTime(number:<Ticks>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "standUp_desc", "()standUp()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "sitDown_desc", "()sitDown()"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setZoneState_desc", "()setZoneState(\"text:<ZoneName>\",number:<0.0/1.0>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setMode_desc", "()setMode(\"text:<Normal/Sit/Eat/Rest/Alert/Hungry/Death>\""})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "aiActionSelf_desc", "()aiActionSelf(\"text:<name.aiaction>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "aiAction_desc", "()aiAction(\"text:<name.aiaction>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "emote_desc", "()emote(\"text:<Emote>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "vpx_desc", "()vpx(\"text:<VPA:HEX>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "vpx_desc", "()vpx(\"text:<VPB:HEX>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "vpx_desc", "()vpx(\"text:<VPC:HEX>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setMaxHP_desc", "()setMaxHP(number:<Hp>, number:<0/1>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "npcSay_desc", "()npcSay(\"text:<Message>\", \"text:<say/shout>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "ignoreOffensiveActions_desc", "()ignoreOffensiveActions(number:<0/1>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "maxHitRange_desc", "()maxHitRange(number:<Meter>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "addHP_desc", "()addHP(number:<Hp>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setClientSheet_desc", "()setClientSheet(\"text:<Sheet>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setSheet_desc", "()setSheet(\"text:<Sheet>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setUrl_desc", "()setUrl(\"text:<MENU_NAME>\", \"text:<Url/*>\")"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "facing_desc", "()facing(rad:<3.14/-3.14>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setCanAggro_desc", "()setCanAggro(number:<0/1>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setHealer_desc", "()setHealer(number:<0/1>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setHPScale_desc", "()setHPScale(number:<0.0/..0.5../1.0>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "followPlayer_desc", "()followPlayer(number:<PlayerEID>, number:<Meter>)"})
table.insert(SearchCommand.commands_list,{"eScript", ":DEV:SGM:GM:EM:", "setFactionAttackableBelow_desc", "()setFactionAttackableBelow(\"text:<TribeName>\", number:<0/1>, number:<6000 points per 1 fame>)"})

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

function SearchCommand:pars_help_on_window(content_of_window,height)
    local whm = getUI("ui:interface:web_transactions")
    local whm_html = getUI("ui:interface:web_transactions:content:html")
    
    local html_help_content=""
    html_help_content=[[<table width="100%" border="0"><tr><td>]]..content_of_window..[[</td></tr></table>]]
    
    whm.title = "command help"
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
    
    if(parameter == "all")then
        build_content=build_content.."<tr><td colspan=4>######################## command help all #######################</td></tr>"
    else
        build_content=build_content.."<tr><td colspan=4>######################## command help all / filter '"..SearchCommand:htmlentities(parameter).."' #######################</td></tr>"
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
                for ad = 1, max_arguments do
                    if(ad > 1)then
                        arg_display=arg_display.." [arg"..ad.."]"
                    else
                        arg_display="[arg"..ad.."]"
                    end
                end
                
                count=count+1
                
                build_content=build_content.."<tr><td width='10px'>"..count..".</td><td colspan=3>"..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.." '"..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][3]))).."'</td></tr>"
                build_content=build_content.."<tr><td>&nbsp;</td><td>type: "..SearchCommand:htmlentities(self.commands_list[c][1]).."</td></tr>"
                build_content=build_content.."<tr><td>&nbsp;</td><td>priv: "..SearchCommand:htmlentities(self.commands_list[c][2]).."</td></tr>"
                
                for ac = 1, max_arguments do
                    build_content=build_content.."<tr><td>&nbsp;</td><td>arg"..ac.." :</td></tr>"
                    for pc = 1, #self.commands_list[c][4+ac] do
                        if(self.commands_list[c][4+ac][pc][2] == "")then
                            build_content=build_content.."<tr><td>&nbsp;</td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(self.commands_list[c][4+ac][pc][1]).."</td></tr>"
                        else
                            build_content=build_content.."<tr><td>&nbsp;</td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(self.commands_list[c][4+ac][pc][1]).." '"..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][4+ac][pc][2]))).."'</td></tr>"
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
    build_content=build_content.."<tr><td colspan=3>############################ end ##########################</td></tr>"
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
                build_content=build_content.."<tr><td colspan=3>######################## command help '"..command_split[1].."' #######################</td></tr>"
                
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
                    for ad = 1, max_arguments do
                        if(ad > 1)then
                            arg_display=arg_display.." [arg"..ad.."]"
                        else
                            arg_display="[arg"..ad.."]"
                        end
                    end
                    build_content=build_content.."<tr><td>desc: "..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][3]))).."</td></tr>"
                    build_content=build_content.."<tr><td>type: "..SearchCommand:htmlentities(self.commands_list[c][1]).."</td></tr>"
                    build_content=build_content.."<tr><td>priv: "..SearchCommand:htmlentities(self.commands_list[c][2]).."</td></tr>"
                    build_content=build_content.."<tr><td>&nbsp;</td></tr>"
                    build_content=build_content.."<tr><td>"..SearchCommand:htmlentities(self.commands_list[c][4]).." "..arg_display.."</td></tr>"
                    
                    for ac = 1, max_arguments do
                        build_content=build_content.."<tr><td>    arg"..ac.." :</td></tr>"
                        for pc = 1, #self.commands_list[c][4+ac] do
                            if(self.commands_list[c][4+ac][pc][2] == "")then
                                build_content=build_content.."<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(self.commands_list[c][4+ac][pc][1]).."</td></tr>"
                            else
                                build_content=build_content.."<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"..SearchCommand:htmlentities(self.commands_list[c][4+ac][pc][1]).." '"..SearchCommand:htmlentities(tostring(i18n.get(self.commands_list[c][4+ac][pc][2]))).."'</td></tr>"
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
        build_content=build_content.."<tr><td colspan=3>############################ end ############################</td></tr>"
        
        if(command_found == 0)then
            displaySystemInfo(ucstring("Command not found"), "AROUND")
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
        --debug("last_input_is_a_number: "..get_last_char_from_input)
        
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
            SearchCommand:write_command_help(uiId,"/<command> or /? all")
            SearchCommand:close_modal(uiId)
            
            --update process_status
            SearchCommand:update_process_list(uiId,0)
        else
            SearchCommand:close_modal(uiId)
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

function SearchCommand:build_valid_command_list(command_input,uiId)
    self.valid_commands_list = {}
    local count_found=0
    local found_command=0
    
    for c = 1, #self.commands_list do
        --check if we want used a client or shared command
        if(self.command_self == "a" or self.command_self == "b" or self.command_self == "c")then
            --debug(self.commands_list[c][1])
            if(self.commands_list[c][1] == "shard" and self.command_parameter_list[2] ~= "eScript")then
                command_are_allowed = 1
            elseif(self.commands_list[c][1] == "eScript")then
                if(self.command_parameter_list[2] == "eScript")then
                    command_are_allowed = 1
                else
                    command_are_allowed = 0
                end
            else
                command_are_allowed = 0
            end
        else
            if(self.commands_list[c][1] == "shard")then
                if(self.command_self == "?")then
                    command_are_allowed = 1
                else
                    command_are_allowed = 0
                end
            elseif(self.commands_list[c][1] == "eScript")then
                command_are_allowed = 0
            end
        end
        
        local command_are_allowed = 0
        if(self.commands_list[c][2] == "player")then
            command_are_allowed = 1
        else
            if(SearchCommand:check_prvis(self.commands_list[c][2]) == "true")then
                command_are_allowed = 1
            end
        end
        
        if(command_are_allowed == 1)then
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
    player_list = {}
    player_list[1] = "rias"
    player_list[2] = "uluk"
    player_list[3] = "riasan"
    player_list[4] = "Limay"
    player_list[5] = "Neira"
    player_list[6] = "Beastie"
    player_list[7] = "Audeva"
    player_list[8] = "Decacaon"
    player_list[9] = "Livege"
    player_list[10] = "Purg"
    player_list[11] = "Xxramusxx"
    player_list[12] = "Kronoss"
    player_list[13] = "Livan"
    player_list[14] = "Mifisto"
    player_list[15] = "Progulschik"
    player_list[16] = "Darwyn"
    player_list[17] = "Aprak"
    player_list[18] = "Dorothee"
    player_list[19] = "Zillah"

    self.valid_commands_list = {}
    local count_found=0
    local found_playername=0

    for c = 1, #player_list do
        if(playername_input ~= "")then
            if(string.lower(player_list[c]) == string.lower(playername_input))then
                found_playername=c
                count_found=1
            else
                if string.find(string.lower(player_list[c]), string.lower(playername_input))then
                    table.insert(self.valid_commands_list,player_list[c])
                    count_found=count_found+1
                end
            end
        end
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
                    if(pc > 1)then
                        argument_help=argument_help.."/"..self.commands_list[command_index][4+ac][pc][1]
                    else
                        argument_help=argument_help.." "..self.commands_list[command_index][4+ac][pc][1]
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
            
            argument_help=argument_help.." Warning to many Arguments"
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
    debug("write_command_help: "..text)
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
    menu:addLine(ucstring("Found: "..#self.valid_commands_list), "lua", "SearchCommand:close_modal('"..uiId.."')", "")
    
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

function SearchCommand:finish_commands(command_name,uiId)
    local process_status = SearchCommand:read_process_status(uiId)
    local input_search_string = getUI(uiId)
    local final_command = ""
    
    --debug("process_status: "..process_status)
    
    for fc = 1, process_status do
        if(fc == process_status)then
            self.command_parameter_list[fc] = command_name
        end
    end
    
    for pc = 1, #self.command_parameter_list do
        if(final_command == "")then
            final_command = self.command_parameter_list[pc]
        else
            final_command = final_command.." "..self.command_parameter_list[pc]
        end
    end
    
    input_search_string.input_string = "/"..final_command
    --debug("/"..final_command)
    
    input_search_string:setFocusOnText()
    
    SearchCommand:close_modal(uiId)
    SearchCommand:search(uiId)
end