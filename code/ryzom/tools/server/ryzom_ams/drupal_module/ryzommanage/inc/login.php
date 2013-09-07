<?php

function login(){
	if ( helpers :: check_if_game_client () ){
		//check if you are logged in ingame.
		$result = Helpers::check_login_ingame();
		if( $result != "FALSE"){
			//handle successful login
			$_SESSION['user'] = $result['name'];
			$_SESSION['id'] = WebUsers::getId($result['name']);
			$_SESSION['ticket_user'] = serialize(Ticket_User::constr_ExternId($_SESSION['id']));
			//go back to the index page.
			header( 'Location: '.$INGAME_WEBPATH );
			exit;
		}
	}
	global $INGAME_WEBPATH;
	$pageElements['ingame_webpath'] = $INGAME_WEBPATH;
	return $pageElements;

}
