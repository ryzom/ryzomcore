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
		$login_value = isset($_POST['LoginValue']) ? trim((string)$_POST['LoginValue']) : '';
		// never run a password through a sanitizer that rewrites &, <, quotes
		$password = isset($_POST['Password']) ? (string)$_POST['Password'] : '';

		//check if the filtered sent POST data returns a match with the DB
		$result = WebUsers::checkLoginMatch($login_value, $password);

		if( $result != "fail"){
			//handle successful login
			$_SESSION['user'] = $result['Login'];
			$_SESSION['id'] = $result['UId'];
			$_SESSION['ticket_user'] = serialize(Ticket_User::constr_ExternId($_SESSION['id']));
			$user = new WebUsers($_SESSION['id']);
			$_SESSION['Language'] = $user->getLanguage();

			// this is pasted onto the redirect target, so encode it
			$GETString = "";
			foreach($_GET as $key => $value){
				if (!is_scalar($value)) continue;
				$GETString = $GETString . rawurlencode($key) . '=' . rawurlencode($value) . "&";
			}
			if($GETString != ""){
				$GETString = '?'.$GETString;
			}


			//go back to the index page.
                header("Cache-Control: max-age=1");
			if (Helpers::check_if_game_client()) {
				header( 'Location: '. $INGAME_WEBPATH . $GETString);
			}else{
				header( 'Location: '. $WEBPATH . $GETString);
			}
			throw new SystemExit();
		}else{
			//handle login failure
			$result = Array();
			$result['login_error'] = 'TRUE';
			$result['no_visible_elements'] = 'TRUE';
			helpers :: loadtemplate( 'login', $result);
			throw new SystemExit();
		}


	}catch (PDOException $e) {
	     //go to error page or something, because can't access website db
	     error_log($e->getMessage());
	     throw new SystemExit();
	}

}
