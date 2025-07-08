if not Ryzhide then
	-- global Ryzhide class
	Ryzhide = {
		main_window_name = "ui:interface:hide_n_hype_main",
		abort_window_name = "ui:interface:hide_n_hype_asked_abort",
		timer_str = "@UI:VARIABLES:CURRENT_SERVER_TICK",
		game_is_now_running = 0,
		pull_data = 10,
		request_server = 0,
		json_data_ready = 0,
		need_json_update = 1,
		hide_n_hide_json_data = nil,
		hidenhype_feedback = nil,
		load_animation_timer = 0,
		load_animation = 0,
		player_already_registerd = 0,
		game_is_full_loaded = 0,

		main_window_old_x = 0,
		main_window_old_y = 0,
		main_window_old_w = 0,
		main_window_old_h = 0,

		player_click_to_register = 0,
		register_player_online = 0,
		register_player_online_color = "255 0 0 255",
		regist_as_hunter = 0,
		regist_as_hunter_or_most_wanted = 0,
		regist_as_most_wanted = 0,
		regist_as_reject = 0,

		hunter_array = 0,
		hunter_or_most_wanted_array = 0,
		most_wanted_array = 0,
		reject_array = 0,

		reject_invite_round_id = 0,

		hunter_array_color = "255 0 0 255",
		hunter_or_most_wanted_array_color = "255 0 0 255",
		most_wanted_array_color = "255 0 0 255",
		reject_array_color = "0 255 0 255",

		hunter_accept_image = "",
		hunter_or_most_wanted_accept_image = "",
		most_wanted_accept_image = "",

		hunter_accept_active = "true",
		hunter_or_most_wanted_accept_active = "true",
		most_wanted_accept_active = "true",

		hunter_ok_false_image = "",
		hunter_or_most_wanted_ok_false_image = "",
		most_wanted_ok_false_image = "",

		current_round_start = 0,
		current_round_id = 0,

		player_name = "",

		json_pull_counter = 0,
		json_pull_counter_old = 0,
		manuell_action = 0,

		round_time_to_start = 0,
		spawn_a_object = 0,
		save_x_pos = 0,
		save_y_pos = 0,
		save_z_pos = 0,
		player_type = "",

		current_round_end = 0,
		timer_hint_1 = 0,
		timer_hint_2 = 0,
		timer_hint_3 = 0,

		hint_1_text = "????",
		hint_2_text = "????",
		hint_3_text = "????",
		hint_4_text = "????",
		pull_request_data = 0,

		reward_already_claimed = 0,
		closed_most_wanted_not_found_hunter = 0,
		closed_loos_hunter = 0,

		target_name_old = "",

		debug_window_border = 0,
		debug_enable_debug_message = 0,

		duration_time_invite = 120,
		duration_time_prepartion = 180,
		duration_time_game_running = 600,
		duration_time_hint_1 = 240,
		duration_time_hint_2 = 180,
		duration_time_hint_3 = 150,
		duration_time_claim_reward = 180,
		duration_time_most_wanted_offline = 180,
		needed_player_amount_to_start = 4
	}
end

function Ryzhide:display_debug_messanges(debug_message)
	if(self.debug_enable_debug_message == 1)then
		Ryzhide:display_message_to_player('sys', 'SYS', debug_message)
	end
end

function Ryzhide:set_debug_vars(vars_to_toggle)
	if(vars_to_toggle == "border")then
		if(self.debug_window_border == 0)then
			self.debug_window_border = 1
		else
			self.debug_window_border = 0
		end
		Ryzhide:display_message_to_player('sys', 'SYS', "set debug border to"..self.debug_window_border)
	end

	if(vars_to_toggle == "message")then
		if(self.debug_enable_debug_message == 0)then
			self.debug_enable_debug_message = 1
		else
			self.debug_enable_debug_message = 0
		end
		Ryzhide:display_message_to_player('sys', 'SYS', "set debug message to"..self.debug_enable_debug_message)
	end
end

function Ryzhide:update_timer(id,remaining_time_display,duration_display)
	local mainui = getUI(self.main_window_name)
	local timer_hard_text = mainui:find(id)

	local minutes = math.floor(remaining_time_display / 60)
	local seconds = remaining_time_display % 60

	timer_hard_text:find("text").hardtext = string.format("%02d:%02d", minutes, seconds)

	if(remaining_time_display <= duration_display * 0.5 and remaining_time_display > duration_display * 0.1)then
		--change coloro to orange by 50% or less but bigger as 10%
		timer_hard_text:find("text").color = "255 165 0 255"
	elseif(remaining_time_display <= duration_display * 0.1)then
		--change coloro to red by 10% or less
		timer_hard_text:find("text").color = "255 0 0 255"
	elseif(remaining_time_display > duration_display * 0.5)then
		--change coloro to green by more as 50%
		timer_hard_text:find("text").color = "0 185 0 255"
	end
end

function Ryzhide:convert_secound_to_string(secound_to_convert)
	if(secound_to_convert == nil)then
		return "00:00"
	end

	local minutes = math.floor(secound_to_convert / 60)
	local secs = secound_to_convert % 60
	return string.format("%02d:%02d", minutes, secs)
end

function Ryzhide:position_json_load_window()
	local json_load_window = getUI("ui:interface:load_pars_json")
	local mainui = getUI(self.main_window_name)

	main_window_x = mainui.x
	main_window_y = mainui.y
	main_window_h = mainui.h
	main_window_w = mainui.w

	if(main_window_h == 0 or main_window_w == 0)then
		local interface_window = getUI("ui:interface")

		self.main_window_old_x = 0
		self.main_window_old_y = 0
		self.main_window_old_w = 0
		self.main_window_old_h = 0

		json_load_window.x = interface_window.w - 36
		json_load_window.y = 36
	else
		if(self.main_window_old_x ~= main_window_x or self.main_window_old_y ~= main_window_y or self.main_window_old_h ~= main_window_h or self.main_window_old_w ~= main_window_w)then
			self.main_window_old_x = main_window_x
			self.main_window_old_y = main_window_y
			self.main_window_old_w = main_window_w
			self.main_window_old_h = main_window_h

			json_load_window.x = main_window_x + main_window_w
			json_load_window.y = main_window_y

			Ryzhide:display_debug_messanges("main_window_x: "..main_window_x.."main_window_y: "..main_window_y.."main_window_h: "..main_window_h.."main_window_w: "..main_window_w)
		end
	end

	if(self.load_animation_timer == 25)then
		self.load_animation_timer = 0
		if(self.load_animation == 0)then
			json_load_window:find("display_loading_img").texture="ico_time.tga"
			self.load_animation = 1
		elseif(load_animation == 1)then
			json_load_window:find("display_loading_img").texture="ico_time_right.tga"
			self.load_animation = 2
		elseif(load_animation == 2)then
			json_load_window:find("display_loading_img").texture="ico_time_buttom.tga"
			self.load_animation = 3
		else
			json_load_window:find("display_loading_img").texture="ico_time_left.tga"
			self.load_animation = 0
		end
	else
		self.load_animation_timer = self.load_animation_timer + 1
	end
end

function Ryzhide:start_fetch_json_data(url_to_fetch)
	Ryzhide:display_debug_messanges("start_fetch_data: "..url_to_fetch)
	self.pull_data = 10
	self.request_server = 0
	self.json_data_ready = 0
	self.load_animation = 0
	self.load_animation_timer = 0

	local load_pars_json_window_ui = getUI("ui:interface:load_pars_json")
	setOnDraw(load_pars_json_window_ui, "Ryzhide:fetch_json_data('"..url_to_fetch.."')")
	load_pars_json_window_ui.active=true
end

function Ryzhide:fetch_json_data(url_to_load)
	--run every 2 secound
	Ryzhide:position_json_load_window()
	if(self.pull_data == 10)then
		self.pull_data = 0
		if(self.request_server == 0)then
			self.request_server = 1
			self.json_data_ready = 0
			self.hidenhype_feedback = nil

			webig:openUrlInBg(url_to_load)
		end
	else
		self.pull_data = self.pull_data + 1
	end

	if(self.request_server == 1)then
		if(self.hidenhype_feedback ~= nil)then
			self.request_server = 0
			self.hide_n_hide_json_data = nil
			self.hide_n_hide_json_data = Json.decode(self.hidenhype_feedback)

			if (self.hide_n_hide_json_data) then
				if (self.hide_n_hide_json_data.error) then
					Ryzhide:display_debug_messanges("Load Json Process ' error: " .. self.hide_n_hide_json_data.error.."'")
					Ryzhide:display_debug_messanges("Load Json Process ' current_time: " .. self.hide_n_hide_json_data.current_time.."'")
				elseif (self.hide_n_hide_json_data.success) then
					Ryzhide:display_debug_messanges("Load Json Process 'success: " .. self.hide_n_hide_json_data.success.."'")
					Ryzhide:display_debug_messanges("Load Json Process 'current_time: " .. self.hide_n_hide_json_data.current_time.."'")
				else
					Ryzhide:display_debug_messanges("Load Json Process 'no error or success found'")
				end
				self.json_data_ready = 1
			else
				Ryzhide:display_debug_messanges("Load Json Process 'JSON-Decode failed'")
				self.json_data_ready = 0
			end
		end
	end

	if(self.json_data_ready == 1)then
		Ryzhide:close_window("ui:interface:load_pars_json", "")
	end
end

function Ryzhide:display_message_to_player(msg_type, msg_option, msg_text)
	local msg = ucstring()
	local msg_color = ""

	if(msg_type == "error")then
		--error red
		msg_color="F00F"
	elseif(msg_type == "sys")then
		msg_color="FFBF"
	else
		--other yellow
		msg_color="FF0F"
	end

	msg:fromUtf8("@{"..msg_color.."}"..msg_text)
	displaySystemInfo(msg, msg_option)
end

function Ryzhide:open_resize_window(window_id, win_h, win_w, render_html_content, close_function_id, close_function_parameter)
	local mainui = getUI(window_id)
	local mainui_html = mainui:find("html")
	local mainui_close_button = mainui:find("rightbut")

mainui.active = true

	if(mainui.active == false)then
		mainui.active = true
	end

	if(mainui.opened == false)then
		mainui.opened = true
	end

	if(close_function_id == "")then
		mainui_close_button.active = false
		mainui_close_button.onclick_l = ""
		mainui_close_button.params_l =  ""
	else
		mainui_close_button.active = true
		mainui_close_button.onclick_l = "lua"
		mainui_close_button.params_l = "Ryzhide:"..close_function_id.."("..close_function_parameter..")"
	end

	mainui.h = win_h
	mainui.w = win_w

	mainui_heade_open = mainui:find("header_opened")
	mainui_heade_open.h = 10
	mainui_heade_open.w = win_w

	mainui_html:renderHtml(render_html_content)
end

function Ryzhide:close_window(window_id, function_id)
	if(function_id == "abort_because_do_nothing" or function_id == "abort_because_in_team")then
		removeOnDbChange(getUI(self.main_window_name),self.timer_str)
	end

	if(getUI(window_id) ~= nil)then
		getUI(window_id).active=false
	end
end

function Ryzhide:click_close_button(window_id)
	if(window_id == "close_window_build_most_wanted_not_found_hunter")then
		self.closed_most_wanted_not_found_hunter = 1
	end

	if(window_id == "close_window_build_loos_hunter")then
		self.closed_loos_hunter = 1
	end

	if(window_id == "close_window_unregister_window")then
		removeOnDbChange(getUI(self.main_window_name),self.timer_str)
	end

	if(window_id == "close_window_register_window")then
		removeOnDbChange(getUI(self.main_window_name),self.timer_str)
	end

	if(getUI(self.main_window_name) ~= nil)then
		getUI(self.main_window_name).active=false
	end
end

function Ryzhide:load_translation(translation_var)
	return Ryzhide:htmlentities(translation_var)
end

function Ryzhide:htmlentities(text)
	local html_trans_content = ""
	local html_client_translation = ""

	html_client_translation = i18n.get(text):toUtf8()

	html_trans_content = html_client_translation:gsub("<NotExist:", "{")
	html_trans_content = html_trans_content:gsub(">", "}")
	html_trans_content = html_trans_content:gsub("'", "`")

	return html_trans_content
end

function Ryzhide:check_local_player_name()
	if(self.player_name == "" or self.player_name == nil)then
		self.player_name = getPlayerName()
	end
end

function Ryzhide:distance_calc(x1, y1, z1, x2, y2, z2)
	if(x1 == nil or y1 == nil or z1 == nil or x2 == nil or y2 == nil or z2 == nil)then
		Ryzhide:display_debug_messanges("calc_distand_a_nil_value")
		return 999
	end

	local dx = x2 - x1
	local dy = y2 - y1
	local dz = z2 - z1
	return math.sqrt(dx * dx + dy * dy + dz * dz)
end

