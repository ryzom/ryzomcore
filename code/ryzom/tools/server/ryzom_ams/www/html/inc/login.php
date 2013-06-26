<?php

function login(){
	
	global $WEBDBHOST;
	global $WEBDBPORT;
	global $WEBDBNAME;
	global $WEBDBUSERNAME;
	global $WEBDBPASSWORD;
	
	try{
		$dbw = new PDO("mysql:host=$WEBDBHOST;port=$WEBDBPORT;dbname=$WEBDBNAME", $WEBDBUSERNAME, $WEBDBPASSWORD);
		$dbw->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
		
		$statement = $dbw->prepare("SELECT * FROM ams_user WHERE Login=:user");
		$statement->execute(array('user' => $_POST['Username']));
		
		$row = $statement->fetch();
		$salt = substr($row['Password'],0,2);
		$hashed_input_pass = crypt($_POST["Password"], $salt);
		if($hashed_input_pass == $row['Password']){
			//handle successful login
			$_SESSION['user'] = $_POST["Username"];
			$_SESSION['permission'] = $row['Permission'];
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