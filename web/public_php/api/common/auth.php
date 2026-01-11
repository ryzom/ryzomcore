<?php

/* Copyright (C) 2009 Winch Gate Property Limited
 *
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */


function ryzom_app_authenticate(&$user, $ask_login=true, $welcome_message='', $webprivs=true) {
	$name = ryzom_get_param('name');
	$urluser = ryzom_get_param('user'); // user serialization send by auth server
	$urlusercheksum = ryzom_get_param('checksum');  // user serialization checksum
	$authkey = ryzom_get_param('authkey'); // InGame authkey
	$lang = ryzom_get_param('lang');
	$cid = intval(ryzom_get_param('cid'));
	$is_auth_ingame = false;
	// we have to set the $user['lang'] even for anonymous user or we cannot display the test in the right langage
    if($lang == '') {
		if  (!isset($_SESSION['lang'])) {
			$l = isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])?substr($_SERVER['HTTP_ACCEPT_LANGUAGE'], 0, 2):'en';
			if ($l=='fr'||$l=='en'||$l=='de'||$l=='ru'||$l=='es')
				$lang = $l;
			else
				$lang = 'en';
		} else
			$lang = $_SESSION['lang'];
	}
	if ($lang!='fr'&&$lang!='en'&&$lang!='de'&&$lang!='ru'&&$lang!='es')
		$lang = 'en';

	$user['message'] = '';
	$user['lang'] = $lang;
	$user['groups'] = array();

	if ((isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'Ryzom')) || ryzom_get_param('ig'))
		$user['ig'] = true;
	else
		$user['ig'] = false;

	if (isset($_SESSION['user'])) {
		if (ryzom_get_param('action') == 'logout')
			unset($_SESSION['user']);
		else {
			$_SESSION['user']['ig'] = $user['ig'];
			define('RYZOM_IG', $user['ig']);
			$user = $_SESSION['user'];
			return true;
		}
	}

	if ($urluser && $urlusercheksum) {
		// Check $authuser (used to test app from another server ingame)
		if (hash_hmac('sha1', $urluser, RYAPI_AUTH_KEY) == $urlusercheksum) {
			$ig = $user['ig'];
			$user = array_merge($user, unserialize(base64_decode($urluser)));
			$user['ig'] = $ig;
			if (!isset($user['groups']))
				$user['groups'] = array();
			define('RYZOM_IG', $user['ig']);
			$_SESSION['user'] = $user;
			return true;
		}
	}

	if ($user['ig']) {
		// Ingame
		$shardid = ryzom_get_param('shardid');
		$error_message = '';
		if (ryzom_authenticate_ingame($shardid, $cid, $name, $authkey) || ryzom_authenticate_with_session($name, $cid, $error_message)) {
			$is_auth_ingame = true;
		}
	} else {
		// Outgame or bad ingame auth (external server) : Use session
		$error_message = '';
		if (!ryzom_authenticate_with_session($name, $cid, $error_message)) {
			define('RYZOM_IG', false);
			if ($ask_login) {

				if ($error_message)
					$c = '<h3>'._t($error_message).'</h3>';
				else
					$c = '';
				if (!$welcome_message)
					$welcome_message = '<span style="font-size:11pt; color: #AAAAFF">The application <strong style="color: #99FFFF">'._t(APP_NAME).'</strong> require authentication. Please enter your credentials</span>';

				$c .=  '<div style="text-align: center">'.$welcome_message.'</div><br />';

				if ($user['message'])
					$c .=  '<div style="text-align: center"><strong style="color: #FF5555">'._t($user['message']).'</strong></div><br />';
				$c .= ryzom_render_login_form($name, false);
				echo ryzom_app_render(_t('app_'.APP_NAME), $c);
				die();
			}
			return false;
		}
	}

	$_SESSION['lang'] = $lang;

	define('RYZOM_IG', $user['ig']);
	// get user informations
	$ig = $user['ig'];
	$user = ryzom_user_get_info($cid, $webprivs, RYAPI_USE_PLAYER_STATS);

	if (isset($user['creation_date']))
		$user['id'] = ryzom_get_user_id($cid, $user['char_name'], $user['creation_date'], $user);

	$user['gender'] = ryzom_get_user_gender($user['id']);

	$user['ig'] = $ig;
	$user['lang'] = $_SESSION['lang'];
	if (!isset($user['groups']))
		$user['groups'] = array();

	if ($is_auth_ingame && $user['last_played_date'] != '0')
		$user['auth_ig'] = true;
	else
		$user['auth_ig'] = false;

	if (!isset($_SESSION['translater_mode']) || ryzom_get_param('translate_this') == '0')
		$_SESSION['translater_mode'] = false;

	// Set/unset translation mode
	if (in_array('WTRS', $user['groups']) && ryzom_get_param('translate_this') == '1')
		$_SESSION['translater_mode'] = true;

	$user['translation_mode'] = $_SESSION['translater_mode'];

//	$user['after_merge'] = $user['uid'] >= 671686;

	ryzom_unset_url_param('translate_this');

	if (isset($user['last_played_date']))
		$_SESSION['last_played_date'] = $user['last_played_date'];
	 // don't send this informations to external apps
	unset($user['last_played_date']);
	unset($user['creation_date']);
	return true;
}

?>