function Ryzhide:check_have_team_member()
	local team_member_return = "false"

	local have_team_member = getUI("ui:interface:team_list_0")
	if(have_team_member.active ~= nil)then
		Ryzhide:display_debug_messanges("member: "..tostring(have_team_member.active))
		if(have_team_member.active ~= false)then
			Ryzhide:display_debug_messanges("player_join_a_team"..tostring(have_team_member.active))
			team_member_return = "true"
		end
	end

	return team_member_return
end

function Ryzhide:check_for_update_server_config(json_data_from_join)
	if(json_data_from_join == nil)then
		return
	end

	if(self.needed_player_amount_to_start ~= tonumber(json_data_from_join.needed_player_amount_to_start))then
		--load needed_player_amount_to_start from server
		self.needed_player_amount_to_start = tonumber(json_data_from_join.needed_player_amount_to_start)
	end

	--set timer times load from server
	if(self.duration_time_invite ~= tonumber(json_data_from_join.duration_time_invite))then
		self.duration_time_invite = tonumber(json_data_from_join.duration_time_invite)
	end
	if(self.duration_time_prepartion ~= tonumber(json_data_from_join.duration_time_prepartion))then
	self.duration_time_prepartion = tonumber(json_data_from_join.duration_time_prepartion)
	end
	if(self.duration_time_game_running ~= tonumber(json_data_from_join.duration_time_game_running))then
		self.duration_time_game_running = tonumber(json_data_from_join.duration_time_game_running)
	end
	if(self.duration_time_hint_1 ~= tonumber(json_data_from_join.duration_time_hint_1))then
		self.duration_time_hint_1 = tonumber(json_data_from_join.duration_time_hint_1)
	end
	if(self.duration_time_hint_2 ~= tonumber(json_data_from_join.duration_time_hint_2))then
		self.duration_time_hint_2 = tonumber(json_data_from_join.duration_time_hint_2)
	end
	if(self.duration_time_hint_3 ~= tonumber(json_data_from_join.duration_time_hint_3))then
		self.duration_time_hint_3 = tonumber(json_data_from_join.duration_time_hint_3)
	end
	if(self.duration_time_claim_reward ~= tonumber(json_data_from_join.duration_time_claim_reward))then
		self.duration_time_claim_reward = tonumber(json_data_from_join.duration_time_claim_reward)
	end
	if(self.duration_time_most_wanted_offline ~= tonumber(json_data_from_join.duration_time_most_wanted_offline))then
		self.duration_time_most_wanted_offline = tonumber(json_data_from_join.duration_time_most_wanted_offline)
	end
end

--###################################### START load and start at login function #######################################


function Ryzhide:check_data_at_login()
	local mainui = getUI(self.main_window_name)
	self.need_json_update = 1
	self.json_data_ready = 0

	addOnDbChange(mainui, self.timer_str, "Ryzhide:pars_json_data_login()")
end

function Ryzhide:pars_json_data_login()
	if(self.need_json_update == 1)then
		self.need_json_update = 0
		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=jsonData&need_json_data=login')
	end

	if(self.json_data_ready == 1)then
		if (self.hide_n_hide_json_data) then
			if (self.hide_n_hide_json_data.error) then
				Ryzhide:display_debug_messanges(self.hide_n_hide_json_data.error)

				--player is not registrated
				if(self.hide_n_hide_json_data.error == "not_registerd")then
					self.player_already_registerd = 0
				end

				Ryzhide:login_data_ok_process_error()

			elseif (self.hide_n_hide_json_data.success) then
				self.game_is_full_loaded = 1
				self.reward_already_claimed = 0

				--set flag player is registrated
				self.player_already_registerd = 1

				--load needed_player_amount_to_start from server
				self.needed_player_amount_to_start = tonumber(self.hide_n_hide_json_data.needed_player_amount_to_start)

				--set timer times load from server
				self.duration_time_invite = tonumber(self.hide_n_hide_json_data.duration_time_invite)
				self.duration_time_prepartion = tonumber(self.hide_n_hide_json_data.duration_time_prepartion)
				self.duration_time_game_running = tonumber(self.hide_n_hide_json_data.duration_time_game_running)
				self.duration_time_hint_1 = tonumber(self.hide_n_hide_json_data.duration_time_hint_1)
				self.duration_time_hint_2 = tonumber(self.hide_n_hide_json_data.duration_time_hint_2)
				self.duration_time_hint_3 = tonumber(self.hide_n_hide_json_data.duration_time_hint_3)
				self.duration_time_claim_reward = tonumber(self.hide_n_hide_json_data.duration_time_claim_reward)
				self.duration_time_most_wanted_offline = tonumber(self.hide_n_hide_json_data.duration_time_most_wanted_offline)

				Ryzhide:display_debug_messanges("ready: "..self.hide_n_hide_json_data.ready)
				Ryzhide:display_debug_messanges("player_round_id: "..self.hide_n_hide_json_data.player_round_id)
				Ryzhide:display_debug_messanges("round_id: "..self.hide_n_hide_json_data.round_id)

				--player already take rewards dont open the window again
				if(tonumber(self.hide_n_hide_json_data.ready) == 69 and tonumber(self.hide_n_hide_json_data.player_round_id) == tonumber(self.hide_n_hide_json_data.round_id))then
					self.reward_already_claimed = 1
				end

				--player reject play this round
				if(tonumber(self.hide_n_hide_json_data.ready) == 4 and tonumber(self.hide_n_hide_json_data.player_round_id) == tonumber(self.hide_n_hide_json_data.round_id))then
					self.reject_invite_round_id = tonumber(self.hide_n_hide_json_data.player_round_id)
					self.current_round_id = tonumber(self.hide_n_hide_json_data.round_id)
					Ryzhide:login_data_ok_process()
				end

				if(self.hide_n_hide_json_data.game_status == "asked_to_join")then
					 Ryzhide:open_ask_for_join_window(self.hide_n_hide_json_data.current_round_start, self.hide_n_hide_json_data.round_id)
				end

				if(self.hide_n_hide_json_data.game_status == "start_new_round")then
					if(tonumber(self.hide_n_hide_json_data.ready) == 0 and tonumber(self.hide_n_hide_json_data.player_round_id) == tonumber(self.hide_n_hide_json_data.round_id))then
						self.reject_invite_round_id = tonumber(self.hide_n_hide_json_data.player_round_id)
						self.current_round_id = tonumber(self.hide_n_hide_json_data.round_id)
						Ryzhide:login_data_ok_process()
					else
						Ryzhide:start_new_round(self.hide_n_hide_json_data.current_round_start, self.hide_n_hide_json_data.most_wanted, self.hide_n_hide_json_data.round_id)
					end
				end

				if(self.hide_n_hide_json_data.game_status == "game_is_running")then
					if(tonumber(self.hide_n_hide_json_data.ready) == 0 and tonumber(self.hide_n_hide_json_data.player_round_id) == tonumber(self.hide_n_hide_json_data.round_id))then
						self.reject_invite_round_id = tonumber(self.hide_n_hide_json_data.player_round_id)
						self.current_round_id = tonumber(self.hide_n_hide_json_data.round_id)
						Ryzhide:login_data_ok_process()
					else
						Ryzhide:start_the_hide_now(self.hide_n_hide_json_data.current_round_end, self.hide_n_hide_json_data.most_wanted, self.hide_n_hide_json_data.round_id)
					end
				end

				--status waiting, so close window and wait for global broadcatse
				if(self.hide_n_hide_json_data.game_status == "waiting")then
					Ryzhide:login_data_ok_process()
				end
			else
				Ryzhide:display_debug_messanges("no 'error' or 'success' found")
			end
		else
			Ryzhide:display_debug_messanges("JSON-Decode failed")
		end

		self.need_json_update = 1
	end
end

function Ryzhide:login_data_ok_process()
	local mainui = getUI(self.main_window_name)

	self.json_data_ready = 0

	Ryzhide:display_debug_messanges("Ryzhide success: "..self.hide_n_hide_json_data.success)
	Ryzhide:display_debug_messanges("Ryzhide game_status: "..self.hide_n_hide_json_data.game_status)
	Ryzhide:display_debug_messanges("Ryzhide round_id: "..self.hide_n_hide_json_data.round_id)

	removeOnDbChange(mainui, self.timer_str)

	mainui.active = false
end

function Ryzhide:login_data_ok_process_error()
	local mainui = getUI(self.main_window_name)
	removeOnDbChange(mainui, self.timer_str)
	mainui.active = false
	self.game_is_full_loaded = 1
end

--###################################### END load and start at login function #######################################


--###################################### START build and load registration window #######################################

function Ryzhide:build_register_window()
	local window_height = 290
	local window_width = 365

	local html_ungister = ""
	html_ungister=[[<title>]]..Ryzhide:load_translation("hide_n_hype_unregister_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype")..[[</h3></td>
			</tr>

			<tr>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_desc")..[[</td>
			</tr>

			<tr>
				<td>&nbsp;</td>
			</tr>

			<tr>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_asked_unregister")..[[</td>
			</tr>

			<tr>
				<td>&nbsp;</td>
			</tr>

			<tr>
				<td align="center"><div id="unregister_accept" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:unregister_accept;bg:ico_opening_hit.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_to_unregister")..[[;lua_function:click_icon_registration;'></div></td>
			</tr>

			<tr>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_unregister_now_and_close")..[[</td>
			</tr>
			</table>]]

	local html_register = ""
	html_register=[[<title>]]..Ryzhide:load_translation("hide_n_hype_register_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype")..[[</h3></td>
			</tr>

			<tr>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_desc")..[[</td>
			</tr>

			<tr>
				<td>&nbsp;</td>
			</tr>

			<tr>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_asked_register")..[[</td>
			</tr>

			<tr>
				<td>&nbsp;</td>
			</tr>

			<tr>
				<td align="center"><div id="register_accept" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:register_accept;bg:ico_dodge.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_to_register")..[[;lua_function:click_icon_registration;'></div></td>
			</tr>

			<tr>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_register_now")..[[</td>
			</tr>
			</table>]]

	if(self.player_already_registerd == 1)then
		Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_ungister, "click_close_button","close_window_unregister_window")
	else
		Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_register, "click_close_button","close_window_register_window")
	end
end

function Ryzhide:open_register_window()
	if(self.game_is_now_running == 1)then
		Ryzhide:display_message_to_player('error', 'AMB', Ryzhide:load_translation("hide_n_hype_cannot_open_reg_dereg_window_because_game_is_running"))
		return
	end

	local mainui = getUI(self.main_window_name)
	Ryzhide:build_register_window()

	self.json_data_ready = 0
	addOnDbChange(mainui, self.timer_str, "Ryzhide:pars_json_data_register()")
end

function Ryzhide:display_register_sucess(sucess_msg)
	self.json_data_ready = 0
	Ryzhide:display_message_to_player('info', 'AMB', sucess_msg)
end

function Ryzhide:display_register_error(error_msg)
	self.json_data_ready = 0

	--unlock register button again
	local mainui_group = getUI(self.main_window_name):find("register_accept")
	mainui_group:find("img1").texture = ""
	mainui_group:find("ctrl").active = true

	Ryzhide:display_message_to_player('error', 'AMB', error_msg)
end

function Ryzhide:display_unregister_error(error_msg)
	self.json_data_ready = 0

	--unlock register button again
	local mainui_group = getUI(self.main_window_name):find("unregister_accept")
	mainui_group:find("img1").texture = ""
	mainui_group:find("ctrl").active = true

	Ryzhide:display_message_to_player('error', 'AMB', error_msg)
end

function Ryzhide:pars_json_data_register()
	if(self.player_already_registerd == 0)then
		if(self.json_data_ready == 1)then
			if (self.hide_n_hide_json_data) then
				if (self.hide_n_hide_json_data.error) then
					if(self.hide_n_hide_json_data.error == "already_registerd")then
						Ryzhide:display_register_error(Ryzhide:load_translation("hide_n_hype_already_registerd"))
						self.player_already_registerd = 1
						Ryzhide:build_register_window()
						self.json_data_ready = 0
					elseif(self.hide_n_hide_json_data.error == "no_active_hide")then
						Ryzhide:display_register_error(Ryzhide:load_translation("hide_n_hype_no_active_hide"))
						self.player_already_registerd = 0
						Ryzhide:close_window(self.main_window_name, "")
						self.json_data_ready = 0
					else
						Ryzhide:display_register_error(self.hide_n_hide_json_data.error)
					end
				elseif (self.hide_n_hide_json_data.success) then
					self.player_already_registerd = 1
					Ryzhide:display_register_sucess(Ryzhide:load_translation("hide_n_hype_reg_sucesfully"))
					Ryzhide:build_register_window()
					Ryzhide:check_data_at_login()
					self.json_data_ready = 0
				else
					Ryzhide:display_register_error("no 'error' or 'success' found")
				end
			else
				Ryzhide:display_register_error("JSON-Decode failed")
			end
		end
	else
		if(self.json_data_ready == 1)then
			if (self.hide_n_hide_json_data) then
				if (self.hide_n_hide_json_data.error) then
					if(self.hide_n_hide_json_data.error == "not_registerd")then
						Ryzhide:display_unregister_error(Ryzhide:load_translation("hide_n_hype_not_registerd"))
						self.player_already_registerd = 0
						self.json_data_ready = 0
						Ryzhide:build_register_window()
					elseif(self.hide_n_hide_json_data.error == "no_active_hide")then
						Ryzhide:display_unregister_error(Ryzhide:load_translation("hide_n_hype_no_active_hide"))
						self.player_already_registerd = 0
						self.json_data_ready = 0
						Ryzhide:close_window(self.main_window_name, "")
					else
						Ryzhide:display_unregister_error(self.hide_n_hide_json_data.error)
					end
				elseif (self.hide_n_hide_json_data.success) then
					Ryzhide:display_register_sucess(Ryzhide:load_translation("hide_n_hype_unreg_sucesfully"))
					self.player_already_registerd = 0
					self.json_data_ready = 0
					Ryzhide:build_register_window()
				else
					Ryzhide:display_unregister_error("no 'error' or 'success' found")
				end
			else
				Ryzhide:display_unregister_error("JSON-Decode failed")
			end
		end
	end
