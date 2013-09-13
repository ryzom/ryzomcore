<?php
/**
* This function is beign used to login a user.
* It will first check if the sent POST data returns a match with the DB, if it does, some session variables will be appointed to the user and he will be redirected to the index page again.
* If it didn't match, the template will be reloaded and a matching error message will be shown.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function login(){
	global $INGAME_WEBPATH;
	global $WEBPATH;
	try{
		$username = filter_var($_POST['Username'],FILTER_SANITIZE_STRING);
		$password = filter_var($_POST['Password'],FILTER_SANITIZE_STRING);
		//check if the filtered sent POST data returns a match with the DB
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