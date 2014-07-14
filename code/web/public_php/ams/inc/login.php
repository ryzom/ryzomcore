<?php
/**
* This function is beign used to load info that's needed for the login page.
* it will try to auto-login, this can only be used while ingame, the web browser sends additional cookie information that's also stored in the open_ring db.
* We will compare the values and if they match, the user will be automatically logged in!
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function login(){
	global $INGAME_WEBPATH;
	global $WEBPATH;
	if ( helpers :: check_if_game_client () ){
		//check if you are logged in ingame, this should auto login
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
	$GETString = "";
	foreach($_GET as $key => $value){
		$GETString = $GETString . $key . '=' . $value . "&";
	}		
	if($GETString != ""){
		$GETString = '?'.$GETString;
	}
	$pageElements['getstring'] = $GETString;
	return $pageElements;

}