end


function Ryzhide:click_icon_registration(id)
	local mainui = getUI(self.main_window_name)

	if(id == "register_accept")then
		Ryzhide:display_debug_messanges("start_register_now")
		Ryzhide:start_fetch_json_data("https://app.ryzom.com/app_ryzhide/?mode=register")
	end

	if(id == "unregister_accept")then
		Ryzhide:build_ask_for_abort_unregister()
	end

	local mainui_group = getUI(self.main_window_name):find(id)
	local image_ui_controll = mainui_group:find("img1").texture
	if(image_ui_controll == "")then
		mainui_group:find("img1").texture = "curs_rotate.png"
		mainui_group:find("ctrl").active = false
	end
end

--###################################### END build and load registration window #######################################


--###################################### START build and load invite window #######################################
function Ryzhide:build_invite_window()
	local window_height = 420
	local window_width = 400


	local html_invite_window=""
	html_invite_window=[[<title>]]..Ryzhide:load_translation("hide_n_hype_a_new_hide_start_soon_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
				<td align="center" colspan="2"><div id="hide_n_hype_timer_invite" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_invite;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_invite)..[[;'></div></td>
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
			</tr>

			<tr>
				<td colspan="4" align="center"><font color="orange">]]..Ryzhide:load_translation("hide_n_hype_forbidden_regions")..[[</font></td>
			</tr>

			<tr>
				<td>&nbsp;</td>
				<td colspan="2" align="center">]]..Ryzhide:load_translation("hide_n_hype_currently_registerd_player_online")..[[</td>
				<td>&nbsp;</td>
			</tr>

			<tr>
				<td>&nbsp;</td>
				<td colspan="2" align="center"><div id="player_online_text" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:player_online_text;font_size:18;text_color:]]..self.register_player_online_color..[[;hardtext:]]..self.register_player_online..[[;'></div></td>
				<td>&nbsp;</td>
			</tr>

			<tr>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_hunter")..[[</td>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_hunter_and_most_wanted")..[[</td>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_most_wanted")..[[</td>
				<td align="center">]]..Ryzhide:load_translation("hide_n_hype_reject")..[[</td>
			</tr>

			<tr>
				<td colspan="4">&nbsp;</td>
			</tr>

			<tr>
				<td align="center"><div id="hunte_accept" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:hunte_accept;bg:tag_rpbg8.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_to_hunter")..[[;lua_function:click_icon_ask_for_join;img1:]]..self.hunter_accept_image..[[;ctrl_active:]]..self.hunter_accept_active..[[;'></div></td>
				<td align="center"><div id="hunter_or_mostwanted_accept" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:hunter_or_mostwanted_accept;bg:ico_feint.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_to_hunter_or_mostwanted")..[[;lua_function:click_icon_ask_for_join;img1:]]..self.hunter_or_most_wanted_accept_image..[[;ctrl_active:]]..self.hunter_or_most_wanted_accept_active..[[;'></div></td>
				<td align="center"><div id="mostwanted_accept" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:mostwanted_accept;bg:ico_dodge.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_to_mostwanted")..[[;lua_function:click_icon_ask_for_join;img1:]]..self.most_wanted_accept_image..[[;ctrl_active:]]..self.most_wanted_accept_active..[[;'></div></td>
				<td align="center"><div id="reject_accept" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:reject_accept;bg:ico_opening_hit.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_to_Reject")..[[;lua_function:click_icon_ask_for_join;'></div></td>
			</tr>

			<tr>
				<td colspan="4">&nbsp;</td>
			</tr>

			<tr>
				<td align="center"><div id="hunte_text" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hunte_text;font_size:13;text_color:]]..self.hunter_array_color..[[;hardtext:1 / ]]..self.hunter_array..[[;'></div></td>
				<td align="center"><div id="hunter_or_mostwanted_text" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hunte_text;font_size:13;text_color:]]..self.hunter_or_most_wanted_array_color..[[;hardtext:1 / ]]..self.hunter_or_most_wanted_array..[[;'></div></td>
				<td align="center"><div id="mostwanted_text" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hunte_text;font_size:13;text_color:]]..self.most_wanted_array_color..[[;hardtext:1 / ]]..self.most_wanted_array..[[;'></div></td>
				<td align="center"><div id="reject_text" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hunte_text;font_size:13;text_color:]]..self.reject_array_color..[[;hardtext:]]..self.reject_array..[[;'></div></td>
			</tr>
			<tr>
				<td align="center"><div id="hunte_display_status" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_only_images;id:hunte_display_status;img1:]]..self.hunter_ok_false_image..[[;tooltip:]]..Ryzhide:load_translation("hide_n_hype_not_enough")..[[;'></div></td>
				<td align="center"><div id="hunter_or_mostwanted_display_status" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_only_images;id:hunter_or_mostwanted_display_status;img1:]]..self.hunter_or_most_wanted_ok_false_image..[[;tooltip:]]..Ryzhide:load_translation("hide_n_hype_not_enough")..[[;'></div></td>
				<td align="center"><div id="mostwanted_display_status" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_only_images;id:mostwanted_display_status;img1:]]..self.most_wanted_ok_false_image..[[;tooltip:]]..Ryzhide:load_translation("hide_n_hype_not_enough")..[[;'></div></td>
				<td align="center">&nbsp;</td>
			</tr>
			</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_invite_window, "", "")
end


function Ryzhide:build_wait_start_window()
	local window_height = 100
	local window_width = 350


	local html_wait_for_start=""
	html_wait_for_start=[[<title>....?!</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><h2>]]..Ryzhide:load_translation("hide_n_hype_wait_for_game_start")..[[</h2></td>
			</tr>
			</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_wait_for_start, "", "")
end

function Ryzhide:open_ask_for_join_window(current_round_time,current_round)
	if(self.game_is_full_loaded == 0)then
		Ryzhide:display_debug_messanges("game_not_loaded -- open_ask_for_join_window")
		return
	end

	if(self.player_already_registerd == 0)then
		return
	end

	self.current_round_id = tonumber(current_round)
	self.game_is_now_running = 1

	Ryzhide:display_debug_messanges("asked_join_id: "..current_round)
	Ryzhide:display_debug_messanges("reject_invite_round_id: "..self.reject_invite_round_id)

	if(tonumber(self.current_round_id) == tonumber(self.reject_invite_round_id))then
		Ryzhide:display_debug_messanges("player_already_reject_dont_open_window_again")
		return
	else
		self.reject_invite_round_id = 0
	end

	DynE.otherMapPoints["hidenhype_hunter"] = {}
	delArkPoints()
	DynE:AddOtherMapPoints()

	local mainui = getUI(self.main_window_name)

	self.player_click_to_register = 0
	self.need_json_update = 1
	self.json_data_ready = 0
	self.current_round_start = 0
	self.json_pull_counter = 0
	self.json_pull_counter_old = 0
	self.reward_already_claimed = 0
	self.manuell_action = 0

	self.hint_1_text = "????"
	self.hint_2_text = "????"
	self.hint_3_text = "????"
	self.hint_4_text = "????"

	Ryzhide:check_local_player_name()

	addOnDbChange(mainui, self.timer_str, "Ryzhide:pars_json_data_ask_for_join()")
end

function Ryzhide:display_asked_for_join_sucess(sucess_msg)
	self.json_data_ready = 0
	Ryzhide:display_message_to_player('info', 'AMB', sucess_msg)
end

function Ryzhide:display_asked_for_join_error(error_msg)
	self.json_data_ready = 0
	Ryzhide:display_message_to_player('error', 'AMB', error_msg)
end

function Ryzhide:invite_timer_stopped()
	removeOnDbChange(getUI(self.main_window_name),self.timer_str)

	Ryzhide:display_debug_messanges("Timer_is_over!")
	self.need_json_update = 1
	self.json_data_ready = 0
	self.current_round_start = 0

	if(self.player_click_to_register == 0)then
		Ryzhide:close_window(self.main_window_name, "abort_because_do_nothing")
		self.reject_invite_round_id = tonumber(self.current_round_id)
		return
	else
		Ryzhide:build_wait_start_window()
	end
end

function Ryzhide:pars_json_data_ask_for_join()
		if(self.need_json_update == 1)then
			self.need_json_update = 0
			Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=jsonData&need_json_data=asked_to_join')
		end

		if(self.current_round_start ~= 0 and self.json_pull_counter > 2)then
			local remaining_time = self.current_round_start - os.time()

			if(remaining_time < 0)then
				Ryzhide:invite_timer_stopped()
			else
				Ryzhide:update_timer("hide_n_hype_timer_invite",remaining_time, self.duration_time_invite)
			end
		end

		if(self.json_pull_counter_old < self.json_pull_counter and self.manuell_action == 1)then
			self.manuell_action = 0
		end

		if(self.json_data_ready == 1)then
			if (self.hide_n_hide_json_data) then
				if (self.hide_n_hide_json_data.error) then
					if(self.hide_n_hide_json_data.error == "not_registerd")then
						Ryzhide:display_asked_for_join_error(Ryzhide:load_translation("hide_n_hype_not_registerd"))
					elseif(self.hide_n_hide_json_data.error == "not_allow_again")then
						Ryzhide:display_asked_for_join_error(Ryzhide:load_translation("hide_n_hype_not_allow_again"))
					else
						Ryzhide:display_asked_for_join_error(Ryzhide:load_translation("hide_n_hype_"..self.hide_n_hide_json_data.error))
					end

					self.json_data_ready = 0
					self.manuell_action = 0
				elseif (self.hide_n_hide_json_data.success) then
					if(self.hide_n_hide_json_data.success == "asked_to_join")then
						--check we need update timed and needed player from server
						Ryzhide:check_for_update_server_config(self.hide_n_hide_json_data)

						self.json_pull_counter = self.json_pull_counter + 1
						--Ryzhide:display_asked_for_join_sucess(self.hide_n_hide_json_data.success)
						self.json_data_ready = 0

						if(self.json_pull_counter == 1)then
							Ryzhide:inti_invite_window(self.hide_n_hide_json_data)
						elseif(self.json_pull_counter == 2)then
							Ryzhide:build_invite_window()
						elseif(self.json_pull_counter > 2)then
							if(self.manuell_action == 0)then
								Ryzhide:update_invite_window(self.hide_n_hide_json_data)
							end
						end
					elseif(self.hide_n_hide_json_data.success == "ok_player_accept")then
						local message_as = ""
						if(self.hide_n_hide_json_data.accept_as == "hunter")then
							message_as = "hide_n_hype_hunter"
						end
						if(self.hide_n_hide_json_data.accept_as == "hunter_and_most_wanted")then
							message_as = "hide_n_hype_hunter_and_most_wanted"
						end
						if(self.hide_n_hide_json_data.accept_as == "most_wanted")then
							message_as = "hide_n_hype_most_wanted"
						end
						if(self.hide_n_hide_json_data.accept_as == "reject")then
							message_as = "hide_n_hype_reject"
						end
						local message = Ryzhide:load_translation("hide_n_hype_ok_player_accept").." : "..Ryzhide:load_translation(message_as)
						Ryzhide:display_asked_for_join_sucess(message)
						self.manuell_action = 0
						self.json_data_ready = 0
					end
				else
					Ryzhide:display_asked_for_join_error("no 'error' or 'success' found")
				end
			else
				Ryzhide:display_asked_for_join_error("JSON-Decode failed")
			end

			self.need_json_update = 1
		end
end

function Ryzhide:inti_invite_window(json_data)
	Ryzhide:display_debug_messanges("inti_invite_window: "..json_data.current_round_start)
	self.register_player_online = json_data.reg_online_player

	local found_hunter = 0
	local found_most_wanted_hunter = 0
	local found_most_wanted = 0

	if(tonumber(self.register_player_online) >= 3)then
		--set to green all ok
		self.register_player_online_color = "0 255 0 255"
	else
		--set to red all ok
		self.register_player_online_color = "255 0 0 255"
	end

	Ryzhide:check_local_player_name()

	if(json_data.current_round_start == "0000-00-00 00:00:00")then
		self.current_round_start = 0
		self.json_pull_counter = 0
	else
		self.current_round_start = json_data.current_round_start
	end

	if (type(json_data.hunter_list) == "boolean") then
		self.regist_as_hunter = 0
		self.hunter_accept_image = ""
	else
		self.hunter_array = 0
		for _, name in ipairs(json_data.hunter_list) do
			self.hunter_array = self.hunter_array + 1
			if(self.player_name == name)then
				found_hunter = 1
				self.regist_as_hunter = 1
				self.hunter_accept_image = "rap_invited_no_dm.tga"
				self.hunter_accept_active = "false"
			end
		end
	end

	if(found_hunter == 0)then
		self.hunter_accept_image = ""
		self.hunter_accept_active = "true"
	end

	if(self.hunter_array == 0)then
		self.hunter_ok_false_image = "w_answer_16_cancel.png"
		self.hunter_array_color = "255 0 0 255"
	else
		self.hunter_ok_false_image = "rap_invited_no_dm.png"
		self.hunter_array_color = "0 255 0 255"
	end

	if (type(json_data.hunter_and_most_wanted_list) == "boolean") then
		self.regist_as_hunter_or_most_wanted = 0
		self.hunter_or_most_wanted_accept_image = ""
	else
		self.hunter_or_most_wanted_array = 0
		for _, name in ipairs(json_data.hunter_and_most_wanted_list) do
			self.hunter_or_most_wanted_array = self.hunter_or_most_wanted_array + 1
			if(self.player_name == name)then
				found_most_wanted_hunter = 1
				self.regist_as_hunter_or_most_wanted = 1
				self.hunter_or_most_wanted_accept_image = "rap_invited_no_dm.tga"
				self.hunter_or_most_wanted_accept_active = "false"
			end
		end
	end

	if(found_most_wanted_hunter == 0)then
		self.hunter_or_most_wanted_accept_image = ""
		self.hunter_or_most_wanted_accept_active = "true"
	end

	if(self.hunter_or_most_wanted_array == 0)then
		self.hunter_or_most_wanted_ok_false_image = "w_answer_16_cancel.png"
		self.hunter_or_most_wanted_array_color = "255 0 0 255"
	else
		self.hunter_or_most_wanted_ok_false_image = "rap_invited_no_dm.png"
		self.hunter_or_most_wanted_array_color = "0 255 0 255"
	end

	if (type(json_data.most_wanted_list) == "boolean") then
		self.regist_as_most_wanted = 0
		self.most_wanted_accept_image = ""
	else
		self.most_wanted_array = 0
		for _, name in ipairs(json_data.most_wanted_list) do
			self.most_wanted_array = self.most_wanted_array + 1
			if(self.player_name == name)then
				found_most_wanted = 1
				self.regist_as_most_wanted = 1
				self.most_wanted_accept_image = "rap_invited_no_dm.tga"
				self.most_wanted_accept_active = "false"
			end
		end
	end

	if(found_most_wanted == 0)then
		self.most_wanted_accept_image = ""
		self.most_wanted_accept_active = "true"
	end

	if(self.most_wanted_array == 0)then
		self.most_wanted_ok_false_image = "w_answer_16_cancel.png"
		self.most_wanted_array_color = "255 0 0 255"
	else
		self.most_wanted_ok_false_image = "rap_invited_no_dm.png"
		self.most_wanted_array_color = "0 255 0 255"
	end

	if (type(json_data.reject_list) == "boolean") then
		self.regist_as_reject = 0
	else
		self.reject_array = 0
		for _, name in ipairs(json_data.reject_list) do
			self.reject_array = self.reject_array + 1
			if(self.player_name == name)then
				self.regist_as_reject = 1
			end
		end
	end

	if(self.reject_array == 0)then
		self.reject_array_color = "0 255 0 255"
	else
		self.reject_array_color = "255 0 0 255"
	end
end

function Ryzhide:update_invite_window(json_data)
	Ryzhide:display_debug_messanges("update_invite_window: "..json_data.current_round_start)

	if(json_data.current_round_start == "0000-00-00 00:00:00")then
		self.current_round_start = 0
		self.json_pull_counter = 0
	else
		self.current_round_start = json_data.current_round_start
	end

	local mainui = getUI(self.main_window_name)

	local player_online_text = mainui:find("player_online_text")
	player_online_text:find("text").hardtext = json_data.reg_online_player

	Ryzhide:display_debug_messanges("reg_player: "..tonumber(json_data.reg_online_player).." needed_player: "..tonumber(self.needed_player_amount_to_start))

	if(tonumber(json_data.reg_online_player) >= tonumber(self.needed_player_amount_to_start))then
		--set to green all ok
		Ryzhide:display_debug_messanges("set_to_green")
		player_online_text:find("text").color = "0 255 0 255"
	else
		--set to red not ok
		Ryzhide:display_debug_messanges("set_to_red")
		player_online_text:find("text").color = "255 0 0 255"
	end

	Ryzhide:check_and_pars_player_infos(json_data.hunter_list,"hunte_accept","hunte_text","hunte_display_status")
	Ryzhide:check_and_pars_player_infos(json_data.hunter_and_most_wanted_list,"hunter_or_mostwanted_accept","hunter_or_mostwanted_text","hunter_or_mostwanted_display_status")
	Ryzhide:check_and_pars_player_infos(json_data.most_wanted_list,"mostwanted_accept","mostwanted_text","mostwanted_display_status")
	Ryzhide:check_and_pars_player_infos(json_data.reject_list,"reject_accept","reject_text","none")
end

function Ryzhide:check_and_pars_player_infos(json_data,action_button,action_count_text,action_ok_image)
	local mainui = getUI(self.main_window_name)
	local count_array = 0

	local found_player_in_list = 0

	Ryzhide:check_local_player_name()

	if (type(json_data) == "boolean") then
		local regist_hunter_img = mainui:find(action_button)
		regist_hunter_img:find("img1").texture = ""
		regist_hunter_img:find("ctrl").active = true

		local regist_hunter_text = mainui:find(action_count_text)
		if(action_count_text == "reject_text")then
			regist_hunter_text:find("text").hardtext = "0"
			regist_hunter_text:find("text").color = "0 255 0 255"
		else
			regist_hunter_text:find("text").hardtext = "1 / 0"
			regist_hunter_text:find("text").color = "255 0 0 255"
		end
	else
		count_array = 0
		local regist_hunter_img = mainui:find(action_button)

		for _, name in ipairs(json_data) do
			count_array = count_array + 1
			if(self.player_name == name)then
				found_player_in_list = 1
				Ryzhide:display_debug_messanges("player_name: "..self.player_name.." array_name: "..name)
				regist_hunter_img:find("img1").texture = "rap_invited_no_dm.tga"
				regist_hunter_img:find("ctrl").active = false
			end
		end
	end

	if(found_player_in_list == 0)then
		local regist_hunter_img = mainui:find(action_button)
		regist_hunter_img:find("img1").texture = ""
		regist_hunter_img:find("ctrl").active = true
	end

	if(count_array == 0)then
		if(action_ok_image ~= "none")then
			local regist_hunter_ok_img = mainui:find(action_ok_image)
			regist_hunter_ok_img:find("img1").texture = "w_answer_16_cancel.png"
			regist_hunter_ok_img:find("ctrl").tooltip = Ryzhide:load_translation("hide_n_hype_not_enough")
		end
	else
		local regist_hunter_text = mainui:find(action_count_text)
		if(action_count_text == "reject_text")then
			regist_hunter_text:find("text").hardtext = count_array
		else
			regist_hunter_text:find("text").hardtext = "1 / "..count_array
		end

		if(action_count_text == "reject_text")then
			regist_hunter_text:find("text").color = "255 0 0 255"
		else
			regist_hunter_text:find("text").color = "0 255 0 255"
		end

		if(action_ok_image ~= "none")then
			local regist_hunter_ok_img = mainui:find(action_ok_image)
			regist_hunter_ok_img:find("img1").texture = "rap_invited_no_dm.png"
			regist_hunter_ok_img:find("ctrl").tooltip = Ryzhide:load_translation("hide_n_hype_ok_enough_player")
		end
	end
end

function Ryzhide:click_icon_ask_for_join(id)
	local have_team_member = Ryzhide:check_have_team_member()
	if(have_team_member == "true" and id ~= "reject_accept" and id ~= "hunte_accept")then
		Ryzhide:display_asked_for_join_error(Ryzhide:load_translation("hide_n_hype_you_cannot_be_in_team"))
		return
	end

	if(id == "hunte_accept")then
		self.player_click_to_register = 1
		self.manuell_action = 1
		self.json_pull_counter_old = self.json_pull_counter + 3
		self.need_json_update = 0
		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=accept_invite&accept_as=hunter')
	end

	if(id == "hunter_or_mostwanted_accept")then
		self.player_click_to_register = 1
		self.manuell_action = 1
		self.json_pull_counter_old = self.json_pull_counter + 3
		self.need_json_update = 0
		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=accept_invite&accept_as=hunter_and_most_wanted')
	end

	if(id == "mostwanted_accept")then
		self.player_click_to_register = 1
		self.manuell_action = 1
		self.json_pull_counter_old = self.json_pull_counter + 3
		self.need_json_update = 0
		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=accept_invite&accept_as=most_wanted')
	end

	if(id == "reject_accept")then
		Ryzhide:build_ask_for_abort_skip_game()
	end

	local mainui_group = getUI(self.main_window_name):find(id)
	local image_ui_controll = mainui_group:find("img1").texture
	if(image_ui_controll == "")then
		mainui_group:find("img1").texture = "curs_rotate.png"
		mainui_group:find("ctrl").active = false
	end
end

--###################################### END build and load invite window #######################################




--###################################### START build and load prepartion window #######################################


function Ryzhide:start_new_round(time_to_start, most_wanted_name, current_round_id)
	if(self.game_is_full_loaded == 0)then
		Ryzhide:display_debug_messanges("game_not_loaded -- start_new_round")
		return
	end

	if(self.player_already_registerd == 0)then
		return
	end

	if(tonumber(self.reject_invite_round_id) == tonumber(current_round_id))then
		return
	end

	local mainui = getUI(self.main_window_name)

	Ryzhide:check_local_player_name()

	Ryzhide:display_debug_messanges("time_to_start"..time_to_start)
	Ryzhide:display_debug_messanges("most_wanted_name"..most_wanted_name)

	self.round_time_to_start = time_to_start

	if(self.player_name == most_wanted_name)then
		self.player_type = "most_wanted"
		Ryzhide:display_debug_messanges("You must hide!")
		Ryzhide:build_most_wanted_window()
	else
		self.player_type = "hunter"
		Ryzhide:display_debug_messanges("You are hunter wait for most wanted to hide!")
		Ryzhide:build_hunter_window()
	end

	addOnDbChange(mainui, self.timer_str, "Ryzhide:pars_json_data_wait_for_most_wanted()")
end

function Ryzhide:wait_most_wanted_timer_stopped()
	local mainui = getUI(self.main_window_name)
	removeOnDbChange(mainui, self.timer_str)

	Ryzhide:build_wait_start_window()

	if(self.player_type == "most_wanted")then
		if(self.spawn_a_object == 2)then
			Ryzhide:display_debug_messanges("mostwanted_must_hidden_now_start")
		else
			Ryzhide:display_debug_messanges("error_bad_position")
		end
	end
end

function Ryzhide:pars_json_data_wait_for_most_wanted()
	if(self.player_type == "most_wanted")then
		local have_team_member = Ryzhide:check_have_team_member()
		if(have_team_member == "true")then
			Ryzhide:display_asked_for_join_error(Ryzhide:load_translation("hide_n_hype_you_cannot_be_in_team"))
			if(self.spawn_a_object == 2)then
				Ryzhide:despawn_object(self.save_x_pos, self.save_y_pos, "in_team")
			else
				Ryzhide:despawn_object(0, 0, "in_team")
			end
			Ryzhide:close_window(self.main_window_name, "abort_because_in_team")
		end
	end

	local mainui = getUI(self.main_window_name)

	if(self.round_time_to_start ~= 0)then
		local remaining_time = self.round_time_to_start - os.time()

		if(remaining_time < 0)then
			Ryzhide:wait_most_wanted_timer_stopped()
		else
			Ryzhide:update_timer("hide_n_hype_timer_hunter",remaining_time, self.duration_time_prepartion)
		end
	else
		Ryzhide:wait_most_wanted_timer_stopped()
	end

	if(self.player_type == "most_wanted")then
		if(self.spawn_a_object == 0)then
			local regist_hunter_text = mainui:find("ready_most_wanted_text")
			regist_hunter_text:find("text").hardtext = Ryzhide:load_translation("hide_n_hype_not_ready")
			regist_hunter_text.w = "250"
			regist_hunter_text:find("text").color = "255 0 0 255"

			local regist_hunter_img = mainui:find("display_most_wanted_status")
			regist_hunter_img:find("img1").texture = "w_answer_16_cancel.tga"
			regist_hunter_img:find("ctrl").tooltip = Ryzhide:load_translation("hide_n_hype_not_ready")
		end

		if(self.spawn_a_object == 1)then
			local regist_hunter_text = mainui:find("ready_most_wanted_text")
			regist_hunter_text:find("text").hardtext = Ryzhide:load_translation("hide_n_hype_ready_wait_for_countdown")
			regist_hunter_text.w = "250"
			regist_hunter_text:find("text").color = "0 255 0 255"

			local regist_hunter_img = mainui:find("display_most_wanted_status")
			regist_hunter_img:find("img1").texture = "rap_invited_no_dm.tga"
			regist_hunter_img:find("ctrl").tooltip = Ryzhide:load_translation("hide_n_hype_ready_wait_for_countdown")

			self.spawn_a_object = 2
			self.save_x_pos,self.save_y_pos,self.save_z_pos = getPlayerPos()
		end

		if(self.spawn_a_object == 2)then
			local x,y,z = getPlayerPos()
			local current_distance = Ryzhide:distance_calc(x, y, z, self.save_x_pos, self.save_y_pos, self.save_z_pos)
			if(current_distance > 1)then
				Ryzhide:display_debug_messanges("you_moved!!!!")

				Ryzhide:despawn_object(self.save_x_pos, self.save_y_pos, "")

				local regist_hunter_text = mainui:find("ready_most_wanted_text")
				regist_hunter_text:find("text").hardtext = Ryzhide:load_translation("hide_n_hype_not_ready")
				regist_hunter_text.w = "250"
				regist_hunter_text:find("text").color = "255 0 0 255"

				local regist_hunter_img = mainui:find("display_most_wanted_status")
				regist_hunter_img:find("img1").texture = "w_answer_16_cancel.tga"
				regist_hunter_img:find("ctrl").tooltip = Ryzhide:load_translation("hide_n_hype_not_ready")

				self.spawn_a_object = 0
			end
		end
	end
end

function Ryzhide:build_hunter_window()
	local window_height = 170
	local window_width = 365


	local html_hunter=""
	html_hunter=[[<title>]]..Ryzhide:load_translation("hide_n_hype_close_your_eyes_and_count_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
				<td align="center"><div id="hide_n_hype_timer_hunter" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_hunter;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_prepartion)..[[;'></div></td>
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
			</tr>

			<tr>
				<td colspan="3" align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_wait_for_most_wanted")..[[</h3></td>
			</tr>
			</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_hunter, "", "")
end

function Ryzhide:build_most_wanted_window()
	local window_height = 530
	local window_width = 415
	local table_width = window_width + 15


	local html_most_wanted=""
	html_most_wanted=[[<title>]]..Ryzhide:load_translation("hide_n_hype_go_and_hide_fast")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
				<td align="center" width="180"><div id="hide_n_hype_timer_hunter" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_hunter;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_prepartion)..[[;'></div></td>
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
			</tr>

			<tr>
				<td align="center" colspan="3"><h3>]]..Ryzhide:load_translation("hide_n_hype_follow_this_rules")..[[</h3></td>
			</tr>

			<tr>
				<td align="center" colspan="3"><h4>1. ]]..Ryzhide:load_translation("hide_n_hype_find_a_good_hiding_spot")..[[</h4></td>
			</tr>
			<tr>
				<td align="center" colspan="3"><h4>2. ]]..Ryzhide:load_translation("hide_n_hype_used_a_object_to_hide")..[[</h4></td>
			</tr>
			<tr>
				<td align="center" colspan="3"><h4>3. ]]..Ryzhide:load_translation("hide_n_hype_wait_of_start_of_the_round")..[[</h4></td>
			</tr>

			<tr>
				<td align="center" colspan="3"><h4><font color="orange">4. ]]..Ryzhide:load_translation("hide_n_hype_forbidden_regions")..[[</font></h4></td>
			</tr>

			<tr>
				<td colspan="3">&nbsp;</td>
			</tr>

			<tr>
				<td  align="center" colspan="3">]]..Ryzhide:load_translation("hide_n_hype_avaible_objects")..[[</td>
			</tr>

			<tr>
				<td align="center" colspan="3">
					<table width="]]..table_width..[[" border="]]..self.debug_window_border..[[">
						<tr>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_totem_kitin_sel_nocollision')">]]..Ryzhide:load_translation("object_totem_kitin_sel_nocollision")..[[</a></td>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_tent_sel_nocollision')">]]..Ryzhide:load_translation("object_tent_sel_nocollision")..[[</a></td>
						</tr>
						<tr>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_spot_goo_sel_nocollision')">]]..Ryzhide:load_translation("object_spot_goo_sel_nocollision")..[[</a></td>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_tent_zorai_sel_nocollision')">]]..Ryzhide:load_translation("object_tent_zorai_sel_nocollision")..[[</a></td>
						</tr>
						<tr>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_giant_skull_b_lokness_c_sel_nocollision')">]]..Ryzhide:load_translation("object_giant_skull_b_lokness_c_sel_nocollision")..[[</a></td>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_tent_tryker_sel_nocollision')">]]..Ryzhide:load_translation("object_tent_tryker_sel_nocollision")..[[</a></td>
						</tr>
						<tr>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_1_crate_sel_nocollision')">]]..Ryzhide:load_translation("object_1_crate_sel_nocollision")..[[</a></td>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_tent_matis_sel_nocollision')">]]..Ryzhide:load_translation("object_tent_matis_sel_nocollision")..[[</a></td>
						</tr>
						<tr>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_1_barrel_sel_nocollision')">]]..Ryzhide:load_translation("object_1_barrel_sel_nocollision")..[[</a></td>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_tent_fyros_sel_nocollision')">]]..Ryzhide:load_translation("object_tent_fyros_sel_nocollision")..[[</a></td>
						</tr>
						<tr>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_fo_s2_bigroot_c_sel_nocollision')">]]..Ryzhide:load_translation("object_fo_s2_bigroot_c_sel_nocollision")..[[</a></td>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_ju_s3_dead_tree_sel_nocollision')">]]..Ryzhide:load_translation("object_ju_s3_dead_tree_sel_nocollision")..[[</a></td>
						</tr>
						<tr>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_fy_s1_burnedtree_d_sel_nocollision')">]]..Ryzhide:load_translation("object_fy_s1_burnedtree_d_sel_nocollision")..[[</a></td>
							<td align="center"><a href="ah:lua&Ryzhide:spawn_object('object_tr_s2_lokness_c_sel_nocollision')">]]..Ryzhide:load_translation("object_tr_s2_lokness_c_sel_nocollision")..[[</a></td>
						</tr>
					</table>
				</td>
			</tr>

			<tr>
				<td colspan="2" align="center"><div id="ready_most_wanted_text" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:ready_most_wanted_text;font_size:16;text_color:255 0 0 255;hardtext:]]..Ryzhide:load_translation("hide_n_hype_not_ready")..[[;w:250;'></div></td>
				<td align="center"><div id="display_most_wanted_status" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_only_images;id:display_most_wanted_status;img1:w_answer_16_cancel.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_not_ready")..[[;w:46;h:46;'></div></td>
			</tr>
			</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted, "", "")
end

function Ryzhide:spawn_object(sheet_id)
	local have_team_member = Ryzhide:check_have_team_member()
	if(have_team_member == "true")then
		Ryzhide:display_asked_for_join_error(Ryzhide:load_translation("hide_n_hype_you_cannot_be_in_team"))
		return
	else
		webig:openUrlInBg("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=12811&command=reset_all&sheet_to_spawn="..sheet_id)
	end
end

function Ryzhide:despawn_object(old_x, old_y, reason_why)
	webig:openUrlInBg("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=12835&command=reset_all&play_pos_x="..old_x.."&play_pos_y="..old_y.."&reason_why="..reason_why)
end


--###################################### END build and load prepartion window #######################################




--###################################### START build and load game_running window #######################################


function Ryzhide:start_the_hide_now(time_to_end, most_wanted_name, current_round_id)
	if(self.game_is_full_loaded == 0)then
		Ryzhide:display_debug_messanges("game_not_loaded -- start_the_hide_now")
		return
	end

	if(self.player_already_registerd == 0)then
		return
	end

	if(tonumber(self.reject_invite_round_id) == tonumber(current_round_id))then
		return
	end

	local mainui = getUI(self.main_window_name)

	self.current_round_end = 0
	self.json_pull_counter = 0
	self.timer_hint_1 = 0
	self.timer_hint_2 = 0
	self.timer_hint_3 = 0
	self.target_name_old = "unlocked"

	--reset vars for game is over
	self.closed_most_wanted_not_found_hunter = 0
	self.closed_loos_hunter = 0

	Ryzhide:display_debug_messanges("time_to_end: "..time_to_end)
	Ryzhide:display_debug_messanges("most_wanted_name: "..most_wanted_name)

	Ryzhide:check_local_player_name()

	self.manuell_action = 0
	self.json_pull_counter_old = 0
	self.json_pull_counter = 0
	self.need_json_update = 1
	self.current_round_end = time_to_end

	if(self.player_name == most_wanted_name)then
		player_type = "most_wanted"
		Ryzhide:display_debug_messanges("You must hide!")
		Ryzhide:build_game_running_most_wanted_window()
		addOnDbChange(mainui, self.timer_str, "Ryzhide:pars_json_data_game_running_most_wanted()")
	else
		player_type = "hunter"
		Ryzhide:display_debug_messanges("You are hunter , hunt the most wanted!")
		Ryzhide:build_game_running_hunter_window()
		addOnDbChange(mainui, self.timer_str, "Ryzhide:pars_json_data_game_running_hunter()")
	end
end

function Ryzhide:game_running_timer_stopped()
	removeOnDbChange(getUI(self.main_window_name),self.timer_str)

	Ryzhide:display_debug_messanges("Timer_is_over_and_game_over")
	self.need_json_update = 1
	self.json_data_ready = 0

	DynE.otherMapPoints["hidenhype_hunter"] = {}
    delArkPoints()
	DynE:AddOtherMapPoints()

	Ryzhide:build_wait_start_window()
end

function Ryzhide:display_game_running_sucess(sucess_msg)
	self.json_data_ready = 0
	Ryzhide:display_message_to_player('info', 'AMB', sucess_msg)
end

function Ryzhide:display_game_running_error(error_msg)
	self.json_data_ready = 0
	Ryzhide:display_message_to_player('error', 'AMB', error_msg)
end

function Ryzhide:check_player_are_in_special_state()
	local current_player_mode = getPlayerMode()
	local current_player_invisible = getDbProp("SERVER:USER:IS_INVISIBLE")
	local have_team_member = "false"
	local valid_a_check = 0

	local player_special_state = 0

	local have_team_member = Ryzhide:check_have_team_member()
	if(have_team_member == "true")then
		valid_a_check = valid_a_check + 1
	end

	if(self.json_pull_counter > 6)then
		--now the tatus need to be fine and not allow to break
		if(current_player_invisible == 0)then
		    Ryzhide:display_debug_messanges("you_are_no_longer_invisible")
		    valid_a_check = valid_a_check + 1
		end

		if(current_player_mode ~= "REST")then
		    if(current_player_mode ~= "SIT")then
		        Ryzhide:display_debug_messanges("player_no_longer_rest_or_sit")
		        valid_a_check = valid_a_check + 1
		    end
		end
	end

	if(valid_a_check >= 1)then
	    player_special_state = 69
	end

	return player_special_state
end

function Ryzhide:pars_json_data_game_running_most_wanted()
		local special_state = Ryzhide:check_player_are_in_special_state()

		if(self.need_json_update == 1)then
			self.need_json_update = 0
			Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=heartbeat&special_state='..special_state)
		end

		if(self.current_round_end ~= 0)then
			local remaining_time = self.current_round_end - os.time()

			if(remaining_time < 0)then
				Ryzhide:game_running_timer_stopped()
			else
				Ryzhide:update_timer("hide_n_hype_timer_game_running_most_wanted",remaining_time, self.duration_time_game_running)
			end
		end

		Ryzhide:display_debug_messanges("json_pull_counter: "..self.json_pull_counter)

		if(self.json_data_ready == 1)then
			if (self.hide_n_hide_json_data) then
				if (self.hide_n_hide_json_data.error) then
					if(self.hide_n_hide_json_data.error == "not_most_wanted")then
						Ryzhide:display_game_running_error(Ryzhide:load_translation("hide_n_hype_not_most_wanted"))
					end
				elseif (self.hide_n_hide_json_data.success) then
					if(self.hide_n_hide_json_data.success == "ok_most_wanted_heartbeat")then
						self.json_pull_counter = self.json_pull_counter + 1
						--Ryzhide:display_game_running_sucess("sucess_heartbeat: "..self.hide_n_hide_json_data.success)
						Ryzhide:display_hunter_on_map(self.hide_n_hide_json_data.hunter_pos_array)
						self.json_data_ready = 0
					end
				else
					Ryzhide:display_game_running_error("no 'error' or 'success' found")
				end
			else
				Ryzhide:display_game_running_error("JSON-Decode failed")
			end

			self.need_json_update = 1
		end
end

function Ryzhide:display_hunter_on_map(hunter_pos_array)
	if (type(hunter_pos_array) == "boolean") then
		--reset only the map flags
		DynE.otherMapPoints["hidenhype_hunter"] = {}
		delArkPoints()
		DynE:AddOtherMapPoints()
	else
	    DynE.otherMapPoints["hidenhype_hunter"] = {}

		for _, positions in ipairs(hunter_pos_array) do
			local xpos_hunter, ypos_hunter = positions:match("([^,]+),([^,]+)")
			if(tonumber(xpos_hunter) ~= 0 or tonumber(ypos_hunter)~= 0)then
				Ryzhide:display_debug_messanges("Hx:"..xpos_hunter.." Hy: "..ypos_hunter)
				table.insert(DynE.otherMapPoints["hidenhype_hunter"], {tonumber(xpos_hunter), tonumber(ypos_hunter), Ryzhide:load_translation("hide_n_hype_hunter"), "teammate_map.png"})
			end
		end

		delArkPoints()
        DynE:AddOtherMapPoints()
	end
end

function Ryzhide:update_hint_data()
	self.hint_1_text = self.hide_n_hide_json_data.hint_1
	self.hint_2_text = self.hide_n_hide_json_data.hint_2
	self.hint_3_text = self.hide_n_hide_json_data.hint_3
	self.hint_4_text = self.hide_n_hide_json_data.hint_4
end

function Ryzhide:check_player_target()
	local mainui = getUI(self.main_window_name)
	local check_found_most_wanted_button = mainui:find("check_found_most_wanted")

	local hnh_target_x_pos = 0
	local hnh_target_y_pos = 0
	local hnh_target_z_pos = 0

	local hnh_player_x_pos = 0
	local hnh_player_y_pos = 0
	local hnh_player_z_pos = 0


	if(isTargetNPC() == false and isTargetPlayer() == false and isTargetUser() == false)then
		hnh_player_x_pos,hnh_player_y_pos,hnh_player_z_pos = getPlayerPos()

		if(getTargetPos() ~= nil)then
			hnh_target_x_pos,hnh_target_y_pos,hnh_target_z_pos = getTargetPos()
		end

		local current_distance = Ryzhide:distance_calc(hnh_target_x_pos, hnh_target_y_pos, hnh_target_z_pos, hnh_player_x_pos, hnh_player_y_pos, hnh_player_z_pos)
		if(current_distance <= 5)then

			if(self.target_name_old == "locked")then
				local old_pos_distance = Ryzhide:distance_calc(hnh_player_x_pos, hnh_player_y_pos, hnh_player_z_pos, self.save_x_pos, self.save_y_pos, self.save_z_pos)
				if(old_pos_distance > 1)then
					Ryzhide:display_debug_messanges("Reset Butto now!!!!!!!!")
					self.target_name_old = "unlocked"
					self.save_x_pos,self.save_y_pos,self.save_z_pos = getPlayerPos()
					check_found_most_wanted_button:find("img1").texture = ""
					check_found_most_wanted_button:find("ctrl").active = true
				end
			end

			if(self.target_name_old == "unlocked" and check_found_most_wanted_button:find("img1").texture ~= "curs_rotate.tga")then
				--unlock the check_found button
				Ryzhide:display_debug_messanges("texture: "..check_found_most_wanted_button:find("img1").texture)
				Ryzhide:display_debug_messanges("unlocked")
				check_found_most_wanted_button:find("back").texture = "r2_icon_animation_target_green.png"
				check_found_most_wanted_button:find("ctrl").active = true
			end
		else
			--disable the check_found button
			check_found_most_wanted_button:find("back").texture = "r2_icon_animation_target.png"
			check_found_most_wanted_button:find("ctrl").active = false
		end
	else
		--disable the check_found button
		check_found_most_wanted_button:find("back").texture = "r2_icon_animation_target.png"
		check_found_most_wanted_button:find("ctrl").active = false
	end
end

function Ryzhide:display_try_found_sucess(sucess_msg)
	self.json_data_ready = 0
	Ryzhide:display_message_to_player('info', 'AMB', sucess_msg)
end

function Ryzhide:display_try_found_error(error_msg)
	self.json_data_ready = 0
	Ryzhide:display_message_to_player('error', 'AMB', error_msg)
end

function Ryzhide:feedback_found_button(feedback)
	local mainui = getUI(self.main_window_name)
	local mainui_group = getUI(self.main_window_name):find("check_found_most_wanted")

	if(feedback == "success")then
		mainui_group:find("img1").texture = "rap_invited_no_dm.png"
		mainui_group:find("ctrl").active = false

		local mainui = getUI(self.main_window_name)
		removeOnDbChange(mainui, self.timer_str)

		Ryzhide:build_wait_finish_window()
	else
		self.save_x_pos,self.save_y_pos,self.save_z_pos = getPlayerPos()
		self.target_name_old = "locked"
		mainui_group:find("img1").texture = "w_answer_16_cancel.png"
		mainui_group:find("ctrl").active = false
	end
end

function Ryzhide:pars_json_data_game_running_hunter()
	if(self.need_json_update == 1 and self.manuell_action == 0)then
		self.need_json_update = 0
		local send_to_server_x = 0
		local send_to_server_y = 0
		local send_to_server_z = 0

		send_to_server_x,send_to_server_y,send_to_server_z = getPlayerPos()
		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=jsonData&need_json_data=running_game&x='..send_to_server_x..'&y='..send_to_server_y)
		Ryzhide:unlock_hints()
	end

	if(self.need_json_update == 1 and self.manuell_action == 1)then
		self.need_json_update = 0
		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=check_found')
	end

	Ryzhide:display_debug_messanges("old counter:"..self.json_pull_counter_old.."counter"..self.json_pull_counter)
	Ryzhide:display_debug_messanges("data ready: "..self.json_data_ready.."manuell_action: "..self.manuell_action)

	if(self.json_pull_counter_old < self.json_pull_counter and self.manuell_action == 1)then
		Ryzhide:display_debug_messanges("Timeout_reset")
		self.manuell_action = 0
		local mainui_group = getUI(self.main_window_name):find("check_found_most_wanted")
		mainui_group:find("img1").texture = ""
		mainui_group:find("ctrl").active = true
	end

	Ryzhide:check_player_target()

	if(self.json_data_ready == 1)then
			if (self.hide_n_hide_json_data) then
				if (self.hide_n_hide_json_data.error) then
					if(self.manuell_action == 1)then
						if(self.hide_n_hide_json_data.error == "not_near_most_wanted")then
							Ryzhide:display_try_found_error(Ryzhide:load_translation("hide_n_hype_"..self.hide_n_hide_json_data.error))
							Ryzhide:feedback_found_button("error")
							self.manuell_action = 0
							self.json_data_ready = 0
						else
							Ryzhide:display_try_found_error(Ryzhide:load_translation(self.hide_n_hide_json_data.error))
						end
					end
				elseif (self.hide_n_hide_json_data.success) then
					Ryzhide:display_debug_messanges("success: "..self.hide_n_hide_json_data.success)
					self.json_pull_counter = self.json_pull_counter + 1
					self.json_data_ready = 0

					if(self.manuell_action == 1)then
						if(self.hide_n_hide_json_data.success == "ok_you_found_most_wanted")then
							if(self.hide_n_hide_json_data.success == "ok_you_found_most_wanted")then
								local sucess_message = "hide_n_hype_ok_you_found_most_wanted"
							end
							Ryzhide:display_try_found_sucess(Ryzhide:load_translation(sucess_message))
							Ryzhide:feedback_found_button("success")
							self.manuell_action = 0
						end
					else
						Ryzhide:update_hint_data()
					end
				else
					Ryzhide:display_try_found_error("no 'error' or 'success' found")
				end
			else
				Ryzhide:display_try_found_error("JSON-Decode failed")
			end

			self.need_json_update = 1
		end


	if(self.current_round_end ~= 0)then
		local remaining_time = self.current_round_end - os.time()

		local total_timer_need = self.duration_time_hint_1 + self.duration_time_hint_2 + self.duration_time_hint_3

		local remaining_time_hint_1 = remaining_time - (self.duration_time_hint_2 + self.duration_time_hint_3 + (self.duration_time_game_running - total_timer_need))
		local remaining_time_hint_2 = remaining_time - (self.duration_time_hint_2)
		local remaining_time_hint_3 = remaining_time - (self.duration_time_hint_3 - (self.duration_time_hint_3 - (self.duration_time_game_running - total_timer_need)) )

		local time_to_unlock_hint_1 = self.duration_time_hint_1
		local time_to_unlock_hint_2 = self.duration_time_hint_1 + self.duration_time_hint_2
		local time_to_unlock_hint_3 = self.duration_time_hint_1 + self.duration_time_hint_2 + self.duration_time_hint_3


		if(remaining_time < 0)then
			Ryzhide:game_running_timer_stopped()
		else
			local diff_remaining_time = self.duration_time_game_running - remaining_time

			--Ryzhide:display_debug_messanges("diff_remaining_time: "..diff_remaining_time)
			--Ryzhide:display_debug_messanges("time_to_unlock_hint_1: "..time_to_unlock_hint_1)
			--Ryzhide:display_debug_messanges("time_to_unlock_hint_2: "..time_to_unlock_hint_2)
			--Ryzhide:display_debug_messanges("time_to_unlock_hint_3: "..time_to_unlock_hint_3)


			--Ryzhide:display_debug_messanges("remaining_time_hint_1: "..remaining_time_hint_1)
			--Ryzhide:display_debug_messanges("remaining_time_hint_2: "..remaining_time_hint_2)
			--Ryzhide:display_debug_messanges("remaining_time_hint_3: "..remaining_time_hint_3)

			if(remaining_time_hint_1 >= 0 and self.timer_hint_1 == 0)then
				Ryzhide:update_timer("hide_n_hype_timer_game_running_hint",remaining_time_hint_1, self.duration_time_hint_1)
			end

			if(remaining_time_hint_2 >= 0 and self.timer_hint_2 == 0 and self.timer_hint_1 == 1)then
				Ryzhide:update_timer("hide_n_hype_timer_game_running_hint",remaining_time_hint_2, self.duration_time_hint_2)
			end

			if(remaining_time_hint_3 >= 0 and self.timer_hint_3 == 0 and self.timer_hint_1 == 1 and self.timer_hint_2 == 1)then
				Ryzhide:update_timer("hide_n_hype_timer_game_running_hint",remaining_time_hint_3, self.duration_time_hint_3)
			end

			--unlock hint_2
			if(diff_remaining_time > time_to_unlock_hint_1 and self.timer_hint_1 == 0)then
				self.timer_hint_1 = 1
				Ryzhide:display_debug_messanges("self.timer_hint_1")
			end

			--unlock hint_3
			if(diff_remaining_time > time_to_unlock_hint_2 and self.timer_hint_2 == 0)then
				self.timer_hint_2 = 1
				Ryzhide:display_debug_messanges("self.timer_hint_2")
			end

			--unlock hint_4
			if(diff_remaining_time > time_to_unlock_hint_3 and self.timer_hint_3 == 0)then
				self.timer_hint_3 = 1
				Ryzhide:display_debug_messanges("self.timer_hint_3")
			end

			Ryzhide:update_timer("hide_n_hype_timer_game_running_hunter",remaining_time, self.duration_time_game_running)
		end
	end
end

function Ryzhide:unlock_hints()
	local mainui = getUI(self.main_window_name)

	if(self.hint_1_text ~= "????")then
		local hint_1_text_controll = mainui:find("hint_1")
		Ryzhide:display_debug_messanges("set_hint_1: "..self.hint_1_text)
		hint_1_text_controll:find("text").hardtext = self.hint_1_text
		hint_1_text_controll:find("text").color = "0 255 0 255"
	end

	if(self.timer_hint_1 == 1 and self.hint_2_text ~= "????")then
		local hint_2_text_controll = mainui:find("hint_2")
		Ryzhide:display_debug_messanges("set_hint_2: "..self.hint_2_text)
		hint_2_text_controll:find("text").hardtext = self.hint_2_text
		hint_2_text_controll:find("text").color = "0 255 0 255"
	end
	if(self.timer_hint_2 == 1 and self.hint_3_text ~= "????")then
		local hint_3_text_controll = mainui:find("hint_3")
		Ryzhide:display_debug_messanges("set_hint_3: "..self.hint_3_text)
		hint_3_text_controll:find("text").hardtext = self.hint_3_text
		hint_3_text_controll:find("text").color = "0 255 0 255"
	end
	if(self.timer_hint_3 == 1 and self.hint_3_text ~= "????")then
		local hint_3_text_controll = mainui:find("hint_4")
		Ryzhide:display_debug_messanges("set_hint_4: "..self.hint_4_text)
		hint_3_text_controll:find("text").hardtext = Ryzhide:load_translation("hide_n_hype_"..self.hint_4_text)
		hint_3_text_controll:find("text").color = "0 255 0 255"
	end
end

function Ryzhide:click_icon_found_most_wanted(id)
	local mainui = getUI(self.main_window_name)

	if(id == "check_found_most_wanted")then
		self.json_pull_counter_old = self.json_pull_counter + 3
		self.manuell_action = 1
		self.need_json_update = 0
		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=check_found')
	end

	local mainui_group = getUI(self.main_window_name):find(id)
	local image_ui_controll = mainui_group:find("img1").texture
	if(image_ui_controll == "")then
		mainui_group:find("img1").texture = "curs_rotate.png"
		mainui_group:find("ctrl").active = false
	end
end

function Ryzhide:build_game_running_most_wanted_window()
	local window_height = 200
	local window_width = 365

	local html_running_most_wanted=""
	html_running_most_wanted=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_running_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><div id="hide_n_hype_timer_game_running_most_wanted" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_game_running_most_wanted;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_game_running)..[[;'></div></td>
			</tr>

			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_keep_calm")..[[</h3></td>
			</tr>
			<tr>
				<td align="center"><h4>]]..Ryzhide:load_translation("hide_n_hype_dont_break_the_afk_status")..[[</h4></td>
			</tr>
			</table>]]
	Ryzhide:open_resize_window(self.main_window_name,window_height, window_width, html_running_most_wanted, "", "")
