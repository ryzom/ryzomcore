<?php
	if(!defined('APP_NAME')) {
		die(-1);
	}

	$_CONF = array();

	$_CONF['summary_size'] = 3;
	$_CONF['default_lang'] = 'en';
	$_CONF['enable_webig'] = true;
	$_CONF['enable_offgame'] = true;
	$_CONF['image_url'] = "http://www.3025-game.de/special/app_achievements/";
	$_CONF['image_cdate'] = 0; // timestamp to bybass image cache ingame

	$_CONF['use_fb'] = false;
	$_CONF['fb_id'] = "447985781893176";
	$_CONF['fb_secret'] = "f953772f1f7d871db022a6023e7a3f42";
?>