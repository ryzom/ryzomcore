<?php

include_once('config.php');
include_once('lang.php');

// List of apps
$apps = array(
	'app_test',
);

$c = '';

// Ask to authenticate user (using ingame or session method) and fill $user with all information
$logged = ryzom_app_authenticate($user, false);
if ($logged) {
	$c .= '<h1>'._t('welcome', $user['char_name']).'</h1>';
} else {
	if (!$user['ig']) {
		if ($user['message'])
			$c .=  '<div style="text-align: center"><strong style="color: #FF5555">'._t($user['message']).'</strong></div><br />';
		$c .= ryzom_render_login_form(ryzom_get_param('name'));
	}
}

foreach ($apps as $app) {
	$c .= '<a href="'.RYAPP_URL.'/'.$app.'/index.php"><img src="'.RYAPP_URL.'/'.$app.'/favicon.png" />'._t($app).'</a><br />';
}

if ($logged && !$user['ig'])
	$c .= '<br /><a href="'.RYAPP_URL.'/index.php?action=logout">'._t('logout').'</a>';

// Print GET values on debug view
p($_GET);
echo ryzom_app_render('Ryzom', $c, $user['ig']);

?>