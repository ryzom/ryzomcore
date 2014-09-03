<?php
/**
* This function is beign used to change the users receiveMail setting.
* It will first check if the user who executed this function is the person of whom the setting is or if it's a mod/admin. If this is not the case the page will be redirected to an error page.
* it will check if the new value equals 1 or 0 and it will update the setting and redirect the page again.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function change_receivemail(){

    try{
        //if logged in
    	global $INGAME_WEBPATH;
    	global $WEBPATH;
        if(WebUsers::isLoggedIn()){

            if(isset($_POST['target_id'])){

                //check if the user who executed this function is the person of whom the setting is or if it's a mod/admin.
                if( ( ($_POST['target_id'] == $_SESSION['id']) || Ticket_User::isMod(unserialize($_SESSION['ticket_user']))) && isset($_POST['ReceiveMail']) ){
			$user_id = filter_var($_POST['target_id'], FILTER_SANITIZE_NUMBER_INT);
		    	$receiveMail = filter_var($_POST['ReceiveMail'], FILTER_SANITIZE_NUMBER_INT);
			if($receiveMail == 0 || $receiveMail == 1){
			    WebUsers::setReceiveMail($user_id, $receiveMail);
			}
			if (Helpers::check_if_game_client()) {
				header("Location: ".$INGAME_WEBPATH."?page=settings&id=".$user_id);
			}else{
				header("Location: ".$WEBPATH."?page=settings&id=".$user_id);
			}
			die();

                }else{
                    //ERROR: permission denied!
		    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    die();
                }

            }else{
                //ERROR: The form was not filled in correclty
		header("Location: index.php?page=settings");
		die();
            }
        }else{
            //ERROR: user is not logged in
	    header("Location: index.php");
	    die();
        }

    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         die();
    }

}