end

function Ryzhide:build_game_running_hunter_window()
	local window_height = 360
	local window_width = 380


	local html_running_hunter=""
	html_running_hunter=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_running_window")..[[</title>
		   <table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center" colspan="2"><div id="hide_n_hype_timer_game_running_hunter" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_game_running_hunter;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_game_running)..[[;'></div></td>
			</tr>

			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_new_hint_in")..[[</h3></td>
				<td align="center"><div id="hide_n_hype_timer_game_running_hint" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_game_running_hint;font_size:25;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_hint_1)..[[;'></td>
			</tr>
			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_hint_1")..[[</h3></td>
				<td align="center"><div id="hint_1" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hint_1;font_size:16;text_color:255 0 0 255;hardtext:????;w:250;'></div></td>
			</tr>
			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_hint_2")..[[</h3></td>
				<td align="center"><div id="hint_2" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hint_2;font_size:16;text_color:255 0 0 255;hardtext:????;w:250;'></div></td>
			</tr>
			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_hint_3")..[[</h3></td>
				<td align="center"><div id="hint_3" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hint_3;font_size:16;text_color:255 0 0 255;hardtext:????;w:250;'></div></td>
			</tr>
			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_hint_4")..[[</h3></td>
				<td align="center"><div id="hint_4" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_text;id:hint_4;font_size:16;text_color:255 0 0 255;hardtext:????;w:250;'></div></td>
			</tr>

			<tr>
				<td align="center" colspan="2"><div id="check_found_most_wanted" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:check_found_most_wanted;bg:r2_icon_animation_target.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_to_check_found_most_wanted")..[[;lua_function:click_icon_found_most_wanted;'></div></td>
			</tr>
			</table>]]
	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_running_hunter, "", "")
