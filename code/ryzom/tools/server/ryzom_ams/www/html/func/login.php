<?php

function login(){
	
	global $cfg;
	
	try{
		$result = WebUsers::checkLoginMatch($_POST["Username"],$_POST["Password"]);
		if( $result != "fail"){
			//handle successful login
			$_SESSION['user'] = $_POST["Username"];
			$_SESSION['permission'] = $result['Permission'];
			//go back to the index page.
			header( 'Location: index.php' );
			exit;
		}else{
			//handle login failure
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