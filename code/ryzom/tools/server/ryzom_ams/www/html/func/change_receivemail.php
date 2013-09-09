<?php

function change_receivemail(){
	
    try{
        //if logged in
    	global $INGAME_WEBPATH;
    	global $WEBPATH;
        if(WebUsers::isLoggedIn()){
            
            if(isset($_POST['target_id'])){
		
                
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
			exit;
                    
                }else{
                    //ERROR: permission denied!
		    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    exit;
                }
        
            }else{
                //ERROR: The form was not filled in correclty
		header("Location: index.php?page=settings");
		exit;
            }    
        }else{
            //ERROR: user is not logged in
	    header("Location: index.php");
	    exit;
        }
                  
    }catch (PDOException $e) {
         //go to error page or something, because can't access website db
         print_r($e);
         exit;
    }
    
}



