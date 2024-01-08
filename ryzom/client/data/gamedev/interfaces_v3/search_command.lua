if not SearchCommand then
    --global SearchCommand class
	SearchCommand = {
	    identifier_found = {},
	    command_self = "",
	    command_parameter_list = {},
	    valid_commands_list = {},
	    commands_list = {},
	    key_tab_down = 0,
	    modal_open=0,
	    process_list={}
	}
end

--setup data
--commands_list[x] = {"type(client/shard)","priv(player/privs)","uitranslation for description","command", "parameter1(playername/text/number)", "parameter2(playername/text/number)" ..}
SearchCommand.commands_list = {}

local player_priv = isPlayerPrivilege()
if(player_priv)then
    table.insert(SearchCommand.commands_list,{"client", "player", "help_desc", "?", {{"text:<Command>",""}, {"all","help_all_desc"}}, {{"client/shard",""}}})
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
table.insert(SearchCommand.commands_list,{"shard", "privs", "b_desc", "b", {{"text:<Command>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "privs", "c_desc", "c", {{"text:<TargetName>",""}}, {{"text:<Command>",""}}})

table.insert(SearchCommand.commands_list,{"shard", "player", "showOnline_desc", "showOnline", {{"1","showOnline_1_desc"}, {"2","showOnline_2_desc"},{"0","showOnline_0_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "roomInvite_desc", "roomInvite", {{"text:<PlayerName>",""}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "setDontTranslateLangs_desc", "setDontTranslateLangs", {{"codelang|codelang","setDontTranslateLangs_codelang_desc"}}})
table.insert(SearchCommand.commands_list,{"shard", "player", "connectLangChannel_desc", "connectLangChannel",{{"fr",""},{"en",""},{"de",""},{"es",""},{"ru",""}}})
table.insert(SearchCommand.commands_list,{"shard", "privs", "Position_desc", "Position", {{"number:<posx>,<posy>[,<posz>]",""},{"text:<bot name>",""},{"text:<PlayerName>",""}}})
--END shard commands

function SearchCommand:find(tbl, value)
    for k, v in pairs(tbl) do
        if v == value then
            return k
        end
    end
    return nil
end

function SearchCommand:help_show_all(parameter)
    local player_priv = isPlayerPrivilege()
    
    debug("help parameter: "..parameter)
    
    if(parameter == "all")then
        displaySystemInfo(ucstring("######################## command help all #######################"), "AROUND")
    else
        displaySystemInfo(ucstring("######################## command help all / filter '"..parameter.."' #######################"), "AROUND")
    end
    
    for c = 1, #self.commands_list do
        local command_are_allowed = 0
        if(self.commands_list[c][2] == "player")then
            command_are_allowed = 1
        else
            if(player_priv)then
                command_are_allowed = 1
            end
        end
        
        if(command_are_allowed == 1)then
            if(self.commands_list[c][1] == parameter or parameter == "all")then
                local arg_display=""
                displaySystemInfo(ucstring(""), "AROUND")
                
                max_arguments = #self.commands_list[c] - 4
                for ad = 1, max_arguments do
                    if(ad > 1)then
                        arg_display=arg_display.." [arg"..ad.."]"
                    else
                        arg_display="[arg"..ad.."]"
                    end
                end
                
                displaySystemInfo(ucstring(c..".   "..self.commands_list[c][4].." "..arg_display.."    '"..self.commands_list[c][3].."'"), "AROUND")
                displaySystemInfo(ucstring("      type: "..self.commands_list[c][1]), "AROUND")
                
                for ac = 1, max_arguments do
                    displaySystemInfo(ucstring("      arg"..ac.." :"), "AROUND")
                    for pc = 1, #self.commands_list[c][4+ac] do
                        if(self.commands_list[c][4+ac][pc][2] == "")then
                            displaySystemInfo(ucstring("          "..self.commands_list[c][4+ac][pc][1]), "AROUND")
                        else
                            displaySystemInfo(ucstring("          "..self.commands_list[c][4+ac][pc][1].." '"..self.commands_list[c][4+ac][pc][2].."'"), "AROUND")
                        end
                    end
                end
                if(self.commands_list[c][4] ~= "a" and self.commands_list[c][4] ~= "b" and self.commands_list[c][4] ~= "c")then
                    if(self.commands_list[c][1] == "shard")then
                        displaySystemInfo(ucstring("      example: /a "..self.commands_list[c][4].." , /c riasan "..self.commands_list[c][4]), "AROUND")
                    end
                end
            end
        end
    end
    
    displaySystemInfo(ucstring(""), "AROUND")
    displaySystemInfo(ucstring("############################ end ##########################"), "AROUND")
end

function SearchCommand:help(uiId,input)
    --debug("search_input: "..input)
    
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
        --show help for command input
        local command_found = 0
        for c = 1, #self.commands_list do
            local command_are_allowed = 0
            if(self.commands_list[c][4] == command_split[1])then
                command_found = 1
                displaySystemInfo(ucstring("######################## command help "..command_split[1].." #######################"), "AROUND")
                
                local player_priv = isPlayerPrivilege()
                if(self.commands_list[c][2] == "player")then
                    command_are_allowed = 1
                else
                    if(player_priv)then
                        command_are_allowed = 1
                    end
                end
                
                if(command_are_allowed == 1)then
                    local arg_display=""
                    displaySystemInfo(ucstring(""), "AROUND")
                    
                    max_arguments = #self.commands_list[c] - 4
                    for ad = 1, max_arguments do
                        if(ad > 1)then
                            arg_display=arg_display.." [arg"..ad.."]"
                        else
                            arg_display="[arg"..ad.."]"
                        end
                    end
                    displaySystemInfo(ucstring("desc: "..self.commands_list[c][3]), "AROUND")
                    displaySystemInfo(ucstring("type: "..self.commands_list[c][1]), "AROUND")
                    displaySystemInfo(ucstring(""), "AROUND")
                    displaySystemInfo(ucstring(self.commands_list[c][4].." "..arg_display), "AROUND")
                    
                    for ac = 1, max_arguments do
                        displaySystemInfo(ucstring("    arg"..ac.." :"), "AROUND")
                        for pc = 1, #self.commands_list[c][4+ac] do
                            if(self.commands_list[c][4+ac][pc][2] == "")then
                                displaySystemInfo(ucstring("        "..self.commands_list[c][4+ac][pc][1]), "AROUND")
                            else
                                displaySystemInfo(ucstring("        "..self.commands_list[c][4+ac][pc][1].." '"..self.commands_list[c][4+ac][pc][2].."'"), "AROUND")
                            end
                        end
                    end
                    if(self.commands_list[c][4] ~= "a" and self.commands_list[c][4] ~= "b" and self.commands_list[c][4] ~= "c")then
                        if(self.commands_list[c][1] == "shard")then
                            displaySystemInfo(ucstring("      example: /a "..self.commands_list[c][4].." , /c riasan "..self.commands_list[c][4]), "AROUND")
                        end
                    end
                    displaySystemInfo(ucstring(""), "AROUND")
                    displaySystemInfo(ucstring("############################ end ############################"), "AROUND")
                end
            end
        end
        if(command_found == 0)then
            displaySystemInfo(ucstring("Command not found"), "AROUND")
        end
    end
end

function SearchCommand:check_autocomplet(uiId)
    --debug("try_autocomplte")
    if(self.modal_open==1)then
        if next(self.valid_commands_list)then
            --debug("check_autocomplet"..self.valid_commands_list[1])
            SearchCommand:finish_commands(self.valid_commands_list[1],uiId)
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
    
    is_tab_down = isTabDown()
    if(key_tab_down == 1)then
        key_tab_down = 0
        SearchCommand:check_autocomplet(uiId)
    elseif(is_tab_down)then
        key_tab_down = 0
        SearchCommand:check_autocomplet(uiId)
    end
    
    --check is input are empty if yes cancel all stuff
    local text_from_input = getUI(uiId)
    if(text_from_input.input_string == "")then
        SearchCommand:search(uiId)
    end
    
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
    is_tab_down = isTabDown()
    if(is_tab_down)then
        key_tab_down = 1
    end
    --trigger command by onchange a singel input
    
    self.command_parameter_list = {}
    local text_from_input = getUI(uiId)
    command_identifier = string.sub(text_from_input.input_string, 0, 1)

    --check if first char are a "/" from text_from_input
    if(command_identifier == "/")then
        --debug("identifier found")
        
        if(SearchCommand:find(self.identifier_found, uiId) == nil)then
            table.insert(self.identifier_found, uiId)
            
            local timer_str = "@UI:VARIABLES:CURRENT_SERVER_TICK"
            local timer_function_on_ui_window = getUI(uiId)
            addOnDbChange(timer_function_on_ui_window, timer_str, "SearchCommand:key_trigger('"..uiId.."')")
        end
        
        
        
        max_string_count = string.len(text_from_input.input_string)
        
        if(max_string_count == 1)then
            self.command_self=""
            SearchCommand:write_command_help(uiId,"/<command> or /? all")
            SearchCommand:close_modal()
            
            --update process_status
            SearchCommand:update_process_list(uiId,0)
        else
            local command_first = string.sub(text_from_input.input_string, 2, (max_string_count))
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

function SearchCommand:build_valid_command_list(command_input)
    self.valid_commands_list = {}
    local player_priv = isPlayerPrivilege()
    local count_found=0
    local found_command=0
    
    for c = 1, #self.commands_list do
        local command_are_allowed = 0
        if(self.commands_list[c][2] == "player")then
            command_are_allowed = 1
        else
            if(player_priv)then
                command_are_allowed = 1
            end
        end
        
        --check if we want used a client or shared command
        if(self.command_self == "a" or self.command_self == "b" or self.command_self == "c")then
            if(self.commands_list[c][1] == "shard")then
                command_are_allowed = 1
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
        SearchCommand:close_modal()
    end
    return found_command
end

function SearchCommand:build_valid_player_list(playername_input)
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
        SearchCommand:close_modal()
    end
    return found_playername
end

function SearchCommand:search_build_player_list(uiId,playername)
    local found_player=0
    local found_command=0
    found_player=SearchCommand:build_valid_player_list(playername)
    
    SearchCommand:search_build_argument_list(uiId,self.command_self)

    if(found_player ~= 0)then
        SearchCommand:close_modal()
    else
        if next(self.valid_commands_list) ~= nil then
            SearchCommand:show_more_options(uiId)
        end
    end
end

function SearchCommand:search_build_command_list(uiId,command,show_argument_help)
    local found_command=0
    --debug("search_build_command_list")
    
    found_command=SearchCommand:build_valid_command_list(command)

    if(found_command ~= 0)then
        SearchCommand:close_modal()
    else
        if next(self.valid_commands_list) ~= nil then
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
        if(string.find(string.lower(argu_name), string.lower("<command>")))then
            --debug("parm is a command")
            SearchCommand:search_build_command_list(uiId,self.command_parameter_list[process_status],false)
        elseif(string.find(string.lower(argu_name), string.lower("<PlayerName>")) or string.find(string.lower(argu_name), string.lower("<TargetName>")))then 
            --debug("parm is a playername")
            if(player_priv)then
                SearchCommand:search_build_player_list(uiId,self.command_parameter_list[process_status])
            else
                SearchCommand:search_build_argument_list(uiId,self.command_self)
            end
        else
            SearchCommand:search_build_argument_list(uiId,self.command_self)
        end
    end
end

function SearchCommand:write_command_help(uiId,text)
    --debug("write_command_help: "..text)
    local behind_help_text = getUI(uiId.."h")
    behind_help_text.input_string = text
end

function SearchCommand:write_command_help_clear(uiId)
    local behind_help_text = getUI(uiId.."h")
    behind_help_text.input_string = ""
end

function SearchCommand:close_modal()
    self.modal_open=0
    --debug("close_modal")
    runAH(nil, "leave_modal", "group=ui:interface:search_command_add_menu")
end

function SearchCommand:show_more_options(uiId)
    self.modal_open=1
    --debug("build_menu")
    launchContextMenuInGame("ui:interface:search_command_add_menu")
    menu = getUI("ui:interface:search_command_add_menu")
    
    menu:setMinW(85)
    menu:updateCoords()
    menu = menu:getRootMenu()
    menu:reset()
    
    menu:addLine(ucstring("Options..."), "", "", "")
    
    for c = 1, #self.valid_commands_list do
        menu:addLine(ucstring(self.valid_commands_list[c]), "lua", "SearchCommand:finish_commands('"..self.valid_commands_list[c].."','"..uiId.."')", "")
    end
end

function SearchCommand:finish_commands(command_name,uiId)
    local process_status=SearchCommand:read_process_status(uiId)
    local input_search_string = getUI(uiId)
    local final_command=""
    
    for fc = 1, process_status do
        if(fc == process_status)then
            self.command_parameter_list[fc]=command_name
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
    
    input_search_string:setFocusOnText()
    
    SearchCommand:close_modal()
    SearchCommand:search(uiId)
end