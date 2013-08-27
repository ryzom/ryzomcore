<?php

function login(){
	if ( helpers :: check_if_game_client () ){
		//check if you are logged in ingame.
		$result = Helpers::check_login_ingame();
		if( $result != "FALSE"){
			//handle successful login
			$_SESSION['user'] = $result['name'];
			$_SESSION['id'] = WebUsers::getId($result['name']);
			$_SESSION['ticket_user'] = Ticket_User::constr_ExternId($_SESSION['id']);
			//go back to the index page.
			header( 'Location: index.php' );
			exit;
		}
	}
}