end

function Ryzhide:build_wait_finish_window()
	local window_height = 100
	local window_width = 380


	local html_wait_finish_window=""
	html_wait_finish_window=[[<title>....?!</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><h2>]]..Ryzhide:load_translation("hide_n_hype_wait_for_game_finish")..[[</h2></td>
			</tr>
			</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_wait_finish_window, "", "")
end


--###################################### END build and load game_running window #######################################




--###################################### START build and load claim_your_price window #######################################


function Ryzhide:game_over_most_wanted_not_found(most_wanted_name,current_round_id,timer_claim_reward,most_wanted_position)
	if(self.game_is_full_loaded == 0)then
		Ryzhide:display_debug_messanges("game_not_loaded -- game_over_most_wanted_not_found")
		return
	end

	if(self.player_already_registerd == 0)then
		return
	end

	if(tonumber(self.reject_invite_round_id) == tonumber(current_round_id))then
		return
	end

	self.game_is_now_running = 0
	local mainui = getUI(self.main_window_name)
	removeOnDbChange(mainui,self.timer_str)

	Ryzhide:display_debug_messanges("game_over_most_wanted_not_found: "..timer_claim_reward)
	Ryzhide:display_debug_messanges("most_wanted: "..most_wanted_name)
	Ryzhide:display_debug_messanges("most_wanted_pos: "..most_wanted_position)

	if(most_wanted_name == "")then
		Ryzhide:display_debug_messanges("error_most_wanted_name_empty")
		Ryzhide:close_window(self.main_window_name, "")
		return
	end

	DynE.otherMapPoints["hidenhype_hunter"] = {}
	delArkPoints()
	DynE:AddOtherMapPoints()

	Ryzhide:check_local_player_name()

	if(self.player_name == most_wanted_name)then
		if(self.reward_already_claimed == 0)then
			Ryzhide:build_most_wanted_not_found_most_wanted()
			addOnDbChange(mainui, self.timer_str, "Ryzhide:timer_to_claim_rewards("..timer_claim_reward..")")
		end
	else
		Ryzhide:display_debug_messanges("already_close"..self.closed_most_wanted_not_found_hunter)
		if(self.closed_most_wanted_not_found_hunter == 0)then
		    local xpos_most_wanted, ypos_most_wanted = most_wanted_position:match("([^,]+),([^,]+)")
			if(tonumber(xpos_most_wanted) ~= 0 or tonumber(ypos_most_wanted)~= 0)then
		       Ryzhide:build_most_wanted_not_found_hunter(most_wanted_name,tonumber(xpos_most_wanted),tonumber(ypos_most_wanted))
		    else
		        Ryzhide:build_most_wanted_not_found_hunter(most_wanted_name,0,0)
		    end
		end
	end
end

function Ryzhide:game_over_most_wanted_found(hunter_name,most_wanted_name,current_round_id,timer_claim_reward,most_wanted_position)
	if(self.game_is_full_loaded == 0)then
		Ryzhide:display_debug_messanges("game_not_loaded -- game_over_most_wanted_found")
		return
	end

	if(self.player_already_registerd == 0)then
		return
	end

	if(tonumber(self.reject_invite_round_id) == tonumber(current_round_id))then
		return
	end

	Ryzhide:display_debug_messanges("game_over_most_wanted_found: "..timer_claim_reward)
	Ryzhide:display_debug_messanges("hunter: "..hunter_name)
	Ryzhide:display_debug_messanges("most_wanted: "..most_wanted_name)

	self.game_is_now_running = 0
	local mainui = getUI(self.main_window_name)
	removeOnDbChange(mainui,self.timer_str)

	Ryzhide:check_local_player_name()

	DynE.otherMapPoints["hidenhype_hunter"] = {}
	delArkPoints()
	DynE:AddOtherMapPoints()

	if(self.player_name == most_wanted_name)then
		if(self.reward_already_claimed == 0)then
			self.player_type = "most_wanted"
			Ryzhide:display_debug_messanges("You loose! Found by "..hunter_name)
			Ryzhide:build_finished_most_wanted(hunter_name,most_wanted_name)
			addOnDbChange(mainui, self.timer_str, "Ryzhide:timer_to_claim_rewards("..timer_claim_reward..")")
		end
	elseif(self.player_name == hunter_name)then
		if(self.reward_already_claimed == 0)then
			self.player_type = "hunter_found"
			Ryzhide:display_debug_messanges("You win!")
			Ryzhide:build_finished_hunter(hunter_name,most_wanted_name)
			addOnDbChange(mainui, self.timer_str, "Ryzhide:timer_to_claim_rewards("..timer_claim_reward..")")
		end
	else
		if(self.closed_loos_hunter == 0)then
			self.player_type = "hunter"
			Ryzhide:display_debug_messanges("You loose:"..hunter_name.." was faster then you")

			local xpos_most_wanted, ypos_most_wanted = most_wanted_position:match("([^,]+),([^,]+)")
			if(tonumber(xpos_most_wanted) ~= 0 or tonumber(ypos_most_wanted)~= 0)then
		       Ryzhide:build_loos_hunter(hunter_name,most_wanted_name,tonumber(xpos_most_wanted),tonumber(ypos_most_wanted))
		    else
		       Ryzhide:build_loos_hunter(hunter_name,most_wanted_name,0,0)
		    end
		end
	end
end

function Ryzhide:timer_to_claim_rewards(timer_claim_reward)
	if(timer_claim_reward ~= 0)then
		local remaining_time = timer_claim_reward - os.time()

		if(remaining_time < 0)then
			Ryzhide:game_running_timer_stopped()
		else
			Ryzhide:update_timer("hide_n_hype_timer_claim_reward",remaining_time, self.duration_time_claim_reward)
		end
	end
end

function Ryzhide:game_running_timer_stopped()
	removeOnDbChange(getUI(self.main_window_name),self.timer_str)

	Ryzhide:display_debug_messanges("Timer_is_over_no_rewards")
	self.need_json_update = 1
	self.json_data_ready = 0

	Ryzhide:close_window(self.main_window_name, "")
end

function Ryzhide:open_clain_reward_script()
	webig:openUrlInBg("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=12815&command=reset_all")
end

function Ryzhide:build_finished_hunter(hunter_name,most_wanted_name)
	local window_height = 330
	local window_width = 365


	local html_finished_hunter=""
	html_finished_hunter=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_over_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
				<tr>
					<td align="center"><div id="hide_n_hype_timer_claim_reward" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_claim_reward;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_claim_reward)..[[;'></div></td>
				</tr>

				<tr>
					<td align="center"><img src="ico_happy_winner.png" width="80" height="80" ></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_you_found_the_most_wanted")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..most_wanted_name..[[</h3></td>
				</tr>

				<tr>
					<td>&nbsp;</td>
				</tr>

				<tr>
					<td align="center"><a href="ah:lua&Ryzhide:open_clain_reward_script()"><h3>]]..Ryzhide:load_translation("hide_n_hype_click_here_to_claim_your_price")..[[</h3></a></td>
				</tr>
			</table>]]
	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_finished_hunter, "", "")
