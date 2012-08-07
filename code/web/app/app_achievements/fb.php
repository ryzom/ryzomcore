<?php
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	define('APP_NAME', 'app_achievements');

	require_once('conf.php');
	require_once("fb/facebook.php");


	$facebook = new Facebook(array(
        'appId' => $_CONF['fb_id'],
        'secret' => $_CONF['fb_secret'],
        'cookie' => true
	));

	// Get the url to redirect for login to facebook
	// and request permission to write on the user's wall.
	$login_url = $facebook->getLoginUrl(
		array('scope' => 'publish_stream')
	);

	 
	// If not authenticated, redirect to the facebook login dialog.
	// The $login_url will take care of redirecting back to us
	// after successful login.
	if (!$facebook->getUser()) {
		
		echo '<script type="text/javascript">
	top.location.href = "'.$login_url.'";
	</script>';
	}
	else {
		 echo var_export($facebook->getUser(),true);
		// Do the wall post.
		try {

			$facebook->api("/me/feed", "post", array(
				message => "My character Talvela just earned <b>'Bejeweled'</b> on Ryzom!",
				picture => "http://www.3025-game.de/special/app_achievements/pic/icon/test.png",
				link => "http://www.ryzom.com",
				name => "Ryzom - MMO",
				caption => "Join and play for fee!"
			));
			echo "post";

		} catch (FacebookApiException $e) {
			echo $e;
			$login_url = $facebook->getLoginUrl( array(
						   'scope' => 'publish_stream'
						   )); 
			echo 'Please <a href="' . $login_url . '">login.</a>';
			#error_log($e->getType());
			#error_log($e->getMessage());
	  }
	}
?>