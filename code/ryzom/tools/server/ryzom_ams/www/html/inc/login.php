<?php

function login(){
	global $INGAME_WEBPATH;
	global $WEBPATH;
	if ( helpers :: check_if_game_client () ){
		//check if you are logged in ingame.
		$result = Helpers::check_login_ingame();
		if( $result != "FALSE"){
			//handle successful login
			$_SESSION['user'] = $result['name'];
			$_SESSION['id'] = WebUsers::getId($result['name']);
			$_SESSION['ticket_user'] = serialize(Ticket_User::constr_ExternId($_SESSION['id']));
			//go back to the index page.
			if (Helpers::check_if_game_client()) {
				header( 'Location: '.$INGAME_WEBPATH );
			}else{
				header( 'Location: '.$WEBPATH );
			}
			exit;
		}
	}
	$pageElements['ingame_webpath'] = $INGAME_WEBPATH;
	return $pageElements;

}
