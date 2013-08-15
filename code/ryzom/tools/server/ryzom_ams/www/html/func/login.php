<?php

function login(){
	
	try{
		$username = filter_var($_POST['Username'],FILTER_SANITIZE_STRING);
		$password = filter_var($_POST['Password'],FILTER_SANITIZE_STRING);
		$result = WebUsers::checkLoginMatch($username, $password);
		if( $result != "fail"){
			//handle successful login
			$_SESSION['user'] = $username;
			$_SESSION['id'] = $result['UId'];
			$_SESSION['ticket_user'] = Ticket_User::constr_ExternId($result['UId']);
			
			//go back to the index page.
			header( 'Location: index.php' );
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