end

function Ryzhide:build_finished_most_wanted(hunter_name,most_wanted_name)
	local window_height = 370
	local window_width = 380


	local html_finished_most_wanted=""
	html_finished_most_wanted=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_over_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
				<tr>
					<td align="center"><div id="hide_n_hype_timer_claim_reward" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_claim_reward;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_claim_reward)..[[;'></div></td>
				</tr>

				<tr>
					<td align="center"><img src="icon_unhappy.png" width="80" height="80" ></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_time_to_come_out")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..hunter_name..[[</h3></td>
				</tr>

				<tr >
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_found_you")..[[</h3></td>
				</tr>

				<tr>
					<td>&nbsp;</td>
				</tr>

				<tr>
					<td align="center"><a href="ah:lua&Ryzhide:open_clain_reward_script()"><h3>]]..Ryzhide:load_translation("hide_n_hype_click_here_to_claim_your_price")..[[</h3></a></td>
				</tr>
			</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_finished_most_wanted, "", "")
end

function Ryzhide:build_loos_hunter(hunter_name,most_wanted_name,pox_mostwanted,poy_mostwanted)
	local window_height = 250
	local window_width = 380
	local show_map_or_not = 0

	if(tonumber(pox_mostwanted) ~= 0 or tonumber(poy_mostwanted)~= 0)then
	    show_map_or_not = 1
	end

	local html_loos_hunter_without_map=""
	html_loos_hunter_without_map=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_over_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
				<tr>
					<td align="center"><img src="icon_unhappy.png" width="80" height="80" ></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_better_luck_next_time")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..hunter_name..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_has_out_searched_you")..[[</h3></td>
				</tr>
			</table>]]

	local html_loos_hunter_with_map=""
	html_loos_hunter_with_map=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_over_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
				<tr>
					<td align="center"><img src="icon_unhappy.png" width="80" height="80" ></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_better_luck_next_time")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..hunter_name..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_has_out_searched_you")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><img src="https://api.bmsite.net/maps/static?center=]]..pox_mostwanted..[[,]]..poy_mostwanted..[[&zoom=7&language=auto&markers=icon:npc|color:0xef4e4e|]]..pox_mostwanted..[[,]]..poy_mostwanted..[[&maptype=atys&mapmode=world&size=300x200" width="300" height="200" ></td>
				</tr>
			</table>]]

	if(show_map_or_not == 0)then
	    --dont show map bad pos
	    Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_loos_hunter_without_map, "click_close_button", "close_window_build_loos_hunter")
    else
        window_height = 470
        --show map good pos
        Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_loos_hunter_with_map, "click_close_button", "close_window_build_loos_hunter")
    end
