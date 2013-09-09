<?php

function login(){
	global $INGAME_WEBPATH;
	global $WEBPATH;
	try{
		$username = filter_var($_POST['Username'],FILTER_SANITIZE_STRING);
		$password = filter_var($_POST['Password'],FILTER_SANITIZE_STRING);
		$result = WebUsers::checkLoginMatch($username, $password);

		if( $result != "fail"){
			//handle successful login
			$_SESSION['user'] = $username;
			$_SESSION['id'] = WebUsers::getId($username);
			$_SESSION['ticket_user'] = serialize(Ticket_User::constr_ExternId($_SESSION['id']));
			$user = new WebUsers($_SESSION['id']);
			$_SESSION['Language'] = $user->getLanguage();

			//go back to the index page.
			if (Helpers::check_if_game_client()) {
				header( 'Location: '. $INGAME_WEBPATH );
			}else{
				header( 'Location: '. $WEBPATH );
			}
			exit;
		}else{
			//handle login failure
			$result = Array();
			$result['login_error'] = 'TRUE';
			$result['no_visible_elements'] = 'TRUE';
			helpers :: loadtemplate( 'login', $result);
			exit;
		}	
		
		
	}catch (PDOException $e) {
	     //go to error page or something, because can't access website db
	     print_r($e);
	     exit;
	}
	
}