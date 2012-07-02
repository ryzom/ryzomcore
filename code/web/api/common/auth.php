<?php

function ryzom_app_authenticate(&$user, $ask_login=true, $welcome_message='') {
	$name = ryzom_get_param('name');
	$authserver = ryzom_get_param('authserver');
	$authkey = ryzom_get_param('authkey');
	$lang = ryzom_get_param('lang');
	$cid = ryzom_get_param('cid', '');
	$is_ingame = false;
	// we have to set the $user['lang'] even for anonymous user or we cannot display the test in the right langage
    if($lang == '') {
        $l = substr($_SERVER['HTTP_ACCEPT_LANGUAGE'], 0, 2);
        if($l=='fr'||$l=='en'||$l=='de'||$l=='ru'||$l=='es')
            $lang = $l;
        else
            $lang = 'en';
	}
	$user['message'] = '';
	$user['lang'] = $lang;
	$user['ig'] = false;
	
	if ((isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'Ryzom')) || ryzom_get_param('ig')) {
		$user['ig'] = true;
		// Ingame
		$shardid = ryzom_get_param('shardid');
		if (!ryzom_authenticate_ingame($shardid, $cid, $name, $authkey))
			return false;
		$is_ingame = true;
	} else {
		// Outgame : Use session
		$error_message = '';
		if (!ryzom_authenticate_with_session($name, $cid, $error_message)) {
			if ($ask_login) {
				$c = '';
				if (!$welcome_message)
					$welcome_message = '<span style="font-size:11pt; color: #AAAAFF">The application <strong style="color: #99FFFF">'._t(APP_NAME).'</strong> require authentication. Please enter your credentials</span>';

				$c .=  '<div style="text-align: center">'.$welcome_message.'</div><br />';

				if ($user['message'])
					$c .=  '<div style="text-align: center"><strong style="color: #FF5555">'._t($user['message']).'</strong></div><br />';
				$c .= ryzom_render_login_form($name, false);
				echo ryzom_app_render(_t('app_'.APP_NAME), $c);
				exit;
			}
			return false;
		}
	}

	if ($lang)
		$_SESSION['lang'] = $lang;

	// get user informations
	$user = ryzom_user_get_info($cid);
	$user['lang'] = $_SESSION['lang'];
	if (isset($user['creation_date']))
		$user['id'] = ryzom_get_user_id($cid, $user['char_name'], $user['creation_date']);
	if ($is_ingame && $user['last_played_date'] != '0')
		$user['ig'] = true;
	else
		$user['ig'] = false;
	unset($user['last_played_date']);
	unset($user['creation_date']);
	return true;
}

?>