end

function Ryzhide:build_most_wanted_not_found_hunter(most_wanted_name,pox_mostwanted,poy_mostwanted)
	local window_height = 250
	local window_width = 370
	local show_map_or_not = 0

	if(tonumber(pox_mostwanted) ~= 0 or tonumber(poy_mostwanted)~= 0)then
	    show_map_or_not = 1
	end

	local html_most_wanted_not_found_hunter_without_map=""
	html_most_wanted_not_found_hunter_without_map=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_over_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
				<tr>
					<td align="center"><img src="icon_unhappy.png" width="80" height="80" ></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_better_luck_next_time")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..most_wanted_name..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_keep_untouched_and_not_found")..[[</h3></td>
				</tr>
			</table>]]

	local html_most_wanted_not_found_hunter_with_map=""
	html_most_wanted_not_found_hunter_with_map=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_over_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
				<tr>
					<td align="center"><img src="icon_unhappy.png" width="80" height="80" ></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_better_luck_next_time")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..most_wanted_name..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_keep_untouched_and_not_found")..[[</h3></td>
				</tr>

				<tr>
					<td align="center"><img src="https://api.bmsite.net/maps/static?center=]]..pox_mostwanted..[[,]]..poy_mostwanted..[[&zoom=7&language=auto&markers=icon:npc|color:0xef4e4e|]]..pox_mostwanted..[[,]]..poy_mostwanted..[[&maptype=atys&mapmode=world&size=300x200" width="300" height="200" ></td>
				</tr>
			</table>]]

	if(show_map_or_not == 0)then
	    --dont show map bad pos
	    Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted_not_found_hunter_without_map, "click_close_button", "close_window_build_most_wanted_not_found_hunter")
    else
        window_height = 470
        --show map good pos
        Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted_not_found_hunter_with_map, "click_close_button", "close_window_build_most_wanted_not_found_hunter")
    end
