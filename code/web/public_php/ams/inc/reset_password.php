<?php

function reset_password(){
    $email = filter_var($_GET["email"], FILTER_SANITIZE_EMAIL);
    $user = filter_var($_GET["user"], FILTER_SANITIZE_STRING);
    $key = filter_var($_GET["key"], FILTER_SANITIZE_STRING);

    $target_id = WebUsers::getId($user);
    $webUser = new WebUsers($target_id);

    if( (WebUsers::getIdFromEmail($email) == $target_id) && (hash('sha512',$webUser->getHashedPass()) == $key) ){
        //you are allowed on the page!

        $GETString = "";
	foreach($_GET as $key => $value){
		$GETString = $GETString . $key . '=' . $value . "&";
	}
	if($GETString != ""){
		$GETString = '?'.$GETString;
	}
	$pageElements['getstring'] = $GETString;

        return $pageElements;

    }else{
        global $WEBPATH;
        $_SESSION['error_code'] = "403";
        header("Location: ".$WEBPATH."?page=error");
        die();
    }
}