end

function Ryzhide:build_most_wanted_not_found_most_wanted()
	local window_height = 310
	local window_width = 365


	local html_most_wanted_not_found_most_wanted=""
	html_most_wanted_not_found_most_wanted=[[<title>]]..Ryzhide:load_translation("hide_n_hype_game_is_over_window")..[[</title>
			<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
				<tr>
					<td align="center"><div id="hide_n_hype_timer_claim_reward" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_claim_reward;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_claim_reward)..[[;'></div></td>
				</tr>

				<tr>
					<td align="center"><img src="ico_happy_winner.png" width="80" height="80" ></td>
				</tr>

				<tr>
					<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_you_won_no_player_found_you")..[[</h3></td>
				</tr>

				<tr>
					<td>&nbsp;</td>
				</tr>

				<tr>
					<td align="center"><a href="ah:lua&Ryzhide:open_clain_reward_script()"><h3>]]..Ryzhide:load_translation("hide_n_hype_click_here_to_claim_your_price")..[[</h3></a></td>
				</tr>
			</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted_not_found_most_wanted, "", "")
end


--###################################### END build and load claim_your_price window #######################################



--###################################### Start error handling #######################################

function Ryzhide:build_most_wanted_bad_position()
	local window_height = 180
	local window_width = 400


	local html_most_wanted_bad_position = ""
	html_most_wanted_bad_position=[[<title>]]..Ryzhide:load_translation("hide_n_hype_abort_most_wanted_bad_position_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><img src="icon_dead.png" width="80" height="80" ></td>
			</tr>

			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_abort_most_wanted_bad_position")..[[</h3></td>
			</tr>
		</table>]]
		Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted_bad_position, "click_close_button", "close_window_abort_most_wanted_bad_position_window")
end

function Ryzhide:build_error_not_enough_players()
	local window_height = 200
	local window_width = 370


	local html_error_not_enough_players = ""
	html_error_not_enough_players=[[<title>]]..Ryzhide:load_translation("hide_n_hype_abort_not_enough_player_to_start_a_round_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><img src="icon_dead.png" width="80" height="80" ></td>
			</tr>

			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_abort_not_enough_player")..[[</h3></td>
			</tr>
		</table>]]
		Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_error_not_enough_players, "click_close_button", "close_window_abort_not_enough_player_to_start_a_round_window")
end

function Ryzhide:build_most_wanted_offline_window()
	local window_height = 170
	local window_width = 450


	local html_most_wanted_offline_window=""
	html_most_wanted_offline_window=[[<title>]]..Ryzhide:load_translation("hide_n_hype_wait_for_most_wanted_come_back_online_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr align="center">
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
				<td align="center"><div id="hide_n_hype_timer_most_wanted_offline" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_timer;id:hide_n_hype_timer_most_wanted_offline;timer:]]..Ryzhide:convert_secound_to_string(self.duration_time_most_wanted_offline)..[[;'></div></td>
				<td align="center" width="40"><img src="ico_time.png" width="40"></td>
			</tr>

			<tr>
				<td colspan="3" align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_most_wanted_lost_connection")..[[</h3></td>
			</tr>
		</table>]]

	Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted_offline_window, "", "")
end

function Ryzhide:build_most_wanted_offline()
	local window_height = 200
	local window_width = 370


	local html_most_wanted_offline = ""
	html_most_wanted_offline=[[<title>]]..Ryzhide:load_translation("hide_n_hype_abort_most_wanted_offline_game_abort_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><img src="icon_dead.png" width="80" height="80" ></td>
			</tr>

			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_most_wanted_offline_game_abort")..[[</h3></td>
			</tr>
		</table>]]
		Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted_offline, "click_close_button", "close_window_abort_most_wanted_offline_game_abort_window")
end

function Ryzhide:build_most_wanted_moved()
	local window_height = 180
	local window_width = 400


	local html_most_wanted_moved = ""
	html_most_wanted_moved=[[<title>]]..Ryzhide:load_translation("hide_n_hype_abort_most_wanted_moved_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center"><img src="icon_dead.png" width="80" height="80" ></td>
			</tr>

			<tr>
				<td align="center"><h3>]]..Ryzhide:load_translation("hide_n_hype_most_wanted_moved_game_abort")..[[</h3></td>
			</tr>
		</table>]]
		Ryzhide:open_resize_window(self.main_window_name, window_height, window_width, html_most_wanted_moved, "click_close_button", "close_window_abort_most_wanted_moved_window")
end

function Ryzhide:abort_because_of(process_info_name,round_id)
	self.game_is_now_running = 0

	if(self.game_is_full_loaded == 0)then
		Ryzhide:display_debug_messanges("game_not_loaded -- abort_because_of: "..process_info_name)
		return
	end

	if(self.player_already_registerd == 0)then
		return
	end


	local mainui = getUI(self.main_window_name)
	removeOnDbChange(mainui, self.timer_str)

	Ryzhide:display_debug_messanges("reason: "..process_info_name.." round_id: "..round_id)
	Ryzhide:display_debug_messanges("self: "..self.reject_invite_round_id.." round: "..round_id)

	if(tonumber(self.reject_invite_round_id) == tonumber(round_id))then
		return
	end

	if(process_info_name == "abort_not_enough_player")then
		Ryzhide:build_error_not_enough_players()
	end

	if(process_info_name == "most_wanted_moved")then
		Ryzhide:build_most_wanted_moved()
	end


	if(process_info_name == "most_wanted_bad_position")then
		Ryzhide:build_most_wanted_bad_position()
	end

	if(process_info_name == "error_no_player_founds" or process_info_name == "error_no_hunter_found" or process_info_name == "error_no_most_wanted_found")then
		Ryzhide:build_error_not_enough_players()
	end

	if(process_info_name == "most_wanted_offline")then
		Ryzhide:build_most_wanted_offline()
	end
end

function Ryzhide:most_wanted_offline(time_until_abort,round_id)
	local mainui = getUI(self.main_window_name)
	removeOnDbChange(mainui, self.timer_str)

	if(self.game_is_full_loaded == 0)then
		Ryzhide:display_debug_messanges("game_not_loaded -- abort_because_of: "..process_info_name)
		return
	end

	if(self.player_already_registerd == 0)then
		return
	end

	if(tonumber(self.reject_invite_round_id) == tonumber(round_id))then
		return
	end

	self.current_round_end = time_until_abort
	Ryzhide:build_most_wanted_offline_window()
	addOnDbChange(mainui, self.timer_str, "Ryzhide:run_timer_most_wanted_offline()")
end

function Ryzhide:run_timer_most_wanted_offline()
	if(self.current_round_end ~= 0)then
		local remaining_time = self.current_round_end - os.time()

		if(remaining_time < 0)then
			Ryzhide:game_running_timer_stopped()
		else
			Ryzhide:update_timer("hide_n_hype_timer_most_wanted_offline",remaining_time, self.duration_time_most_wanted_offline)
		end
	end
end

--###################################### END error handling #######################################



--###################################### START asked_for_abort #######################################

function Ryzhide:build_ask_for_abort_skip_game()
	local window_height = 250
	local window_width = 370

	local html_ask_for_abort_skip_game = ""
	html_ask_for_abort_skip_game=[[<title>]]..Ryzhide:load_translation("hide_n_hype_you_realy_want_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center" colspan="2"><img src="icon_sad.png" width="80" height="80" ></td>
			</tr>

			<tr>
				<td align="center" colspan="2"><h3>]]..Ryzhide:load_translation("hide_n_hype_you_realy_want_to_skip")..[[</h3></td>
			</tr>

			<tr>
				<td align="center" colspan="2">&nbsp;</td>
			</tr>

			<tr>
				<td align="center"><div id="abort_skip_yes" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:abort_skip_yes;bg:rap_invited_no_dm.png;w:46;h:46;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_yes")..[[;lua_function:click_icon_abort;'></div></td>
				<td align="center"><div id="abort_skip_no" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:abort_skip_no;bg:w_answer_16_cancel.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_no")..[[;lua_function:click_icon_abort;'></div></td>
			</tr>
		</table>]]
		Ryzhide:open_resize_window(self.abort_window_name, window_height, window_width, html_ask_for_abort_skip_game, "", "")
end

function Ryzhide:build_ask_for_abort_unregister()
	local window_height = 270
	local window_width = 370

	local html_ask_for_abort_unregister = ""
	html_ask_for_abort_unregister=[[<title>]]..Ryzhide:load_translation("hide_n_hype_you_realy_want_window")..[[</title>
		<table width="100%" height="100%" cellpadding="2" cellspacing="2" border="]]..self.debug_window_border..[[">
			<tr>
				<td align="center" colspan="2"><img src="icon_sad.png" width="80" height="80" ></td>
			</tr>

			<tr>
				<td align="center" colspan="2"><h3>]]..Ryzhide:load_translation("hide_n_hype_you_realy_want_to_unregister")..[[</h3></td>
			</tr>

			<tr>
				<td align="center" colspan="2"><h4>]]..Ryzhide:load_translation("hide_n_hype_you_will_loos_all_your_progress")..[[</h4></td>
			</tr>

			<tr>
				<td align="center"><div id="abort_unregister_yes" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:abort_unregister_yes;bg:rap_invited_no_dm.png;w:46;h:46;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_yes")..[[;lua_function:click_icon_abort;'></div></td>
				<td align="center"><div id="abort_unregister_no" class='ryzom-ui-grouptemplate' style='display:inline-block;template:hide_n_hype_button_images;id:abort_unregister_no;bg:w_answer_16_cancel.png;tooltip:]]..Ryzhide:load_translation("hide_n_hype_click_no")..[[;lua_function:click_icon_abort;'></div></td>
			</tr>
		</table>]]
		Ryzhide:open_resize_window(self.abort_window_name, window_height, window_width, html_ask_for_abort_unregister, "", "")
end

function Ryzhide:click_icon_abort(trigger_id)
	if(trigger_id == "abort_skip_yes")then
		self.player_click_to_register = 1
		self.manuell_action = 1
		self.json_pull_counter_old = self.json_pull_counter + 3
		self.need_json_update = 0
		self.reject_invite_round_id = tonumber(self.current_round_id)
		self.game_is_now_running = 0

		Ryzhide:start_fetch_json_data('https://app.ryzom.com/app_ryzhide/?mode=accept_invite&accept_as=reject')
		removeOnDbChange(getUI(self.main_window_name),self.timer_str)
		Ryzhide:close_window(self.main_window_name, "")
	end

	if(trigger_id == "abort_skip_no")then
		local mainui_group = getUI(self.main_window_name):find("reject_accept")
		if(mainui_group ~= nil)then
			mainui_group:find("img1").texture = ""
			mainui_group:find("ctrl").active = true
		end
	end

	if(trigger_id == "abort_unregister_yes")then
		Ryzhide:display_debug_messanges("start_unregister_now")
		Ryzhide:start_fetch_json_data("https://app.ryzom.com/app_ryzhide/?mode=unregister")
	end

	if(trigger_id == "abort_unregister_no")then
		local mainui_group = getUI(self.main_window_name):find("unregister_accept")
		if(mainui_group ~= nil)then
			mainui_group:find("img1").texture = ""
			mainui_group:find("ctrl").active = true
		end
	end

	Ryzhide:close_window(self.abort_window_name, "")
end

--###################################### END asked_for_abort #######################################

debug("!!! Hide n Hype overload from Payload !!!!")
